/* 
** Copyright 1995 - 2004 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

//	monitor.h - definition file for dynamically loaded monitor modules

#ifndef _CMONITOR_H_
#define _CMONITOR_H_
#include <CodeInjector.h>
#include <imonitor.h>

//	CCodeInjector - Wrapper class for the ICodeInjector

class CCodeInjector
{
public:
	CCodeInjector(ICodeInjector *pInjector) { m_pInjector = pInjector; };
	virtual ~CCodeInjector() { };

	//	Interface routines

	virtual void EmitArbitraryBytes(UINT8 *pBytes, int nLength)
		{ m_pInjector->EmitArbitraryBytes(pBytes, nLength); };
	virtual void Mem2Mem2(void *pSource1AndDest, void *pSource2, INJECTSIZE isSize, INJECTOP ioOperation)
		{ m_pInjector->Mem2Mem2(pSource1AndDest, pSource2, isSize, ioOperation); };
	virtual void Imm2Mem2(void *pSource1AndDest, UINT64 nSource1, INJECTSIZE isSize, INJECTOP ioOperation)
		{ m_pInjector->Imm2Mem2(pSource1AndDest, nSource1, isSize, ioOperation); };
	virtual void AddCodeFetchEvent()
		{ m_pInjector->AddCodeFetchEvent(); };
	virtual void AddMemoryEvent()
		{ m_pInjector->AddMemoryEvent(); };
	virtual void AddExcEvent()
		{ m_pInjector->AddExcEvent(); };
	virtual void AddHeartBeatEvent()
		{ m_pInjector->AddHeartBeatEvent(); };
	virtual void AddBranchEvent()
		{ m_pInjector->AddBranchEvent(); };
	virtual void AddTokens(UINT64* pTokens, int num)
		{ m_pInjector->AddTokens(pTokens, num); };
	virtual bool AddRegisterValueTokens(const char *psRegisterName)
		{ return(m_pInjector->AddRegisterValueTokens(psRegisterName)); };
	virtual int OpcodeBytes(UINT8 *pDestination, int nLength)
		{ return(m_pInjector->OpcodeBytes(pDestination, nLength)); };
	virtual bool BuildInstructionInfo(INSTRUCTIONINFO *pInfo, UINT64 nSizeOfStruct)
		{ return(m_pInjector->BuildInstructionInfo(pInfo, nSizeOfStruct)); };
	virtual void GetX8664Reg(X8664REG& x8664Reg)
		{ return(m_pInjector->GetX8664Reg(x8664Reg));};
	virtual void SetX8664Reg(X8664REG& x8664Reg)
		{ return(m_pInjector->SetX8664Reg(x8664Reg));};
	virtual void GetFX86Reg(FX86REG& Fx86Reg)
		{ return(m_pInjector->GetFX86Reg(Fx86Reg));};
	virtual void SetFX86Reg(FX86REG& Fx86Reg)
		{ return(m_pInjector->SetFX86Reg(Fx86Reg));};
	virtual ADDRTYPE GetPhysAddress(ADDRTYPE LinAddress) 
		{ return(m_pInjector->GetPhysAddress(LinAddress));};
	virtual char* Disassemble(UINT8 *pBuffer, int &Length)
		{ return(m_pInjector->Disassemble(pBuffer, Length));};
	virtual bool ReadPhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone)
		{ return(m_pInjector->ReadPhysicalMemory(PhysAddress, pBuffer, nCount, pBytesDone));};
	virtual bool WritePhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone)
		{ return(m_pInjector->WritePhysicalMemory(PhysAddress, pBuffer, nCount, pBytesDone));};
	virtual bool ReadLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone)
		{ return(m_pInjector->ReadLinearMemory(LinAddress, pBuffer, nCount, pBytesDone));};
	virtual bool WriteLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone)
		{ return(m_pInjector->WriteLinearMemory(LinAddress, pBuffer, nCount, pBytesDone));};
	virtual bool ReadIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount)
		{ return(m_pInjector->ReadIOPort(Port, pBuffer, nCount));};
	virtual bool WriteIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount)
		{ return(m_pInjector->WriteIOPort(Port, pBuffer, nCount));};


protected:
	ICodeInjector *m_pInjector;
};

//	CMonitorConsumer - Class that users can derive from to implement an monitor module

class CMonitorConsumer : public IMonitorConsumer
{

	//	The following routines are called by the framework to initiate and terminate simulation. If overridden, be sure that the
	//		these base class routines get called

public:
	virtual bool Initialize(IMonitorProvider *pProvider) { m_pProvider = pProvider; return(true); };
	virtual void Terminate() {};

	//	The following routines can be called by a descendant monitor module

public:

	//**	EnableEventCollection - Create a new event queue on the given machine
	//
	//	nMachineID		=	Logical ID of the machine to enable event collection on.
	//

	virtual bool EnableEventCollection(int nMachineID)
		{ return(m_pProvider->EnableEventCollection(nMachineID)); };

	//**	GetEventQueue - Returns a pointer to the event queue for the given machine ID
	//
	//	nMachineID		=	Logical ID of the machine whose queue we want
	//
	//	Returns a pointer to the first EVENTQUEUEINFO block if successful
	//			NULL if an error ocurred. The caller is responsible for walking the list
	//			to find the specific queue of interest.
	//

	virtual EVENTQUEUEINFO *GetEventQueue(int nMachineID)
		{ return(m_pProvider->GetEventQueue(nMachineID)); };

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

	virtual bool DeviceCharacteristics(int nMachineID, UINT64 nDeviceID, DEVICECHARACTERISTICS *pBuffer, int *pSize)
		{ return(m_pProvider->DeviceCharacteristics(nMachineID, nDeviceID, pBuffer, pSize)); };

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

	virtual bool UpdateEventOptions(int nMachineID, UINT64 nDeviceID, UINT32 nEventID, UINT16 nEventOptions)
		{ return(m_pProvider->UpdateEventOptions(nMachineID, nDeviceID, nEventID, nEventOptions)); };

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

	virtual bool RequestCallbackDelayed(int nMachineID, UINT64 nTimeDelay)
		{ return(m_pProvider->RequestCallbackDelayed(nMachineID, nTimeDelay)); };

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

	virtual bool QuantumEndConfig(int nMachineID, UINT64 nTime, UINT64 nError)
		{ return(m_pProvider->QuantumEndConfig(nMachineID, nTime, nError));};

	//**	CurrentSimulationTime - Returns the current simulation time for the indicated logical machine
	//
	//	nMachineID		=	Logical ID of the machine to get the time for
	//
	//	Returns ((UINT64) -1) if an error occurred, otherwise it returns the simulation time, in nanoseconds
	//

	virtual UINT64 CurrentSimulationTime(int nMachineID)
		{ return(m_pProvider->CurrentSimulationTime(nMachineID)); };

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

	virtual UINT64 ExactCurrentSimulationTime(int nMachineID)
		{ return(m_pProvider->ExactCurrentSimulationTime(nMachineID)); };

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

	virtual bool ProcessorStatistics(int nMachineID, int nCpuID, PROCESSORSTATISTICS *pInfo, int nCount)
		{ return(m_pProvider->ProcessorStatistics(nMachineID, nCpuID, pInfo, nCount)); };

	//**	InstructionCount - Return the # of instructions executed by the processor.
	//
	//	nMachineID		=	Logical ID of the machine to get the information for
	//	nCpuID			=	Logical ID (starting at 0) for which core to get information for
	//
	//	Note: This returns the same value that is available in the processor statistics structure
	//

	virtual UINT64 InstructionCount(int nMachineID, int nCpuID)
		{ return(m_pProvider->InstructionCount(nMachineID, nCpuID)); };

	//**	IdleCount - Return the # of cycles spent "idle" by the given processor
	//
	//	nMachineID		=	Logical ID of the machine to get the information for
	//	nCpuID			=	Logical ID (starting at 0) for which core to get information for
	//
	//	Note: This returns a value that is the aggregate of the various idle counts available
	//		in the processor statistics structure (i.e. I/O count, HLT count, etc.)
	//

	virtual UINT64 IdleCount(int nMachineID, int nCpuID)
		{ return(m_pProvider->IdleCount(nMachineID, nCpuID)); };

	//**	SetProcessorIPC - Set the IPC used by one or more processors
	//
	//	nMachindID		=	Logical ID of the machine whose CPUs are to be adjusted
	//	nCpuID			=	Logical ID (starting at 0) of the CPU to modify. Use -1 to change all cpus
	//	nInstr			=	# of instructions executed per # of cycles given
	//	nCycles			=	# of cycles to use to execute <nInstr> instructions
	//

	virtual bool SetProcessorIPC(int nMachineID, int nCpuID, UINT64 nInstr, UINT64 nCycles)
		{ return(m_pProvider->SetProcessorIPC(nMachineID, nCpuID, nInstr, nCycles)); };


	//**	MonitorGetPState - Get the P-State of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine 
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//
	//	return the value of P-State of the processor.
	//

	virtual UINT64 MonitorGetPState(int nMachineID, int nCpuID)
		{ return(m_pProvider->MonitorGetPState(nMachineID, nCpuID)); };	

	//**	MonitorSetPState - Set the P-State of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine whose CPUs are to be adjusted
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//	nPState			=	desired P-State
	//
	//	return the value of P-State of the processor.
	//

	virtual bool MonitorSetPState(int nMachineID, int nCpuID, int nPState)
		{ return(m_pProvider->MonitorSetPState(nMachineID, nCpuID, nPState)); };

	//**	GetProcessorFreq - Get the frequency of the particular processor
	//
	//	nMachindID		=	Logical ID of the machine
	//	nCpuID			=	Logical ID (starting at 0) of the CPU.
	//
	virtual UINT32 GetProcessorFreq(int nMachineID, int nCpuID)
		{ return(m_pProvider->GetProcessorFreq(nMachineID, nCpuID)); };


	//	The following routines should be overriden by the user

public:

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

	//**	CodeInjection - Allow for code injection into the translation cache
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector that can be used to inject code
	//

	virtual void CodeInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector) = 0;

	//**	MemoryInjection - Allow for memory events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector that can be used to log memory
	//

	virtual void MemoryInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector) = 0;

	//**	BranchInjection - Allow for branch events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector that can be used to log exceptions
	//

	virtual void BranchInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector) = 0;

	//**	ExcInjection - Allow for exception events logging to the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector that can be used to log exceptions
	//

	virtual void ExcInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector) = 0;

	//**	HeartBeatInjection - Allow for heart beat events being placed in the queue
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	pInjector	=	Pointer to ICodeInjector that can be used to log heart beat events
	//

	virtual void HeartBeatInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector) = 0;

	//**	CodeTranslation - Hook called every time an new instruction is being translated for later execution
	//
	//	nMachineID		=	Logical machine ID of the machine translating the code
	//	pInjector		=	Pointer to ICodeInjector that can be used as needed
	//	pTranslateStruc	=	Pointer to CODETRANSLATESTRUCT containing information about this instruction, and
	//							allowing the hook to tag the instruction for later tagged callout
	//

	virtual void CodeTranslation(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, CODETRANSLATESTRUCT *pTranslateStruc) = 0;

	//**	TaggedExecution - A previously tagged instruction is executing
	//
	//	nMachineID		=	Logical machine ID of the machine translating the code
	//	pInjector		=	Pointer to ICodeInjector that can be used as needed
	//	pParam			=	Tag attached to this instruction
	//

	virtual void TaggedExecution(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, void *pParam) = 0;

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

	//**	FrequencyCallBack - CPU clock frequency event call back
	//
	//	nMachineID	=	Logical machine ID of the machine translating the code
	//	nDeviceID	=	DeviceID of the device
	//	pInjector	=	Pointer to ICodeInjector
	//	mhz			=	Frequency in MHz

	virtual void FrequencyCallBack(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, UINT32 mhz) = 0;	

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

	//	Hooks to the CCodeInjector variants of these routines. These must not be called by descendant objects, but instead
	//		are called by the framework, and simply hook into the virtual functions listed above.

public:
	virtual void CodeInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); CodeInjection(nMachineID, nDeviceID, pInjector); delete pInjector; };
	virtual void MemoryInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); MemoryInjection(nMachineID, nDeviceID, pInjector); delete pInjector; };
	virtual void BranchInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); BranchInjection(nMachineID, nDeviceID, pInjector); delete pInjector; };
	virtual void ExcInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); ExcInjection(nMachineID, nDeviceID, pInjector); delete pInjector; };
	virtual void HeartBeatInjection(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); HeartBeatInjection(nMachineID, nDeviceID, pInjector); delete pInjector; };
	virtual void CodeTranslation(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector, CODETRANSLATESTRUCT *pTranslateStruc)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); CodeTranslation(nMachineID, nDeviceID, pInjector, pTranslateStruc); delete pInjector; };
	virtual void TaggedExecution(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector, void *pParam)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); TaggedExecution(nMachineID, nDeviceID, pInjector, pParam); delete pInjector; };
	virtual void FrequencyCallBack(int nMachineID, UINT64 nDeviceID, ICodeInjector *pRealInjector, UINT32 mhz)
		{ CCodeInjector *pInjector = new CCodeInjector(pRealInjector); FrequencyCallBack(nMachineID, nDeviceID, pInjector, mhz); delete pInjector; };

private:
	IMonitorProvider *m_pProvider;
};
#endif // _CMONITOR_H_

