/* 
** Copyright 1995 - 2005 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

// Sample.cpp : Defines the entry point for the DLL application.
//

#include <typedefs.h>
#include <vector>
#include <assert.h>
#include "commoncpu.h"
#include "Sample.h"
#include "Analyzers.h"
#include <string.h>
#include <stdio.h>

#if defined(__sun)
#include <stdio.h>
#include <strings.h>
#endif

#ifdef WIN32
//	Windows standard DLL entry point

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

//	AnalyzerLoaded - This analyzer has been loaded into memory

DLLEXPORT bool AnalyzerLoaded(CAweSim *pAweSim, const char *pszArguments, void **ppVoid, CCaller* pCaller, CArchive* pAr)
{
	if(HasEntry(pCaller))
		return(false);	// Called multiple times?
	// create CpuLogger
	CpuLogger* pNewCpuLogger = new CpuLogger(pAweSim, pszArguments, ppVoid, pAr);
	AddEntry(pNewCpuLogger, pCaller);
	return(true);
}

DLLEXPORT void SaveAnalyzer(CArchive* pAr, CCaller* pCaller)
{
	GetEntry(pCaller)->pCpuLogger->SaveAnalyzer(pAr);
}

//	AnalyzerUnloaded - This analyzer is being unloaded from memory

DLLEXPORT void AnalyzerUnloaded(void *pVoid, CCaller* pCaller)
{
	delete(GetEntry(pCaller)->pCpuLogger);
	RemoveEntry(pCaller);
}

//	GetAnalyzerVersion - Get the version number of the interface this Analyzer is using

DLLEXPORT UINT32 GetAnalyzerVersion(CCaller* pCaller)
{
	return(ANALYZER_CURRENT_VERSION);
}

//	GetAnalyzerFlags - Return various flag bits. See the ANALYZER_xxxx bits for definitions

DLLEXPORT UINT32 GetAnalyzerFlags(CCaller* pCaller)
{
	return(GetEntry(pCaller)->pCpuLogger->GetAnalyzerFlags());
}

//	GetAnalyzerDescription - Returns a short description of this analyzer

DLLEXPORT bool GetAnalyzerDescription(char* pszDest, int nDestSize, CCaller* pCaller)
{
	return(GetEntry(pCaller)->pCpuLogger->GetAnalyzerDescription(pszDest, nDestSize));
}

//	AnalyzerNotify - Called when one of the events requested by GetAnalyzerFlags() happens

DLLEXPORT void AnalyzerNotify(void* pVoid, UINT32 nNotifyCode, void* pParam1, void* pParam2, CCaller* pCaller)
{
	GetEntry(pCaller)->pCpuLogger->AnalyzerNotify(pVoid, nNotifyCode, pParam1, pParam2);
}

bool HasEntry(CCaller* pCaller)
{
	std::vector<Entry>::iterator it = v.begin();
	while(it != v.end())
	{
		if((*it).pCaller->GetProcessor() == pCaller->GetProcessor())
			return(true);
		it++;
	}
	return(false);
}

Entry* GetEntry(CCaller* pCaller)
{
	std::vector<Entry>::iterator it = v.begin();
	while(it != v.end())
	{
		if((*it).pCaller == pCaller)
			return(&(*it));
		it++;
	}
	return(NULL);
}

void RemoveEntry(CCaller* pCaller)
{
	std::vector<Entry>::iterator it = v.begin();
	while(it != v.end())
	{
		if((*it).pCaller == pCaller)
		{
			v.erase(it);
			break;
		}
		it++;
	}
}

void AddEntry(CpuLogger* pCpuLogger, CCaller* pCaller)
{
	Entry NewEntry;
	NewEntry.pCpuLogger = pCpuLogger;
	NewEntry.pCaller = pCaller;
	v.insert(v.begin(), NewEntry);
}

CpuLogger::CpuLogger(CAweSim* pAweSim, const char* pszArguments, void* /*ppVoid*/, CArchive *pAr)
{
	m_pAweSimModule = pAweSim;
	m_bFirstTime = TRUE;
	if( pszArguments && strcmp(pszArguments, ""))
	{
		// allowed arguments are
		// -d (disassembly)
		// -s (state changes)
		// -i (I/O)
		// -m (memory)
		// -e (exceptions & interrupts)
		// -p (power)

		m_LogDisassembly = m_LogStateChanges = m_LogIO = m_LogLinMem = m_LogExceptions = m_LogPower = false;

		char* myArgs = new char[strlen(pszArguments)+1];
		strcpy(myArgs, pszArguments);
		char* tokArgs = myArgs;
		char* curArg;
		while((curArg = strtok(tokArgs, " ")) != NULL)
		{
			if(!strcmp(curArg, "-d"))
			{
				printf("Logging Disassembly\n");
				m_LogDisassembly = true;
			}
			else if(!strcmp(curArg, "-s"))
			{
				printf("Logging State Changes\n");
				m_LogStateChanges = true;
			}
			else if(!strcmp(curArg, "-i"))
			{
				printf("Logging I/O\n");
				m_LogIO = true;
			}
			else if(!strcmp(curArg, "-m"))
			{
				printf("Logging Memory\n");
				m_LogLinMem = true;
			}
			else if(!strcmp(curArg, "-e"))
			{
				printf("Logging Exceptions\n");
				m_LogExceptions = true;
			}
			else if(!strcmp(curArg, "-p"))
			{
				printf("Logging Power\n");
				m_LogPower = true;
			}
			tokArgs = NULL;
		}
		delete[] myArgs;
	}
	else
	{
		printf("No arguments supplied - logging dissasembly, state changes, I/O, memory, and exceptions\n");
		m_LogDisassembly = m_LogStateChanges = m_LogIO = m_LogLinMem = m_LogExceptions = m_LogPower = true;
	}

	if(pAr != NULL)
	{
		int tRev;
		m_pAweSimModule->LoadBSD(pAr, &tRev, sizeof(tRev));
		m_pAweSimModule->LoadBSD(pAr, &m_LogDisassembly, sizeof(m_LogDisassembly));
		m_pAweSimModule->LoadBSD(pAr, &m_LogStateChanges, sizeof(m_LogStateChanges));
		m_pAweSimModule->LoadBSD(pAr, &m_LogIO, sizeof(m_LogIO));
		m_pAweSimModule->LoadBSD(pAr, &m_LogLinMem, sizeof(m_LogLinMem));
		m_pAweSimModule->LoadBSD(pAr, &m_LogExceptions, sizeof(m_LogExceptions));
		if(tRev > 1)
			m_pAweSimModule->LoadBSD(pAr, &m_LogPower, sizeof(m_LogPower));
	}
}

CpuLogger::~CpuLogger()
{
}

UINT32 CpuLogger::GetAnalyzerFlags()
{
	UINT32 tFlags = 0;
	if(m_LogDisassembly || m_LogStateChanges)
		tFlags |= ANALYZER_IN_EXECUTEALL;
	if(m_LogIO)
		tFlags |= ANALYZER_IN_MONITORIO;
	if(m_LogLinMem)
		tFlags |= ANALYZER_IN_MONITORMEM;
	if(m_LogExceptions)
		tFlags |= (ANALYZER_IN_SWI | ANALYZER_IN_HWI | ANALYZER_IN_EXC | ANALYZER_IN_VMEXIT);
	if(m_LogPower)
		tFlags |= ANALYZER_IN_POWER;

	return(tFlags);
}

bool CpuLogger::GetAnalyzerDescription(char* pszDest, int nDestSize)
{
	char* Description = new char[128];
	strcpy(Description, "Sample Logging Analyzer {");
	if(m_LogDisassembly)
		strcat(Description, " -d");
	if(m_LogStateChanges)
		strcat(Description, " -s");
	if(m_LogIO)
		strcat(Description, " -i");
	if(m_LogLinMem)
		strcat(Description, " -m");
	if(m_LogExceptions)
		strcat(Description, " -e");
	if(m_LogPower)
		strcat(Description, " -p");
	strcat(Description, " }");
	if((unsigned)nDestSize <= strlen(Description))
		return(false);
	strcpy(pszDest, Description);
	return(true);
}

void CpuLogger::SaveAnalyzer(CArchive* pAr)
{
	int tRev = CPULOGGER_BSD_REV;
	m_pAweSimModule->SaveBSD(pAr, &tRev, sizeof(tRev));
	m_pAweSimModule->SaveBSD(pAr, &m_LogDisassembly, sizeof(m_LogDisassembly));
	m_pAweSimModule->SaveBSD(pAr, &m_LogStateChanges, sizeof(m_LogStateChanges));
	m_pAweSimModule->SaveBSD(pAr, &m_LogIO, sizeof(m_LogIO));
	m_pAweSimModule->SaveBSD(pAr, &m_LogLinMem, sizeof(m_LogLinMem));
	m_pAweSimModule->SaveBSD(pAr, &m_LogExceptions, sizeof(m_LogExceptions));
	m_pAweSimModule->SaveBSD(pAr, &m_LogPower, sizeof(m_LogPower));
}

void CpuLogger::AnalyzerNotify(void* pVoid, UINT32 nNotifyCode, void* pParam1, void* pParam2)
{
	INMONITORIOSTRUCT *pIo;
	INMONITORMEMSTRUCT *pMem;
	INEXCSTRUCT *pExc;
	INVMEXITSTRUCT *pVMEXIT;

	switch(nNotifyCode)
	{
		case ANALYZER_IN_MONITORMEM:
			pMem = (INMONITORMEMSTRUCT *) pParam1;
			HandleMem(pMem);
			break;
		case ANALYZER_IN_MONITORIO:
			pIo = (INMONITORIOSTRUCT *) pParam1;
			HandleIO(pIo);
			break;
		case ANALYZER_IN_EXECUTEALL:
			HandleExecute();
			break;
		case ANALYZER_IN_SWI:
			HandleSWI(static_cast<UINT8>(reinterpret_cast<UINT64>(pParam1)));
			break;
		case ANALYZER_IN_HWI:
			HandleHWI(static_cast<UINT8>(reinterpret_cast<UINT64>(pParam1)));
			break;
		case ANALYZER_IN_EXC:
			pExc = (INEXCSTRUCT *) pParam1;
			HandleEXC(pExc);
			break;
		case ANALYZER_IN_POWER:
			HandlePower((const char*)pParam1);
			break;
		case ANALYZER_IN_VMEXIT:
			pVMEXIT = (INVMEXITSTRUCT *) pParam1;
			HandleVMEXIT(pVMEXIT);
			break;
	}
}

//	HandleIO - Handle an ANALYZER_IN_MONITORIO event
//
//	This event is generated before WRITE and after READ I/O operations, so that
//		the data in the structure is always valid.
//

void CpuLogger::HandleIO(INMONITORIOSTRUCT *pIo)
{
	m_pAweSimModule->LogPrintf("   I%c %04X ", pIo->bIORead ? 'r' : 'w', pIo->nPort);
	UINT32 nData = pIo->bData;
	for(int q = 0; q < pIo->nCount; q++)
	{
		m_pAweSimModule->LogPrintf("%02X ", nData & 0xFF);
		nData >>= 8;
	}
	m_pAweSimModule->LogPrintf("\n");
}

//	HandleMem - Handle an ANALYZER_IN_INTERCEPTMEM event
//
//	This event is generated for every memory read/write. Note that you can set the
//		bAbort to "true", and the memory access will not happen. If the access is a
//		read, you should provide the appropriate data in this case.
//

void CpuLogger::HandleMem(INMONITORMEMSTRUCT *pMem)
{
	bool IsLin = (pMem->AccessType == e_LinearAccess);
	ADDRTYPE PhyAddr = IsLin ? m_pAweSimModule->GetPhysAddress(pMem->LinearAddress) : pMem->PhysicalAddress;
	ADDRTYPE LinAddr = IsLin ? pMem->LinearAddress : 0;
	char PageSize[3] = {0,0,0};

	// add memory page size output
	if(IsLin)
	{
		unsigned int pagebit = m_pAweSimModule->GetPageBit(LinAddr);
		assert(pagebit < 32);
		memcpy(PageSize, &Bit2Str[(pagebit % 32) * 2], 2);
	}

	m_pAweSimModule->LogPrintf("   M%c%s%c %016"FMT64"X/%016"FMT64"X[%02X] ", IsLin ? 'v' : 'p', PageSize, pMem->bMemCode ? 'c' : pMem->bMemRead ? 'r' : 'w', LinAddr, PhyAddr, pMem->nCount);

	UINT32 ByteCount = pMem->nCount;
	UINT32 Index = 0;
	UINT8 *pBuffer = pMem->pBuffer;
	while(ByteCount > 0)
	{
		UINT32 BytesToShow = 16;		
		if(BytesToShow > ByteCount)
			BytesToShow = ByteCount;

		for(Index = 0; Index < BytesToShow; Index++)
			m_pAweSimModule->LogPrintf("%02X ", *(pBuffer++));

		ByteCount -= BytesToShow;
		if(ByteCount > 0)
			m_pAweSimModule->LogPrintf("\n                                            ");
		else
			m_pAweSimModule->LogPrintf("\n");
	}

}

//	HandleExecute - Handle an ANALYZER_IN_EXECUTEALL event
//
//	This event is generated at the beginning of every instruction. Note that the
//		instruction may not complete, due to exceptions or internal simulator
//		behavior, therefore, we use the start of one instruction as an indicator
//		of the end of the previous instruction.
//

void CpuLogger::HandleExecute()
{
	X8664SMMSTATE xState;
	FXSAVEAREA xFxState;
	UINT8 CodeBytes[16];
	UINT32 CodeBytesPresent;

	m_pAweSimModule->GetX8664SMMState(&xState);			// Get the CPU state
	m_pAweSimModule->GetFXSAVEState(&xFxState);

	//	Note: AweSim may attempt to execute the instruction several times. The following code is used
	//		to detect a retry, and to ignore such cases. This is done by comparing the linear address
	//		of the instruction as well as the value of RCX (to catch REP'd instructions) in the previous
	//		and current CPU states. If they are the same, it is assumed that this is a retry of the
	//		same instruction.

	ADDRTYPE xLinear = xState.RegisterRIP;
	if((xState.FlagsCS & 0x200) == 0) // LongBit
		xLinear = (UINT32)(xLinear + xState.BaseCS);

	if(m_bOldStateValid)
	{
		if((xLinear == m_xLinear) && (xState.IntegerRegs[14] == m_OldState.IntegerRegs[14]))
			return;
	}

	//	We've detected a change in the state. This implies that the previous instruction has finished executing
	if(m_LogStateChanges)
		ReportStateChanges(&xState, &xFxState);	// State changes are a result of the previous instruction

	if(m_LogDisassembly)
	{
		m_xLinear = xLinear;

		//	Read some code bytes

		m_pAweSimModule->ReadLinearMemory(xLinear, CodeBytes, sizeof(CodeBytes), &CodeBytesPresent);

		int InstLength;
		char *pDisasm = m_pAweSimModule->Disassemble(CodeBytes, InstLength);
		m_pAweSimModule->LogPrintf("[%"FMT64"d] %04X:%04X:%016"FMT64"X  %s\n   -- Opcodes: ", m_pAweSimModule->InstructionCount(), xState.SelectorTR,
			xState.SelectorCS, xState.RegisterRIP, pDisasm);
		for(int q = 0; q < InstLength; q++)
			m_pAweSimModule->LogPrintf("%02X ", CodeBytes[q]);
		m_pAweSimModule->LogPrintf("\n");
	}
}

//	HandleSWI - Handle an ANALYZER_IN_SWI event
//
//	This event is generated at the start of every Software Interrupt
//

void CpuLogger::HandleSWI(UINT8 nVector)
{
	X8664SMMSTATE xState;
	FXSAVEAREA xFxState;

	//	We've detected a change in the state. This implies that the previous instruction has finished executing
	if(m_LogStateChanges)
	{
		m_pAweSimModule->GetX8664SMMState(&xState);			// Get the CPU state
		m_pAweSimModule->GetFXSAVEState(&xFxState);
		ReportStateChanges(&xState, &xFxState);	// State changes are a result of the previous instruction
	}
	m_pAweSimModule->LogPrintf("   In %02X Software Interrupt\n", nVector);
}

//	HandleHWI - Handle an ANALYZER_IN_HWI event
//
//	This event is generated at the start of every Hardware Interrupt
//

void CpuLogger::HandleHWI(UINT8 nVector)
{
	X8664SMMSTATE xState;
	FXSAVEAREA xFxState;

	//	We've detected a change in the state. This implies that the previous instruction has finished executing
	if(m_LogStateChanges)
	{
		m_pAweSimModule->GetX8664SMMState(&xState);			// Get the CPU state
		m_pAweSimModule->GetFXSAVEState(&xFxState);
		ReportStateChanges(&xState, &xFxState);	// State changes are a result of the previous instruction
	}

	m_pAweSimModule->LogPrintf("[%"FMT64"d] In %02X Hardware Interrupt\n", m_pAweSimModule->InstructionCount(), nVector);
}

//	HandleEXC - Handle an ANALYZER_IN_EXC event
//
//	This event is generated at the start of every processor initiated exception
//

void CpuLogger::HandleEXC(INEXCSTRUCT* pExc)
{
	X8664SMMSTATE xState;
	FXSAVEAREA xFxState;

	//	We've detected a change in the state. This implies that the previous instruction has finished executing
	if(m_LogStateChanges)
	{
		m_pAweSimModule->GetX8664SMMState(&xState);			// Get the CPU state
		m_pAweSimModule->GetFXSAVEState(&xFxState);
		ReportStateChanges(&xState, &xFxState);	// State changes are a result of the previous instruction
	}
	m_pAweSimModule->LogPrintf("[%"FMT64"d] In %02X Exception %s\n", m_pAweSimModule->InstructionCount(), pExc->nVector, pExc->dCause);
}

void CpuLogger::HandleVMEXIT(INVMEXITSTRUCT* pVMEXIT)
{
	X8664SMMSTATE xState;
	FXSAVEAREA xFxState;

	//	We've detected a change in the state. This implies that the previous instruction has finished executing
	if(m_LogStateChanges)
	{
		m_pAweSimModule->GetX8664SMMState(&xState);			// Get the CPU state
		m_pAweSimModule->GetFXSAVEState(&xFxState);
		ReportStateChanges(&xState, &xFxState);	// State changes are a result of the previous instruction
	}
	m_pAweSimModule->LogPrintf("[%"FMT64"d] In VMEXIT exitcode %x\n", m_pAweSimModule->InstructionCount(), pVMEXIT->nExitcode);
}

//	HandlePower - Handle an ANALYZER_IN_POWER event
//
//	This event is generated at the start of every power state change
//

void CpuLogger::HandlePower(const char* sPower)
{
	m_pAweSimModule->LogPrintf("POWER: %s\n", sPower);
}

//	ReportStateChanges - Log any changes between the previous state and the new state

void CpuLogger::ReportStateChanges(X8664SMMSTATE *pState, FXSAVEAREA *pFxState)
{
	int Index = 0;
	
	if(!m_bOldStateValid)
	{
		m_bOldStateValid = true;
#ifdef FIRST_STATE_ZEROS
		memset(&m_OldState, 0, sizeof(m_OldState));
		memset(&m_OldFxState, 0, sizeof(m_OldFxState));
#else
		m_OldState = *pState;
		m_OldFxState = *pFxState;
		return;
#endif
	}

	//	Dump the general registers and EFlags

	DumpIntRegChange(pState, 15, "RAX");
	DumpIntRegChange(pState, 14, "RCX");
	DumpIntRegChange(pState, 13, "RDX");
	DumpIntRegChange(pState, 12, "RBX");
	DumpIntRegChange(pState, 11, "RSP");
	DumpIntRegChange(pState, 10, "RBP");
	DumpIntRegChange(pState, 9,  "RSI");
	DumpIntRegChange(pState, 8,  "RDI");
	DumpIntRegChange(pState, 7,  "R8 ");
	DumpIntRegChange(pState, 6,  "R9 ");
	DumpIntRegChange(pState, 5,  "R10");
	DumpIntRegChange(pState, 4,  "R11");
	DumpIntRegChange(pState, 3,  "R12");
	DumpIntRegChange(pState, 2,  "R13");
	DumpIntRegChange(pState, 1,  "R14");
	DumpIntRegChange(pState, 0,  "R15");

	if(pState->RegisterEFlags != m_OldState.RegisterEFlags)
		m_pAweSimModule->LogPrintf("   -- EFL = %08X/%08X\n", m_OldState.RegisterEFlags, pState->RegisterEFlags);

	if (m_bFirstTime)
	{
		if (pState->RegisterRIP != m_OldState.RegisterRIP)
			m_pAweSimModule->LogPrintf("   -- RIP = %016X/%016X\n", m_OldState.RegisterRIP, pState->RegisterRIP);
	}

	//	Dump changes in the segment registers
	if((m_OldState.SelectorCS != pState->SelectorCS) ||
	   (m_OldState.FlagsCS != pState->FlagsCS) ||
	   (m_OldState.LimitCS != pState->LimitCS) ||
	   (m_OldState.BaseCS != pState->BaseCS))
	{
		m_pAweSimModule->LogPrintf("   -- CS = %04X/%04X", m_OldState.SelectorCS, pState->SelectorCS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseCS, pState->BaseCS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitCS, pState->LimitCS);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsCS, pState->FlagsCS);
	}

	if((m_OldState.SelectorDS != pState->SelectorDS) ||
	   (m_OldState.FlagsDS != pState->FlagsDS) ||
	   (m_OldState.LimitDS != pState->LimitDS) ||
	   (m_OldState.BaseDS != pState->BaseDS))
	{
		m_pAweSimModule->LogPrintf("   -- DS = %04X/%04X", m_OldState.SelectorDS, pState->SelectorDS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseDS, pState->BaseDS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitDS, pState->LimitDS);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsDS, pState->FlagsDS);
	}

	if((m_OldState.SelectorES != pState->SelectorES) ||
	   (m_OldState.FlagsES != pState->FlagsES) ||
	   (m_OldState.LimitES != pState->LimitES) ||
	   (m_OldState.BaseES != pState->BaseES))
	{
		m_pAweSimModule->LogPrintf("   -- ES = %04X/%04X", m_OldState.SelectorES, pState->SelectorES);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseES, pState->BaseES);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitES, pState->LimitES);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsES, pState->FlagsES);
	}

	if((m_OldState.SelectorFS != pState->SelectorFS) ||
	   (m_OldState.FlagsFS != pState->FlagsFS) ||
	   (m_OldState.LimitFS != pState->LimitFS) ||
	   (m_OldState.BaseFS != pState->BaseFS))
	{
		m_pAweSimModule->LogPrintf("   -- FS = %04X/%04X", m_OldState.SelectorFS, pState->SelectorFS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseFS, pState->BaseFS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitFS, pState->LimitFS);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsFS, pState->FlagsFS);
	}

	if((m_OldState.SelectorGS != pState->SelectorGS) ||
	   (m_OldState.FlagsGS != pState->FlagsGS) ||
	   (m_OldState.LimitGS != pState->LimitGS) ||
	   (m_OldState.BaseGS != pState->BaseGS))
	{
		m_pAweSimModule->LogPrintf("   -- GS = %04X/%04X", m_OldState.SelectorGS, pState->SelectorGS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseGS, pState->BaseGS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitGS, pState->LimitGS);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsGS, pState->FlagsGS);
	}

	if((m_OldState.SelectorSS != pState->SelectorSS) ||
	   (m_OldState.FlagsSS != pState->FlagsSS) ||
	   (m_OldState.LimitSS != pState->LimitSS) ||
	   (m_OldState.BaseSS != pState->BaseSS))
	{
		m_pAweSimModule->LogPrintf("   -- SS = %04X/%04X", m_OldState.SelectorSS, pState->SelectorSS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseSS, pState->BaseSS);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitSS, pState->LimitSS);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsSS, pState->FlagsSS);
	}

	//	Do the IDT, GDT, LDT, and TSS registers

	if((m_OldState.LimitGDT != pState->LimitGDT) ||
	   (m_OldState.BaseGDT  != pState->BaseGDT))
		m_pAweSimModule->LogPrintf("   -- GDT = %016"FMT64"X/%016"FMT64"X %04X/%04X\n",
			m_OldState.BaseGDT, pState->BaseGDT,
			m_OldState.LimitGDT, pState->LimitGDT);

	if((m_OldState.LimitIDT != pState->LimitIDT) ||
	   (m_OldState.BaseIDT  != pState->BaseIDT))
		m_pAweSimModule->LogPrintf("   -- IDT = %016"FMT64"X/%016"FMT64"X %04X/%04X\n",
			m_OldState.BaseIDT, pState->BaseIDT,
			m_OldState.LimitIDT, pState->LimitIDT);

	if((m_OldState.SelectorLDT != pState->SelectorLDT) ||
	   (m_OldState.FlagsLDT != pState->FlagsLDT) ||
	   (m_OldState.LimitLDT != pState->LimitLDT) ||
	   (m_OldState.BaseLDT != pState->BaseLDT))
	{
		m_pAweSimModule->LogPrintf("   -- LDT = %04X/%04X", m_OldState.SelectorLDT, pState->SelectorLDT);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseLDT, pState->BaseLDT);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitLDT, pState->LimitLDT);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsLDT, pState->FlagsLDT);
	}

	if((m_OldState.SelectorTR != pState->SelectorTR) ||
	   (m_OldState.FlagsTR != pState->FlagsTR) ||
	   (m_OldState.LimitTR != pState->LimitTR) ||
	   (m_OldState.BaseTR != pState->BaseTR))
	{
		m_pAweSimModule->LogPrintf("   -- TSS = %04X/%04X", m_OldState.SelectorTR, pState->SelectorTR);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.BaseTR, pState->BaseTR);
		m_pAweSimModule->LogPrintf("  %08X/%08X", m_OldState.LimitTR, pState->LimitTR);
		m_pAweSimModule->LogPrintf("  %04X/%04X\n", m_OldState.FlagsTR, pState->FlagsTR);
	}

	if(pState->RegisterDR6 != m_OldState.RegisterDR6)
		m_pAweSimModule->LogPrintf("   -- DR6 = %08X/%08X\n", m_OldState.RegisterDR6, pState->RegisterDR6);
	if(pState->RegisterDR7 != m_OldState.RegisterDR7)
		m_pAweSimModule->LogPrintf("   -- DR7 = %08X/%08X\n", m_OldState.RegisterDR7, pState->RegisterDR7);

	//	Dump changes in the CRx registers

	if(pState->RegisterCR0 != m_OldState.RegisterCR0)
		m_pAweSimModule->LogPrintf("   -- CR0 = %08X/%08X\n", m_OldState.RegisterCR0, pState->RegisterCR0);
	if(pState->RegisterCR3 != m_OldState.RegisterCR3)
		m_pAweSimModule->LogPrintf("   -- CR3 = %08X/%08X\n", m_OldState.RegisterCR3, pState->RegisterCR3);
	if(pState->RegisterCR4 != m_OldState.RegisterCR4)
		m_pAweSimModule->LogPrintf("   -- CR4 = %08X/%08X\n", m_OldState.RegisterCR4, pState->RegisterCR4);

	//	Do the XMM registers

	if(pFxState->mxcsr != m_OldFxState.mxcsr)
		m_pAweSimModule->LogPrintf("   -- MXC = %08X/%08X\n", m_OldFxState.mxcsr, pFxState->mxcsr);
	for(Index = 0; Index < 16; Index++)
	{
		if(pFxState->xmm[Index] != m_OldFxState.xmm[Index])
			m_pAweSimModule->LogPrintf("   -- XM%01x = %016"FMT64"X%016"FMT64"X/%016"FMT64"X%016"FMT64"X\n", Index,
				m_OldFxState.xmm[Index].msw, m_OldFxState.xmm[Index].lsw,
				pFxState->xmm[Index].msw, pFxState->xmm[Index].lsw);
	}

	//	Do the FPU/MMX registers

	if(m_OldFxState.fcw != pFxState->fcw)
		m_pAweSimModule->LogPrintf("   -- FCW = %04X/%04X\n", m_OldFxState.fcw, pFxState->fcw);
	if(m_OldFxState.fsw != pFxState->fsw)
		m_pAweSimModule->LogPrintf("   -- FSW = %04X/%04X\n", m_OldFxState.fsw, pFxState->fsw);
	if(m_OldFxState.ftw != pFxState->ftw)
		m_pAweSimModule->LogPrintf("   -- FTW = %04X/%04X\n", m_OldFxState.ftw, pFxState->ftw);
	if((m_OldFxState.cs != pFxState->cs) ||
	   (m_OldFxState.ip != pFxState->ip))
		m_pAweSimModule->LogPrintf("   -- FCS = %04X:%08X/%04X:%08X\n", m_OldFxState.cs, m_OldFxState.ip,
		pFxState->cs, pFxState->ip);
	if((m_OldFxState.ds != pFxState->ds) ||
	   (m_OldFxState.dp != pFxState->dp))
		m_pAweSimModule->LogPrintf("   -- FDS = %04X:%08X/%04X:%08X\n", m_OldFxState.ds, m_OldFxState.dp,
		pFxState->ds, pFxState->dp);
	if(m_OldFxState.fop != pFxState->fop)
		m_pAweSimModule->LogPrintf("   -- FOP = %04X/%04X\n", m_OldFxState.fop, pFxState->fop);
	
	if((m_OldFxState.mmx0Mant != pFxState->mmx0Mant) ||
	   (m_OldFxState.mmx0Exp != pFxState->mmx0Exp))
		m_pAweSimModule->LogPrintf("   -- FP0 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx0Exp, m_OldFxState.mmx0Mant, pFxState->mmx0Exp, pFxState->mmx0Mant);

	if((m_OldFxState.mmx1Mant != pFxState->mmx1Mant) ||
	   (m_OldFxState.mmx1Exp != pFxState->mmx1Exp))
		m_pAweSimModule->LogPrintf("   -- FP1 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx1Exp, m_OldFxState.mmx1Mant, pFxState->mmx1Exp, pFxState->mmx1Mant);

	if((m_OldFxState.mmx2Mant != pFxState->mmx2Mant) ||
	   (m_OldFxState.mmx2Exp != pFxState->mmx2Exp))
		m_pAweSimModule->LogPrintf("   -- FP2 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx2Exp, m_OldFxState.mmx2Mant, pFxState->mmx2Exp, pFxState->mmx2Mant);

	if((m_OldFxState.mmx3Mant != pFxState->mmx3Mant) ||
	   (m_OldFxState.mmx3Exp != pFxState->mmx3Exp))
		m_pAweSimModule->LogPrintf("   -- FP3 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx3Exp, m_OldFxState.mmx3Mant, pFxState->mmx3Exp, pFxState->mmx3Mant);

	if((m_OldFxState.mmx4Mant != pFxState->mmx4Mant) ||
	   (m_OldFxState.mmx4Exp != pFxState->mmx4Exp))
		m_pAweSimModule->LogPrintf("   -- FP4 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx4Exp, m_OldFxState.mmx4Mant, pFxState->mmx4Exp, pFxState->mmx4Mant);

	if((m_OldFxState.mmx5Mant != pFxState->mmx5Mant) ||
	   (m_OldFxState.mmx5Exp != pFxState->mmx5Exp))
		m_pAweSimModule->LogPrintf("   -- FP5 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx5Exp, m_OldFxState.mmx5Mant, pFxState->mmx5Exp, pFxState->mmx5Mant);

	if((m_OldFxState.mmx6Mant != pFxState->mmx6Mant) ||
	   (m_OldFxState.mmx6Exp != pFxState->mmx6Exp))
		m_pAweSimModule->LogPrintf("   -- FP6 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx6Exp, m_OldFxState.mmx6Mant, pFxState->mmx6Exp, pFxState->mmx6Mant);

	if((m_OldFxState.mmx7Mant != pFxState->mmx7Mant) ||
	   (m_OldFxState.mmx7Exp != pFxState->mmx7Exp))
		m_pAweSimModule->LogPrintf("   -- FP7 = %04X:%016"FMT64"X/%04X:%016"FMT64"X\n", m_OldFxState.mmx7Exp, m_OldFxState.mmx7Mant, pFxState->mmx7Exp, pFxState->mmx7Mant);

	if(pState->CurrentPL != m_OldState.CurrentPL)
		m_pAweSimModule->LogPrintf("   -- CPL %d/%d\n", m_OldState.CurrentPL, pState->CurrentPL);

	m_OldState = *pState;
	m_OldFxState = *pFxState;
	m_bFirstTime = FALSE;
}

void CpuLogger::DumpIntRegChange(X8664SMMSTATE *pState, int nIndex, const char *sRName)
{
	UINT64 nOld = m_OldState.IntegerRegs[nIndex];
	UINT64 nNew = pState->IntegerRegs[nIndex];

	if(nOld != nNew)
		m_pAweSimModule->LogPrintf("   -- %s = %016"FMT64"X/%016"FMT64"X\n", sRName, nOld, nNew);
}

