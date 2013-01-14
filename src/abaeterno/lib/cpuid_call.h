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

#define COTSON_RESERVED_CPUID_BASE       0x80000009
#define COTSON_RESERVED_CPUID_RANGE      30000
#define COTSON_RESERVED_CPUID_EXT        (COTSON_RESERVED_CPUID_BASE+1000)
#define COTSON_RESERVED_CPUID_SELECTIVE  10
#define COTSON_RESERVED_ASM_BASE         5000

#define IS_COTSON_CPUID(rax) ( \
    ((rax) >= COTSON_RESERVED_CPUID_BASE) && \
    ((rax) <  COTSON_RESERVED_CPUID_BASE+COTSON_RESERVED_CPUID_RANGE) )

#define IS_COTSON_EXT_CPUID(x) \
    ((x) >= COTSON_RESERVED_CPUID_EXT)

#define COTSON_EXT_CPUID_OP(x) \
    ((x)-COTSON_RESERVED_CPUID_BASE)

#include "instruction.h"

typedef enum { ONLY_FUNCTIONAL, FUNCTIONAL_AND_TIMING } FunctionalState;
typedef enum { DISCARD, KEEP } InstructionInQueue;

typedef boost::function<void(FunctionalState,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t)> FunctionalCall;
typedef boost::function<InstructionInQueue(Instruction*,uint64_t)> SimulationCall;

class CpuidCall : public boost::noncopyable
{
    typedef std::map<uint64_t,std::pair<FunctionalCall,SimulationCall> > Funcs;
    Funcs funcs;

    public:
         
    static CpuidCall& get() 
    {
        static CpuidCall singleton;
        return singleton;
    }
    
    static void add(uint64_t a, FunctionalCall& f, SimulationCall& s) 
    {
        if(a >= COTSON_RESERVED_CPUID_RANGE)
            throw std::runtime_error("cpuid op "+ 
                boost::lexical_cast<std::string>(a) + " exceeds max of " +
                boost::lexical_cast<std::string>(COTSON_RESERVED_CPUID_RANGE-1));

        CpuidCall& me=get();
        if(me.funcs.find(a)!=me.funcs.end())
            throw std::runtime_error("cannot add two hooks to the same cpuid: "+
                boost::lexical_cast<std::string>(a));
        me.funcs[a]=std::make_pair(f,s);
    }

    static bool has(uint64_t a) 
    {
        CpuidCall& me=get();
        return me.funcs.find(a)!=me.funcs.end();
    }

    static void functional(
        FunctionalState f,
        uint64_t nanos,
        uint64_t devid,
        uint64_t a,uint64_t b,uint64_t c) 
    {
        CpuidCall& me=get();
        Funcs::iterator i=me.funcs.find(a);
        if(i==me.funcs.end())
            return;
        FunctionalCall fcall=i->second.first;
        if(!fcall)
            return;
        fcall(f,nanos,devid,a,b,c);      
    }

    static void add_xdata(
	    Instruction *inst, 
	    uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rbx)
	{
	    inst->add_xdata(rax); // 0
		inst->add_xdata(rdi); // 1
		inst->add_xdata(rsi); // 2
		inst->add_xdata(rbx); // 3
	}

    static InstructionInQueue simulation(Instruction* inst, uint64_t devid)
    {
		if (!inst->is_cpuid())
            return KEEP;
        CpuidCall& me=get();
        Funcs::iterator i=me.funcs.find(inst->get_xdata(1)); // RDI (opcode), see above
        if(i==me.funcs.end())
            return KEEP;
        SimulationCall scall=i->second.second;
        if(!scall)
            return KEEP;
        return scall(inst,devid);     
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
