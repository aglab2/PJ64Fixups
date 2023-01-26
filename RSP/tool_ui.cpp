#include "ui.h"
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

			uiBox* b;

			auto w = uiNewWindow("Fixups", 200, 240, 0);
			uiWindowSetMargined(w, true);
			uiWindowSetResizeable(w, false);

			b = uiNewVerticalBox();
			uiBoxSetPadded(b, 1);
			uiWindowSetChild(w, uiControl(b));

			addCheckbox(b, fastStates_ = uiNewCheckbox("Do not reinitialize emu on state load"));
			addCheckbox(b, fastResets_ = uiNewCheckbox("Do not reinitialize emu on soft reset"));
			addCheckbox(b, fixSlowResets_ = uiNewCheckbox("Fix slow resets"));
			addCheckbox(b, noLoadSlowdown_ = uiNewCheckbox("Remove sleeps on ROM open"));
			addCheckbox(b, cf1ByDefault_ = uiNewCheckbox("CF1 by default"));
			addCheckbox(b, useFastDecompression_ = uiNewCheckbox("Use fast ZIP decompression"));
			addCheckbox(b, cacheLastSavestate_ = uiNewCheckbox("Cache last loaded savestate"));

			uiWindowOnClosing(w, onClosing, NULL);
			uiControlShow(uiControl(w));
		}

		void run()
		{
			uiMain();
		}

	private:
		uiCheckbox* fastStates_;
		uiCheckbox* fastResets_;
		uiCheckbox* fixSlowResets_;
		uiCheckbox* noLoadSlowdown_;
		uiCheckbox* cf1ByDefault_;
		uiCheckbox* useFastDecompression_;
		uiCheckbox* cacheLastSavestate_;

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
