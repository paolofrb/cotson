//	monitor.cpp - Sample monitor module
//

#include <typedefs.h>
#include "CMonitor.h"
#include "monitor.h"
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

//	SimNowGetMonitorInterface - Create and return a pointer to an IMonitorConsumer object
//
//	Note: The "pModuleName" is the fully qualified load path to the monitor module being loaded.
//

long s_drCounter[9];

extern "C" DLLEXPORT CMonitorConsumer *SimNowGetMonitorInterface(const char *pModuleName, const char *pArgs)
{
	printf("Monitor: Being loaded from %s. Args = <%s>\n", pModuleName, pArgs);
	return(new CMonitor(pModuleName, pArgs));
}

//	CMonitor - Basic constructor

CMonitor::CMonitor(const char *pModuleName, const char *pArgs)
{
	m_CommandIndex = 0;
}

//	~CMonitor - Basic destructor

CMonitor::~CMonitor()
{
}

//	InitializeMachine - The identified logical machine is being created.
//
//	In this sample, we enable the collection of events on this machine.
//

void CMonitor::InitializeMachine(int nMachineID)
{
	EnableEventCollection(nMachineID);
	for(int i = 0; i < 9; i++)
		s_drCounter[i] = 0;
}

//	RunControl - Simulation is starting/stopping
//
//	Note that we don't set any options for events in the "InitializeMachine" call, because the user may alter what devices
//		are loaded into that machine. We wait until "RunControl", so that we are sure that the contains of the logical machine
//		are stable.
//

void CMonitor::RunControl(int nMachineID, bool bStarting)
{
	if(bStarting)
	{
		//	Set the IPC for each processor in this machine to 1:1

		SetProcessorIPC(nMachineID, -1, 1, 1);
		
		// example implementation to get processor's frequency
		// comment out for less noise on the console output
		//UINT32 freq = GetProcessorFreq(nMachineID, 0);
		//printf("\nfrequency for processor 0 is %d", freq);		

		//	Set options for events that we wish to recieve during this run

		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PROCESSOR_CODEFETCH, EVENT_CODEFETCH_ENABLE | EVENT_CODEFETCH_TRANSLATION);
		 UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PROCESSOR_MEMORY, EVENT_MEMORY_ENABLE | EVENT_MEMORY_HOSTLIN | EVENT_MEMORY_OMITPHYS);
		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PROCESSOR_BRANCH, EVENT_BRANCH_ENABLE | EVENT_BRANCH_LWP);
		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PROCESSOR_EXCEPTION, EVENT_EXCEPTION_ENABLE);
		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PROCESSOR_HEARTBEAT, EVENT_HEARTBEAT_ENABLE | EVENT_HEARTBEAT_FLUSHONEVENT);

		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_BLOCKDEV_IDENTIFY, EVENT_BLOCKDEV_ENABLE);
		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_BLOCKDEV_READWRITE, EVENT_BLOCKDEV_ENABLE | EVENT_BLOCKDEV_EXTTIMING);

		UpdateEventOptions(nMachineID, (UINT64)-1, EVENT_PACKETDEV_READWRITE, EVENT_PACKETDEV_ENABLE | EVENT_PACKETDEV_EXTTIMING);

		//	As a test - generate a callback event after 1 mS(delay is in nS units)

		RequestCallbackDelayed(nMachineID, 1000 * 1000);
		QuantumEndConfig(nMachineID, 0, 20000); //example notification on every sync point if nError is larger then SyncQuantum.
	}
}

//	TerminateMachine - The identified logical machine is terminating.

void CMonitor::TerminateMachine(int nMachineID, EVENTQUEUEINFO *pQueueInfo)
{
	//	Note: The event queues for this machine are still valid. We can process any remaining entries before returning, as the
	//		queues will then be freed up by SimNow. In this example, we simply drain the queue of any remaining tokens. Note that
	//		the logical machine has already been unlinked by SimNow, so any calls that need to lookup the machine by its ID,
	//		(such as GetEventQueue() or RequestCallbackDelayed()) will fail at this point. Also note that some of the queues
	//		may already have been unregistered.

	while(pQueueInfo != (EVENTQUEUEINFO *)0)
	{
		DeliverEventQueue(DELIVERYREASON_DELIVERYSTART | DELIVERYREASON_TERMINATING, nMachineID, pQueueInfo);					// Be sure to run both phases of the "EventQueueFull"
		DeliverEventQueue(DELIVERYREASON_DELIVERYEND | DELIVERYREASON_TERMINATING, nMachineID, pQueueInfo);					// ... To insure that everything stays synced up
		pQueueInfo = pQueueInfo->pFptr;											// Flush all queues
	}
}

//	DeliverEventQueue - The given event queue needs to be emptied. In this sample, we simply drain all
//		items out of the event queue.
//
//	Note: The caller will empty the queue (and possibly reset the head and tail pointers) on return, so
//		get everything you need out of the queue before returning.
//
//	Note: We don't cache any pointers or access the queues in a separate thread, so we aren't interested in
//		any of the flush phases except eqfPhaseStart - the initial phase.
//

void CMonitor::DeliverEventQueue(UINT64 nReason, int nMachineID, EVENTQUEUEINFO *pQueueInfo)
{
	if((nReason & DELIVERYREASON_DELIVERYSTART) != 0)
	{
		UINT64 *pNext = pQueueInfo->pHead;
		UINT64 *pTail = pQueueInfo->pTail;

		if(pQueueInfo->eqtType == eqtCircular)				// Circular queue - bump the head, and wrap if at physical end
		{
			UINT64 *pLast = pQueueInfo->pLast;
			while(pNext != pTail)
			{
				HandleNextToken(*pNext, pQueueInfo);
				if(pNext == pLast)
					pNext = pQueueInfo->pFirst;
				else
					pNext++;
			}
		}
		else if(pQueueInfo->eqtType == eqtLinear)			// Linear queue - just bump up the head pointer
		{
			while(pNext != pTail)
			{
				HandleNextToken(*pNext, pQueueInfo);
				pNext++;
			}
		}
		pQueueInfo->pHead = pNext;											// Set a new head pointer
	}
}

//	MonitorCallback - Callback from a CPUID in simulated code

UINT64 CMonitor::MonitorCallback(int nMachineID, UINT64 nTag)
{
	return(nTag);
}

//	TimeCallback - Callback from an earlier RequestCallbackDelayed() request.

void CMonitor::TimeCallback(int nMachineID)
{
	RequestCallbackDelayed(nMachineID, (1000 * 1000));				//	Request another callback 1 msec further down the road
}

//	CodeInjection - Our opportunity to inject code into the AweSim code stream

void CMonitor::CodeInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector)
{
	UINT64 nSpecialTokens[2] = { 0x0000000280000000,  0};			// ID = 80000000, 2 tokens, options = 0, 2nd token is first 3 bytes of opcode

	INSTRUCTIONINFO iiInstruction;

	//	First - tag all instructions with a code fetch

	pInjector->AddCodeFetchEvent();

	//	Decode this instruction. If this is a MOV CR3,xxx, then inject a special event for it

	if(pInjector->BuildInstructionInfo(&iiInstruction, sizeof(iiInstruction)))
	{
		UINT8 nOpcodeBytes[16];

		if(iiInstruction.bPagingEnabled)	// If running w/paging turned off, we don't really care
		{
			int nLen = pInjector->OpcodeBytes(nOpcodeBytes, sizeof(nOpcodeBytes));
			if(nLen > (iiInstruction.nOpcodeOffset + 2)
				&& nOpcodeBytes[iiInstruction.nOpcodeOffset] == 0x0F
				&& nOpcodeBytes[iiInstruction.nOpcodeOffset + 1] == 0x22)
			{
				if(iiInstruction.bHasModrm && ((nOpcodeBytes[iiInstruction.nModrmOffset] & 0x38) == 0x18))	// /r = 3?
				{
					nSpecialTokens[1] = (UINT64) nOpcodeBytes[iiInstruction.nOpcodeOffset+2] 
						+ ((UINT64) nOpcodeBytes[iiInstruction.nOpcodeOffset+1] << 8) 
						+ ((UINT64) nOpcodeBytes[iiInstruction.nOpcodeOffset] << 16);
					pInjector->AddTokens(nSpecialTokens, sizeof(nSpecialTokens) / sizeof(UINT64));
				}
			}
		}
	}
}

//  MemoryInjection - Our opportunity to flag Awesim to add memory events to the queue

void CMonitor::MemoryInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector)
{
	pInjector->AddMemoryEvent();
}

//	BranchInjection - Our oppotunity to flag Awesim to add branch events to the queue

void CMonitor::BranchInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector)
{
	pInjector->AddBranchEvent();
}

//	ExcInjection - Our opportunity to flag Awesim to add exception events to the queue

void CMonitor::ExcInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector)
{
	pInjector->AddExcEvent();
}

//	HeartBeatInjection - Our oppotunity to flag Awesim to add hearbeat events to the queue

void CMonitor::HeartBeatInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector)
{
	pInjector->AddHeartBeatEvent();
}

//	CodeTranslation - Our opportunity to effect the way code is translated, or to tag specific instructions

void CMonitor::CodeTranslation(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, CODETRANSLATESTRUCT *pTranslateStruc)
{
	if((pTranslateStruc->pOpcodeBuffer[pTranslateStruc->nOpcodeOffset] == 0x0f) &&
		(pTranslateStruc->nInstructionLength > (pTranslateStruc->nOpcodeOffset + 1)) &&
		(pTranslateStruc->pOpcodeBuffer[pTranslateStruc->nOpcodeOffset + 1] == 0x22) &&
		(pTranslateStruc->nInstructionLength > (pTranslateStruc->nOpcodeOffset + 2)) &&
		((pTranslateStruc->pOpcodeBuffer[pTranslateStruc->nOpcodeOffset + 2] & 0x38) == 0x18))
			pTranslateStruc->nInstructionTag = 0xaa55ff00;
}

void CMonitor::TaggedExecution(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, void *pParam)
{
}

//	HandleNextToken - Handle the next token from the event queue

void CMonitor::HandleNextToken(UINT64 nToken, EVENTQUEUEINFO *pQueueInfo)
{
	MONITORMACHINESTATE *pMachineState = (MONITORMACHINESTATE *)pQueueInfo->pConsumerData;
	int nState = pMachineState->nTokenState;
	if(nState == -1)																		// First token?
	{
		pMachineState->nTokenIndex = 0;
		nState = (int)((nToken & TOKEN_ID_COUNT_MASK) >> TOKEN_ID_COUNT_SHIFT);
//		UINT16 nEventOptions = (UINT16)((nToken & TOKEN_ID_OPTIONS_MASK) >> TOKEN_ID_OPTIONS_SHIFT);

		switch((nToken & TOKEN_ID_EVENT_MASK) >> TOKEN_ID_EVENT_SHIFT)
		{
			case EVENT_PROCESSOR_CODEFETCH:
			case EVENT_PROCESSOR_MEMORY:
			case EVENT_PROCESSOR_EXCEPTION:
			case EVENT_PROCESSOR_REGVALUE:
			case EVENT_PROCESSOR_HEARTBEAT:
			case EVENT_PROCESSOR_BRANCH:
			case EVENT_BLOCKDEV_IDENTIFY:
			case EVENT_BLOCKDEV_READWRITE:
			case EVENT_PACKETDEV_READWRITE:
			case 0x80000000:																// This is our private "CR3 may have changed" token
				break;
			default:
				printf("An unrecognized token type was received.\n");
				break;
		}
	}
	pMachineState->nTokenList[pMachineState->nTokenIndex++] = nToken;

	//	nState now has the # of tokens we are expecting

	nState--;
	if(nState == 0)
	{
		nState = -1;																	// Back to first
		DumpTokenList(pMachineState->nTokenList, pMachineState->nTokenIndex, pQueueInfo->nDeviceID, pQueueInfo->nDeviceFlags);
	}
	pMachineState->nTokenState = nState;												// Reset to new value
}

void CMonitor::DumpTokenList(UINT64 *pTokens, int nTokenCount, UINT64 nDeviceID, UINT64 nDeviceFlags)
{
	if(nTokenCount <= 0)
		return;

	UINT64 nFirst = pTokens[0];
//	UINT32 nEventType = (UINT32)((nFirst & TOKEN_ID_EVENT_MASK) >> TOKEN_ID_EVENT_SHIFT);
	int nEventCount = (int)((nFirst & TOKEN_ID_COUNT_MASK) >> TOKEN_ID_COUNT_SHIFT);
//	UINT32 nEventOptions = (UINT32)((nFirst & TOKEN_ID_OPTIONS_MASK) >> TOKEN_ID_OPTIONS_SHIFT);
	if(nEventCount != nTokenCount)
		return;																			// Should never occur

	//	Now parse the remaining tokens for this event

//	switch(nEventType)
//	{
		//	TODO: Parse and deal with individual event packets here
//		default:
//			break;
//	}
}

void CMonitor::RegisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo)
{
	pQueueInfo->pConsumerData = (void *) new MONITORMACHINESTATE();					// Set the state to indicate we are waiting for the first token
	((MONITORMACHINESTATE *)pQueueInfo->pConsumerData)->nTokenState = -1;
}

void CMonitor::UnregisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo)
{
	//	Note: The event queue is still valid. We can process any remaining entries before returning, as the
	//		queues will then be freed up by SimNow. In this example, we simply drain the queue of any remaining tokens.

	if(pQueueInfo != (EVENTQUEUEINFO *)0)
	{
		DeliverEventQueue(DELIVERYREASON_REGISTRATION | DELIVERYREASON_DELIVERYSTART, nMachineID, pQueueInfo);					// Be sure to run both phases of the "EventQueueFull"
		DeliverEventQueue(DELIVERYREASON_REGISTRATION | DELIVERYREASON_DELIVERYEND, nMachineID, pQueueInfo);					// ... To insure that everything stays synced up
	}
	if(pQueueInfo->pConsumerData != (void *) NULL)
		delete (MONITORMACHINESTATE *) pQueueInfo->pConsumerData;
}

//	ReturnNextCommand - Return the next command to execute back to the shell
//
//	Return <false> if no more commands to execute (switches back to standard console command processing)
//
//	Note: The <psResponse> pointer points to the ASCII response from the previously executed command. For this first
//		command, this will be just a null string.
//
//	In this sample, we just have a quick list of commands we execute, and at the end of the list, we return control
//		to the normal console input.
//

const char *CMonitor::m_CommandArray[] = {
	//	TODO: Place command strings here such as:
	"shell.new",
	// "newmachine", 
	// "shell.open c:\\test1.bsd",
	(const char *) NULL,
};

bool CMonitor::ReturnNextCommand(int nMachineID, const char *psResponse, char *psCommand, int nBufferSize)
{
	if(m_CommandArray[m_CommandIndex] == (char *) NULL)
		return(false);
	else
	{
		strcpy(psCommand, m_CommandArray[m_CommandIndex]);
		m_CommandIndex++;
		return(true);
	}
}

//	ExternalEventTiming - Allows delays for device data transfers to be controlled
//

void CMonitor::ExternalEventTiming(EVENTTIMINGINFO *pInfo)
{
	EVENTTIMINGINFO_BLOCK *pBlock;
	EVENTTIMINGINFO_PACKET *pPacket;

	switch(pInfo->ettType)
	{
		case ettBlock:
			pBlock = (EVENTTIMINGINFO_BLOCK *)pInfo;
			break;
		case ettPacket:
			pPacket = (EVENTTIMINGINFO_PACKET *)pInfo;
			break;
		default:
			printf("Monitor: Unknown EVENTTIMINGINFO::ettType (%d) Specified\n", pInfo->ettType);
			break;
	}
}

//	QuantumEnd - The processors for this machine have all reached the end of a quantum

void CMonitor::QuantumEnd(int nMachineID)
{
}

//	FrequencyCallBack - Clock Frequency Set event call back

void CMonitor::FrequencyCallBack(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, UINT32 mhz)
{
	// Note: Avoid infinite recursion, do not call MonitorSetPState() 
	// in this call back function.
	// Comment out implementation for less noise, 
	// but leave the example implementation for the user
	//UINT64 PState = MonitorGetPState(nMachineID, (int)nDeviceID);
	//printf("\nPState for CPU %d is %d", (int)nDeviceID, (int)PState);
	//printf("\nDeviceID %d, frequency %dMHz", (int)nDeviceID, mhz);

}
