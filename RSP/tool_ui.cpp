#include <ui.h>

#include <windows.h>
#include <ui_windows.h>

#include "config.h"

#include <stdlib.h>
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
	class Window
	{
	public:
		Window() : hwnd_(GetActiveWindow())
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
			uiWindowOnClosing(window, onClosing, NULL);
			uiControlShow(uiControl(window));
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

		static int onClosing(uiWindow* w, void* data)
		{
			uiQuit();
			return 1;
		}
	};

	class InputWindow : public Window
	{
	public:
		InputWindow()
		{
			auto window = uiNewWindow("Input", 20, 20, 0, hwnd_);
			uiWindowSetMargined(window, true);
			uiWindowSetResizeable(window, false);

			auto main = uiNewHorizontalBox();
			uiBoxSetPadded(main, 1);
			uiWindowSetChild(window, uiControl(main));

			auto label = uiNewLabel("Press key to record...\nPress Space Key to erase\nClose window to ignore\n");
			uiBoxAppend(main, uiControl(label), false);

			show(window);
		}
	};

	class MainWindow : public Window
	{
	public:
		MainWindow()
		{
			auto window = uiNewWindow("PJ64 Fixups", 200, 200, 0, hwnd_);
			uiWindowSetMargined(window, true);
			uiWindowSetResizeable(window, false);

			auto main = uiNewHorizontalBox();
			uiBoxSetPadded(main, 1);
			uiWindowSetChild(window, uiControl(main));

			{
				auto group = uiNewGroup("General");
				auto box = uiNewVerticalBox();
				uiGroupSetChild(group, uiControl(box));
				auto label = uiNewLabel("[!] Restart PJ64 after changing configs [!]");
				uiBoxAppend(box, uiControl(label), false);

#define CONFIG(name, desc) addCheckbox(box, name##_ = uiNewCheckbox(desc), &sConfig.name);
#include "xconfig.h"
#undef CONFIG

				auto openOrig = uiNewButton("Open original RSP configs");
				uiButtonOnClicked(openOrig, [](auto button, auto data) { ((MainWindow*)data)->openOriginalRSPConfig(); }, this);
				uiBoxAppend(box, uiControl(openOrig), false);

				uiBoxAppend(main, uiControl(group), false);
			}
#if 0
			{
				auto group = uiNewGroup("Shortcuts");
				auto grid = uiNewGrid();
				uiGroupSetChild(group, uiControl(grid));

				int counter = 0;
#define HOTKEY(name, desc) addHotKey(counter, grid, desc);
#include "xhotkeys.h"
#undef HOTKEY

				uiBoxAppend(main, uiControl(group), false);
			}
#endif

			show(window);
		}

	private:
#define CONFIG(name, desc) uiCheckbox* name##_;
#include "xconfig.h"
#undef CONFIG

		void addCheckbox(uiBox* b, uiCheckbox* cb, bool* modifying)
		{
			uiCheckboxOnToggled(cb, [](auto cb, auto modifyingCb) { *((bool*)modifyingCb) = uiCheckboxChecked(cb); }, modifying);
			uiBoxAppend(b, uiControl(cb), false);
			uiCheckboxSetChecked(cb, *modifying);
		}

		void addHotKey(int& counter, uiGrid* grid, const char* desc)
		{
			int val = counter++;
			uiGridAppend(grid, uiControl(uiNewLabel(desc)), 0, val, 1, 1, 0, uiAlignStart, 0, uiAlignCenter);
			uiGridAppend(grid, uiControl(uiNewButton("XD")), 1, val, 1, 1, 1, uiAlignFill, 0, uiAlignFill);
			uiGridAppend(grid, uiControl(uiNewButton("UWU")), 2, val, 1, 1, 1, uiAlignFill, 0, uiAlignFill);
			uiGridAppend(grid, uiControl(uiNewButton("PEKA")), 3, val, 1, 1, 1, uiAlignFill, 0, uiAlignFill);
		}

		void openOriginalRSPConfig()
		{
#if 0
			InputWindow wnd;
			uiMain();
#endif
		}
	};

	static std::unique_ptr<MainWindow> Dialog;
	void show()
	{
		if (!Dialog)
		{
			Dialog = std::make_unique<MainWindow>();
			uiMain(); // blocks till window is closed
			Dialog.reset();
			sConfig.save();
		}
	}
}
