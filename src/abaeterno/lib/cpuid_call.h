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
#ifndef CPUID_CALL_H
#define CPUID_CALL_H

// Reserved function of CPUID in AMD64.
// We will use this function to pass information from the
// code executing in the vm to abaeterno
#define RESERVED_CPUID_FUNCTION 0x80000009
#define CPUID_FUNCTION_MASK 	  0xFFFFFFFF

#include "instruction.h"

typedef enum { ONLY_FUNCTIONAL, FUNCTIONAL_AND_TIMING } FunctionalState;
typedef enum { DISCARD, KEEP } InstructionInQueue;

typedef boost::function<void(FunctionalState,uint64_t,uint64_t,
	uint64_t,uint64_t,uint64_t)> FunctionalCall;
typedef boost::function<InstructionInQueue(Instruction*,uint64_t,uint64_t)> 
	SimulationCall;

class CpuidCall : public boost::noncopyable
{
	typedef std::map<uint64_t,std::pair<FunctionalCall,SimulationCall> > Funcs;
	Funcs funcs;

	public:
		 
	static CpuidCall& get() {
		static CpuidCall singleton;
		return singleton;
	}
	
	static void add(uint64_t a,FunctionalCall&f,SimulationCall& s) {
		CpuidCall& me=get();
		if(me.funcs.find(a)!=me.funcs.end())
			throw std::runtime_error("cannot add two hooks to the same cpuid: "+
				boost::lexical_cast<std::string>(a));
		me.funcs[a]=std::make_pair(f,s);
	}

	static bool has(uint64_t a) {
		CpuidCall& me=get();
		return me.funcs.find(a)!=me.funcs.end();
	}

	static void functional(FunctionalState f,uint64_t nanos,uint64_t devid,
		uint64_t a,uint64_t b,uint64_t c) 
	{
		CpuidCall& me=get();
		Funcs::iterator i=me.funcs.find(a);
		if(i==me.funcs.end())
			return;
		FunctionalCall foo=i->second.first;
		if(!foo)
			return;
		foo(f,nanos,devid,a,b,c);		
	}

	static InstructionInQueue simulation(Instruction*inst,uint64_t nanos,uint64_t devid)
	{
		CpuidCall& me=get();
	 	uint64_t a,b,c;
		boost::tie(a,b,c)=inst->cpuid_registers();
		Funcs::iterator i=me.funcs.find(a);
		if(i==me.funcs.end())
			return DISCARD;
		SimulationCall bar=i->second.second;
		if(!bar)
			return DISCARD;
		return bar(inst,nanos,devid);		
	}
};

struct cpuid_call
{
	cpuid_call(uint64_t a,FunctionalCall f,SimulationCall s)
	{
		CpuidCall::add(a,f,s); 
	}
};

#endif
