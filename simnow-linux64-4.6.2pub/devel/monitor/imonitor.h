/* 
** Copyright 1995 - 2004 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

//	monitor.h - definition file for dynamically loaded monitor modules

#ifndef _MONITOR_H_
#define _MONITOR_H_

//	Forward class definitions

class ICodeInjector;
class IMonitorConsumer;

//	Define the initialization function that is used to verify and initialize an monitor module

#ifndef WINAPI
#define WINAPI
#endif
#ifndef FAR
#define FAR
#endif

typedef IMonitorConsumer *(FAR WINAPI GETMONITORINTERFACE)(const char *pModuleName, const char *pArgs);
typedef GETMONITORINTERFACE *PGETMONITORINTERFACE;

//	Define the layout of the first token of every event

#define TOKEN_ID_EVENT_MASK				0x00000000FFFFFFFF					// Mask to isolate the event type from the first token
#define TOKEN_ID_EVENT_SHIFT			0
#define TOKEN_ID_COUNT_MASK				0x0000FFFF00000000					// Mask to isolate the # of tokens in this event
#define TOKEN_ID_COUNT_SHIFT			32
#define TOKEN_ID_OPTIONS_MASK			0xFFFF000000000000					// 16 bits worth of options
#define TOKEN_ID_OPTIONS_SHIFT			48

//***	EVENTQUEUEINFO - Info block that defines a queue. Public. Passed to event consumers

enum EVENTQUEUETYPE { eqtLinear, eqtCircular };

struct EVENTQUEUEINFO
{
	EVENTQUEUEINFO *pFptr;						// Forward pointer to next in chain
	UINT64 *pFirst;								// Pointer to physical first entry in queue
	UINT64 *pLast;								// Pointer to physical last entry in queue
	UINT64 *pHead;								// Pointer to logical next entry in queue to read
	UINT64 *pTail;								// Pointer to logical next entry in queue to write
	UINT64 nDeviceID;							// Device ID for the device that owns this queue
	EVENTQUEUETYPE eqtType;						// Type of queue (Linear, circular, etc.)
	UINT64 nDeviceFlags;						// Flags for this device/queue combination. See below.
	void *pConsumerData;						// IMonitorConsumer usable
};

//	Bit definitions for the nDeviceFlags member

#define EVENTQUEUEFLAGS_PROCESSOR		0x0000000000000001				// Processor events will be in this queue
#define EVENTQUEUEFLAGS_BLOCKDEV		0x0000000000000002				// BLOCKDEV events will be in this queue
#define EVENTQUEUEFLAGS_PACKETDEV		0x0000000000000004				// PACKETDEV events will be in this queue

//	Define bit flags for the eventqueue delivery reason. One or more of these should always be set

#define DELIVERYREASON_QUEUEFULL		0x0000000000000001				// Queue has filled
#define DELIVERYREASON_TERMINATING		0x0000000000000004				// This simulation machine is terminating
#define DELIVERYREASON_REGISTRATION		0x0000000000000008				// Queue is being unregistered, get events while you can ...
#define DELIVERYREASON_HEARTBEAT		0x0000000000000010				// Processor heartbeat event, delivery requested
#define DELIVERYREASON_SIMSTOPPING		0x0000000000000020				// Simulation is stopping (non-processor devices only).
#define DELIVERYREASON_PROCYIELD		0x0000000000000040				// Processor is yielding for some other reason
#define DELIVERYREASON_PROCEVENT		0x0000000000000080				// Processor is yielding because of a pending event
#define DELIVERYREASON_PROCHALT			0x0000000000000100				// Processor is yielding because it entered a HLT state
#define DELIVERYREASON_DELIVERYSTART	0x8000000000000000				// First notification phase
#define DELIVERYREASON_DELIVERYEND		0x4000000000000000				// Last notification phase

//	Define the various events that SimNow defines, and the options for each

#define EVENT_PROCESSOR_CODEFETCH 0x00000001	// Code fetch events from processors
#define EVENT_PROCESSOR_MEMORY    0x00000002    // Memory access events from processors
#define EVENT_PROCESSOR_EXCEPTION 0x00000003    // Exception events from processors
#define EVENT_PROCESSOR_REGVALUE  0x00000004	// Value of register (requested using ICodeInjector::AddRegisterValueTokens()
#define EVENT_PROCESSOR_HEARTBEAT 0x00000005	// HeartBeat (i.e. fast status) events

#define EVENT_BLOCKDEV_IDENTIFY	  0x00000006	// Identification of media in a block device
#define EVENT_BLOCKDEV_READWRITE  0x00000007	// A block device is performing a read

#define EVENT_PACKETDEV_READWRITE 0x00000008	// A packet (i.e. network) device transaction

#define EVENT_PROCESSOR_BRANCH	  0x00000009    // branch instruction events from processors

//	Block device options. Used for all EVENT_BLOCKDEV_XXXXX events
#define EVENT_BLOCKDEV_ENABLE	  0x0001		// 1 = Enable, 0 = Disable
#define EVENT_BLOCKDEV_EXTTIMING  0x0002		// 1 = Make callouts to time transfers, 0 = use default timing

//	Packet device options.
#define EVENT_PACKETDEV_ENABLE	  0x0001		// 1 = Enable, 0 = Disable
#define EVENT_PACKETDEV_EXTTIMING 0x0002		// 1 = Make callouts to time transfers, 0 = use default timing

// Heart beat options token
#define EVENT_HEARTBEAT_ENABLE	0x0001			// 1 = Enable, 0 = Disable
#define EVENT_HEARTBEAT_FLUSHONEVENT 0x0002		// 1 = Flush queue whenever event occurs

// code fetch options token
#define EVENT_CODEFETCH_ENABLE	0x0001			// 1 = Enable, 0 = Disable
#define EVENT_CODEFETCH_TRANSLATION 0x0002		// 1 = Enable code translation hook, 0 = disable
// memory options token
#define EVENT_MEMORY_ENABLE 0x0001				//  1 = Enable, 0 = Disable
#define EVENT_MEMORY_OMITPHYS 0x0002			//  1 = omit simulated physical address token
#define EVENT_MEMORY_HOSTLIN 0x0004				//	1 = Include host linear address token (or -1 if not available)

// memory access type token
#define EVENT_MEMORY_ISREAD 0x0001
#define EVENT_MEMORY_ISWRITE 0x0002
#define EVENT_MEMORY_HASLIN 0x0004
#define EVENT_MEMORY_STACKV  0x0008 
#define EVENT_MEMORY_STACKVP  0x0010 
#define EVENT_MEMORY_TSS  0x0020 
#define EVENT_MEMORY_DESCRIPTOR  0x0040 
#define EVENT_MEMORY_SVM  0x0080 
#define EVENT_MEMORY_SMM  0x0100 
#define EVENT_MEMORY_TLB  0x0200 
#define EVENT_MEMORY_MSR  0x0400 
#define EVENT_MEMORY_CODE  0x0800 
#define EVENT_MEMORY_INTERNAL  0x80000000 

// branch options token
#define EVENT_BRANCH_ENABLE 0x0001			//  1 = Enable, 0 = Disable
#define EVENT_BRANCH_LWP	0x0002			//  1 = queue events specific for lwp (light weight profiling).

// exception options token
#define EVENT_EXCEPTION_ENABLE 0x0001			// 1 = Enabled, 0 = Disable

// exception type token
#define EVENT_EXCEPTION_VECTOR (usign64(0x00000000FFFFFFFF))
#define EVENT_EXCEPTION_EXCEPTION (usign64(1) << 32)
#define EVENT_EXCEPTION_SWI (usign64(1) << 33)
#define EVENT_EXCEPTION_HWI (usign64(1) << 34)

//	Define the structure used by EVENT_CODEFETCH_TRANSLATION

struct CODETRANSLATESTRUCT {
	UINT8 *pOpcodeBuffer;								// Pointer to buffer containing opcode bytes
	int nOpcodeCount;									// # of valid bytes in opcode buffer
	int nOpcodeOffset;									// offset of first opcode byte (i.e., number of prefixes)
	bool bTranslateNormally;							// Tag, but use AweSim generated code
	UINT32 nInstructionTag;								// !0 if we should tag this instruction
	int nInstructionLength;								// If tag is set and bTranslateNormally is <false>, this is the number of bytes in this instruction
};

//	Define the structure that is returned by the DeviceCharacteristics() function

enum CHARACTERISTICSTYPE { Processor };

struct DEVICECHARACTERISTICS
{
	CHARACTERISTICSTYPE eType;
};

struct PROCESSORDEVICECHARACTERISTICS : DEVICECHARACTERISTICS
{
	UINT64 nPhysicalAddressBits;
	UINT64 nVirtualAddressBits;
};

//	Define the PROCESSORSTATISTICS structure - returns information about a given processor in the simulation

enum PROCESSORIDENTIFIER { PIDUnknown, PIDAweSim, PIDSimCPU };
struct PROCESSORSTATISTICS
{
	int	nSize;									// Size of this structure, in bytes
	PROCESSORIDENTIFIER piID;					// Which processor type we are dealing with
};

//	This is the structure used by SimCPU

struct PROCESSORSTATISTICS_SIMCPU : PROCESSORSTATISTICS
{
};

//	This is the structure used by AWESIM

struct PROCESSORSTATISTICS_AWESIM : PROCESSORSTATISTICS
{
	int nTraceCacheSize;
	int nValidTranslationBytes;
	int nInvalidTranslationBytes;
	int nMetaDataBytes;
	double fHostTime;
	UINT64 nInstructions;
	UINT64 nPlainInvalidations;
	UINT64 nRangeInvalidations;
	UINT64 nOtherExceptions;
	UINT64 nSegvExceptions;
	UINT64 nReadPios;
	UINT64 nWritePios;
	UINT64 nReadMmios;
	UINT64 nWriteMmios;
	UINT64 nIOCount;
	UINT64 nHaltCount;
};

//	EVENTTIMINGINFO - This is the basic structure used to pass information for the ExternalEventTiming function
//
//	Note: Specific devices will descend from this and add other information as needed

enum ACCESSTIMINGTYPE { attDMARead, attDMAWrite, attPIORead, attPIOWrite, attEventEnd };

enum EVENTTIMINGTYPE { ettBlock, ettPacket };

struct EVENTTIMINGINFO
{
	EVENTTIMINGTYPE ettType;
	ACCESSTIMINGTYPE attType;
	int nMachineID;
	UINT64 nDeviceID;

	UINT32 nDelay;
};

//	EVENTTIMINGINFO for block device types (i.e. EVENTTIMINGINFO::ettType == ettBlock

struct EVENTTIMINGINFO_BLOCK : EVENTTIMINGINFO
{
	UINT64 nBlockNumber;
	UINT64 nBlockCount;
};

//	EVENTTIMINGINFO for packet device types (i.e. EVENTTIMINGINFO::ettType == ettPacket

struct EVENTTIMINGINFO_PACKET : EVENTTIMINGINFO
{
	void *pPacketData;
	UINT32 nPacketLength;
};

class IMonitorControl;
class IMonitorProvider;
class IMonitorProducer;
class IMonitorConsumer;

//***	IMonitorControl - Communicates with IMonitorProducers to control the generation and collection of events

class IMonitorControl
{
public:
	virtual ~IMonitorControl() {};											// Virtual destructor

public:
	virtual bool RegisterEventQueue(int nMachineID, IMonitorProducer *pProducer, EVENTQUEUEINFO *pEventQueue) = 0;
	virtual bool UnregisterEventQueue(int nMachineID, IMonitorProducer *pProducer, EVENTQUEUEINFO *pEventQueue) = 0;
	virtual bool DeliverEventQueue(UINT64 nReason, int nMachineID, IMonitorProducer *pProducer, EVENTQUEUEINFO *pEventQueue) = 0;
	virtual void CodeInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;
	virtual void MemoryInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;
	virtual void ExcInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;
	virtual void HeartBeatInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;
	virtual void BranchInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;
	virtual void CodeTranslation(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, CODETRANSLATESTRUCT *pTranslateStruc) = 0;
	virtual void TaggedExecution(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, void *pParam) = 0;
	virtual void TimeCallback(int nMachineID) = 0;
	virtual void FrequencyCallBack(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, UINT32 mhz) = 0;
	virtual UINT64 MonitorCallback(int nMachineID, UINT64 nTag) = 0;
	virtual void ExternalEventTiming(EVENTTIMINGINFO *pInfo) = 0;
	virtual void QuantumEnd(int nMachineID) = 0;
};

//***	IMonitorProducer - Devices that can produce monitor events inherit from this interface

class IMonitorProducer
{
public:
	virtual ~IMonitorProducer() {};											// Virtual destructor

public:
	virtual void Initialize(IMonitorControl *pControl) = 0;
	virtual void UpdateEventOptions(UINT64 nDeviceID, UINT32 nEventID, UINT16 nEventOptions) = 0;
	virtual bool DeviceCharacteristics(UINT64 nDeviceID, DEVICECHARACTERISTICS *pBuffer, int *pSize) = 0;
};

//***	IMonitorProvider - Provides the SimNow end of the connection to an external monitor module.

class IMonitorProvider
{
public:
	virtual ~IMonitorProvider() {};											// Virtual destructor

public:

	//**	EnableEventCollection - Create a new event queue on the given machine
	//
	//	nMachineID		=	Logical ID of the machine to enable event collection on.
	//

	virtual bool EnableEventCollection(int nMachineID) = 0;

	//**	GetEventQueue - Returns a pointer to the event queue for the given machine ID
	//
	//	nMachineID		=	Logical ID of the machine whose queue we want
	//
	//	Returns a pointer to the first EVENTQUEUEINFO block if successful
	//			NULL if an error ocurred. The caller is responsible for walking the list
	//			to find the specific queue of interest.
	//

	virtual EVENTQUEUEINFO *GetEventQueue(int nMachineID) = 0;

	//**	DeviceCharacteristics - Fills in a structure containing characteristics for this device
	//
	//	nMachineID		=	Logical ID of the machine to query
	//	nDeviceID		=	Logical device ID of the device to query
	//	pBuffer			=	Pointer to a DEVICECHARACTERISTICS derived buffer to contain the result
	//	pSize			=	Pointer to an int size of the buffer. On return, has the size of the returned buffer
	//
	//	Returns <true> if OK, <false> if an error occurs.
	//
	//	You can initialize *pSize to 0, and on return, (even if the return value is <false>) if *pSize is > 0,
	//		then you know that a particular device has responded, and *pSize is the size of the structure buffer
	//		needed to contain the resulting structure.
	//

	virtual bool DeviceCharacteristics(int nMachineID, UINT64 nDeviceID, DEVICECHARACTERISTICS *pBuffer, int *pSize) = 0;

	//**	UpdateEventOptions - Update the event collection options for the given event to the given bits
	//
	//	nMachineID		=	Logical ID of the machine to create the event queue for
	//	nEventID		=	32 bit ID of the event being modified
	//	nEventOptions	=	Bitmask of events that should be placed in this queue
	//
	//	Returns <true> if ok, <false> if an error occurs.
	//
	//	Note: Setting an "options" value of all zeros is defined as turning the event off - Any other setting
	//		controls the various options available for this event (event specific)
	//

	virtual bool UpdateEventOptions(int nMachineID, UINT64 nDeviceID, UINT32 nEventID, UINT16 nEventOptions) = 0;

	//**	RequestCallbackDelayed - Request a callback when a certain amount of time has elapsed
	//
	//	nMachineID		=	Logical ID of the machine to create the event on
	//	nTimeDelay		=	Amount of time (in nanoseconds) in the future to schedule the delay
	//
	//	Returns <true> if ok, <false> if an error occurs
	//
	//	NOTE: The time delay is expressed in nanoseconds of simulated time. A value of ((UINT64)-1) cancels
	//		any previous callback.
	//

	virtual bool RequestCallbackDelayed(int nMachineID, UINT64 nTimeDelay) = 0;

	//**	QuantumEndConfig - Set the parameters for QuantumEnd callback interval
	//
	//	nMachineID		=	Logical ID of the machine to create the event on
	//  nTime 			= Minimum time (in nanoseconds) for notification inverval 
	//  nError			= Allowable error (in nanoseconds) such that nTime + nError is the maximum interval
	//
	//	Returns <true> if ok, <false> if an error occurs
	//
	// Note:
	// Setting nTime = 0 will result in a notification on every sync point.
	// Setting nError = 0 will result in a notification every nTime ns.
	// Setting nError < SyncQuantum may introduce new sync points.
	// Setting nError >= SyncQuantum will not introduce new sync points.
	// Setting nTime = 0, nError = 0 will disable all notifications.

	virtual bool QuantumEndConfig(int nMachineID, UINT64 nTime, UINT64 nError) = 0;

	//**	CurrentSimulationTime - Returns the current simulation time for the indicated logical machine
	//
	//	nMachineID		=	Logical ID of the machine to get the time for
	//
	//	Returns ((UINT64) -1) if an error occurred, otherwise it returns the simulation time, in nanoseconds
	//

	virtual UINT64 CurrentSimulationTime(int nMachineID) = 0;

	//**	ExactCurrentSimulationTime - Returns the current simulation time for the indicated logical machine
	//
	//	nMachineID		=	Logical ID of the machine to get the time for
	//
	//	Returns ((UINT64) -1) if an error occurred, otherwise it returns the simulation time, in nanoseconds
	//
	//	NOTE: Only use this function if you are sure you are executing out of the main execution thread for the
	//		indicated nMachineID. If there is any chance that you are not, you must use CurrentSimulationTime()
	//		instead.
	//

	virtual UINT64 ExactCurrentSimulationTime(int nMachineID) = 0;

	//**	ProcessorStatistics - Returns information about the given processor simulation
	//
	//	nMachineID		=	Logical ID of the machine to get the information for
	//	nCpuID			=	Logical ID (starting at 0) for which core to get information for
	//	pInfo			=	Pointer to the PROCESSORSTATISTICS structure to fill in
	//	nCount			=	Size of the PROCESSORSTATISTICS structure, in bytes
	//
	//	Returns <true> if ok, <false> if the machine or cpu does not exist, or the structure given is too short
	//		to contain the data
	//

	virtual bool ProcessorStatistics(int nMachineID, int nCpuID, PROCESSORSTATISTICS *pInfo, int nCount) = 0;

	//**	InstructionCount - Return the # of instructions executed by the processor.
	//
	//	nMachineID		=	Logical ID of the machine to get the information for
	//	nCpuID			=	Logical ID (starting at 0) for which core to get information for
	//
	//	Note: This returns the same value that is available in the processor statistics structure
	//

	virtual UINT64 InstructionCount(int nMachineID, int nCpuID) = 0;

	//**	IdleCount - Return the # of cycles spent "idle" by the given processor
	//
	//	nMachineID		=	Logical ID of the machine to get the information for
	//	nCpuID			=	Logical ID (starting at 0) for which core to get information for
	//
	//	Note: This returns a value that is the aggregate of the various idle counts available
	//		in the processor statistics structure (i.e. I/O count, HLT count, etc.)
	//

	virtual UINT64 IdleCount(int nMachineID, int nCpuID) = 0;

	//**	SetProcessorIPC - Set the IPC used by one or more processors
	//
	//	nMachindID		=	Logical ID of the machine whose CPUs are to be adjusted
	//	nCpuID			=	Logical ID (starting at 0) of the CPU to modify. Use -1 to change all cpus
	//	nInstr			=	# of instructions executed per # of cycles given
	//	nCycles			=	# of cycles to use to execute <nInstr> instructions
	//

	virtual bool SetProcessorIPC(int nMachineID, int nCpuID, UINT64 nInstr, UINT64 nCycles) = 0;

	//**	MonitorGetPState - Get the P-State of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine 
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//
	//	return the value of P-State of the processor.
	//

	virtual UINT64 MonitorGetPState(int nMachineID, int nCpuID) = 0;

	//**	MonitorSetPState - Set the P-State of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine whose CPUs are to be adjusted
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//	nPState			=	desired P-State
	//
	//	return the value of P-State of the processor.
	//
	virtual bool MonitorSetPState(int nMachineID, int nCpuID, int nPState) = 0;

	//**	GetProcessorFreq - Get the frequency of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//
	virtual UINT32 GetProcessorFreq(int nMachineID, int nCpuID) = 0;

};

//***	IMonitorConsumer - Defines the "consumer" end, or external monitor module, that SimNow IMonitorProvider
//			communicates with
//


class IMonitorConsumer
{
public:
	virtual ~IMonitorConsumer() {};											// Virtual destructor

public:

	//**	Initialize - Module loaded.
	//
	//	pProvider	=	Pointer to IMonitorProvider
	//
	//	return <true> if ok, <false> if to unload
	//

	virtual bool Initialize(IMonitorProvider *pProvider) = 0;

	//**	Terminate - Module unloaded
	//
	//	Only called if Initialize() returned <true>
	//

	virtual void Terminate() = 0;

	//**	InitalizeMachine - A new SimNow machine is being created
	//
	//	nMachineID	=	Logical ID of the machine being created
	//

	virtual void InitializeMachine(int nMachineID) = 0;

	//**	TerminateMachine - An existing SimNow machine is being terminated
	//
	//	nMachineID	=	Logical ID of the machine being terminated
	//	pQueueInfo	=	Pointer to the first event queue info block for this machine (or NULL if no queue)
	//
	//	NOTE: Event queues on this machine are still valid when this routine is called. They
	//		are released automatically after this function returns.
	//
	//	NOTE: The logical machine has been removed from the machine queue, so calls that require a
	//		machine ID (such as GetEventQueue) will fail.
	//

	virtual void TerminateMachine(int nMachineID, EVENTQUEUEINFO *pQueueInfo) = 0;

	//**	RegisterEventQueue - An event queue is being registerd by a device in the given logical machine
	//
	//	nMachineID	=	Logical ID of the machine owning the device that is registering the queue
	//	pQueueInfo	=	Pointer to EVENTQUEUEINFO that is being registered
	//
	//
	//	Note: The EVENTQUEUEINFO structure has already been linked into the chain of event queues for this logical machine
	//

	virtual void RegisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo) = 0;

	//**	UnregisterEventQueue - An event queue is being unregisterd by a device in the given logical machine
	//
	//	nMachineID	=	Logical ID of the machine owning the device that is unregistering the queue
	//	pQueueInfo	=	Pointer to EVENTQUEUEINFO that is being registered
	//
	//	NOTE: The EVENTQUEUEINFO structure has not yet been unlinked from the chain of event queues for this logical machine
	//

	virtual void UnregisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo) = 0;

	//**	DeliverEventQueue - A specific event queue needs to be emptied.
	//
	//	nReason		=	Various DELIVERYREASON_xxx bit flags
	//	nMachineID	=	Logical machine ID of the event queue that is full
	//	nQueueID	=	Logical Queue ID of the event queue that is full
	//	pQueueInfo	=	Pointer to the event queue info block for this queue
	//
	//	Note: The caller will empty the queue (and possibly reset the head and tail pointers) on return, so
	//		get everything you need out of the queue before returning.
	//

	virtual void DeliverEventQueue(UINT64 nReason, int nMachineID, EVENTQUEUEINFO *pQueueInfo) = 0;

	//**	TimeCallback - Called after the time elapsed from an IMonitorProvider::RequestCallbackAtTime()
	//
	//	nMachineID	=	Logical machine ID of the machine whose callback time has expired
	//

	virtual void TimeCallback(int nMachineID) = 0;

	//**	MonitorCallback - Called when simulated code executes the CPUID backdoor instruction
	//
	//		The CPUID is RAX = 0xBACCD00D, RDI = 0xCA110000
	//				RBX is user defined, passed to monitor module.
	//				RBX is set to return value after monitor module process
	//

	virtual UINT64 MonitorCallback(int nMachineID, UINT64 nTag) = 0;

	//**	FrequencyCallBack - CPU clock frequency event call back
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector 
	//	mhz     	=	Frequency in MHz

	virtual void FrequencyCallBack(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, UINT32 mhz) = 0;

	//**	CodeInjection - Allow for code injection into the translation cache
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	pInjector	=	Pointer to ICodeInjector that can be used to inject code
	//
	//	TODO: We need to provide a CPU ID or DEVICE ID or something here!
	//

	virtual void CodeInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;

	//**	MemoryInjection - Allow for memory events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	pInjector	=	Pointer to ICodeInjector that can be used to log memory
	//
	//	TODO: We need to provide a CPU ID or DEVICE ID or something here!
	//

	virtual void MemoryInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;

	//**	BranchInjection - Allow for branch events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	pInjector	=	Pointer to ICodeInjector that can be used to log memory
	//
	//	TODO: We need to provide a CPU ID or DEVICE ID or something here!
	//

	virtual void BranchInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;

	//**	ExcInjection - Allow for exception events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	pInjector	=	Pointer to ICodeInjector that can be used to log exceptions
	//
	//	TODO: We need to provide a CPU ID or DEVICE ID or something here!
	//

	virtual void ExcInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;

	//**	HeartBeatInjection - Allow for heart beat events being placed in the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	// 	nDeviceID	=	Logical device ID of the device to query
	//	pInjector	=	Pointer to ICodeInjector that can be used to log heart beat events
	//

	virtual void HeartBeatInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector) = 0;

	//**	CodeTranslation - Hook called every time an new instruction is being translated for later execution
	//
	//	nMachineID		=	Logical machine ID of the machine translating the code
	// 	nDeviceID		=	Logical device ID of the device to query
	//	pInjector		=	Pointer to ICodeInjector that can be used as needed
	//	pTranslateStruc	=	Pointer to CODETRANSLATESTRUCT containing information about this instruction, and
	//							allowing the hook to tag the instruction for later tagged callout
	//

	virtual void CodeTranslation(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, CODETRANSLATESTRUCT *pTranslateStruc) = 0;

	//**	TaggedExecution - A previously tagged instruction is executing
	//
	//	nMachineID		=	Logical machine ID of the machine translating the code
	// 	nDeviceID		=	Logical device ID of the device to query
	//	pInjector		=	Pointer to ICodeInjector that can be used as needed
	//	pParam			=	Tag attached to this instruction
	//

	virtual void TaggedExecution(int nMachineID, UINT64 nDeviceID, ICodeInjector *pInjector, void *pParam) = 0;

	//**	RunControl - A simulated machine is starting/stopping
	//
	//	nMachineID	=	Logical machine ID of the machine that is being started/stopped
	//	bStarting	=	<true> if starting, <false> if stopping
	//
	//	NOTE: Options for any desired events need to be requested w/UpdateEventOptions() during a RunControl
	//		with bStarting = <true>. This is because the user may have removed or added a new device
	//		and that device will not have knowledge of any options that were sent previously.
	//

	virtual void RunControl(int nMachineID, bool bStarting) = 0;

	//**	ReturnNextCommand - Return the next ASCII command for the simulation shell
	//
	//	If requested by the --AInp argument on the command line, SimNow will call this routine instead of its
	//		normal console read routine to get input commands.
	//
	//	nMachineID	=	The machine ID to which this command will apply
	//	psResponse	=	Pointer to response from previous command executed (initially a null string)
	//	psCommand	=	Pointer to buffer to place ASCII command to execute
	//	nBufferSize	=	# of bytes in the input buffer that can be used
	//
	//	Returns <true> to execute the command, and then repeat this call, or false to not execute the command, and
	//		return to normal console input functionality.
	//

	virtual bool ReturnNextCommand(int nMachineID, const char *psResponse, char *psCommand, int nBufferSize) = 0;

	//**	ExternalEventTiming - Allows a delay to be inserted before a device transfers data or responds to certain commands
	//
	//	This must be enabled by setting the EVENT_BLOCKDEV_EXTTIMING or EVENT_PACKETDEV_EXTTIMING option bits on the Read/Write
	//		events for the specific device you wish to control. Once enabled, the device will call this routine.
	//
	//	nMachineID		=	Logical machine ID of the machine the device belongs to
	//	nDeviceID		=	The device ID (same as in the queue header for the device, if it has a queue)
	//	eAccessType		=	The type of transfer/access the device is performing
	//	pEventQueue		=	Pointer to the event queue for this device, or NULL if no event queue is being used. This allows the
	//							monitor module to see what the read/write event (which is always the last read/write in the queue)
	//							is in order to help determine the appropriate delay (i.e. block # and # of blocks)
	//	*pDelay			=	Pointer to a UINT32. The initial value is the value the device would normally use. You can
	//							update this to be any "reasonable" non-zero value.
	//

	virtual void ExternalEventTiming(EVENTTIMINGINFO *pInfo) = 0;

	//**	QuantumEnd - The simulation has reached the end of a quantum
	//
	//	nMachineID		=	Logical machine ID of the machine whose quantum has ended
	//

	virtual void QuantumEnd(int nMachineID) = 0;
};
#endif // _MONITOR_H_
