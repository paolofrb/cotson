/* 
** Copyright 1995 - 2005 ADVANCED MICRO DEVICES, INC.  All Rights Reserved. 
*/

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#define BOGUS_PTR usign64(0xffff8000badbad00)

struct INMONITORIOSTRUCT;
struct INMONITORMEMSTRUCT;
struct INEXCSTRUCT;
struct INVMEXITSTRUCT;
struct X8664SMMSTATE;
struct FXSAVEAREA;

class CAweSim;
class CpuLogger;
class CCaller;
class CArchive;

extern "C" {
	DLLEXPORT UINT32 GetAnalyzerVersion(CCaller* pCaller);
	DLLEXPORT UINT32 GetAnalyzerFlags(CCaller* pCaller);
	DLLEXPORT bool AnalyzerLoaded(CAweSim *pAweSim, const char *pszArguments, void **ppVoid, CCaller* pCaller, CArchive* pAr);
	DLLEXPORT void SaveAnalyzer(CArchive* pAr, CCaller* pCaller);
	DLLEXPORT void AnalyzerUnloaded(void *pVoid, CCaller* pCaller);
	DLLEXPORT void AnalyzerNotify(void *pVoid, UINT32 nNotifyCode, void *pParam1, void *pParam2, CCaller* pCaller);
	DLLEXPORT bool GetAnalyzerDescription(char *pszDest, int nDestSize, CCaller* pCaller);
}

struct Entry
{
	CpuLogger* pCpuLogger;
	CCaller* pCaller;
};

// list of cpuloggers that have been instantiated
std::vector<Entry> v;
// functions to query the list of cpuloggers
bool HasEntry(CCaller* pCaller);
Entry* GetEntry(CCaller* pCaller);
void RemoveEntry(CCaller* pCaller);
void AddEntry(CpuLogger* pCpuLogger, CCaller* pCaller);


#define CPULOGGER_BSD_REV 2
static const char *Bit2Str = "????????????????????????4K????????????????2M4M??????????????1G??";

//	Internal class, specific to this sample
class CpuLogger
{

//constructors & destructors
public: 
	CpuLogger(CAweSim *pAweSim, const char* pszArguments, void* ppVoid, CArchive *pAr);
	~CpuLogger();

// public interface	
public: 
	UINT32 GetAnalyzerFlags();
	bool GetAnalyzerDescription(char* pszDest, int nDestSize);
	void AnalyzerNotify(void* pVoid, UINT32 NotifyCode, void* pParam1, void* pParam2);
	void SaveAnalyzer(CArchive* pAr);

// internal routines
protected:

	void HandleIO(INMONITORIOSTRUCT *pIo);
	void HandleMem(INMONITORMEMSTRUCT *pMem);
	void HandleExecute();
	void HandleSWI(UINT8 nVector);
	void HandleHWI(UINT8 nVector);
	void HandleEXC(INEXCSTRUCT *pExc);
	void HandleVMEXIT(INVMEXITSTRUCT *pVMEXIT);
	void HandlePower(const char* sPower);

	void ReportStateChanges(X8664SMMSTATE *pState, FXSAVEAREA *pFxState);
	void DumpIntRegChange(X8664SMMSTATE *pState, int nIndex, const char *sRName);
	void DumpSegRegChange(X8664SMMSTATE *pState, int nIndex, const char *sRName);
	void Dump32BitField(UINT32 nNew, UINT32 nOld, const char *sRName);
	void DumpMSRChange(int nIndex, UINT64 nOld, UINT64 nNew);
// internal data
protected: 
	CAweSim* m_pAweSimModule;
	bool m_bOldStateValid;
	X8664SMMSTATE m_OldState;
	FXSAVEAREA m_OldFxState;
	ADDRTYPE m_xLinear;
	BOOL m_LogDisassembly;
	BOOL m_LogStateChanges;
	BOOL m_LogIO;
	BOOL m_LogLinMem;
	BOOL m_LogExceptions;
	BOOL m_LogPower;
	BOOL m_bFirstTime;
};
