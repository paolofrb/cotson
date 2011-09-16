// (C) Copyright 2006-2009 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// #define _DEBUG_THIS_

// $Id$
#include "abaeterno_config.h"
#include "interleaver.h"
#include "error.h"
#include "logger.h"
#include "cpuid_call.h"
#include "cotson.h"

using namespace std;

void Interleaver::config(uint64_t dev,Instructions& insns,const TraceNeeds* tn)
{
	LOG("config device",dev);
	ERROR_IF(dev!=cpus.size(),"Device '", dev, "' is not the device I was hoping to get");
	insns.create(100,tn->history+1);
	cpus.push_back(CpuData(&insns,tn,dev));
}

namespace
{
    static bool exec_cpuid(const Instruction* inst)
    {
	    uint64_t a,b,c;
	    boost::tie(a,b,c)=inst->cpuid_registers();
	    LOG("CPUID");
	    LOG("\tRDI:   ",a);
	    LOG("\tRSI:   ",b);
	    LOG("\tRBX:   ",c);
	    Instruction* ii=const_cast<Instruction*>(inst);
	    return CpuidCall::simulation(ii,Cotson::nanos(),10)==DISCARD;
    }
}

void Interleaver::update_cpus() 
{
    // At every mpquantum CPUs must be re-aligned to the
	// max cycle of all CPUs, to account for idle time of CPUs that
	// may have executed fewer instructions
	uint64_t max_cycle = 0;
 	for(uint i=0;i<cpus.size();i++)
	{
	    uint64_t cycle = *(cpus[i].pcycles);
		max_cycle = cycle > max_cycle ? cycle : max_cycle;
	}
 	for(uint i=0;i<cpus.size();i++) 
	    cpus[i].update(max_cycle);
}

void Interleaver::CpuData::update(uint64_t max_cycle)
{
	order=0;
    emit=tn->st[sim_state()].emit;
	pcycles=tn->cycles;
	tn->idle(max_cycle-*pcycles);
	initial_cycles=*pcycles;
	LOG("CPU",dev,"cycle",*pcycles,"elems",ins->elems());
}

void Interleaver::end_quantum() 
{
 	LOG("end_quantum");
	update_cpus();

	// one cpu shortcut
	if (cpus.size()==1)
	{
 	    LOG("one cpu shortcut");
		Instructions* ins = cpus[0].ins;
		const EmitFunction& emit = cpus[0].emit;
		while(ins->elems())
		{
		    const Instruction* nn=ins->next();
            if(! (nn->is_cpuid() && exec_cpuid(nn)) )
		        emit(nn);
		}
	    return;
	}

	priority_queue<CpuData*,vector<CpuData*>,CpuCmp> cpu_queue;

	for(uint i=0;i<cpus.size();i++) {
		const Instructions* ins = cpus[i].ins;
	    if(ins && ins->elems())
	        cpu_queue.push(&cpus[i]);
	}

	while (!cpu_queue.empty())
	{
		CpuData *fcpu = cpu_queue.top(); 
		const Instruction* nn=fcpu->ins->next();
        if(!(nn->is_cpuid() && exec_cpuid(nn)))
		{
		    fcpu->emit(nn);   // emit instruction to the timer
		    fcpu->set_now();  // set new cpu time
		    cpu_queue.pop();  // remove CPU from queue
			// and re-insert it with new timing if
			// there are instructions left to be processed
		    if(fcpu->ins->elems())
		        cpu_queue.push(fcpu); 
		}
	}
}

void Interleaver::break_sample() 
{
	for(uint i=0;i<cpus.size();i++)
		cpus[i].ins->clear();
}

