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
#ifndef INTERLEAVER_H
#define INTERLEAVER_H

#include "instructions.h"
#include "trace_needs.h"
#include "state_observer.h"

class Interleaver : public SimpleStateObserver
{
	Interleaver() {}
	~Interleaver() {}

public:

	inline static Interleaver& get()
	{
		static Interleaver singleton;
		return singleton;
	}

	void config(uint64_t,Instructions&,const TraceNeeds*);
	void end_quantum();
	void break_sample();

protected:
	void update_cpus();
	struct CpuData 
	{
	    Instructions *ins;
		const TraceNeeds *tn;
		uint32_t dev;
		EmitFunction emit;
		uint64_t* pcycles;
	    uint64_t initial_cycles;
	    uint64_t order;

		CpuData(Instructions* i,const TraceNeeds *t, uint32_t d):ins(i),tn(t),dev(d) { update(0); }
	    inline void set_now() { order = *pcycles - initial_cycles; }
		void update(uint64_t);
	};
    struct CpuCmp 
	{ 
	    inline bool operator()(const CpuData* p1,const CpuData* p2) { return !(p1->order<p2->order); }
	};
	std::vector<CpuData> cpus;
};

#endif
