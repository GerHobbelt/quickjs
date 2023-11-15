/**
 * @file Window.hpp
 * @author your name (you@domain.com)
 * @brief This functions creates an stable window interface with low level OS access
 * to read the input and initalize grapgic library
 * it is OS dependent, means must write low level access code for each OS
 * @version 0.1
 * @date 2022-06-9
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef WINDOW_H
#define WINDOW_H

#ifdef _WIN32
	#include <windows.h>
	#include "glcorearb.h"
	#include <GL/wglext.h>
#else
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	// #include <X11/extensions/XInput.h>
	#include <signal.h>
	#include "glcorearb.h"
	#include <GL/glx.h>
	#include <unistd.h>
	//#include <GL/gl.h>
	#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
	#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	#include<errno.h>
	#define GetLastError() errno
#endif // _WIN32
#include "imgui_impl_my.h"

// all functions return non-zero if fail
namespace WindowManager
{
/************************ Initialize ************************/

	extern bool isRunning;
	// sleep lenght after reciving event messages (in millisecond)
	extern double SleepLenght;
	#define window_initial_width 400
	#define window_initial_height 300
	extern ImGui_ImplMy_Data* bd;
#ifdef _WIN32
	/* register window class */
    extern WNDCLASS		wc;
	extern HWND			console;
	extern HWND			hWnd;
	extern HDC			hDC;
	extern HGLRC		hGLRC;
	extern PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB;
    extern PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;
    extern PFNWGLGETEXTENSIONSSTRINGARBPROC		wglGetExtensionsStringARB;
	LRESULT CALLBACK WndProc (HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam);
	extern uint32_t MouseButtonsDown;
	extern bool MouseTracked;
#else
	extern Display		*display;
	extern Colormap		cmap;
	extern Window		win;
	extern Window       rootScreen;
	extern GLXContext	ctx;
	extern GLXFBConfig	fbConfig;
	extern Atom wmDeleteMessage;
	extern glXCreateContextAttribsARBProc		glXCreateContextAttribsARB;
	// extern int num_devices;
	// extern XDeviceInfo *devices;
	int WndProc (XEvent*);
	int IOErrorHandler (Display*);
	int   ErrorHandler (Display*,XErrorEvent*);
	void  close_signal(int);
#endif // _WIN32

	int MultiThreadAccess(void);
#ifdef _WIN32
	int InitWindow(HINSTANCE i);
#else
	int InitWindow();
#endif
	// can be called any time
	int RetrieveFunctions();
	// call this if only RetrieveFunctions and InitWindow is called
	int CreateFramebuffer();
	int CreateContext();
	int DestroyContext();
	int DestroyFramebuffer();
	int DestroyWindow();

/************************ Rendering ************************/

	int OnDraw();
	int CurrentContext();
	int Sleep(int);
	// call this if only CurrentContext was successfull
	int SwapBuffer();
	void Exit();

/************************ Window Utils ************************/

	void ShowConsole();
	void HideConsole();
	// call this if only CreateFramebuffer was successfull
	int SetTitle(const char*);
	// Enter ininput event loop - will block till recive exit message
	int Event();
}



#endif // WINDOW_H
