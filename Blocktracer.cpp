#include "stdafx.h"
#include "BlockTracer.h"


#pragma comment(lib, "distorm3.lib")


Debugger*debugger;

void runDebugger() {
	wstring fileName = L"f:\\java\\cpp\\OutputDebugSender\\Release\\OutputDebugSender.exe";
	wstring args = L"";
	debugger = new Debugger();
	debugger->startProcess(fileName, args);
	debugger->processEvents(1000);
}

void runDebuggerThread() {
	thread*thr = new thread(runDebugger);
	thr = NULL;
}

void showHelp() {
	wcout << L"Tracer\n";
	wcout << L"Commands:\n";
	wcout << L"'app info' - Show information about debugging process.\n";
	wcout << L"'exit' or 'quit' - close the application. This command also immidiately terminates debugging application.\n";
	wcout << L"'detach' - detach from currently debugging application.\n";
	wcout << L"'debug strings' or 'ds' - show strings that was printed by debugging application with OutputDebugString function.\n";
	wcout << L"'threads' - list of threads running in debugging application.\n";
	wcout << L"'dlls' - list of loaded dlls. You can also specify optional query to filter dlls. Like \"dlls kernel\"\n";
}

void showDebugStrings() {
	if (debugger == NULL) {
		wcout << L"No attached process\n";
		return;
	}

	vector<DebugString>debugStrings = debugger->getDebugStrings();
	if (debugStrings.size() == 0) {
		wcout << "No debug strings\n";
		return;
	}
	wcout << L"Index" << L"\t" << L"Time" << L"\t" << L"String" << "\n";
	for (int i = debugStrings.size() - 1; i >= 0; i--) {
		DebugString debugString = debugStrings.at(i);
		wcout << debugString.debugStringIndex << L"\t" << timestampToString(debugString.timestamp) << L"\t" << debugString.string << "\n";
	}
}

void showAppInfo() {
	if (debugger == NULL) {
		wcout << L"No attached process\n";
		return;
	}
	wcout << "Process information:\n";
	wcout << "Exe Path          :" << debugger->getExePath() << "\n";
	wcout << "Process Handle    :" << debugger->getProcessHandle() << "\n";
	wcout << "Process Id        :" << debugger->getProcessId() << "\n";
	wcout << "Main Thread Handle:" << debugger->getMainThreadHandle() << "\n";
	wcout << "Entry Point       :" << (void *)debugger->getStartAddress() << "\n";
	wcout << "Dlls count        :" << debugger->getDlls().size() << "\n";
	wcout << "Threads count     :" << debugger->getThreads().size() << "\n";
}

void showDlls(wstring query) {
	if (debugger == NULL) {
		wcout << L"No attached process\n";
		return;
	}

	if (debugger->getDlls().size() == 0) {
		wcout << L"No loaded dlls\n";
		return;
	}
	query = trim(query);
	wcout << L"Loaded dlls";
	if (query.size() > 0) {
		wcout << L"[" << query << L"]";
	}
	wcout << L":\n";
	for (UINT i = 0; i < debugger->getDlls().size(); i++) {
		LOAD_DLL_DEBUG_INFO&loadDllDebugEventInfo = debugger->getDlls().at(i);
		wstring imagePath = GetFileNameFromHandle(loadDllDebugEventInfo.hFile);
		if (imagePath.find(query) != string::npos) {
			wcout << L"Dll name:        " << imagePath << L"\n";
			wcout << L"Base dll address:" << loadDllDebugEventInfo.lpBaseOfDll << L"\n";
			wcout << L"Debug info size: " << loadDllDebugEventInfo.nDebugInfoSize << L"\n";
			wcout << L"\n";
		}
	}
}

void showThreads() {
	if (debugger == NULL) {
		wcout << L"No attached process\n";
		return;
	}

	if (debugger->getThreads().size() == 0) {
		wcout << L"No child threads\n";
		return;
	}
	wcout << "Child threads:\n";
	for (UINT i = 0; i < debugger->getThreads().size(); i++) {
		CREATE_THREAD_DEBUG_INFO&threadInfo = debugger->getThreads().at(i);
		wcout << L"Thread handle       :" << threadInfo.hThread << L"\n";
		wcout << L"Thread start address:" << threadInfo.lpStartAddress << L"\n";
		wcout << L"\n";
	}
}

bool processUserInput(wstring&input) {
	wstring dllsString = L"dlls";
	wstring threadsString = L"threads";

	if (input == L"") {
		//skip
	}
	else
		if (input == L"detach") {
			if (debugger == NULL) {
				wcout << L"No attached process";
			}
			else {
				if (debugger->detach()) {
					delete debugger;
					debugger = NULL;
				}
				else {
					cout << "Error:" << getWindowsLastErrorAsString();
				}
			}
		}
		else
			if (input == L"quit" || input == L"exit") {
				return false;
			}
			else if (input == L"debug strings" || input == L"ds") {
				showDebugStrings();
			}
			else if (input == L"app info") {
				showAppInfo();
			}
			else if (input == L"help") {
				showHelp();
			}
			else if (startWith(input, dllsString)) {
				wstring query = input.substr(dllsString.size(), input.size() - dllsString.size());
				showDlls(query);
			}
			else if (input == L"threads") {
				showThreads();
			}
			else {
				wcout << "Unknown command [" << input << "]\n";
			}
			return true;
}

int getWidthHeight(ustring&str, int totalSize) {
	int value;
	if (str.indexOf('%') != -1) {
		ustring onlyNumber = str.extractBefore('%');
		value = (int)(onlyNumber.asInt() / 100.0f*totalSize);
	}
	else if (str.indexOf('w') != -1) {
		ustring onlyNumber = str.extractBefore('w');
		return onlyNumber.asInt();
	}
	else if (str.indexOf('h') != -1) {
		ustring onlyNumber = str.extractBefore('h');
		return onlyNumber.asInt();
	}
	else {
		value = str.asInt();
	}
	if (value < 0) {
		value = value*-1;
		value = totalSize - value;
	}

	return value;
}

void layoutWindow(TopLevelWindow*window) {
	RECT rect = window->getClientRect();
	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;
	for (Window*children : window->getChildren()) {
		ustring*placement = (ustring*)children->getPlacementInfo();
		if (placement != NULL) {
			vector<ustring>splitted = placement->split((wchar_t)',');
			if (splitted.size() != 4) {
				ustring message;
				message.add(L"Placement string should contain 4 elements, but it contains ").add(splitted.size()).add(L". Full string \"").add(*placement).add(L"\"");
				MessageBoxW(NULL, message.c_str(), L"error", 0);
			}

			int x = getWidthHeight(splitted[0], windowWidth);
			int y = getWidthHeight(splitted[1], windowHeight);

			int x2;
			int y2;

			x2 = getWidthHeight(splitted[2], windowWidth);
			if (splitted[2].indexOf('w') != -1) {
				x2 = x + x2;
			}

			y2 = getWidthHeight(splitted[3], windowHeight);
			if (splitted[3].indexOf('h') != -1) {
				y2 = y + y2;
			}

			children->setPosition(x, y);
			children->setSize(x2 - x, y2 - y);
		}
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	Application application(hInstance);
	TopLevelWindow*window = application.createWindow();
	window->setMinWidthHeight(300, 300);

	Label*logLabel = new Label(window, L"Log");
	logLabel->setPlacementInfo(new ustring(L"2,2,-2,20h"));

	EditBox*logEditBox = new EditBox(window, true);
	logEditBox->setPlacementInfo(new ustring(L"2,22,-2,-45"));

	Label*commandLabel = new Label(window, L"Command:");
	commandLabel->setPlacementInfo(new ustring(L"2,-45,-2,20h"));

	EditBox*commandEditBox = new EditBox(window, false);
	commandEditBox->setPlacementInfo(new ustring(L"2,-25,-2,-5"));
	layoutWindow(window);
	commandEditBox->addKeyDownHandler([commandEditBox](int key) {
		if (key == VK_RETURN) {
			commandEditBox->setEnabled(false);
			return true;
		}
		return false;
	});

	window->addOnWindowResizeHandler([window](int width, int height) {
		layoutWindow(window);
	});

	application.start();
	//runDebuggerThread();
	//mainLoop();
	return 0;
}