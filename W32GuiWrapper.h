#pragma once
#include<vector>
#include<map>
#include<functional>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
namespace simpleGui {
	using namespace std;
	template<typename ... Args>
	
	static LRESULT CALLBACK W32GuiWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	class Application;
	class Window;
	class TopLevelWindow;

	class ustring :public wstring {
	public:
		ustring() {
		}
		ustring(const wchar_t*buff) {
			assign(buff);
		}
		ustring(const wchar_t*buff,int size):wstring(buff,size) {
		}

		ustring ustring::operator=(const ustring & rhs) {
			if (this == &rhs)
				return *this;
			*this = rhs;
			return *this;
		}
		ustring ustring::operator=(const wchar_t * rhs) {
			this->assign(rhs);
			return *this;
		}

		ustring ustring::operator+(const wchar_t * rhs) {
			this->append(rhs);
			return *this;
		}

		ustring ustring::operator+(const ustring&rhs) {
			this->append(rhs);
			return *this;
		}

		ustring& add(wchar_t*data) {
			this->append(data);
			return *this;
		}

		ustring& add(ustring&data) {
			this->append(data);
			return *this;
		}

		ustring& add(int value) {
			this->append(std::to_wstring(value));
			return *this;
		}
		ustring& add(unsigned int value) {
			this->append(std::to_wstring(value));
			return *this;
		}
		ustring& add(long value) {
			this->append(std::to_wstring(value));
			return *this;
		}
		ustring& add(unsigned long value) {
			this->append(std::to_wstring(value));
			return *this;
		}
		ustring& add(float value) {
			this->append(std::to_wstring(value));
			return *this;
		}
		ustring& add(double value) {
			this->append(std::to_wstring(value));
			return *this;
		}

		ustring ustring::operator+(const int  rhs) {
			wstring w = std::to_wstring(rhs);
			this->append(w);
			return *this;
		}

		ustring clone() {
			return c_str();
		}

		int indexOf(wchar_t character) {
			const wchar_t*buffer = c_str();
			for (int i = 0; buffer[i] != 0; i++) {
				if (buffer[i] == character) {
					return i;
				}
			}

			return -1;
		}

		int asInt() {
			return _wtoi(c_str());
		}

		int asLong() {
			return _wtol(c_str());
		}

		int asFloat() {
			return _wtof(c_str());
		}

		ustring extractBefore(wchar_t character) {
			int index = indexOf(character);
			if (index == -1) {
				return ustring(this->c_str());
			}

			return ustring(substr(0, index).c_str());
		}

		ustring ustring::operator+(const long  rhs) {
			wstring w = std::to_wstring(rhs);
			this->append(w);
			return *this;
		}

		ustring ustring::operator+(const float rhs) {
			wstring w = std::to_wstring(rhs);
			this->append(w);
			return *this;
		}

		ustring& appendAsHex(int value) {
			wchar_t buffer[50];
			wsprintf(buffer,L"0x%x",value);
			append(buffer);
			return *this;
		}

		vector<ustring>split(wchar_t separator) {
			const wchar_t*buffer=c_str();
			int wordStart = 0;
			vector<ustring>result;
			int i = 0;
			for (; buffer[i] != 0; i++) {
				wchar_t c = buffer[i];
				if (c == separator) {
					if (i != wordStart) {
						ustring str(substr(wordStart,i-wordStart).c_str());
						result.push_back(str);
						wordStart = i+1;
					}
				}
			}

			if (i != wordStart) {
				ustring str(substr(wordStart, i - wordStart).c_str());
				result.push_back(str);
			}
			return result;
		}
	};
	
	class Window {
	private:
		static int componentIdCounter;
		vector<Window*>children;
		HWND handle;
		Application*application;
		Window*parent;
		vector<std::function<bool(int key)>> onKeyDownHandlers;
		vector<std::function<bool(int key)>> onKeyUpHandlers;
		void*placementInfo;
	public:
		void setEnabled(bool enabled) {
			EnableWindow(handle,enabled);
		}
		void*getPlacementInfo() {
			return placementInfo;
		}
		void setPlacementInfo(void*placementInfo) {
			this->placementInfo = placementInfo;
		}
		void addKeyDownHandler(std::function<bool(int key)>keyDownHandler) {
			onKeyDownHandlers.push_back(keyDownHandler);
		}
		void addKeyUpHandler(std::function<bool(int key)>keyUpHandler) {
			onKeyUpHandlers.push_back(keyUpHandler);
		}
		vector<std::function<bool(int key)>>& getKeyDownHandlers() {
			return onKeyDownHandlers;
		}
		vector<std::function<bool(int key)>>& getKeyUpHandlers() {
			return onKeyUpHandlers;
		}
		virtual bool isTopLevelWindow() {
			return false;
		}
		int getNextComponentId() {
			return componentIdCounter++;
		}

		

		void setParentWindow(Window*window) {
			this->parent = window;
		}

		Window*getParentWindow() {
			return this->parent;
		}

		void setApplication(Application*application) {
			this->application = application;
		}

		Application*getApplication() {
			return this->application;
		}

		RECT getClientRect() {
			RECT rect;
			GetClientRect(this->handle,&rect);
			return rect;
		}

		vector<Window*>&getChildren() {
			return children;
		};
		virtual void setHWND(HWND handle) {
			this->handle = handle;
		}
		virtual HWND getHWND() {
			return handle;
		}
		virtual void setPosition(int x, int y);
		virtual POINT getPosition();
		virtual void setSize(int width, int height);
		virtual SIZE getSize();
		void setText(ustring text);
		ustring getText();
		virtual void setVisible(bool visible);
		virtual TopLevelWindow*getTopLevelWindow();
		virtual LRESULT processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		//empty. Should be overriden in children
			return true;
		};
	};

	
	class Label :public Window {
	public:
		Label(Window*parent);
		Label(Window*parent,ustring text);
	};

	class Button :public Window {
		vector<function<void(Button*button)>>actionHandlers;
	public:
		Button(Window*parent);
		void addActionListener(function<void(Button*button)>actionHandler) {
			actionHandlers.push_back(actionHandler);
		}
		virtual LRESULT processMessage(UINT msg, WPARAM wParam, LPARAM lParam);
	};
	struct SELECTIONINFO {
		int start;
		int end;
	};

	class EditBox :public Window {
	public:
		EditBox(Window*parent,bool multiline);
		virtual LRESULT processMessage(UINT msg, WPARAM wParam, LPARAM lParam);
		void appendText(ustring&value, bool scrollToBottom);
		SELECTIONINFO getSelection();
		void setSelection(int selectionStart,int selectionEnd);
		void replaceSelection(ustring&value);
	};

	class CheckBox :public Window {
	public:
		CheckBox(Window*parent);
		virtual LRESULT processMessage(UINT msg, WPARAM wParam, LPARAM lParam);
		bool isChecked();
		void setChecked(bool state);
	};

	class TopLevelWindow:public Window {
	private:
		Application*application;
		vector<std::function<bool()>> onCloseHandlers;
		vector<std::function<void(int x, int y)>> onWindowMoveHandlers;
		vector<std::function<void(int width, int height)>> onWindowResizeHandlers;
		virtual bool processCloseWindow();
		virtual void processMoveWindow(int x, int y);
		virtual void processResizeWindow(int x, int y);
		map<int, Window*>componentIdToWindowMap;
		map<HWND, Window*>HWNDToWindowMap;
		map<int, function<void()>>hotkeys;
		int minWidth;
		int minHeight;
	public:
		void setMinWidthHeight(int width, int height) {
			minWidth = width;
			minHeight = height;
		}

		TopLevelWindow(HINSTANCE hInstance,Application*application);
		LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		Window*getWindowByComponentId(int id) {
			Window*window=componentIdToWindowMap[id];
			return window;
		}

		Window*getWindowByHWND(HWND hwnd) {
			Window*window = HWNDToWindowMap[hwnd];
			return window;
		}

		virtual bool isTopLevelWindow() {
			return true;
		}

		void addWindowWithComponentId(int id,Window*window) {
			componentIdToWindowMap[id]= window;
		}

		void addWindowWithHWND(HWND handle, Window*window) {
			HWNDToWindowMap[handle] = window;
		}

		virtual void addOnCloseHandler(std::function<bool()>onCloseEvent) {
			onCloseHandlers.push_back(onCloseEvent);
		}

		virtual void addOnWindowMoveHandler(std::function<void(int x, int y)>onWindowMove) {
			onWindowMoveHandlers.push_back(onWindowMove);
		}

		virtual void addOnWindowResizeHandler(std::function<void(int width, int height)>onWindowResize) {
			onWindowResizeHandlers.push_back(onWindowResize);
		}

		void addHotkey(int modifier,int key, function<void()>handler);

		virtual void flashWindow();
		virtual void updateWindow();
	};

	class Application {
	private:
		HINSTANCE instance;
		vector<TopLevelWindow*>windows;
		bool processKeyMessage(MSG&msg,bool down);
	public:
		Application(HINSTANCE instance) {
			this->instance = instance;
		}

		HINSTANCE getInstance() {
			return instance;
		}
		void removeWindow(TopLevelWindow*window);
		TopLevelWindow* findWindowByHwnd(HWND hwnd);
		TopLevelWindow*createWindow();
		void start();
	};
}