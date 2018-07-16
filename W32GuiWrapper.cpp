#include "stdafx.h"
#include "W32GuiWrapper.h"
#include <memory>
namespace simpleGui {
	void validateHandle(int handle, ustring errorMessage);
	void validateHandle(HANDLE handle, ustring errorMessage);
	ustring GetLastErrorAsString() {
		//Get the error message, if any.
		DWORD errorMessageID = GetLastError();
		if (errorMessageID == 0)
			return ustring(); //No error message has been recorded
		LPWSTR messageBuffer = nullptr;
		size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
		ustring message(messageBuffer, size);
		//Free the buffer.
		LocalFree(messageBuffer);
		return message;
	}

	ustring getTextFromWindow(HWND handle) {
		int count = GetWindowTextLengthW(handle);
		wchar_t*buffer = new wchar_t[count + 1];
		buffer[count] = 0;
		GetWindowText(handle, buffer, count + 1);

		ustring result = ustring(buffer, count);
		delete[] buffer;
		return result;
	}

	//template<typename ... Args>
	//wstring string_format(const std::wstring& format, Args ... args){
	//	size_t size = _snwprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	//	unique_ptr<wchar_t[]> buf(new wchar_t[size]);
	//	_snwprintf(buf.get(), size, format.c_str(), args ...);
	//	return wstring(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	//}
	

	static LRESULT CALLBACK W32GuiWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
		TopLevelWindow* me = (TopLevelWindow*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (me) return me->WndProc(hwnd, msg, wParam, lParam);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	int Window::componentIdCounter=0;
	void Window::setVisible(bool visible) {
		if (visible == false) {
			ShowWindow(handle, SW_HIDE);
		}
		else {
			ShowWindow(handle, SW_SHOW);
		}
	}

	void  Window::setPosition(int x, int y) {
		SetWindowPos(getHWND(),NULL,x,y,0,0,SWP_NOSIZE);
	}

	POINT  Window::getPosition() {
		RECT rect;
		GetWindowRect(getHWND(),&rect);
		POINT p;
		p.x = rect.left;
		p.y=rect.top;
		return p;
	}

	void  Window::setSize(int width, int height) {
		SetWindowPos(getHWND(), NULL, 0, 0, width, height, SWP_NOMOVE);
	}
	SIZE  Window::getSize() {
		RECT rect;
		GetWindowRect(getHWND(), &rect);
		SIZE p;
		p.cx = rect.right-rect.left;
		p.cy = rect.bottom-rect.top;
		return p;
	}

	void Window::setText(ustring text) {
		SetWindowText(getHWND(), text.c_str());
	}

	ustring Window::getText() {
		return getTextFromWindow(getHWND());
	}

	TopLevelWindow*Window::getTopLevelWindow() {
		Window*window=getParentWindow();
		if (window->isTopLevelWindow()) {
			return (TopLevelWindow*)window;
		}
		return window->getTopLevelWindow();
	}


	void validateHandle(int handle, ustring errorMessage) {
		if (handle == 0) {
			ustring error = GetLastErrorAsString();
			wchar_t buffer[2048];
			wsprintf(buffer, L"[%s] %s", errorMessage.c_str(), error.c_str());
			MessageBox(NULL,buffer,L"Error",0);
			exit(1);
		}
	}

	void validateHandle(HANDLE handle, ustring errorMessage) {
		validateHandle((int)handle, errorMessage);
	}

	void removeFromVector(vector<TopLevelWindow*>&v, int index) {
		v.erase(v.begin() + index);
	}

	TopLevelWindow::TopLevelWindow(HINSTANCE hInstance, Application*application) {
		WNDCLASSW wc;

		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.lpszClassName = L"Window";
		wc.hInstance = hInstance;
		wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		wc.lpszMenuName = NULL;
		wc.lpfnWndProc = W32GuiWndProc;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

		validateHandle(RegisterClassW(&wc), L"Try to register window class");
		HWND handle = CreateWindowW(wc.lpszClassName, L"Window",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			100, 100, 350, 250, NULL, (HMENU)getNextComponentId(), hInstance, NULL);
		validateHandle(handle, L"Try to create window");
		this->setHWND(handle);
		this->setApplication(application);
		SetWindowLong(handle, GWLP_USERDATA, (long)this);
		this->application = application;
	}

	LRESULT TopLevelWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_GETMINMAXINFO: {
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = minWidth;
			lpMMI->ptMinTrackSize.y = minHeight;
			return 1;
		}
		case WM_CLOSE: {
			return processCloseWindow();
		}
		case WM_MOVE:
		{
			int x = (int)LOWORD(lParam);
			int y = (int)HIWORD(lParam);
			processMoveWindow(x, y);
			return true;
		}
		case WM_SIZE:
		{
			int x = (int)LOWORD(lParam);
			int y = (int)HIWORD(lParam);
			processResizeWindow(x, y);
			return true;
		}
		case WM_HOTKEY: {
			if(hotkeys.find((int)wParam) != hotkeys.end()){
				function<void()>handler = hotkeys[(int)wParam];
				handler();
			}

			return true;
		}
		case WM_COMMAND: {
				int controlIdentifier = LOWORD(wParam);
				Window*window = getWindowByComponentId(controlIdentifier);
				if (window != NULL) {
					return window->processMessage(msg,wParam,lParam);
				}
				
				return true;
			}
		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void TopLevelWindow::processMoveWindow(int x, int y) {
		for (std::function<void(int x, int y)>onWindowMoveEvent : onWindowMoveHandlers) {
			onWindowMoveEvent(x, y);
		}
	}

	void TopLevelWindow::processResizeWindow(int width, int height) {
		for (std::function<void(int width, int height)>onWindowResizeEvent : onWindowResizeHandlers) {
			onWindowResizeEvent(width, height);
		}
	}

	void TopLevelWindow::addHotkey(int modifier, int key, function<void()>handler) {
		int hotkeyId = getNextComponentId();
		RegisterHotKey(getHWND(), hotkeyId, modifier, key);
		hotkeys[hotkeyId] = handler;
	}

	bool TopLevelWindow::processCloseWindow() {
		if (onCloseHandlers.empty()) {
			application->removeWindow(this);
			return true;
		}

		for (std::function<bool()>onCloseEvent : onCloseHandlers) {
			if (onCloseEvent()) {
				application->removeWindow(this);
				return true;
			}
		}

		return false;
	}

	void TopLevelWindow::flashWindow() {
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(fwi);
		fwi.dwFlags = FLASHW_ALL;
		fwi.dwTimeout = 0;
		fwi.hwnd = getHWND();
		fwi.uCount = 4;
		FlashWindowEx(&fwi);
	}

	void TopLevelWindow::updateWindow() {
		UpdateWindow(getHWND());
	}

	TopLevelWindow* Application::createWindow() {
		TopLevelWindow*window = new TopLevelWindow(instance,this);
		windows.push_back(window);
		return window;
	}


	TopLevelWindow* Application::findWindowByHwnd(HWND hwnd) {
		for (UINT i = 0; i < windows.size(); i++) {
			if (windows[i]->getHWND() == hwnd) {
				return windows[i];
			}
		}
		return NULL;
	}

	void Application::removeWindow(TopLevelWindow*window) {
		for (UINT i = 0; i < windows.size(); i++) {
			if (windows[i] == window) {
				removeFromVector(windows, i);
				return;
			}
		}
	}

	bool Application::processKeyMessage(MSG&msg,bool down) {
		for (TopLevelWindow*window : windows) {
			Window*w = window->getWindowByHWND(msg.hwnd);
			if (w != NULL) {
				for (function<bool(int key)>handler : down?w->getKeyDownHandlers(): w->getKeyUpHandlers()) {
					if (handler(msg.wParam)) {
						return true;
					}
				}
			}
		}
		return false;
	}

	void Application::start() {
		MSG  msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			if (msg.message == WM_KEYDOWN) {
				if (processKeyMessage(msg,true)) {
					continue;
				}
			} else
			if (msg.message == WM_KEYUP) {
				if (processKeyMessage(msg,false)) {
					continue;
				}
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (windows.empty()) {
				break;
			}
		}
	}
	Label::Label(Window*parent, ustring text) :Label(parent) {
		setText(text);
	}

	Label::Label(Window*parent) {
		ustring title = ustring(L"Label") + 0;
		int componentId=getNextComponentId();
		HWND handle = CreateWindowW(L"static", title.c_str(),
			WS_CHILD | WS_VISIBLE,
			40, 30, 55, 25,
			parent->getHWND(), (HMENU)componentId, parent->getApplication()->getInstance(), NULL);
		setHWND(handle);
		setApplication(parent->getApplication());
		setParentWindow(parent);
		parent->getChildren().push_back(this);
		getTopLevelWindow()->addWindowWithComponentId(componentId, this);
		getTopLevelWindow()->addWindowWithHWND(handle, this);
	}

	Button::Button(Window*parent) {
		ustring title = ustring(L"Button")+ 0;
		int componentId = getNextComponentId();
		HWND handle = CreateWindowW(L"button", title.c_str(),
			WS_CHILD | WS_VISIBLE| BS_PUSHBUTTON,
			40, 30, 55, 25,
			parent->getHWND(), (HMENU)componentId, parent->getApplication()->getInstance(), NULL);
		setHWND(handle);
		setApplication(parent->getApplication());
		setParentWindow(parent);
		parent->getChildren().push_back(this);
		getTopLevelWindow()->addWindowWithComponentId(componentId,this);
		getTopLevelWindow()->addWindowWithHWND(handle, this);
	}
	
	LRESULT Button::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND) {
			for (function<void(Button*button)>actionHandler : actionHandlers) {
				actionHandler(this);
			}
		}
		return true;
	}

	EditBox::EditBox(Window*parent,bool multiline) {
		int componentId = getNextComponentId();
		int style;
		if (multiline) {
			style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | ES_AUTOVSCROLL;
		}
		else {
			style = WS_CHILD | WS_VISIBLE | WS_BORDER |  ES_AUTOHSCROLL;
		}

		HWND handle = CreateWindowW(L"EDIT", L"",
			style,
			40, 30, 55, 90,
			parent->getHWND(), (HMENU)componentId, parent->getApplication()->getInstance(), NULL);
		setHWND(handle);
		setApplication(parent->getApplication());
		setParentWindow(parent);
		parent->getChildren().push_back(this);
		getTopLevelWindow()->addWindowWithComponentId(componentId, this);
		getTopLevelWindow()->addWindowWithHWND(handle, this);
	}

	SELECTIONINFO EditBox::getSelection() {
		int selStart = 0;
		int selEnd = 0;
		SendMessage(getHWND(), EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
		SELECTIONINFO si;
		si.start = selStart;
		si.end = selEnd;
		return si;
	}

	void EditBox::setSelection(int selectionStart,int selectionEnd) {
		SendMessage(getHWND(),EM_SETSEL,(WPARAM)selectionStart,(LPARAM)selectionEnd);
	}

	void EditBox::appendText(ustring&string, bool scrollToBottom) {
		SendMessage(getHWND(), WM_SETREDRAW, false, 0);
		SELECTIONINFO selectionInfo = getSelection();
		int size=GetWindowTextLength(getHWND());
		
		setSelection(size,size);
		replaceSelection(string);
		
		if (size > 20000) {
			int removeLength = 1000;
			setSelection(0,removeLength);
			replaceSelection(ustring(L""));
			selectionInfo.start -= removeLength;
			selectionInfo.end -= removeLength;
		}

		size = GetWindowTextLength(getHWND());
		
		SendMessage(getHWND(), WM_SETREDRAW, true, 0);
		if (scrollToBottom) {
			setSelection(size, size);
			SendMessage(getHWND(), EM_SCROLLCARET, 0, 0);
		}
		else {
			setSelection(selectionInfo.start, selectionInfo.end);
		}
	}

	void EditBox::replaceSelection(ustring&string) {
		SendMessageW(getHWND(), EM_REPLACESEL, 0, (LPARAM)string.c_str());
	}

	LRESULT EditBox::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND) {
			ustring message = ustring(L"WM_COMMAND ").appendAsHex(msg).add(L" WPARAM=").appendAsHex(wParam).add(L" LPARAM=").appendAsHex(lParam).add(L"\n");
			OutputDebugString(message.c_str());
		}
		return true;
	}

	CheckBox::CheckBox(Window*parent) {
		int componentId = getNextComponentId();

		HWND handle = CreateWindowW(L"BUTTON", L"CheckBox",
			WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
			40, 30, 55, 25,
			parent->getHWND(), (HMENU)componentId, parent->getApplication()->getInstance(), NULL);
		validateHandle(handle, L"Try to create checkbox");
		setHWND(handle);
		setApplication(parent->getApplication());
		setParentWindow(parent);
		parent->getChildren().push_back(this);
		getTopLevelWindow()->addWindowWithComponentId(componentId, this);
		getTopLevelWindow()->addWindowWithHWND(handle, this);
	}

	bool CheckBox::isChecked() {
		UINT t = SendMessage(getHWND(), BM_GETCHECK, 0, 0);
		return t != 0;
	}

	void CheckBox::setChecked(bool state) {
		SendMessage(getHWND(),BM_SETCHECK,state==true?BST_CHECKED:BST_UNCHECKED,0);
	}

	LRESULT CheckBox::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_COMMAND) {
			setChecked(!isChecked());
		}
		return true;
	}
}