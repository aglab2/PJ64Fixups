#include <ui.h>

#include <windows.h>
#include <ui_windows.h>

#include "config.h"
#include "hotkey.h"
#include "rsp.h"
#include "version.h"

#include <stdlib.h>
#include <string>
#include <memory>

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace UI
{
	static const std::string DefaultLine = std::string(22, ' ');
	static constexpr int MaxHotKeys = 3;

	class Window
	{
	public:
		Window(HWND hwnd) : hwnd_(hwnd)
		{
			initOnce();
			EnableWindow(hwnd_, FALSE);
		}

		virtual ~Window()
		{
			EnableWindow(hwnd_, TRUE);
			SetActiveWindow(hwnd_);
		}

	protected:
		HWND hwnd_;

		void show(uiWindow* window)
		{
			uiWindowOnClosing(window, [](uiWindow* w, void* data) { ((Window*) data)->onClosing(); return 1; }, this);
			uiControlShow(uiControl(window));
		}

		virtual void onClosing()
		{
			uiQuit();
		}

	private:
		static void initOnce()
		{
			static bool init = false;
			if (!init)
			{
				uiInitOptions o{ };
				const char* s = uiInit(&o);
				if (s != NULL)
					abort();

				init = true;
			}
		}
	};

	class InputWindow : public Window
	{
	public:
		InputWindow(HWND hwnd, uiButton* button)
			: Window(hwnd)
			, button_(button)
		{
			window_ = uiNewWindow("Input", 20, 20, 0, hwnd_);
			uiWindowSetMargined(window_, true);
			uiWindowSetResizeable(window_, false);

			auto main = uiNewHorizontalBox();
			uiBoxSetPadded(main, 1);
			uiWindowSetChild(window_, uiControl(main));
			uiWindowSetKeyEvents(window_
		        			  , [](auto self, auto wparam, auto lparam) { return ((InputWindow*)self)->WM_KeyDown(wparam, lparam); }
							  , nullptr
							  , this);

			auto label = uiNewLabel("Press key to record...\nPress Space Key to erase\nClose window to ignore\n");
			uiBoxAppend(main, uiControl(label), false);

			show(window_);
		}

		void WM_KeyDown(WPARAM wParam, LPARAM lParam)
		{
			if (HotKey::virtualCodeAllowed(wParam))
			{
				if (VK_SPACE == wParam)
				{
					uiButtonSetText(button_, DefaultLine.c_str());
				}
				else
				{
					bool ctrl = !!(GetKeyState(VK_LCONTROL) & 0x8000);
					bool shift = !!(GetKeyState(VK_LSHIFT) & 0x8000);
					bool alt = !!(GetKeyState(VK_LMENU) & 0x8000);
					HotKey hk{ (ctrl ? HK_MODIFIED_CTRL : 0) | (alt ? HK_MODIFIED_ALT : 0) | (shift ? HK_MODIFIED_SHIFT : 0), wParam };
					uiButtonSetText(button_, hk.encode().c_str());
				}

				uiWindowClose(window_);
			}
		}

	private:
		uiButton* button_;
		uiWindow* window_;
	};

	class MainWindow : public Window
	{
	public:
		explicit MainWindow(HWND hParent) : Window(hParent)
		{
			window_ = uiNewWindow("PJ64 Fixups " VERSION, 200, 200, 0, hwnd_);
			uiWindowSetMargined(window_, true);
			uiWindowSetResizeable(window_, false);

			auto main = uiNewHorizontalBox();
			uiBoxSetPadded(main, 1);
			uiWindowSetChild(window_, uiControl(main));

			{
				auto group = uiNewGroup("General");
				auto box = uiNewVerticalBox();
				uiGroupSetChild(group, uiControl(box));
				auto label = uiNewLabel("[!] Restart PJ64 after changing configs [!]");
				uiBoxAppend(box, uiControl(label), false);

#define CONFIG(name, desc) addCheckbox(box, name##_ = uiNewCheckbox(desc), &Config::get().name);
#include "xconfig.h"
#undef CONFIG

				auto inputDelayGrid = uiNewGrid();

				auto inputDelayLabel = uiNewLabel("Input delay in milliseconds");
				uiGridAppend(inputDelayGrid, uiControl(inputDelayLabel), 0, 0, 1, 1, 0, uiAlignStart, 0, uiAlignCenter);

				inputDelay_ = uiNewEntry();
				uiEntryOnChanged(inputDelay_, [](auto cb, auto modifyingCb) { *((int*)modifyingCb) = std::atoi(uiEntryText(cb)); }, &Config::get().inputDelay);
				uiGridAppend(inputDelayGrid, uiControl(inputDelay_), 1, 0, 1, 1, 0, uiAlignStart, 0, uiAlignCenter);
				uiEntrySetText(inputDelay_, std::to_string(Config::get().inputDelay).c_str());

				uiBoxAppend(box, uiControl(inputDelayGrid), false);

				auto openOrig = uiNewButton("Open original RSP configs");
				uiButtonOnClicked(openOrig, [](auto button, auto data) { ((MainWindow*)data)->openOriginalRSPConfig(); }, this);
				uiBoxAppend(box, uiControl(openOrig), false);

				uiBoxAppend(main, uiControl(group), false);
			}
			{
				auto group = uiNewGroup("General shortcuts");
				auto grid = uiNewGrid();
				uiGroupSetChild(group, uiControl(grid));

				int counter = 0;
#define HOTKEY(row, view, name, cmd, desc) if (0 == row) addHotKey(counter, name##_, grid, desc, Config::get().name);
#include "xhotkeys.h"
#undef HOTKEY

				uiBoxAppend(main, uiControl(group), false);
			}
			{
				auto group = uiNewGroup("Savestate shortcuts");
				auto grid = uiNewGrid();
				uiGroupSetChild(group, uiControl(grid));

				int counter = 0;
#define HOTKEY(row, view, name, cmd, desc) if (1 == row) addHotKey(counter, name##_, grid, desc, Config::get().name);
#include "xhotkeys.h"
#undef HOTKEY

				uiBoxAppend(main, uiControl(group), false);
			}

			show(window_);
		}

	private:
		uiWindow* window_;
#define CONFIG(name, desc) uiCheckbox* name##_;
#include "xconfig.h"
#undef CONFIG
		uiEntry* inputDelay_;

#define HOTKEY(row, view, name, cmd, desc) uiButton* name##_[MaxHotKeys];
#include "xhotkeys.h"
#undef HOTKEY

		void addCheckbox(uiBox* b, uiCheckbox* cb, bool* modifying)
		{
			uiCheckboxOnToggled(cb, [](auto cb, auto modifyingCb) { *((bool*)modifyingCb) = uiCheckboxChecked(cb); }, modifying);
			uiBoxAppend(b, uiControl(cb), false);
			uiCheckboxSetChecked(cb, *modifying);
		}

		uiButton* makeHotKeyButton(const char* text)
		{
			auto button = uiNewButton(text);
			uiButtonOnClicked(button, [](auto button, auto self) { ((MainWindow*)self)->openButtonConfig(button); }, this);
			return button;
		}

		void addHotKey(int& counter, uiButton** buttons, uiGrid* grid, const char* desc, const std::vector<HotKey>& keys)
		{
			int val = counter++;
			uiGridAppend(grid, uiControl(uiNewLabel(desc)), 0, val, 1, 1, 0, uiAlignStart, 0, uiAlignCenter);
			for (int i = 0; i < MaxHotKeys; i++)
			{
				std::string text = i < keys.size() ? keys[i].encode() : DefaultLine;
				uiGridAppend(grid, uiControl(buttons[i] = makeHotKeyButton(text.c_str())), 1 + i, val, 1, 1, 1, uiAlignFill, 0, uiAlignFill);
			}
		}

		void openOriginalRSPConfig()
		{
			RSP::gDllConfig(uiWindowGetHWND(window_));
		}

		void openButtonConfig(uiButton* button)
		{
			InputWindow wnd(uiWindowGetHWND(window_), button);
			uiMain();
		}

		virtual void onClosing() override
		{
#define HOTKEY(row, view, name, cmd, desc) Config::get().name = createHotKeyConfig(name##_);
#include "xhotkeys.h"
#undef HOTKEY

			Config::get().save();
			Window::onClosing();
		}

		std::vector<HotKey> createHotKeyConfig(uiButton** buttons)
		{
			std::vector<HotKey> keys;
			for (int i = 0; i < MaxHotKeys; i++)
			{
				auto button = buttons[i];
				std::string_view buttonText = uiButtonText(button);
				if (buttonText != DefaultLine)
					keys.push_back(HotKey::decode(buttonText));
			}

			return keys;
		}
	};

	static std::unique_ptr<MainWindow> Dialog;
	void show(HWND hParent)
	{
		auto wnd = std::make_unique<MainWindow>(hParent);
		uiMain(); // blocks till window is closed
	}
}
