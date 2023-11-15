#include "Window.hpp"
#include "Log.hpp"
#include "Timer.hpp"
#include "CrossPlatform.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui_impl_my.h"
#include "Exception.hpp"

#define WINDOW_CLASS_NAME "WindowClass"

ImGui_ImplMy_Data* WindowManager::bd;
bool		WindowManager::isRunning=0;
double		WindowManager::SleepLenght=1000.0/30.0;
#if IS_WINDOWS_OS
WNDCLASS	WindowManager::wc;
HWND		WindowManager::console;
HWND		WindowManager::hWnd;
HDC			WindowManager::hDC;
HGLRC		WindowManager::hGLRC;
PFNWGLCREATECONTEXTATTRIBSARBPROC	WindowManager::wglCreateContextAttribsARB;
PFNWGLCHOOSEPIXELFORMATARBPROC		WindowManager::wglChoosePixelFormatARB;
PFNWGLGETEXTENSIONSSTRINGARBPROC	WindowManager::wglGetExtensionsStringARB;
uint32_t WindowManager::MouseButtonsDown=0;
bool WindowManager::MouseTracked=false;

static const int framebuffer_attribs[] =
{
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 24,
	WGL_ALPHA_BITS_ARB, 8,
	WGL_DEPTH_BITS_ARB, 16,
	//WGL_STENCIL_BITS_ARB, 8,
	0, // End
};
static const int gl_context_attribs[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	WGL_CONTEXT_MINOR_VERSION_ARB, 1,
	WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0
};

#else

Display*	WindowManager::display;
Colormap	WindowManager::cmap;
Window		WindowManager::win;
Window      WindowManager::rootScreen;
GLXContext	WindowManager::ctx;
GLXFBConfig	WindowManager::fbConfig;
Atom		WindowManager::wmDeleteMessage;
glXCreateContextAttribsARBProc		WindowManager::glXCreateContextAttribsARB;
// int         WindowManager::num_devices=0;
// XDeviceInfo *WindowManager::devices=nullptr;
// static XEventClass event_list[15];
// static int number = 0;

// Get a matching FB config
static const int framebuffer_attribs[] =
{
	GLX_X_RENDERABLE    , True,
	GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
	GLX_RENDER_TYPE     , GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
	GLX_DOUBLEBUFFER    , True,
	GLX_RED_SIZE        , 8,
	GLX_GREEN_SIZE      , 8,
	GLX_BLUE_SIZE       , 8,
	GLX_ALPHA_SIZE      , 8,
	GLX_DEPTH_SIZE      , 16,
	//GLX_STENCIL_SIZE    , 8,
	//GLX_SAMPLE_BUFFERS  , 1,
	//GLX_SAMPLES         , 4,
	None
};
static const int gl_context_attribs[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	GLX_CONTEXT_MINOR_VERSION_ARB, 2,
	None
};

#endif




int WindowManager::InitWindow(
#if IS_WINDOWS_OS
	HINSTANCE Instance
#endif
){
#if IS_WINDOWS_OS
	//wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = Instance;
	wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
	wc.lpszClassName = WINDOW_CLASS_NAME;
	wc.lpszMenuName = NULL;
	//wc.hIconSm = NULL;
	RegisterClass (&wc);

	console = GetConsoleWindow();
	/* create main window */
	hWnd = CreateWindowEx (
	  WS_EX_CLIENTEDGE, WINDOW_CLASS_NAME, "OpenGL Sample",
	  // WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_SIZEBOX | WS_SYSMENU | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
	  WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_MAXIMIZEBOX,
	  0, 0, window_initial_width, window_initial_height,
	  NULL, NULL, Instance, NULL);
	if(!hWnd)
		throw Exception("unable to create window");
#else
	display = XOpenDisplay(NULL);
	if (!display)
		throw Exception("Failed to open X display");

	signal(SIGINT, close_signal);
	XSetErrorHandler( &WindowManager::ErrorHandler );
	XSetIOErrorHandler( &WindowManager::IOErrorHandler );
	// never ever!
	//XAutoRepeatOff(display);

#endif // _WIN32
	return 0;
}

int WindowManager::MultiThreadAccess()
{
#if IS_LINUX_OS
	return XInitThreads() == 0;
#else
	return 0;
#endif
}

int WindowManager::CreateFramebuffer()
{
// ///Pick a framebuffer pizel configuration to enable OpenGL context in it. ///////////////////////////////////////////////////////////////////////
#if IS_WINDOWS_OS
	/* get the device context (DC) */
	if(!hWnd)
		return 1;
	hDC = GetDC (hWnd);
	if(!hDC)
		throw Exception("unable to get window device context");
	int pixel_format;
	UINT num_formats;
	if(!wglChoosePixelFormatARB(hDC, framebuffer_attribs, NULL, 1, &pixel_format, &num_formats) || !num_formats)
#else
	if (!display)
		return 1;
	int fbcount;
	GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display), framebuffer_attribs, &fbcount);
	if (!fbc || fbcount<1)
#endif // _WIN32
		throw Exception("Failed to choose frame buffer pixel format. \n(may your computer does not support 24bit colors + 8bit alpha and 16 bit depth)");
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if IS_WINDOWS_OS
	PIXELFORMATDESCRIPTOR pfd;
	if(!DescribePixelFormat(hDC, pixel_format, sizeof(pfd), &pfd))
#else
	// framebuffer information
	XVisualInfo *vi;
	// Pick the FB config/visual with the most samples per pixel
	{
		int best_fbc = -1, best_num_samp = -1;// , worst_fbc = -1, worst_num_samp = 999;
		for (int i=0; i<fbcount; ++i)
		{
			vi = glXGetVisualFromFBConfig( display, fbc[i] );
			if ( vi )
			{
				int samp_buf, samples;
				glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
				glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );
				if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) )
					best_fbc = i,
					best_num_samp = samples;
				// if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				// 	worst_fbc = i, worst_num_samp = samples;
			}
			XFree( vi );
		}
		fbConfig = fbc[best_fbc];
	}
	XFree( fbc );
	vi = glXGetVisualFromFBConfig(display, fbConfig);
	if(vi == 0)
#endif // _WIN32
		throw Exception("Failed to get framebuffer pixel description");

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if IS_WINDOWS_OS
	if (!SetPixelFormat(hDC, pixel_format, &pfd))
		throw Exception("Failed to set framebuffer pixel");

#else
	// (&((_XPrivDisplay)display)->screens[vi->screen])->root;
	rootScreen = RootWindow(display, vi->screen);
	cmap = XCreateColormap(display, rootScreen, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = StructureNotifyMask;
	win = XCreateWindow(display, rootScreen, 0, 0, window_initial_width, window_initial_height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
	if (!win)
		throw Exception("Failed to create windows");
	XFree( vi );
#endif // _WIN32
// //////////////////////////////////////////////
#if IS_LINUX_OS
	// capture destroy events.
	// WM_DELETE_WINDOW event is not part of X11, it is created by Window manager.
	// Windows manager will call XDestroyWindow by default (if you dont capture).
	wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, win, &wmDeleteMessage, 1);
	// capture input events.
	XSelectInput(display, win, ExposureMask | VisibilityChangeMask | StructureNotifyMask | PropertyChangeMask |
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | PointerMotionMask );
	XAllowEvents(display, AsyncBoth, CurrentTime);

	// Window title hint
	XClassHint* class_hint = XAllocClassHint();
	if (class_hint) {
		class_hint->res_name = class_hint->res_class = (char*)"title";
		XSetClassHint(display, win, class_hint);
		XFree(class_hint);
	}


	// Default logo
	unsigned long logoBuffer[] = { 0 };
	int logoLength = 1;
	Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(display, "CARDINAL", False);
	XChangeProperty(display, win, net_wm_icon, cardinal, 32, PropModeReplace, (const unsigned char*)logoBuffer, logoLength);

	XMapWindow(display, win);
	XStoreName(display, win, "title");
	XFlush(display);


#endif
// //////////////////////////////////////////////
#if IS_WINDOWS_OS
	ShowWindow(hWnd,true);
#else
	XMapWindow(display, win);
#endif // _WIN32
// //////////////////////////////////////////////
	isRunning = 1;
	return 0;
}
int WindowManager::CreateContext()
{
#if IS_WINDOWS_OS
	if(!hDC)
		return 1;
	/* create and enable the ARB OpenGL render context (RC) */
	hGLRC = wglCreateContextAttribsARB(hDC, NULL, gl_context_attribs);
	if(!hGLRC)
	{
		throw Exception("unable to create opengl context",GetLastError());
		return 1;
	}
#else
	if(!fbConfig || !display)
		return 1;
	ctx = glXCreateContextAttribsARB(display, fbConfig, NULL, true, gl_context_attribs);
	if (!ctx)
		throw Exception("unable to create opengl context",GetLastError());
#endif // _WIN32
	return 0;
}
int WindowManager::DestroyContext()
{
#if IS_WINDOWS_OS
	if(!hGLRC)
		return 1;
	wglMakeCurrent (NULL, NULL);
	wglDeleteContext (hGLRC);
#else
	if(!ctx || !display)
		return 1;
	glXMakeCurrent( display, 0, 0 );
	glXDestroyContext( display, ctx );
#endif // _WIN32
	return 0;
}
int WindowManager::DestroyFramebuffer()
{
	#ifdef _WIN32
	if(!hWnd || !hDC)
		return 1;
	ReleaseDC (hWnd, hDC);
	#else
	if(!cmap || !display || !win)
		return 1;
	XDestroyWindow( display, win );
	XFreeColormap( display, cmap );
	#endif // _WIN32
	return 0;
}
int WindowManager::DestroyWindow()
{
#if IS_WINDOWS_OS
	if(!hWnd)
		return 1;
	::DestroyWindow(hWnd);
#else
	if(!display)
		return 1;
	XCloseDisplay( display );
#endif // _WIN32
	return 0;
}
int WindowManager::CurrentContext()
{
#if IS_WINDOWS_OS
	if(!hDC || !hGLRC)
		return 1;
	if(!wglMakeCurrent( hDC, hGLRC ))
		throw Exception("unable to make current context",GetLastError());
#else
	if(!ctx || !display || !win)
		return 1;
	glXMakeCurrent( display, win, ctx );
#endif // _WIN32
	return 0;
}
int WindowManager::SwapBuffer()
{
#if IS_WINDOWS_OS
	if(!hDC)
		return 1;
	SwapBuffers (hDC);
#else
	if(!display || !win)
		return 1;
	glXSwapBuffers(display, win);
#endif // _WIN32
	return 0;
}

#if IS_LINUX_OS
void WindowManager::close_signal(int sig)
{
	(void)sig;
	isRunning=0;
}
int WindowManager::IOErrorHandler(Display* d)
{
	(void)d;
	Console::log(Console::crit,"X11 OIError occured\n");
	isRunning=0;
	return 0;
}
int WindowManager::ErrorHandler(Display* d,XErrorEvent* e)
{
	(void)d;
	Console::log(Console::crit,"X11 Error %d occured\n",e->error_code);
	isRunning=0;
	return 0;
}
#endif


int WindowManager::RetrieveFunctions()
{
#if IS_WINDOWS_OS
	//the unfortinat is you can only call SetPixelFormat one per HWND, so you must create two window
	HWND hWnd;
	HDC hDC;
	HGLRC hGLRC;
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;

	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, "window", "Fake window",
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		//WS_CAPTION | WS_POPUPWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, 10, 10, NULL, NULL, 0, NULL);
	hDC = GetDC(hWnd);
	if(!hDC){
//		Console("unable to get window device content (dummy) %lu\n",GetLastError());
		return 1;
	}
	// Create dummy GL context that will be used to create the real context
	// We will use the dummy context to use wgl functions to create a ARB supported context
	/* set the pixel format for the DC */
	ZeroMemory (&pfd, sizeof (pfd));
	pfd.nSize = sizeof (pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 0;
	pfd.cAlphaBits = 0;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat (hDC, &pfd);
	if(!SetPixelFormat (hDC, iFormat, &pfd)){
		Console::log("unable to set window pixel format (dummy) %lu (choosen format was %i)\n",GetLastError(),iFormat);
		return 1;
	}
	hGLRC = wglCreateContext(hDC);
	if(!hGLRC){
		Console::log(Console::emerg,"unable to create opengl context (dummy) %lu\n",GetLastError());
		return 1;
	}
	if(!wglMakeCurrent(hDC, hGLRC)){
		Console::log(Console::emerg,"unable to make window current (dummy) %lu\n",GetLastError());
		return 1;
	}
	//get wgl*ARB function
	wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");

	if(wglCreateContextAttribsARB == NULL
	   || wglChoosePixelFormatARB == NULL
	   || wglGetExtensionsStringARB == NULL){
		Console::log(Console::emerg,"failed to load wgl*ARB functions");
		return 1;
	}
	//Tiz::Console(Tiz::Logger::debug,"%s\n",wglGetExtensionsStringARB(hDC) );//wglGetCurrentDC()
	//Tiz::Console(Tiz::Logger::debug,"%s\n",glGetString(GL_EXTENSIONS) );//wglGetCurrentDC()
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hGLRC);
	ReleaseDC(hWnd,hDC);
	DestroyWindow(hWnd);
	return 0;
#else
	glXCreateContextAttribsARB =
		(glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
	return glXCreateContextAttribsARB == 0;
#endif
}
void WindowManager::Exit(){isRunning=false;}






