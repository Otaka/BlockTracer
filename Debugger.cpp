#include "stdafx.h"
#include "Debugger.h"

Debugger::Debugger(){
}

Debugger::~Debugger(){
}

bool Debugger::detach() {
	return DebugActiveProcessStop((DWORD)hProcess);
}

void Debugger::startProcess(wstring&processName,wstring&arguments) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	LPWSTR args = NULL;
	if (arguments.length() != 0) {
		args=(LPWSTR)arguments.c_str();
	}

	BOOL result=CreateProcess(processName.c_str(), args, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS|CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (!result) {
		wcout<<L"Error while trying to start process ";
		wcout << processName<<"\n";
		cout << getWindowsLastErrorAsString();
		wcout << "\n";
	}

	hProcess=pi.hProcess;
	processId = pi.dwProcessId;
	mainThread = pi.hThread;
	isRunFlag = true;
	
}

DEBUG_EVENT lastDebugEvent = { 0 };

void Debugger::processEvents(long waitingTime) {
	for (;;) {
		if (WaitForDebugEvent(&lastDebugEvent,waitingTime)) {
			processEvent(lastDebugEvent);
			if (isRunFlag) {
				ContinueDebugEvent(lastDebugEvent.dwProcessId,
					lastDebugEvent.dwThreadId,
					DBG_CONTINUE);
			}
		}
	}
}

void Debugger::processEvent(DEBUG_EVENT&debugEvent) {
	switch (debugEvent.dwDebugEventCode) {
	case OUTPUT_DEBUG_STRING_EVENT:
		processDebugString(debugEvent.u.DebugString);
		break;
	case CREATE_PROCESS_DEBUG_EVENT:
		processCreateProcess(debugEvent.u.CreateProcessInfo);
		break;
	case CREATE_THREAD_DEBUG_EVENT:
		processCreateThread(debugEvent.u.CreateThread);
		break;
	case LOAD_DLL_DEBUG_EVENT:
		processLoadDll(debugEvent.u.LoadDll);
		break;
	case UNLOAD_DLL_DEBUG_EVENT:
		processUnLoadDll(debugEvent.u.UnloadDll);
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		processExitThread(debugEvent.u.ExitThread,(HANDLE)debugEvent.dwThreadId);
		break;
	}
}

void readProcessMemory(HANDLE hProcess, void*pointer, int size, BYTE*buffer) {
	SIZE_T bytesRead = 0;
	if (!ReadProcessMemory(hProcess, pointer, buffer, size, &bytesRead)) {
		printf("Error, cannot read process memory");
		throw "error";
	}
}

wstring readStringFromProcess(HANDLE hProcess, void*pointer, int size, bool unicode) {
	if (unicode) {
		int length = size*2;
		BYTE*buffer = new BYTE[length];
		readProcessMemory(hProcess, pointer, length, buffer);
		wstring debugStringValue((wchar_t*)buffer);
		delete[]buffer;
		return debugStringValue;
	}
	else {
		int length = size;
		BYTE*buffer = new BYTE[length];
		readProcessMemory(hProcess, pointer, length, buffer);
		const wchar_t*wideCharBuffer = GetWC((char*)buffer);
		wstring debugStringValue(wideCharBuffer);
		delete[]buffer;
		delete[]wideCharBuffer;
		return debugStringValue;
	}
}

void Debugger::processDebugString(OUTPUT_DEBUG_STRING_INFO&debugStringInfo) {
	DebugString debugString;
	debugString.timestamp = (long)time(NULL);
	debugString.string = readStringFromProcess(hProcess,debugStringInfo.lpDebugStringData,debugStringInfo.nDebugStringLength,debugStringInfo.fUnicode);
	debugString.debugStringIndex = debugStringIndex++;
	debugStrings.push_back(debugString);
	if (debugStrings.size() > maxDebugStrings) {
		debugStrings.erase(debugStrings.begin());
	}
}

void testDistorm(void* startAddress, HANDLE hProcess) {
	BYTE testBuffer[512];
	readProcessMemory(hProcess, startAddress, 512, testBuffer);
	
	_DecodeResult res;
	_DecodedInst decodedInstructions[1024];
	unsigned int decodedInstructionsCount = 0, i, next;
	_DecodeType dt = Decode32Bits;
	_OffsetType offset = 0;
	char* errch = NULL;
	// Index to file name in argv.
	int param = 1;
	int dver = distorm_version();
	printf("diStorm version: %d.%d.%d\n", (dver >> 16), ((dver) >> 8) & 0xff, dver & 0xff);
	res = distorm_decode(offset, (const unsigned char*)testBuffer, 512, dt, decodedInstructions, 512, &decodedInstructionsCount);
}

void Debugger::processCreateProcess(CREATE_PROCESS_DEBUG_INFO&createProcessEvent) {
	processDebugInfo = createProcessEvent;
	startAddress = (unsigned long)processDebugInfo.lpStartAddress;
	exePath = GetFileNameFromHandle(createProcessEvent.hFile);
	testDistorm((void*)startAddress, hProcess);
}

void Debugger::processCreateThread(CREATE_THREAD_DEBUG_INFO&createThreadEvent) {
	threads.push_back(createThreadEvent);
}

void Debugger::processLoadDll(LOAD_DLL_DEBUG_INFO&loadDllEvent) {
	dlls.push_back(loadDllEvent);
}

void Debugger::processUnLoadDll(UNLOAD_DLL_DEBUG_INFO&unloadDllEvent) {
	for (int i = 0; i < dlls.size(); i++) {
		LOAD_DLL_DEBUG_INFO&ld = dlls.at(i);
		if(ld.lpBaseOfDll==unloadDllEvent.lpBaseOfDll){
			dlls.erase(dlls.begin() + i);
			break;
		}
	}
}

void Debugger::processExitThread(EXIT_THREAD_DEBUG_INFO&exitThreadEvent, HANDLE hThread) {
	for (int i = 0; i < threads.size(); i++) {
		CREATE_THREAD_DEBUG_INFO&thread = threads.at(i);
		if (thread.hThread == hThread) {
			dlls.erase(dlls.begin() + i);
			break;
		}
	}
}