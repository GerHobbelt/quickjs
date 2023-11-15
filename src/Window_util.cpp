#include "Window.hpp"








int WindowManager::Sleep(int ms)
{
#if IS_WINDOWS_OS
	::Sleep(ms);
#else
	usleep(ms * 1000);
#endif

return 0;
}
int WindowManager::SetTitle(const char* title)
{
	(void)title;
#if IS_WINDOWS_OS
	SetWindowTextA(hWnd, title);
#endif
#if IS_LINUX_OS
	XStoreName(display, win, title);
	XClassHint* class_hint = XAllocClassHint();
	if (class_hint) {
		class_hint->res_name = class_hint->res_class = (char*)title;
		XSetClassHint(display, win, class_hint);
		XFree(class_hint);
	}
#endif
	return 1;
}






void WindowManager::HideConsole() {
#if IS_WINDOWS_OS
	ShowWindow(console, SW_HIDE);
#endif // IS_WINDOWS_OS
}

void WindowManager::ShowConsole() {
#if IS_WINDOWS_OS
	ShowWindow(console, SW_SHOW);
#endif
}
