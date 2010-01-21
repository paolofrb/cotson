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
#include "logger.h"
#include "cotson.h"
#include "cpu_timer.h"
#include "liboptions.h"
#include "dump_gzip.h"
#include "storage_many.h"

using namespace std;
using namespace boost;

class MemoryTracer : public CpuTimer
{
public: 
	MemoryTracer(Parameters&);

	void simulation(const Instruction*);
	void simple_warming(const Instruction*);
	void full_warming(const Instruction*);
	void addMemory(std::string,Memory::Interface::Shared); 

protected:
	void endSimulation();
	void beginSimulation();

private: 
	inline void read(uint64_t, uint64_t, uint64_t);
	inline void write(uint64_t, uint64_t, uint64_t);
	inline void dump(uint64_t, uint64_t, uint64_t, uint64_t, bool);

	typedef Memory::Storage::Many Cache;
	boost::shared_ptr<Cache> cache;
	const std::string tracefile;
	uint64_t cycles;
	uint64_t instructions;
	uint64_t lru;
	uint64_t mem_rd, mem_wr, access;
	static uint64_t timestamp;

	bool shared;
	static boost::shared_ptr<Cache> shared_cache;
	static boost::scoped_ptr<DumpGzip> trace;
};

registerClass<CpuTimer,MemoryTracer> memtracer_c("memtracer");

// static members
boost::shared_ptr<MemoryTracer::Cache> MemoryTracer::shared_cache;
boost::scoped_ptr<DumpGzip> MemoryTracer::trace;
uint64_t MemoryTracer::timestamp = 0;

MemoryTracer::MemoryTracer(Parameters& p) : CpuTimer(&cycles,&instructions),
	tracefile(p.get<string>("tracefile")),
	cycles(0),
	instructions(0),
	lru(0),
	mem_rd(0),
	mem_wr(0),
	access(0),
	shared(p.get<bool>("shared"))
{
	add("instructions",instructions);
	add("cycles",cycles);
	add("mem_rd",mem_rd);
	add("mem_wr",mem_wr);
	add("access",access);
	add_ratio("ipc","instructions","cycles");	
	add_ratio("miss_rate","mem_rd","access");

	trace_needs.types=false;
	trace_needs.opcodes=false;
	trace_needs.history=1;
	trace_needs.st[SIMPLE_WARMING].set(EmitFunction::bind<MemoryTracer,&MemoryTracer::simple_warming>(this));
	trace_needs.st[FULL_WARMING].set(EmitFunction::bind<MemoryTracer,&MemoryTracer::full_warming>(this));
	trace_needs.st[SIMULATION].set(EmitFunction::bind<MemoryTracer,&MemoryTracer::simulation>(this));
	trace_needs.st[SIMULATION].needs_exception=false;
	trace_needs.st[SIMULATION].needs_heartbeat=false;
	trace_needs.st[SIMULATION].needs_register=false;
	if (shared) 
	{
		 // shared cache across all CPUs (static member)
	     if (!shared_cache) 
	         shared_cache.reset(new Cache(p));
	     cache=shared_cache;
	}
	else
	    cache.reset(new Cache(p));
    clear_metrics();

	// Only emit one shared trace (static member)
	if (!trace)
	    trace.reset(new DumpGzip(tracefile,"wb9f"));
}

void MemoryTracer::beginSimulation()
{
    clear_metrics();
	timestamp = Cotson::nanos();
}

void MemoryTracer::endSimulation()
{
	if (cycles > 0)
	    cout << "<<< cpu " << id() << " now=" << Cotson::nanos() 
		     << " instrs=" << instructions
		     << " cycles=" << cycles 
			 << endl;
}

inline void MemoryTracer::dump(uint64_t ts, uint64_t a, uint64_t id, uint64_t cr3, bool w)
{
	 // do some packing (works up to 128 cores)
	 uint8_t tmp = (static_cast<uint8_t>(id)&0x7f) | (static_cast<uint8_t>(w)<<7);
	 (*trace) << ts << a << cr3 << tmp;
	 // cout << ts << hex << " a " << a << " cr3 " <<  cr3 << " id " << id <<  " rw " << w << endl;
}

inline void MemoryTracer::read(uint64_t ts, uint64_t a, uint64_t cr3)
{
	access++;
	Memory::Line* cl = cache->find(a);
	if (cl)
	   cl->lru=++lru;
	else
	{
	    Memory::Line* rl = cache->find_lru(a);
		if (rl->moesi==Memory::MODIFIED)
		{
			// writeback
		    uint64_t wb = cache->cache.addr_from_group_id(rl->tag); 
		    dump(ts,wb,id(),cr3,true); // writeback
			mem_wr++;
		}
	    cache->allocate(rl,a,++lru,Memory::EXCLUSIVE);
		dump(ts,a,id(),cr3,false); // read line
		mem_rd++;
	}
}

inline void MemoryTracer::write(uint64_t ts, uint64_t a, uint64_t cr3)
{
	access++;
	Memory::Line* cl = cache->find(a);
	if (cl)
	{
	    cl->lru = ++lru;
	    cl->moesi = Memory::MODIFIED;
	}
	else
	{
	    Memory::Line* rl = cache->find_lru(a);
		if (rl->moesi==Memory::MODIFIED)
		{
			uint64_t wb = cache->cache.addr_from_group_id(rl->tag); 
		    dump(ts,wb,id(),cr3,true); // writeback
			mem_wr++;
	    }
	    cache->allocate(rl,a,++lru,Memory::MODIFIED);
		dump(ts,a,id(),cr3,false); // read line
		mem_rd++;
	}
}

void MemoryTracer::simulation(const Instruction* inst)
{
	// Check if time has advanced past us
	uint64_t t = Cotson::nanos();
	uint64_t ts = t > timestamp ? t : timestamp; // local timestamp

	uint64_t cr3 = inst->getCR3();
	const uint64_t pctag = cache->cache.group_id(inst->PC().phys);
	read(ts++,cache->cache.addr_from_group_id(pctag),cr3);

	for(Instruction::MemIterator i=inst->LoadsBegin(); i!=inst->LoadsEnd();++i)
	{
		uint64_t tag = cache->cache.group_id(i->phys);
		read(ts++,cache->cache.addr_from_group_id(tag),cr3);
	}

	for(Instruction::MemIterator i=inst->StoresBegin(); i!=inst->StoresEnd();++i)
	{
		uint64_t tag = cache->cache.group_id(i->phys);
		write(ts++,cache->cache.addr_from_group_id(tag),cr3);
	}
    instructions++;
	cycles += (ts-timestamp);
	timestamp = ts;
}

void MemoryTracer::full_warming(const Instruction* inst)
{
    uint64_t pc = inst->PC().phys;
	Memory::Line* pl = cache->find(pc);
    if (pl)
	    pl->lru = ++lru;
    else
	    cache->allocate(cache->find_lru(pc),pc,++lru,Memory::EXCLUSIVE);

	for(Instruction::MemIterator i=inst->LoadsBegin(); i!=inst->LoadsEnd();++i)
	{
		uint64_t a = i->phys;
	    Memory::Line* cl = cache->find(a);
        if (cl)
			cl->lru = ++lru;
		else
	        cache->allocate(cache->find_lru(a),a,++lru,Memory::EXCLUSIVE);
	}

	for(Instruction::MemIterator i=inst->StoresBegin(); i!=inst->StoresEnd();++i)
	{
		uint64_t a = i->phys;
	    Memory::Line* cl = cache->find(a);
        if (cl)
		{
			cl->lru = ++lru;
		    cl->moesi = Memory::MODIFIED;
		}
		else
	        cache->allocate(cache->find_lru(a),a,++lru,Memory::MODIFIED);
	}
    cycles++;
    instructions++;
}

void MemoryTracer::simple_warming(const Instruction* inst)
{
     full_warming(inst);
}

void MemoryTracer::addMemory(string name,Memory::Interface::Shared c) 
{
}

