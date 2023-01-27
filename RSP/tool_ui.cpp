#include "ui.h"
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
		Window()
		{
			initOnce();

			auto window = uiNewWindow("Fixups", 200, 200, 0);
			uiWindowSetMargined(window, true);
			uiWindowSetResizeable(window, false);

			auto box = uiNewVerticalBox();
			uiBoxSetPadded(box, 1);
			uiWindowSetChild(window, uiControl(box));

			auto label = uiNewLabel("[!] Restart PJ64 after changing configs [!]");
			uiBoxAppend(box, uiControl(label), false);

#define CONFIG(name, desc) addCheckbox(box, name##_ = uiNewCheckbox(desc)); uiCheckboxSetChecked(name##_, sConfig.name);
#include "xconfig.h"
#undef CONFIG

			uiWindowOnClosing(window, onClosing, NULL);
			uiControlShow(uiControl(window));
		}

		void run()
		{
			uiMain();
		}

	private:
#define CONFIG(name, desc) uiCheckbox* name##_;
#include "xconfig.h"
#undef CONFIG

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

		void onCheckBoxChange(uiCheckbox* sender)
		{
#define CONFIG(name, desc) sConfig.name = !!uiCheckboxChecked(name##_);
#include "xconfig.h"
#undef CONFIG
			sConfig.save();
		}

		static int onClosing(uiWindow* w, void* data)
		{
			uiQuit();
			return 1;
		}

		void addCheckbox(uiBox* b, uiCheckbox* cb)
		{
			uiCheckboxOnToggled(cb, [](auto cb, auto me) { ((Window*)me)->onCheckBoxChange(cb); }, this);
			uiBoxAppend(b, uiControl(cb), false);
		}
	};

	static std::unique_ptr<Window> Dialog;
	void show()
	{
		if (!Dialog)
		{
			Dialog = std::make_unique<Window>();
			Dialog->run();
			Dialog.reset();
		}
	}
}
