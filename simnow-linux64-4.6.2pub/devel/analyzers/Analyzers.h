/* 
** Copyright 1995 - 2005 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

//	Analyzers.h - Define various constants and prototypes for use in writing
//			AweSim loadable analyzers
//

#ifndef _ANALYZERS_H
#define _ANALYZERS_H

#include <stdlib.h>

class CProcessor;
class CLoadModule;
class CCaller;
class CAweSim;
class CDisassembler;
class CArchive;
struct Tlb;
class ClockCallBack;

//	Define the version number that the analyzer should return from GetAnalyzerVersion()

#define ANALYZER_CURRENT_VERSION			0x00000001

//	Define various flags returned by the "AnalyzerFlags()" function

#define ANALYZER_BB_TRANSLATE				0x00000001		// Start of translating a basic block

#define ANALYZER_IN_TRANSLATE				0x00000002		// Start of translating an instruction
#define ANALYZER_IN_EXECUTEALL				0x00000004		// Start of every instruction execution
#define ANALYZER_IN_SWI						0x00000008		// Software interrupt being executed
#define ANALYZER_IN_HWI						0x00000010		// Hardware interrupt being taken
#define ANALYZER_IN_EXC						0x00000020		// Processor exception being taken
#define ANALYZER_IN_EXECUTETAG				0x00000040		// Tagged instruction is executed
#define ANALYZER_IN_MONITORIO				0x00000080		// An I/O instruction is being executed. No abort allowed.
#define ANALYZER_IN_INTERCEPTIO				0x00000100		// An I/O instruction is being executed. Aborts allowed.
#define ANALYZER_IN_INTERCEPTMEM			0x00000200		// A memory access is ocurring.  Aborts allowed.
#define ANALYZER_IN_MONITORMEM				0x00000400		// An memory access is ocurring.  No abort allowed.
#define ANALYZER_IN_RESET					0x00000800		// Processor model is being reset
#define ANALYZER_IN_EVENT					0x00001000		// analyzer event handler
#define ANALYZER_IN_CODEINJECT				0x00002000		// code injection hook
#define ANALYZER_IN_QUEUEFULL				0x00004000		// queue full hook
#define ANALYZER_IN_POWER					0x00008000		// power state change hook
#define ANALYZER_IN_EXITREQ					0x00010000		// exit requested
#define ANALYZER_IN_VMEXIT					0x00020000      // a #VMEXIT is being taken

//***	Define function prototypes

//**	GetAnalyzerVersion() - Returns the version number of the interface this analyzer expects

typedef UINT32 GETANALYZERVERSIONPROC(CCaller* pCaller);
typedef GETANALYZERVERSIONPROC *PGETANALYZERVERSIONPROC;
extern UINT32 GetAnalyzerVersion(CCaller* pCaller);

//**	GetAnalyzerFlags() - Returns a mask of ANALYZE_xxxxx bits

typedef UINT32 GETANALYZERFLAGSPROC(CCaller* pCaller);
typedef GETANALYZERFLAGSPROC *PGETANALYZERFLAGSPROC;
extern UINT32 GetAnalyzerFlags(CCaller* pCaller);

//**	GetAnalyzerDescription - Returns a short description of the analyzer

typedef bool GETANALYZERDESCRIPTIONPROC(char *pszDest, int nDestSize, CCaller* pCaller);
typedef GETANALYZERDESCRIPTIONPROC *PGETANALYZERDESCRIPTIONPROC;
extern bool GetAnalyzerDescription(char *pszDest, int nDestSize, CCaller* pCaller);

//**	AnalyzerLoaded() - Called when the analyzer is loaded into memory

typedef bool ANALYZERLOADEDPROC(CAweSim *pAweSim, const char *pszArguments, void **ppVoid, CCaller* pCaller, CArchive* pAr);
typedef ANALYZERLOADEDPROC *PANALYZERLOADEDPROC;
extern bool AnalyzerLoaded(CAweSim *pAweSim, const char *pszArguments, void **ppVoid, CCaller* pCaller, CArchive* ar = NULL);

//**	AnalyzerUnloaded() - Called when the analyzer is unloaded from memory

typedef void ANALYZERUNLOADEDPROC(void *pVoid, CCaller* pCaller);
typedef ANALYZERUNLOADEDPROC *PANALYZERUNLOADEDPROC;
extern void AnalyzerUnloaded(void *pVoid, CCaller* pCaller);

//**	SaveAnalyzer() - Called on Save BSD

typedef void SAVEANALYZERPROC(CArchive* pAr, CCaller* pCaller);
typedef SAVEANALYZERPROC *PSAVEANALYZERPROC;
extern void SaveAnalyzer(CArchive* pAr, CCaller* pCaller);

//**	EnableAnalyzer() - Called to enable or disable an analyzer

typedef void ENABLEANALYZERPROC(bool bEnable, CCaller* pCaller);
typedef ENABLEANALYZERPROC *PENABLEANALYZERPROC;
extern void EnableAnalyzer(bool bEnable, CCaller* pCaller);

//**	AnalyzerNotify() - Called when any of the events returned by GetAnalyzerFlags() happens
//
//		Note that the true meaning of the return code, and pParam1 and pParam2 depend on
//		the value of nNotifyCode

typedef void ANALYZERNOTIFYPROC(void *pVoid, UINT32 nNotifyCode, void *pParam1, void *pParam2, CCaller* pCaller);
typedef ANALYZERNOTIFYPROC *PANALYZERNOTIFYPROC;
extern void AnalyzerNotify(void *pVoid, UINT32 nNotifyCode, void *pParam1, void *pParam2, CCaller* pCaller);

//***	Define the structures/parameters used by the various notify routines

//**	ANALYZER_BB_TRANSLATE - Start of Basic Block Translation
//
//	Input:
//
//	Output:
//

//**	ANALYZER_IN_TRANSLATE - Start of instruction translation
//
//	Input:	pParam1	=	Pointer to an INTRANSLATESTRUCT structure
//
//
//	Output:	INTRANSLATESTRUCT.bTranslateNormally	= true to continue translation, false to ignore
//			INTRANSLATESTRUCT.nInstructionTag		= !0 if a tag is set on this instruction
//			INTRANSLATESTRUCT.nInstructionLength	= If !bTranslateNormally, then this is the assumed length
//														in bytes of this instruction.
//

struct INTRANSLATESTRUCT {
	UINT8 *pOpcodeBuffer;								// Pointer to buffer containing opcode bytes
	int nOpcodeCount;									// # of valid bytes in opcode buffer
	int nOpcodeOffset;									// offset of first opcode byte (i.e., number of prefixes)
	bool bIsUndefined;									// Set if this instruction is not defined
	bool bTranslateNormally;							// Tag, but use AweSim generated code
	UINT32 nInstructionTag;								// !0 if we should tag this instruction
	int nInstructionLength;								// If tag is set and bTranslateNormally is <false>, this is the number of bytes in this instruction
};

//**	ANALYZER_IN_EXECUTEALL - An instruction is being executed
//
//	Input:
//
//	Output:
//

//**	ANALYZER_IN_SWI - A software interrupt is ocurring
//
//	Input:	pParam1	=	UINT8 Vector
//
//	Output:
//

//**	ANALYZER_IN_HWI - A hardware interrupt is ocurring
//
//	Input:	pParam1	=	UINT8 Vector
//
//	Output:
//

//**	ANALYZER_IN_EXC - A processor generated exception is ocurring
//
//	Input:	pParam1	=	Pointer to an INEXCSTRUCT structure
//
//	Output:
//

struct INEXCSTRUCT
{
	UINT8 nVector;
	const char* dCause;
};

//**	ANALYZER_IN_EXECUTETAG - A tagged instruction is being executed
//
//	Input:	pParam1	=	UINT32 Tag assigned to this instruction
//
//	Ouptut:
//

//**	ANALYZER_IN_MONITORIO - An I/O operation is being executed
//
//	Input:	pParam1	=	Pointer to an INMONITORIOSTRUCT structure
//
//	Output:
//
//	Note: This event is generated BEFORE the I/O operation has ocurred for writes, and
//		AFTER the I/O operation for reads. That way, the bData member is always valid.
//		However, the I/O can not be aborted.
//

struct INMONITORIOSTRUCT {
	bool   bIORead;							// TRUE = read, FALSE = write
	bool   bRepeat;							// TRUE if a repeated I/O, otherwise single
	UINT16 nPort;							// I/O port being accessed
	UINT8  nCount;							// # of bytes being transferred
	UINT32 bData;							// Data read or being written
};

//**	ANALYZER_IN_INTERCEPTIO - An I/O operation is being executed
//
//	Input:	pParam1	=	Pointer to an ININTERCEPTIOSTRUCT structure
//
//	Output:
//
//	Note: This event is generated BEFORE the I/O operation has ocurred. This means that
//		the bData value is ONLY valid for write operations, but it also means that we can
//		abort the I/O if so desired.
//
//		If the I/O is aborted, the for writes, nothing happens. For reads, the value in
//			bData is used as the result of the I/O operations, so if you set bAbort to
//			true, be sure you also set bData to some reasonable value!
//

struct ININTERCEPTIOSTRUCT {
	bool   bIORead;							// TRUE = read, FALSE = write
	bool   bRepeat;							// TRUE if a repeated I/O, otherwise single
	bool   bAbort;							// Set to TRUE to abort this I/O operation
	UINT16 nPort;							// I/O port being accessed
	UINT8  nCount;							// # of bytes being transferred
	UINT32 bData;							// Data read or being written
};

enum MemAccessType { e_LinearAccess, e_PhysicalAccess };
enum MemAccessReason {};

//**	ANALYZER_IN_INTERCEPTMEM - A Memory Access is ocurring
//
//	Input:	pParam1	=	Pointer to ININTERCEPTMEMSTRUCT structure
//
//	Output:
//
//

struct ININTERCEPTMEMSTRUCT {
	MemAccessType AccessType;			// Type of memory access (linear/physical)
	bool bMemCode;						// TRUE = code access, FALSE = data
	bool bMemRead;						// TRUE = read, FALSE = write
	bool bAbort;						// Set to TRUE to abort this memory operation
	ADDRTYPE LinearAddress;				// Linear address of linear access
	ADDRTYPE PhysicalAddress;			// Physical address of physical access
	MEMTYPE PhysicalMemType;			// Memtype of physical access	
	unsigned int nCount;				// Amount of data being read/written
	UINT8 *pBuffer;						// Pointer to data buffer
};

//**	ANALYZER_IN_MONITORMEM - A Memory Access is ocurring
//
//	Input:	pParam1	=	Pointer to an INMONITORMEMSTRUCT structure
//
//	Output:
//
//	Note: This event is generated BEFORE the memory access has ocurred for 
//  writes, and AFTER the I/O operation for reads. That way, the pBuffer data
//  is always valid.  However, the memory access can not be aborted.
//
//  Note: This event may be generated several times per memory access,
//  depending on size of access, alignment, etc.

struct INMONITORMEMSTRUCT {
	MemAccessType AccessType;			// Type of memory access (linear/physical)
	bool bFromDebugger;					// TRUE = request is a result of a debugger trying to peek at memory
	bool bMemCode;						// TRUE = code access, FALSE = data
	bool bMemRead;						// TRUE = read, FALSE = write
	ADDRTYPE LinearAddress;				// Linear address of access
	ADDRTYPE PhysicalAddress;			// Physical address of physical access
	MEMTYPE PhysicalMemType;			// Memtype of physical access
	unsigned int nCount;				// Amount of data being read/written
	UINT8 *pBuffer;						// Pointer to data buffer
};

struct INQUEUEFULLSTRUCT {
	UINT8* pQueue;
	UINT8* pQueueTail;
};

struct ANALYZEREVENTDATA {
	CLoadModule *pModule;
	UINT64 *pTargetInst;
	ClockCallBack *pAnalyzerEvent;
};

//**	ANALYZER_IN_CODEINJECT - A code injection event is ocurring
//
//	Input:	pParam1	=	A pointer to a CCodeInjector object
//
//	Output:
//

//**	ANALYZER_IN_POWER - A power state change is occuring
//
//	Input:	pParam1	=	const char* PowerState
//
//	Output:
//

//**	ANALYZER_IN_VMEXIT - A #VMEXIT is being executed
//
//	Input:	pParam1 =	Pointer to an INVMEXITSTRUCT structure
//
//	Output:
//

struct INVMEXITSTRUCT
{
	UINT64 nExitcode;
	UINT64 nExitinfo1;
	UINT64 nExitinfo2;
};

//***	Define the CAweSim class. This class allows analyzers to request information
//		about the state of the simulation

class CAweSim
{
public:
	CAweSim();							// Constructor. Analyzers should NEVER call this
	virtual ~CAweSim();							// Destructor. Analyzers should NEVER call this
	void Initialize(CProcessor *pProcessor, CLoadModule *pLoadModule);	// Initialization. Analyzers should NEVER call this

//	Internal routines - do not call these

	bool RequestingMemoryAccess();
	bool RequestingIOAccess();
	
//	Public routines - Use these to interface to the simulator

public:
	virtual int LogPrintf(const char *pFormat, ...);            // Output data to log file
	virtual int ErrPrintf(const char *pFormat, ...);			// Output data to err file

	virtual void LoadBSD(CArchive* pAr, void* pBuffer, int Length);	
	virtual void SaveBSD(CArchive* pAr, const void* pBuffer, int Length);

	virtual void GetX8664SMMState(X8664SMMSTATE *pState);
	virtual void SetX8664SMMState(const X8664SMMSTATE *pState);
	virtual void GetFXSAVEState(FXSAVEAREA *pState);
	virtual void SetFXSAVEState(const FXSAVEAREA *pState);
	
	virtual UINT32 AllocateInstructionTag();					// Return a unique instruction tag

	virtual UINT64 InstructionCount();							// Returns @ of instructions executed

	//	Access to the machines simulated memory
	
	virtual bool ReadPhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone);
	virtual bool WritePhysicalMemory(ADDRTYPE PhysAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone);
	virtual bool ReadLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone);
	virtual bool WriteLinearMemory(ADDRTYPE LinAddress, UINT8 *pBuffer, unsigned int nCount, UINT32 *pBytesDone);
	virtual bool ReadIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount);
	virtual bool WriteIOPort(UINT16 Port, UINT8 *pBuffer, unsigned int nCount);

	virtual bool ControlSimulation(e_SimControl NewControl); // Breakpoint, Save, or Exit
	virtual ADDRTYPE GetA20Mask();
	virtual ADDRTYPE GetPhysAddress(ADDRTYPE LinAddress);
	virtual MEMTYPE GetMemType(ADDRTYPE LinAddress);
	virtual bool ReadMSR(UINT32 MsrNumber, UINT64 *pResult);
	virtual bool WriteMSR(UINT32 MsrNumber, UINT64 MsrValue);
	virtual char *Disassemble(UINT8 *pBuffer, int &Length);
	virtual void ForceRebuildAnalyzerFlags();
	virtual bool IsRunning();
	virtual void NotifyLater(UINT64 Instructions);
	virtual unsigned int GetPageBit(ADDRTYPE LinAddress);
	virtual void SetClkIPC(UINT64 instr, UINT64 cycle);
	virtual UINT64 GetCoreFreq();

//	Internal data.
protected:
	CProcessor *m_pProcessor;
	CLoadModule *m_pLoadModule;
	ClockCallBack *m_AnalyzerEvent;
	bool m_bRequestingMemAccess;
	bool m_bRequestingIOAccess;
	ANALYZEREVENTDATA m_EventData;
	UINT64 m_TargetInst;
};

class CCaller
{
public:
	CCaller() {}
	virtual ~CCaller() {}

public:
	virtual CProcessor* GetProcessor() { return m_pProcessor; }

protected:
	CProcessor* m_pProcessor;
};

#endif  //_ANALYZERS_H
