//	monitor.h - SimNow Monitor dynamic module definitions

//	This is the structure used to track state information for each machine being analyzed

struct MONITORMACHINESTATE
{
	int nTokenState;												// Where we are in the token decode process
	UINT64 nTokenList[32];											// Array of tokens received in the current event
	int nTokenIndex;												// Index into nTokenList for next token in this event
};

class CMonitor : public CMonitorConsumer
{
public:
	CMonitor(const char *pModuleName, const char *pArgs);
	virtual ~CMonitor();

//	Interface routines

public:
	virtual void InitializeMachine(int nMachineID);
	virtual void TerminateMachine(int nMachineID, EVENTQUEUEINFO *pQueueInfo);
	virtual void DeliverEventQueue(UINT64 nReason, int nMachineID, EVENTQUEUEINFO *pQueueInfo);
	virtual void TimeCallback(int nMachineID);
	virtual UINT64 MonitorCallback(int nMachineID, UINT64 nTag);
	virtual void CodeInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector);
	virtual void MemoryInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector);
	virtual void ExcInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector);
	virtual void HeartBeatInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector);
	virtual void BranchInjection(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector);
	virtual void CodeTranslation(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, CODETRANSLATESTRUCT *pTranslateStruc);
	virtual void TaggedExecution(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, void *pParam);
	virtual void RunControl(int nMachineID, bool bStarting);
	virtual void RegisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo);
	virtual void UnregisterEventQueue(int nMachineID, EVENTQUEUEINFO *pQueueInfo);
	virtual bool ReturnNextCommand(int nMachineID, const char *psResponse, char *psCommand, int nBufferSize);
	virtual void ExternalEventTiming(EVENTTIMINGINFO *pInfo);
	virtual void QuantumEnd(int nMachineID);
	virtual void FrequencyCallBack(int nMachineID, UINT64 nDeviceID, CCodeInjector *pInjector, UINT32 mhz);

//	Internal routines

protected:
	void HandleNextToken(UINT64 nToken, EVENTQUEUEINFO *pQueueInfo);
	void DumpTokenList(UINT64 *pTokens, int nTokenCount, UINT64 nDeviceID, UINT64 nDeviceFlags);

//	Internal attributes

protected:
	int m_CommandIndex;
	static const char *m_CommandArray[];
};
