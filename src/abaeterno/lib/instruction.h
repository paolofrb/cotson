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
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "dump_gzip.h"
#include "read_gzip.h"
#include "memory_access.h"
#include "simplevec.h"
#include "inline.h"

class Instructions;

// TODO: this shouldn't be here
class InstType {
public:
	enum itype {
	   UNDEF=0,
	   // hack to preserve validity of old injector traces
	   DEFAULT='F', CTRL_BRANCH='B', CTRL_CALL='C', CTRL_JUMP='J', CTRL_RET='R', CTRL_IRET='r',
	   _MAXCTRL=127,
	   // any other type above MAXCTRL
	   CPUID=_MAXCTRL+1,
	   PREFETCH=_MAXCTRL+2,
	};

	InstType(const uint8_t*);
	INLINE InstType(itype t=UNDEF):type(t){}
	INLINE InstType(uint8_t t):type(t){}

	INLINE bool is_control()  const { return type && type < 128 && type != DEFAULT; }
	INLINE bool is_branch()   const { return type==CTRL_BRANCH; }
	INLINE bool is_cpuid()    const { return type==CPUID; }
	INLINE bool is_prefetch() const { return type==PREFETCH; }
	INLINE bool is_ret() const { return type==CTRL_RET || type==CTRL_IRET; }
	INLINE bool is_call() const { return type==CTRL_CALL; }
	INLINE bool valid() const { return type!=UNDEF; }
	INLINE void clear() { type=UNDEF; }

	INLINE operator uint8_t() const { return type; }
	// major hack to aggregate ret/iret stats
	INLINE uint8_t stat_index() const { return toupper(type); }
private:
    uint8_t type;
};

class Instruction : public boost::noncopyable
{
public:
	typedef simplevec<Memory::Access> Accesses;
	typedef Accesses::const_iterator MemIterator;

private:
	Memory::Access pc;

	const uint8_t* opcode;
	mutable InstType type;

	Accesses loads,stores;
	
	uint64_t RDI, RSI, RBX; // CPUID registers
	uint64_t cr3;
	uint64_t iid;
	static uint64_t unique_id;

	INLINE void clone(const Instruction& other)
	{
		pc=other.pc;
		type=other.type;
		opcode=other.opcode;
		iid=other.iid;
		cr3=other.cr3;
		RDI=other.RDI;
		RSI=other.RSI;
		RBX=other.RBX;
		// copy and shrink to fit
		loads.copy(other.loads);
		stores.copy(other.stores);
	}

public:

	INLINE Instruction():
		pc(0,0,0),opcode(0),cr3(0),iid(0) {}

	// this function reuses another instruction !
	INLINE static void init(Instruction* p,uint64_t vpc,uint64_t ppc,uint l,uint64_t lcr3)
	{
		p->pc=Memory::Access(vpc,ppc,l);
		p->opcode=0;
		p->type.clear();
		p->loads.clear();
		p->stores.clear();
		p->cr3=lcr3;
		p->RDI=0;
		p->RSI=0;
		p->RBX=0;
		p->iid = ++unique_id;
	}

	~Instruction() { }

	uint64_t instruction_id() const { return iid; }

	//these functions are used by the loader of data
	INLINE void CopyOpcode(const uint8_t*o,uint32_t l)
	{
		pc.length=l;
		opcode=o;
	}
	InstType Type() const { return type.valid() ? type : (type=InstType(opcode)); }
	INLINE InstType Type(InstType t) const { return type.valid() ? type : (type=t); }

	INLINE void cpuid_registers(uint64_t _rdi, uint64_t _rsi, uint64_t _rbx)
	{
		RDI = _rdi;
		RSI = _rsi;
		RBX = _rbx;
	}
	
	INLINE boost::tuple<uint64_t,uint64_t,uint64_t> cpuid_registers() const
	{
		return boost::make_tuple(RDI,RSI,RBX);
	}
	
	void disasm(std::ostream&) const;

	INLINE void PC(uint64_t _pc) { pc = _pc; }
	INLINE void Length(int len) { pc.length=len; }

	INLINE void StoresPush(const Memory::Access& a) { stores.push_back(a); }
	INLINE void LoadsPush(const Memory::Access& a) { loads.push_back(a); }

	INLINE int Stores() const { return stores.size(); }
	INLINE int Loads() const { return loads.size(); }

	//these functions are used by the user of the instruction   
	//we should separate this into two different interfaces
	INLINE const Memory::Access& PC() const { return pc; }
	INLINE int Length() const { return pc.length; }

	INLINE MemIterator LoadsBegin() const { return loads.begin(); }
	INLINE MemIterator LoadsEnd() const { return loads.end(); }
	INLINE MemIterator StoresBegin() const { return stores.begin(); }
	INLINE MemIterator StoresEnd() const { return stores.end(); }
	
	INLINE uint64_t getCR3() const { return cr3; }

	friend DumpGzip& operator<<(DumpGzip&,const Instruction&);
	friend std::ostream& operator<<(std::ostream&,const Instruction&);

	friend class Instructions;
};


#endif
