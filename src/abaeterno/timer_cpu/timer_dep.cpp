// (C) Copyright 2010 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

// $Id$

// #define _DEBUG_THIS_

#include "abaeterno_config.h"
#include "logger.h"
#include "cpu_timer.h"
#include "twolev.h"
#include "liboptions.h"

using namespace std;
using namespace boost;

class TimerDep : public CpuTimer
{
public: 
	TimerDep(Parameters&);

	void simple_warming(const Instruction*);
	void full_warming(const Instruction*);
	void simulation(const Instruction*);

	void addMemory(std::string,Memory::Interface::Shared); 

protected:
	void endSimulation();
	void beginSimulation();

private: 
    uint64_t reg_read(const Opcode*, uint64_t);
    void reg_write(const Opcode*, uint64_t);

	const Instruction* prev_inst;
	Memory::Interface::Shared icache;
	Memory::Interface::Shared dcache;
	Memory::Interface::Shared itlb;
	Memory::Interface::Shared dtlb;
	boost::shared_ptr<twolevT>	twolev;

	uint64_t instructions;
	uint64_t fetch_cycle;
	uint64_t cycles;
	
	const uint32_t branch_mispred_penalty;
	const bool bpred_perfect;
	const double commit_cpi;

	uint8_t lastreg;
	uint8_t rename[256]; // register renaming
	uint64_t regcycle[256]; // register ready time
};

registerClass<CpuTimer,TimerDep> timer_dep_c("timer_dep");

TimerDep::TimerDep(Parameters& p) : CpuTimer(&cycles,&instructions),
	prev_inst(0),
	branch_mispred_penalty(p.get<uint32_t>("branch_mispred_penalty","8")),
	bpred_perfect(p.get<bool>("bpred_perfect","false")),
	commit_cpi(p.get<double>("commit_cpi","1.0"))
{
	twolev.reset(new twolevT(
		p.get<uint32_t>("twolev.l1_size","1"),
		p.get<uint32_t>("twolev.l2_size","16kB"),
		p.get<uint32_t>("twolev.hlength","14"),
		p.get<uint32_t>("twolev.use_xor","1")));

	add("instructions",instructions);
	add("cycles",cycles);
	add_ratio("ipc","instructions","cycles");	

	add("twolev.", *twolev);
	clear_metrics();
	fetch_cycle=0;

	trace_needs.history=2;
	trace_needs.st[SIMPLE_WARMING].set(EmitFunction::bind<TimerDep,&TimerDep::simple_warming>(this));
	trace_needs.st[FULL_WARMING].set(EmitFunction::bind<TimerDep,&TimerDep::full_warming>(this));
	trace_needs.st[SIMULATION].set(EmitFunction::bind<TimerDep,&TimerDep::simulation>(this));

	uint32_t wflags = (NEED_CODE | NEED_MEM | NEED_EXC | NEED_HB);
	trace_needs.st[SIMPLE_WARMING].setflags(wflags);
	trace_needs.st[FULL_WARMING].setflags(wflags);

	uint32_t sflags = (NEED_CODE | NEED_MEM | NEED_REG | NEED_EXC | NEED_HB);
	trace_needs.st[SIMULATION].setflags(sflags);
}

uint64_t TimerDep::reg_read(const Opcode* opc, uint64_t fc)
{
	uint64_t rc = fc;
	if (opc)
	{
		const Opcode::regs& src_regs = opc->getSrcRegs();
	    for(Opcode::regs::const_iterator i=src_regs.begin(); i!=src_regs.end(); ++i)
		{
			uint8_t r = *i;
			if (!rename[r]) {
			    rename[r]= ++lastreg;
			    if (lastreg == 256) lastreg = 0;
			}
		    uint64_t t = regcycle[rename[r]];
			if (t > rc) rc = t;
			LOG(" -- reg read",(int)r,":",(int)rename[r],"cycle",t);
		}
		const Opcode::regs& mem_regs = opc->getMemRegs();
	    for(Opcode::regs::const_iterator i=mem_regs.begin(); i!=mem_regs.end(); ++i)
		{
			uint8_t r = *i;
			if (!rename[r]) {
			    rename[r]= ++lastreg;
			    if (lastreg == 256) lastreg = 0;
			}
		    uint64_t t = regcycle[rename[r]];
			if (t > rc) rc = t;
			LOG(" -- mreg read",(int)r,":",(int)rename[r],"cycle",t);
		}
	}
	return rc;
}

void TimerDep::reg_write(const Opcode* opc, uint64_t t)
{
	if (opc) // set ready time for dest registers
	{
		const Opcode::regs& dst_regs = opc->getSrcRegs();
	    for(Opcode::regs::const_iterator i=dst_regs.begin(); i!=dst_regs.end(); ++i)
		{
			uint8_t r = *i;
			rename[r]= ++lastreg;
			if (lastreg == 256) lastreg = 0;
		    regcycle[rename[r]] = t;
			LOG(" -- reg write",(int)r,":",(int)rename[r],"cycle",t);
	    }
	}
}

void TimerDep::simulation(const Instruction* inst)
{
	instructions++;
	const Memory::Access& pc = inst->PC();
	
	/* Branch predictor */
	bool taken=false;
	if (prev_inst && prev_inst->is_branch())
	{
		const Memory::Access& lastPC = prev_inst->PC();
		uint64_t lpc = lastPC.phys;
		taken = ((lpc+lastPC.length) != pc.phys);
	    if (!bpred_perfect)
		{
		    TwolevUpdate twolev_record;
		    if (twolev->Lookup(lpc, twolev_record,taken)) 
			{
			    fetch_cycle += branch_mispred_penalty;
				LOG("mispredict: fetch",fetch_cycle);
		    }
		
		    // Update branch predictor
		    twolev->SpecUpdate(lpc, taken);
		    twolev->Update(lpc, taken, twolev_record.count);
	    }
	}

	/* Instruction Cache */
	if(icache)
	{
		LOG("++ INSTRUCTION:", hex, pc);
		inst->disasm(cout); cout << dec;
		uint64_t latency =   icache->read(pc,fetch_cycle,this)
			               + itlb->read(pc,(uint64_t)fetch_cycle,this);
		fetch_cycle += latency;
		LOG(" icache latency =",latency, "fetch",fetch_cycle);
	}

	/* Register read */
	const Opcode* opc = inst->getOpcode();
	uint64_t rcycle = reg_read(opc,fetch_cycle);

	/* Data Cache */
	if(dcache)
	{
		Instruction::MemIterator il = inst->LoadsBegin();
		const Instruction::MemIterator el = inst->LoadsEnd();
		uint64_t max_load_latency=0;
		bool is_prefetch=inst->is_prefetch();
		for(; il != el; ++il)
		{
			LOG(" -- LOAD:", *il);
			uint64_t cache_lat = dcache->read(*il,rcycle,this);
			uint64_t tlb_lat = dtlb->read(*il,rcycle,this);
			uint64_t latency = tlb_lat + (is_prefetch ? 0 : cache_lat);
			LOG("     load latency =",latency);
			if (latency > max_load_latency)
				max_load_latency=latency;
		}
		if(max_load_latency)
			rcycle += max_load_latency;

		uint64_t max_store_latency=0;
		Instruction::MemIterator is = inst->StoresBegin();
		const Instruction::MemIterator es = inst->StoresEnd();
		for(; is != es; ++is)
		{
			LOG(" -- STORE:", *is);
			dcache->write(*is,rcycle,this);
			uint64_t latency = dtlb->read(*is,rcycle,this);
			LOG("     store latency =",latency);
			if (latency > max_store_latency)
				max_store_latency=latency;
		}
		if(max_store_latency)
			rcycle += max_store_latency;
	}
	// rcycle += commit_cpi;
	reg_write(opc,rcycle);
	if (rcycle > cycles) cycles = rcycle;
	LOG(" -- rcycle =",rcycle, "cycles=", cycles);
	prev_inst = inst;
}

void TimerDep::full_warming(const Instruction* inst)
{
	if(!prev_inst)
	{
		prev_inst = inst;
		return;
	}

	const Memory::Access& prev_pc = prev_inst->PC();
	
	if(icache)
	{
		icache->read(prev_pc,cycles,this);
		itlb->read(prev_pc,cycles,this);
	}

	if(dcache)
	{
		Instruction::MemIterator il = prev_inst->LoadsBegin();
		Instruction::MemIterator el = prev_inst->LoadsEnd();
		for (; il!=el; il++)
		{
			dcache->read(*il,cycles,this);
			dtlb->read(*il,cycles,this);
		}

		Instruction::MemIterator is = prev_inst->StoresBegin();
		Instruction::MemIterator es = prev_inst->StoresEnd();
		for (; is!=es; is++)
		{
			dcache->write(*is,cycles,this);
			dtlb->read(*is,cycles,this);
		}
	}
	
	/* Branch predictor */		
	if (!bpred_perfect && prev_inst->is_branch())
	{
		/* set branch direction */
		uint64_t ppc = prev_pc.phys;
		bool taken = inst->PC().phys != (ppc + prev_pc.length);
		char *counter = twolev->Get2bitCounter (prev_pc.phys);
		twolev->SpecUpdate(ppc, taken);
		twolev->Update(ppc, taken, counter);
	}
	
	cycles+=1000;
	prev_inst = inst;
}

void TimerDep::beginSimulation()
{
	LOG("clear metrics");
	clear_metrics();
	fetch_cycle=0;
	lastreg=0;
	(void)memset(rename,0,256*sizeof(uint8_t));
	(void)memset(regcycle,0,256*sizeof(uint64_t));
}

void TimerDep::endSimulation()
{
	LOG("clearing timer");
	prev_inst=0;
	fetch_cycle=0;
}

void TimerDep::simple_warming(const Instruction* inst)
{
	full_warming(inst);
}

void TimerDep::addMemory(string name,Memory::Interface::Shared c) 
{
	if(name=="icache")
	{
		icache=c; 
		add("icache.",*icache); 
	}
	else if(name=="dcache")
	{
		dcache=c; 
		add("dcache.",*dcache); 
	}
	else if(name=="itlb")
	{
		itlb=c; 
		add("itlb.",*itlb); 
	}
	else if(name=="dtlb")
	{
		dtlb=c; 
		add("dtlb.",*dtlb); 
	}
	else 
		throw runtime_error("unknown memory interface "+name);
}
