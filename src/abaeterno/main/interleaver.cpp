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

namespace {
option o1("align_timers", "align timers to max time every MP quantum");
option o2("interleaver_order", "cycle", "order CPUs by either cycle | round_robin | uniform");
}

void Interleaver::initialize() 
{
    align_timers = !(Option::has("align_timers") && Option::get<bool>("align_timers")==false);
	if (Option::has("interleaver_order"))
	{
	    const string &s=Option::get<string>("interleaver_order");
	    if (s=="cycle")
	        order_by=CYCLE;
	    else if (s=="round_robin")
	        order_by=ROUNDROBIN;
	    else if (s=="uniform")
	        order_by=UNIFORM;
        else
	        throw runtime_error("Unkown option interleaver_order=" + s);
    }
}

void Interleaver::config(uint64_t dev,Instructions& insns,const TraceNeeds* tn)
{
	LOG("config device",dec,dev);
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
	uint64_t max_ins = 0;
 	for(uint i=0;i<cpus.size();i++)
	{
	    if (align_timers) 
		{
	        uint64_t cycle = *(cpus[i].pcycles);
		    max_cycle = cycle > max_cycle ? cycle : max_cycle;
		}
		uint64_t nins = cpus[i].ins->elems();
		max_ins = nins > max_ins ? nins : max_ins;
	}
 	for(uint i=0;i<cpus.size();i++) 
	    cpus[i].update(max_cycle,max_ins,order_by);
}

void Interleaver::CpuData::update(uint64_t max_cycle, uint64_t max_ins,Order order_by)
{
	order = order_by==CYCLE ? 0 : dev; // use CPU "dev" number to disambiguate same order
    emit=tn->st[sim_state()].emit;

	pcycles=tn->cycles;
	if (max_cycle > *pcycles)
	    tn->idle(max_cycle-*pcycles);
	initial_cycles=*pcycles;

	order_rate = 1000;
	if (max_ins > ins->elems())
	    order_rate = static_cast<uint64_t>(1000.0 * static_cast<double>(max_ins) / ins->elems());
	    
	LOG("CPU",dec,dev,"cycle",*pcycles,"elems",ins->elems(),"insrate",order_rate);
}

inline void Interleaver::CpuData::set_order(Interleaver::Order order_by)
{
	switch(order_by) {
	    case CYCLE:
            order = *pcycles - initial_cycles;
		    break;
		case ROUNDROBIN:
	        order += 1000;
		    break;
		case UNIFORM:
	        order += order_rate;
		    break;
	}
}

void Interleaver::end_quantum_onecpu(CpuData* cpu) 
{
    LOG("one cpu shortcut");
	Instructions* ins = cpu->ins;
	const EmitFunction& emit = cpu->emit;
	while(ins->elems())
	{
	    const Instruction* nn=ins->next();
        if(! (nn->is_cpuid() && exec_cpuid(nn)) )
		    emit(nn);
	}
}

void Interleaver::end_quantum() 
{
 	LOG("end_quantum");
	update_cpus();

	if (cpus.size()==1)
	{
	    end_quantum_onecpu(&cpus[0]);
		return;
	}

	priority_queue<CpuData*,vector<CpuData*>,CpuCmp> cpu_queue;
	for(uint i=0;i<cpus.size();i++) {
		const Instructions* ins = cpus[i].ins;
	    if(ins && ins->elems())
	        cpu_queue.push(&cpus[i]);
	}
	if (cpu_queue.size()==1)
	{
	    end_quantum_onecpu(cpu_queue.top());
		return;
	}

	while (!cpu_queue.empty())
	{
		CpuData *fcpu = cpu_queue.top(); 
		const Instruction* nn=fcpu->ins->next();
        if(!(nn->is_cpuid() && exec_cpuid(nn)))
		{
			LOG("emit",fcpu->dev,"order",fcpu->order);
		    fcpu->emit(nn);   // emit instruction to the timer
		    fcpu->set_order(order_by);  // set new cpu order
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

