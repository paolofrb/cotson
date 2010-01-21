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

#include "abaeterno.h"
#include "cotson.h"
#include "callbacks.h"
#include "liboptions.h"
#include "version.h"
#include "selector.h"
#include "error.h"
#include "command_socket.h"
#include "callme.h"
#include "machine.h"
#include "tokens.h"
#include "token_queue.h"

#include <typedefs.h>
#include <CMonitor.h>

#ifdef USE_MEMGATE
#include "memgate.h"
use_static_memgate usm;
#endif

using namespace std;
using namespace boost;
using namespace boost::assign;

struct Proxy : public CMonitorConsumer
{
	Proxy();

	inline static Proxy* get()
	{
		static Proxy singleton;
		return &singleton;
	}
		
	void InitializeMachine(int);
	void TerminateMachine(int,EVENTQUEUEINFO*);

	void CodeInjection(int m,UINT64 d,CCodeInjector *i);
	void MemoryInjection(int m,UINT64 d,CCodeInjector *i);
	void BranchInjection(int m,UINT64 d,CCodeInjector *i);
	void ExcInjection(int m,UINT64 d,CCodeInjector *i);
	void HeartBeatInjection(int m,UINT64 d,CCodeInjector *i);

	void CodeTranslation(int, UINT64, CCodeInjector *, CODETRANSLATESTRUCT *);
	void TaggedExecution(int, UINT64, CCodeInjector *, void *);
	UINT64 MonitorCallback(int nMachineID, UINT64 nTag);
	void FrequencyCallBack(int, UINT64, CCodeInjector*, UINT32);

	set<TokenQueue*> queues;
	map<uint64_t,bool> inject_code;
	map<uint64_t,Cotson::Cpu::Stats> cpu_stats;

	void RegisterEventQueue(int m,EVENTQUEUEINFO *q);
	void UnregisterEventQueue(int m,EVENTQUEUEINFO *q);

	void DeliverEventQueue(UINT64,int,EVENTQUEUEINFO*);
	void TimeCallback(int); 

	void RunControl(int,bool);

	void QuantumEnd(int);

	void ExternalEventTiming(EVENTTIMINGINFO* info);
	bool ReturnNextCommand(int,const char*, char*, int);

	int machineID;

// NOT in the INTF
	bool UpdateCpu(uint64_t,uint64_t,uint64_t);
	bool UpdateDev(uint64_t,uint64_t,uint64_t);
	void SetQuantum(uint64_t);
	void SetIPC(uint64_t,uint64_t,uint64_t);
    void NicTiming(EVENTTIMINGINFO_PACKET *);
    void DiskTiming(EVENTTIMINGINFO_BLOCK *);

	// gets called at the start of each function in which we know
	// that the time is synchronous between cores
	void synch_time();

	difference nanos;

	// these are used to fast forward
	uint64_t ffwd_ns;
	uint64_t ffwd_end;
	bool init_phase;
	void real_start(uint64_t);

	// sync quanta
	uint64_t sync_quantum_f;
	uint64_t sync_quantum_s;

	CCodeInjector *code_injector;
	CODETRANSLATESTRUCT *translate_struct;
	X8664REG regs;
	INSTRUCTIONINFO info;
	bool info_valid;
	bool regs_valid;
	bool time_feedback;

    inline void set_injector(CCodeInjector* i,CODETRANSLATESTRUCT* t=0)
    {
	    code_injector=i;
	    translate_struct=t;
	    info_valid=false;
	    regs_valid=false;
    }

    inline void clear_injector()
    {
	    code_injector=0;
	    translate_struct=0;
	    info_valid=false;
		regs_valid=false;
    }

    const INSTRUCTIONINFO& build_info()
    {
	    ERROR_IF(!code_injector,"no code injector present right now");
	    if (!info_valid) 
		{
	        if (!code_injector->BuildInstructionInfo(&info, sizeof(info)))
		        ERROR("error building instruction info");
            info_valid=true;
		}
		return info;
    }

    const X8664REG& get_regs()
    {
	    ERROR_IF(!code_injector,"no code injector present right now");
	    if (!regs_valid) 
		{
	        code_injector->GetX8664Reg(regs);
            regs_valid=true;
		}
		return regs;
    }
#ifdef NEED_GET_PROC_FREQ
    double GetProcessorFreq(UINT64, UINT64);
#endif
};

namespace {
	Proxy* proxy;
}


extern "C" DLLEXPORT CMonitorConsumer *SimNowGetMonitorInterface(const char *name, const char *args)
{
	Option::init(args);

#ifdef USE_MEMGATE
	MemGate::depth=Option::get<int>("memgate_depth");
	MemGate::check_base=Option::get<bool>("memgate_check_base");
#endif

	Selector::all()=Option::get<int>("debug_messages");
	if(Option::has("debug_this"))
	{
		vector<string> dt=Option::getV<string>("debug_this");
		for(vector<string>::iterator i=dt.begin();i!=dt.end();i++)
			Selector::on(*i);
	}

	if(Option::has("help"))
	{
		Option::print(cout);
		exit(1);
	}

	if(Option::has("version"))
	{
		format fmt("%1%%|20t|%2%");
		printVersion(cerr,fmt);
		exit(1);
	}

	vector<string> unk=Option::unknown(); 
	if(!unk.empty() && Option::get<bool>("print_unknown")) 
	{
		cout << "unknown options: ";
		copy(unk.begin(),unk.end(),ostream_iterator<string>(cout," "));
		cout << endl;
	}

	if(!unk.empty() && !Option::get<bool>("allow_unknown"))
		ERROR("dying due to unknown options in command line");

	Call::init()();
	
	proxy=Proxy::get();
	proxy->sync_quantum_f = proxy->sync_quantum_s = Option::get<uint64_t>("mp_sync_quantum");
	if(Option::has("mp_sync_quantum_sim"))
	    proxy->sync_quantum_s = Option::get<uint64_t>("mp_sync_quantum_sim");

	return proxy;
}

Proxy::Proxy() : 
	machineID(-1),
	ffwd_ns(Option::get<uint64_t>("fastforward")),
	ffwd_end(0),
	init_phase(true),
	code_injector(0),
	translate_struct(0),
	info_valid(0),
	regs_valid(0),
	time_feedback(Option::get<bool>("time_feedback"))
{
}

//INTF
void Proxy::InitializeMachine(int m)
{
	machineID=m;
	if(!EnableEventCollection(machineID))
		ERROR("EnableEventCollection failed");
}

//INTF
void Proxy::TerminateMachine(int m,EVENTQUEUEINFO *) 
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	synch_time();

	Call::terminate()();
	CMonitorConsumer::Terminate(); 
}

//INTF
void Proxy::CodeInjection(int m,UINT64 d,CCodeInjector *i) 
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	set_injector(i);
	if (AbAeterno::get().has_pending_tags())
	{
		build_info();
	    UINT8 opcode[64];
	    int n = i->OpcodeBytes(opcode, 64);
		const uint8_t* op = opcode+info.nOpcodeOffset;
		if (AbAeterno::get().inject_tag(d,n,op)) {
	        LOG("(inject tag)",i->Disassemble(opcode,n));
	    }
	}
	if (inject_code[d])
	    Machine::get().inject(d,CODE);
	clear_injector();
}

//INTF
void Proxy::MemoryInjection(int m,UINT64 d,CCodeInjector *i)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	set_injector(i);
	Machine::get().inject(d,MEMORY);
	clear_injector();
}

//INTF
void Proxy::BranchInjection(int m,UINT64 d,CCodeInjector *i) 
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
}

//INTF
void Proxy::ExcInjection(int m,UINT64 d,CCodeInjector *i)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	set_injector(i);
	Machine::get().inject(d,EXCEPT);
	clear_injector();
}

//INTF
void Proxy::HeartBeatInjection(int m,UINT64 d,CCodeInjector *i)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	set_injector(i);
	Machine::get().inject(d,HEARTBEAT);
	clear_injector();
}

//INTF
void Proxy::CodeTranslation(int m, UINT64 devid, 
	CCodeInjector *i, CODETRANSLATESTRUCT *t)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	set_injector(i,t);
	AbAeterno::get().translate(devid);
	clear_injector();
}

//INTF
void Proxy::TaggedExecution(int m, UINT64 devid, CCodeInjector *i, void *pParam)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	uint32_t tag = static_cast<uint32_t>(reinterpret_cast<uint64_t>(pParam));
	if (!tag) return;

	set_injector(i);
	synch_time();
	LOG("TaggedExecution - tag:",dec,tag,"time",nanos);
	// First, we check whether this is a tagged operation handled in parsing
	if (!Machine::get().execute(devid,tag))
	    // If not, we hand it to abaeterno
	    AbAeterno::get().execute(nanos,devid,tag);
	clear_injector();
}

//INTF
UINT64 Proxy::MonitorCallback(int m, UINT64 nTag)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	synch_time();
	return nTag;
}

//INTF
void Proxy::FrequencyCallBack(int m, UINT64 devid, CCodeInjector* inj, UINT32 freq) 
{
  // Do nothing, we use proxy->getProcessorFreq()
}

//INTF
void Proxy::RegisterEventQueue(int m,EVENTQUEUEINFO *q)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	TokenQueue* tq=reinterpret_cast<TokenQueue*>(q);

	set<TokenQueue*>::iterator i=queues.find(tq);
	if(i!=queues.end()) 
		ERROR("Duplicated queue: id ", tq->devid, " flags ", tq->flags);

	queues.insert(tq);
}

//INTF
void Proxy::UnregisterEventQueue(int m,EVENTQUEUEINFO *q)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	TokenQueue* tq=reinterpret_cast<TokenQueue*>(q);

	set<TokenQueue*>::iterator i=queues.find(tq);
	if(i==queues.end()) 
		ERROR("Unknown queue");

	queues.erase(i);
}

//INTF
void Proxy::DeliverEventQueue(UINT64 reason,int m,EVENTQUEUEINFO*q)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");

	// this call is not synchronous with all cores

	if (init_phase) return;
	if ((reason & DELIVERYREASON_DELIVERYSTART) == 0) return;

	Machine::get().empty_queue(reinterpret_cast<TokenQueue*>(q));
}

//INTF
void Proxy::TimeCallback(int m) 
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	if (init_phase)
	{
	    uint64_t simnow_nanos = ExactCurrentSimulationTime(machineID);
	    LOG("[init] TimeCallback",simnow_nanos);
	    CallMe::hey(simnow_nanos);
	}
	else
	{
	    synch_time();
	    LOG("TimeCallback",nanos);
	    CallMe::hey(nanos);
	}
}

//INTF
void Proxy::QuantumEnd(int m)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	synch_time();
	if (init_phase) return;
	AbAeterno::get().endQuantum();
}

//INTF
void Proxy::RunControl(int m,bool starting)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");
	synch_time();
	if(starting)
	{
		if(init_phase)
			real_start(0);
		else 
			Call::start()();
	}
	else
	{
		if(!init_phase) 
			Call::stop()();
	}
}

//INTF
bool Proxy::ReturnNextCommand(int m,const char* response, char* command, int bufsiz)
{
	ERROR_IF(m!=machineID,"different machines: '", m, "' vs '", machineID,"'");

	return CommandSocket::next(response,command,bufsiz);
}

//INTF
void Proxy::ExternalEventTiming(EVENTTIMINGINFO* einfo)
{
	synch_time();
	switch(einfo->ettType) // class dispatcher
	{
	    case ettBlock:
		    DiskTiming(reinterpret_cast<EVENTTIMINGINFO_BLOCK*>(einfo)); 
		    break;
	    case ettPacket: 
		    NicTiming(reinterpret_cast<EVENTTIMINGINFO_PACKET*>(einfo)); 
			break;
		default: ERROR("unknown type of external timing event");
    }
}


//// NONE OF THE FOLLOWING ARE PART OF THE IAnalysisConsumer INTERFACE

bool Proxy::UpdateDev(uint64_t devid ,uint64_t key,uint64_t val)
{
	// Updating device events does not flush the cache
	return UpdateEventOptions(machineID,devid,key,val);
}

bool Proxy::UpdateCpu(uint64_t devid ,uint64_t key,uint64_t val)
{
	// Updating cpu events flushes the cache
	Cotson::Cpu::flush(devid); 
	return UpdateEventOptions(machineID,devid,key,val);
}

void Proxy::SetIPC(uint64_t devid,uint64_t insts,uint64_t cycles)
{
	if (!time_feedback)
	    return;
	if(!SetProcessorIPC(machineID,devid,insts,cycles))
		ERROR("SetProcessorIPC failed");
}

void Proxy::DiskTiming(EVENTTIMINGINFO_BLOCK *block)
{
	Machine& machine=Machine::get();
	uint64_t devid=block->nDeviceID;
    uint64_t bn=block->nBlockNumber;
    uint64_t bc=block->nBlockCount;
    switch(block->attType)
	{
		case attEventEnd: 
		    block->nDelay=machine.disk_end(devid); 
			break;
		case attPIORead:
			block->nDelay=machine.disk_read(devid,bn,bc,false);
			break;
		case attDMARead:
			block->nDelay=machine.disk_read(devid,bn,bc,true);
			break;
		case attPIOWrite:
			block->nDelay=machine.disk_write(devid,bn,bc,false);
			break;
		case attDMAWrite:
			block->nDelay=machine.disk_write(devid,bn,bc,true);
			break;
		default:
			ERROR("unknown combination of type for this disk access");
	}
}

void Proxy::NicTiming(EVENTTIMINGINFO_PACKET *packet)
{
	Machine& machine=Machine::get();
	uint64_t devid=packet->nDeviceID;
	uint64_t len=packet->nPacketLength;
	void* data=packet->pPacketData;
    switch(packet->attType)
	{
	    case attEventEnd: 
		    packet->nDelay=machine.nic_end(devid); 
			break;
	    case attPIORead:
			packet->nDelay=machine.nic_read(devid,len,data,false);
			break;
		case attDMARead:
			packet->nDelay=machine.nic_read(devid,len,data,true);
			break;
		case attPIOWrite:
			packet->nDelay=machine.nic_write(devid,len,data,false);
			break;
		case attDMAWrite:
			packet->nDelay=machine.nic_write(devid,len,data,true);
			break;
		default:
			ERROR("unknown type combination for this nic access");
	}
}

void Proxy::SetQuantum(uint64_t q)
{
    if(!QuantumEndConfig(machineID, 0, q))
	    ERROR("QuantumEndConfig failed");
}
//**    QuantumEndConfig - Set the parameters for QuantumEnd callback interval
// virtual bool QuantumEndConfig(int nMachineID, UINT64 nTime, UINT64 nError) = 0;
//
//  nMachineID      = Logical ID of the machine to create the event on
//  nTime           = Minimum time (in ns) for notification inverval 
//  nError          = Allowable error (in ns): nTime + nError is the max interval
//
// Setting nTime = 0 will result in a notification on every sync point.
// Setting nError = 0 will result in a notification every nTime ns.
// Setting nError < SyncQuantum may introduce new sync points.
// Setting nError >= SyncQuantum will not introduce new sync points.
// Setting nTime = 0, nError = 0 will disable all notifications.



// this function takes care of preventing the stall problem
void Proxy::real_start(uint64_t when)
{
	synch_time();
	// FIXME: This is a hack to try prevent the "stalling" problem
	// that we observe at SimNow initialization. Basically,
	// we wait until CPU0 has advanced at least 1 instruction.
	// If it hasn't, we just keep advancing time for 1ns
	static uint64_t nwait = 0;
	static uint64_t maxwait = Option::get<uint64_t>("init_wait");
	if (nwait++ == maxwait)
		ERROR("Exceeded initialization wait time");

	uint64_t insts = InstructionCount(machineID,0); 
	uint64_t now = ExactCurrentSimulationTime(machineID); // DON'T USE NANOS

	LOG("real_start",now);
	if(insts == 0) 
	{
		CallMe::later(1,bind(&Proxy::real_start,this,_1));
	}
	else if(init_phase && ffwd_ns > 0) 
	{
		CallMe::later(ffwd_ns,bind(&Proxy::real_start,this,_1));
		ffwd_end = now + ffwd_ns;
		ffwd_ns = 0;
		cout << "Fast forward from " << now << " ns to " << ffwd_end << " ns" << endl;
	}
	else if(init_phase && now >= ffwd_end) 
	{
		init_phase = false;
		SetQuantum(sync_quantum_f);
		Call::prepare()();
		RunControl(machineID,true);
		if (ffwd_end > nwait)
			cout << "Fast forward ended at " << now << " ns\n";
	}
}

void Proxy::synch_time()
{
	uint64_t t = ExactCurrentSimulationTime(machineID);
	if (init_phase) 
		nanos.reset(t);
	nanos.set(t);
}

////////////////////////////////////////////////////////////////////////////////
// Here starts the Cotson interface
////////////////////////////////////////////////////////////////////////////////

const set<string>& Cotson::statistics_names() 
{ 
	static set<string> s;
	s+="trace_cache_size";
	s+="valid_translation_bytes";
	s+="invalid_translation_bytes";
	s+="metadata_bytes";
	s+="plain_invalidations";
	s+="range_invalidations";
	s+="other_exceptions";
	s+="segv_exceptions";
	s+="read_pios";
	s+="write_pios";
	s+="read_mmios";
	s+="write_mmios";
	s+="iocount";
	s+="haltcount";
	return s;
}

uint64_t Cotson::nanos() { return proxy->nanos; }

void Cotson::Cpu::init(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_INSTRUCTION, INSTRUCTIONS_ON))
		ERROR("UpdateEventOptions failed (cpu code on)");
	proxy->inject_code[devid] = false;
	Stats &stats = proxy->cpu_stats[devid];
	stats.ipc_insts=stats.ipc_cycles=1;
	stats.insts.reset(Cotson::Cpu::instructions(devid));
	stats.idles.reset(Cotson::Cpu::idlecount(devid));
	stats.cycles=0;
}

void Cotson::Cpu::force_flush()
{
	for(map<uint64_t,bool>::iterator i=proxy->inject_code.begin();
	    i!=proxy->inject_code.end(); ++i)
	{
	    if(!proxy->UpdateCpu(i->first, EVENT_INSTRUCTION, INSTRUCTIONS_OFF))
		    ERROR("UpdateEventOptions failed (cpu code off)");
	    if(!proxy->UpdateCpu(i->first, EVENT_INSTRUCTION, INSTRUCTIONS_ON))
		    ERROR("UpdateEventOptions failed (cpu code on)");
	}
}

uint64_t Cotson::Cpu::instructions(uint64_t devid)
{
	return proxy->InstructionCount(proxy->machineID,devid);
}

uint64_t Cotson::Cpu::idlecount(uint64_t devid) 
{
	return proxy->IdleCount(proxy->machineID,devid);
}

uint64_t Cotson::Cpu::cycles(uint64_t devid) 
{
	double cycles_per_ns=proxy->GetProcessorFreq(proxy->machineID,devid)/1000.0;
    return llround(static_cast<double>(proxy->nanos)*cycles_per_ns);
}

uint64_t Cotson::Cpu::statistic(uint64_t devid,const string& s) 
{
	PROCESSORSTATISTICS_AWESIM awe;
	if(!proxy->ProcessorStatistics(proxy->machineID,devid,&awe,sizeof(awe)))
		ERROR("ProcessorStatistics failed");

	if(awe.piID != PIDAweSim) 
		ERROR("piID should be AWESIM");

	if(s=="trace_cache_size") 
		return static_cast<uint64_t>(awe.nTraceCacheSize);
	if(s=="valid_translation_bytes") 
		return static_cast<uint64_t>(awe.nValidTranslationBytes);
	if(s=="invalid_translation_bytes") 
		return static_cast<uint64_t>(awe.nInvalidTranslationBytes);
	if(s=="metadata_bytes") 
		return static_cast<uint64_t>(awe.nMetaDataBytes);
	if(s=="plain_invalidations") 
		return awe.nPlainInvalidations;
	if(s=="range_invalidations") 
		return awe.nRangeInvalidations;
	if(s=="other_exceptions") 
		return awe.nOtherExceptions;
	if(s=="segv_exceptions") 
		return awe.nSegvExceptions;
	if(s=="read_pios") 
		return awe.nReadPios;
	if(s=="write_pios") 
		return awe.nWritePios;
	if(s=="read_mmios") 
		return awe.nReadMmios;
	if(s=="write_mmios") 
		return awe.nWriteMmios;
	if(s=="iocount") 
		return awe.nIOCount;
	if(s=="haltcount") 
		return awe.nHaltCount;

	ERROR("unknown statistic "+s);
	return 0;
}

void Cotson::Cpu::flush(uint64_t devid)
{
    Profiler::get().clear_tags(devid);
}

void Cotson::Cpu::set_ipc(uint64_t devid,uint64_t insts,uint64_t cycles) 
{
	stats(devid);
	Stats& stats = proxy->cpu_stats[devid];
	if (stats.ipc_cycles != cycles || stats.ipc_insts != insts)
	{
	    stats.ipc_insts=insts;
	    stats.ipc_cycles=cycles;
	    proxy->SetIPC(devid,insts,cycles);
    }
}

const Cotson::Cpu::Stats& Cotson::Cpu::stats(uint64_t devid)
{
	Stats& stats = proxy->cpu_stats[devid];

	double curcpi = (double)stats.ipc_cycles/(double)stats.ipc_insts;
	uint64_t finsts = Cotson::Cpu::instructions(devid);
	uint64_t fidles = Cotson::Cpu::idlecount(devid);
	uint64_t dinsts = (finsts-stats.insts.base) - stats.insts;
	uint64_t fcycles = static_cast<uint64_t>(curcpi*static_cast<double>(dinsts));

	stats.cycles += fcycles;
	stats.insts.set(finsts);
	stats.idles.set(fidles);
	return stats;
}

void Cotson::Cpu::set_quantum_f()
{
	proxy->SetQuantum(proxy->sync_quantum_f);
}
void Cotson::Cpu::set_quantum_s()
{
	proxy->SetQuantum(proxy->sync_quantum_s);
}

void Cotson::Cpu::Tokens::code_on(uint64_t devid)
{
	proxy->inject_code[devid] = true;
}

void Cotson::Cpu::Tokens::code_off(uint64_t devid)
{
	proxy->inject_code[devid] = false;
}

void Cotson::Cpu::Tokens::memory_on(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_MEMORY, MEMORY_ON))
		ERROR("UpdateEventOptions failed (cpu memory on)");
}

void Cotson::Cpu::Tokens::memory_off(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_MEMORY, MEMORY_OFF))
		ERROR("UpdateEventOptions failed (cpu memory off)");
}

void Cotson::Cpu::Tokens::exception_on(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_EXCEPTION, EXCEPTION_ON))
		ERROR("UpdateEventOptions failed (cpu exception on)");
}

void Cotson::Cpu::Tokens::exception_off(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_EXCEPTION, EXCEPTION_OFF))
		ERROR("UpdateEventOptions failed (cpu exception off)");
}

void Cotson::Cpu::Tokens::heartbeat_on(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_HEARTBEAT, HEARTBEAT_ON))
		ERROR("UpdateEventOptions failed (cpu heartbeat on)");
}

void Cotson::Cpu::Tokens::heartbeat_off(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_HEARTBEAT, HEARTBEAT_OFF))
		ERROR("UpdateEventOptions failed (cpu heartbeat off)");
}

void Cotson::Cpu::Tokens::register_on(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_REGISTER, REGISTER_ON))
		ERROR("UpdateEventOptions failed (cpu register on)");
}

void Cotson::Cpu::Tokens::register_off(uint64_t devid)
{
	if(!proxy->UpdateCpu(devid, EVENT_REGISTER, REGISTER_OFF))
		ERROR("UpdateEventOptions failed (cpu register off)");
}

void Cotson::Disk::Tokens::on(uint64_t devid) 
{
	if(!proxy->UpdateDev(devid, EVENT_DISK_ID, DISK_ID_ON))
		ERROR("UpdateEventOptions failed (disk on, 1)");

	if(!proxy->UpdateDev(devid, EVENT_DISK_IO, DISK_IO_ON))
		ERROR("UpdateEventOptions failed (disk on, 2)");
}

void Cotson::Nic::Tokens::on(uint64_t devid) 
{
	if (!proxy->UpdateDev(devid, EVENT_NIC_IO,NIC_IO_ON))
		ERROR("UpdateEventOptions failed (nic on)");
}

void Cotson::callback(uint64_t after) 
{
	LOG("RequestCallbackDelayed",after);
	if(!proxy->RequestCallbackDelayed(proxy->machineID,after))
		ERROR("RequestCallbackDelayed failed");
}

void Cotson::exit() 
{
	Call::terminate()();
	proxy->Terminate();
}

void Cotson::Inject::code() 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	proxy->code_injector->AddCodeFetchEvent();
}

void Cotson::Inject::memory() 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	proxy->code_injector->AddMemoryEvent();
}

void Cotson::Inject::exception() 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	proxy->code_injector->AddExcEvent();
}

void Cotson::Inject::heartbeat() 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	proxy->code_injector->AddHeartBeatEvent();
}

void Cotson::Inject::token(uint64_t token)
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	UINT64 tokenU=static_cast<UINT64>(token);
	proxy->code_injector->AddTokens(&tokenU,1);
}

void Cotson::Inject::token(uint64_t event,uint64_t len)
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	UINT64 tokenU=static_cast<UINT64>(MAKE_EVENT(event,len));
	proxy->code_injector->AddTokens(&tokenU,1);
}

void Cotson::Inject::register_token(const char* reg) 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	if(!proxy->code_injector->AddRegisterValueTokens(reg))
		ERROR("error injecting register "+string(reg));
}

const Cotson::Inject::info_instruction&
Cotson::Inject::current_opcode(function<uint8_t*(int)> use_this_malloc) 
{
	const INSTRUCTIONINFO& info = proxy->build_info();
	// info has fields for source and destination register operands
	// have a look at MonRegMask ... inside INSTRUCTIONINFO at
	// CodeInjector.h

	static info_instruction ii; // we only need one at a time
	ii.length=info.nLength;
	ii.opcode = use_this_malloc(ii.length);
	ii.length = proxy->code_injector->OpcodeBytes(ii.opcode,ii.length);
	ERROR_IF(ii.length!=info.nLength,"incorrect instruction length");

	ii.pc=info.nPhysicalPC1;
	const uint8_t* opcode = ii.opcode + info.nOpcodeOffset;
	const uint8_t* modrm = ii.opcode + info.nModrmOffset;

	ii.is_cpuid=Cotson::X86::is_cpuid(opcode);

	ii.is_cr3_change=info.bPagingEnabled && 
		info.nLength > (info.nOpcodeOffset + 2) && 
		info.bHasModrm && 
		Cotson::X86::is_cr3mov(opcode,modrm);

	return ii; // only one at a time, return by ref to save a copy
}

Cotson::Inject::info_opcode Cotson::Inject::translate_info() 
{
	const CODETRANSLATESTRUCT *t=proxy->translate_struct;
	ERROR_IF(!t,"no translation structure present right now");
	return info_opcode(t->nOpcodeCount,t->pOpcodeBuffer + t->nOpcodeOffset);
}

void Cotson::Inject::tag(uint32_t tag) 
{
	CODETRANSLATESTRUCT *t=proxy->translate_struct;
	ERROR_IF(!t,"no translation structure present right now");
	t->nInstructionTag = tag;  
}

void Cotson::Inject::save_tag_info(uint64_t d,uint32_t tag,Cotson::Inject::tag_type tt) 
{
	const INSTRUCTIONINFO& ii = proxy->build_info();
	Cotson::Inject::info_tag ti;
	ti.type = tt;
	// Generic memory access info
	ti.base_reg = ii.bHasBase ? 15-ii.nSIBBaseReg : -1;
	ti.index_reg = ii.bHasIndex ? 15-ii.nSIBIndexReg : -1;
	ti.disp = ii.bHasDisplacement ? ii.nDisplacement : 0;
	ti.scale = (ii.bHasIndex && ii.bHasScale) ? (1 << ii.nSibScale) : 1;
    ti.segment = ii.nSegment;
	ti.size_mask =   ii.eAddressSize == Size16Bit ? 0x000000000000FFFF 
	               : ii.eAddressSize == Size32Bit ? 0x00000000FFFFFFFF
			                                      : 0xFFFFFFFFFFFFFFFF;
	Machine::get().tag(d,tag,ti);
}

uint64_t Cotson::Inject::PC(bool phys)
{
	return   phys 
	       ? proxy->build_info().nPhysicalPC1
	       : proxy->build_info().nLinearPC;
}

uint64_t Cotson::Memory::physical_address(uint64_t a)
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	return proxy->code_injector->GetPhysAddress(a);
}

void Cotson::Memory::read_physical_memory(uint64_t address,uint32_t length,uint8_t*buf) 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	uint32_t r;
	proxy->code_injector->ReadPhysicalMemory(address,buf,length,&r);
	ERROR_IF(length!=r,"ReadPhysicalMemoryError");
}

void Cotson::Memory::write_physical_memory(uint64_t address,uint32_t length,uint8_t*buf) 
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");
	uint32_t r;
	proxy->code_injector->WritePhysicalMemory(address,buf,length,&r);
	ERROR_IF(length!=r,"WritePhysicalMemoryError");
}

uint64_t Cotson::Memory::address_from_tag(const Cotson::Inject::info_tag& ti)
{
	ERROR_IF(!proxy->code_injector,"no code injector present right now");

    int64_t base = ti.base_reg < 0 ? 0 : X86::IntegerReg(ti.base_reg);
    int64_t index = ti.index_reg < 0 ? 0 : X86::IntegerReg(ti.index_reg);
    int64_t va = base + ti.disp + (ti.scale * index);

	uint64_t segm = 0;
    bool not64b = X86::FlagsCS() & 0x200;
    switch(ti.segment)
    {
        case 0: if (not64b) segm=X86::BaseES(); break;
        case 1: if (not64b) segm=X86::BaseCS(); break;
        case 2: if (not64b) segm=X86::BaseSS(); break;
        case 3: if (not64b) segm=X86::BaseDS(); break;
        case 4: segm=X86::BaseFS(); break;
        case 5: segm=X86::BaseGS(); break;
        default: break;
    }
	return ((va + segm) & ti.size_mask);
}

uint64_t Cotson::c2t(uint64_t c) 
{
	double cpu0_cycles_per_ns=proxy->GetProcessorFreq(proxy->machineID,0)/1000.0;
    return llround(static_cast<double>(c)/cpu0_cycles_per_ns);
}
uint64_t Cotson::t2c(uint64_t t) 
{
	double cpu0_cycles_per_ns=proxy->GetProcessorFreq(proxy->machineID,0)/1000.0;
    return llround(static_cast<double>(t)*cpu0_cycles_per_ns);
}

const set<TokenQueue*>& Cotson::queues() { return proxy->queues; }

// Register access
uint64_t Cotson::X86::IntegerReg(int reg) { return proxy->get_regs().IntegerRegs[reg]; }
uint16_t Cotson::X86::SelectorES() { return proxy->get_regs().SelectorES; }
uint16_t Cotson::X86::FlagsES() { return proxy->get_regs().FlagsES; }
uint32_t Cotson::X86::LimitES() { return proxy->get_regs().LimitES; }
uint64_t Cotson::X86::BaseES() { return proxy->get_regs().BaseES; }
uint16_t Cotson::X86::SelectorCS() { return proxy->get_regs().SelectorCS; }
uint16_t Cotson::X86::FlagsCS() { return proxy->get_regs().FlagsCS; }
uint32_t Cotson::X86::LimitCS() { return proxy->get_regs().LimitCS; }
uint64_t Cotson::X86::BaseCS() { return proxy->get_regs().BaseCS; }
uint16_t Cotson::X86::SelectorSS() { return proxy->get_regs().SelectorSS; }
uint16_t Cotson::X86::FlagsSS() { return proxy->get_regs().FlagsSS; }
uint32_t Cotson::X86::LimitSS() { return proxy->get_regs().LimitSS; }
uint64_t Cotson::X86::BaseSS() { return proxy->get_regs().BaseSS; }
uint16_t Cotson::X86::SelectorDS() { return proxy->get_regs().SelectorDS; }
uint16_t Cotson::X86::FlagsDS() { return proxy->get_regs().FlagsDS; }
uint32_t Cotson::X86::LimitDS() { return proxy->get_regs().LimitDS; }
uint64_t Cotson::X86::BaseDS() { return proxy->get_regs().BaseDS; }
uint16_t Cotson::X86::SelectorFS() { return proxy->get_regs().SelectorFS; }
uint16_t Cotson::X86::FlagsFS() { return proxy->get_regs().FlagsFS; }
uint32_t Cotson::X86::LimitFS() { return proxy->get_regs().LimitFS; }
uint64_t Cotson::X86::BaseFS() { return proxy->get_regs().BaseFS; }
uint16_t Cotson::X86::SelectorGS() { return proxy->get_regs().SelectorGS; }
uint16_t Cotson::X86::FlagsGS() { return proxy->get_regs().FlagsGS; }
uint32_t Cotson::X86::LimitGS() { return proxy->get_regs().LimitGS; }
uint64_t Cotson::X86::BaseGS() { return proxy->get_regs().BaseGS; }
uint16_t Cotson::X86::SelectorGDT() { return proxy->get_regs().SelectorGDT; }
uint16_t Cotson::X86::FlagsGDT() { return proxy->get_regs().FlagsGDT; }
uint32_t Cotson::X86::LimitGDT() { return proxy->get_regs().LimitGDT; }
uint64_t Cotson::X86::BaseGDT() { return proxy->get_regs().BaseGDT; }
uint16_t Cotson::X86::SelectorLDT() { return proxy->get_regs().SelectorLDT; }
uint16_t Cotson::X86::FlagsLDT() { return proxy->get_regs().FlagsLDT; }
uint32_t Cotson::X86::LimitLDT() { return proxy->get_regs().LimitLDT; }
uint64_t Cotson::X86::BaseLDT() { return proxy->get_regs().BaseLDT; }
uint16_t Cotson::X86::SelectorIDT()	 { return proxy->get_regs().SelectorIDT;	 }
uint16_t Cotson::X86::FlagsIDT() { return proxy->get_regs().FlagsIDT; }
uint32_t Cotson::X86::LimitIDT() { return proxy->get_regs().LimitIDT; }
uint64_t Cotson::X86::BaseIDT() { return proxy->get_regs().BaseIDT; }
uint16_t Cotson::X86::SelectorTR() { return proxy->get_regs().SelectorTR; }
uint16_t Cotson::X86::FlagsTR() { return proxy->get_regs().FlagsTR; }
uint32_t Cotson::X86::LimitTR() { return proxy->get_regs().LimitTR; }
uint64_t Cotson::X86::BaseTR() { return proxy->get_regs().BaseTR; }
uint64_t Cotson::X86::IORestartRIP() { return proxy->get_regs().IORestartRIP; }
uint64_t Cotson::X86::IORestartRCX() { return proxy->get_regs().IORestartRCX; }
uint64_t Cotson::X86::IORestartRSI() { return proxy->get_regs().IORestartRSI; }
uint64_t Cotson::X86::IORestartRDI() { return proxy->get_regs().IORestartRDI; }
uint64_t Cotson::X86::IORestartDword() { return proxy->get_regs().IORestartDword; }
uint8_t Cotson::X86::IORestartFlag() { return proxy->get_regs().IORestartFlag; }
uint8_t Cotson::X86::HLTRestartFlag() { return proxy->get_regs().HLTRestartFlag; }
uint8_t Cotson::X86::NMIBlocked() { return proxy->get_regs().NMIBlocked; }
uint8_t Cotson::X86::CurrentPL() { return proxy->get_regs().CurrentPL; }
uint64_t Cotson::X86::EFERRegister() { return proxy->get_regs().EFERRegister; }
uint32_t Cotson::X86::MachineState() { return proxy->get_regs().MachineState; }
uint32_t Cotson::X86::Revision() { return proxy->get_regs().Revision; }
uint64_t Cotson::X86::SMMBase() { return proxy->get_regs().SMMBase; }
uint64_t Cotson::X86::RegisterCR4() { return proxy->get_regs().RegisterCR4; }
uint64_t Cotson::X86::RegisterCR3() { return proxy->get_regs().RegisterCR3; }
uint64_t Cotson::X86::RegisterCR0() { return proxy->get_regs().RegisterCR0; }
uint64_t Cotson::X86::RegisterDR7() { return proxy->get_regs().RegisterDR7; }
uint64_t Cotson::X86::RegisterDR6() { return proxy->get_regs().RegisterDR6; }
uint64_t Cotson::X86::RegisterEFlags() { return proxy->get_regs().RegisterEFlags; }
uint64_t Cotson::X86::RegisterRIP() { return proxy->get_regs().RegisterRIP; }
uint64_t Cotson::X86::R15() { return proxy->get_regs().IntegerRegs[0]; }
uint64_t Cotson::X86::R14() { return proxy->get_regs().IntegerRegs[1]; }
uint64_t Cotson::X86::R13() { return proxy->get_regs().IntegerRegs[2]; }
uint64_t Cotson::X86::R12() { return proxy->get_regs().IntegerRegs[3]; }
uint64_t Cotson::X86::R11() { return proxy->get_regs().IntegerRegs[4]; }
uint64_t Cotson::X86::R10() { return proxy->get_regs().IntegerRegs[5]; }
uint64_t Cotson::X86::R9()  { return proxy->get_regs().IntegerRegs[6]; }
uint64_t Cotson::X86::R8()  { return proxy->get_regs().IntegerRegs[7]; }
uint64_t Cotson::X86::RDI() { return proxy->get_regs().IntegerRegs[8]; }
uint64_t Cotson::X86::RSI() { return proxy->get_regs().IntegerRegs[9]; }
uint64_t Cotson::X86::RBP() { return proxy->get_regs().IntegerRegs[10]; }
uint64_t Cotson::X86::RSP() { return proxy->get_regs().IntegerRegs[11]; }
uint64_t Cotson::X86::RBX() { return proxy->get_regs().IntegerRegs[12]; }
uint64_t Cotson::X86::RDX() { return proxy->get_regs().IntegerRegs[13]; }
uint64_t Cotson::X86::RCX() { return proxy->get_regs().IntegerRegs[14]; }
uint64_t Cotson::X86::RAX() { return proxy->get_regs().IntegerRegs[15]; }

//##################################################################################
// FIXME: Hack to guess frequency from old SimNow registry if the 
// monitor interface does not provide the GetProcessorFreq API.
// To be removed when simnow 4.5.3 comes out
//##################################################################################
#ifdef NEED_GET_PROC_FREQ
typedef enum{SRV_IRegistry = 8} eServiceID;
class MIDTYPE{private: int t;int m;UINT64 v;public:MIDTYPE(int);};
class IService{public:IService();virtual ~IService();static IService* GetServiceByID(eServiceID,MIDTYPE);static void DeleteService(IService*);};
class IRegistry:public IService{public: IRegistry();virtual ~IRegistry();virtual bool FindKey(const char *,char *,int)=0;};

double Proxy::GetProcessorFreq(UINT64 m, UINT64 devid) 
{
	static double f = 1000.0;
	static bool found=false;
	if (!found)
	{
        char buf[256];
        IRegistry *s = dynamic_cast<IRegistry*>(IService::GetServiceByID(SRV_IRegistry,m));
        if(s && s->FindKey("Instructions per Microsecond",buf,256))
		{
            f = static_cast<double>(strtoull(buf,0,10));
		    found=true;
		}
        IService::DeleteService(s);
	}
    return f;
}
#endif
//##################################################################################
