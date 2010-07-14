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
	ERROR_IF(dev!=it.size(),"Device '", dev, "' is not the device I was hoping to get");
	insns.create(100,tn->history+1);
	it.push_back(make_pair(&insns,tn));
}

namespace {
struct cpu_time_ordering
{
	Instructions* ins;
	const TraceNeeds* tn;
	uint64_t initial_cycles;
	uint64_t order;

	cpu_time_ordering(pair<Instructions*,const TraceNeeds*>p):
		ins(p.first),tn(p.second),initial_cycles(*tn->cycles),order(0) {}
	void set_now() { order=*tn->cycles-initial_cycles; }
	
	inline bool operator<(const cpu_time_ordering& other) const 
	{
		return !(order<other.order); 
	}
};

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

void Interleaver::notify() 
{
// 	cout << "notify A: " << Cotson::nanos() << " : ";
// 	for(uint i=0;i<it.size();i++)
// 		cout << it[i].first->elems() << " ";
// 	cout << endl;
 	LOG("notify");

	// one cpu shortcut
	if (it.size()==1)
	{
 	    LOG("one cpu shortcut");
		Instructions* ins = it[0].first;
		const TraceNeeds* tn = it[0].second;
		const EmitFunction& emit = tn->st[sim_state()].emit;
		while(ins->elems())
		{
		    const Instruction* nn=ins->next();
            if(! (nn->is_cpuid() && exec_cpuid(nn)) )
		        emit(nn);
		}
	    return;
	}

	priority_queue<cpu_time_ordering> pq;
	for(uint i=0;i<it.size();i++)
		pq.push(cpu_time_ordering(it[i]));

	for(;;)
	{
		cpu_time_ordering t=pq.top();pq.pop();
		if(t.ins->elems()==0)
			break;
		
		const Instruction* nn=t.ins->next();
        if(nn->is_cpuid() && exec_cpuid(nn))
		{
		    pq.push(t);
		    continue;
		}
		
		t.tn->st[sim_state()].emit(nn);
		t.set_now();
		pq.push(t);
	}

// 	cout << "notify B: " << Cotson::nanos() << " : ";
// 	for(uint i=0;i<it.size();i++)
// 		cout << it[i].first->elems() << " ";
// 	cout << endl;

	for(;!pq.empty();)
	{
		cpu_time_ordering t=pq.top();pq.pop();
		if(t.ins->elems()==0)
			continue;
		
		const Instruction* nn=t.ins->next();
        if(nn->is_cpuid() && exec_cpuid(nn))
		{
		    pq.push(t);
		    continue;
		}
		
		t.tn->st[sim_state()].emit(nn);
		t.set_now();
		pq.push(t);
	}
}

void Interleaver::break_sample() {
	for(uint i=0;i<it.size();i++)
		it[i].first->clear();
}

