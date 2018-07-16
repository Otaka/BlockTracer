#pragma once
#include "stdafx.h"

class DebugString{
public:
	wstring string;
	long timestamp;
	int debugStringIndex;
};

class Debugger
{
private:
	HANDLE hProcess;
	DWORD processId;
	HANDLE mainThread;
	bool isRunFlag = false;
	unsigned long startAddress;
	wstring exePath;
	vector<DebugString>debugStrings;
	vector<LOAD_DLL_DEBUG_INFO>loadedDlls;
	CREATE_PROCESS_DEBUG_INFO processDebugInfo;
	unsigned int maxDebugStrings =50;
	unsigned int debugStringIndex;
	vector<CREATE_THREAD_DEBUG_INFO>threads;
	vector<LOAD_DLL_DEBUG_INFO>dlls;

private:
	void processEvent(DEBUG_EVENT&debugEvent);
	void processDebugString(OUTPUT_DEBUG_STRING_INFO&debugString);
	void processCreateProcess(CREATE_PROCESS_DEBUG_INFO&createProcessEvent);
	void processCreateThread(CREATE_THREAD_DEBUG_INFO&createThreadEvent);
	void processLoadDll(LOAD_DLL_DEBUG_INFO&loadDllEvent);
	void processUnLoadDll(UNLOAD_DLL_DEBUG_INFO&unloadDllEvent);
	void processExitThread(EXIT_THREAD_DEBUG_INFO&exitThreadEvent,HANDLE hThread);

public:
	Debugger();
	~Debugger();
	void startProcess(wstring&processName, wstring&arguments);
	void processEvents(long waitingTime);
	void run();
	void pause();
	bool detach();

	unsigned long getStartAddress() {
		return startAddress;
	}

	HANDLE getProcessHandle() {
		return hProcess;
	}

	DWORD getProcessId() {
		return processId;
	}

	HANDLE getMainThreadHandle() {
		return mainThread;
	}

	bool isRun() {
		return isRunFlag;
	}

	wstring&getExePath() {
		return exePath;
	}

	CREATE_PROCESS_DEBUG_INFO&getProcessDebugInfo() {
		return processDebugInfo;
	}

	vector<DebugString>& getDebugStrings() {
		return debugStrings;
	}

	vector<CREATE_THREAD_DEBUG_INFO>& getThreads() {
		return threads;
	}

	vector<LOAD_DLL_DEBUG_INFO>& getDlls() {
		return dlls;
	}
};