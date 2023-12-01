#include "Window.hpp"
#include "Log.hpp"
#include "math.h"
#include "Timer.hpp"
#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui_impl_my.h"
#include "Exception.hpp"
#include <math.h>
#include <array>
#include <vector>



Timer renderTime;

//ImGui::ShowDemoWindow(&show_window);

int WindowManager::OnDraw()
{
	static bool show_btn=true;
	static bool checkbox1=true;
	bool show_window=true;
	constexpr int array_size=128;
	static std::array<float,array_size> histo;
	static int delta_sum_count=0;
	static float delta_sum=0;
	// max value in histo array
	static float delta_max=0;
	//delta_sum+=ImGui::GetIO().DeltaTime;
	delta_sum+=renderTime.dtime;
	delta_sum_count++;
	if(delta_sum_count>30){
        delta_max=0;
		delta_sum_count=0;
		for(int i=0;i<(array_size-1);i++){
			histo[i]=histo[i+1];
			if(histo[i]>delta_max)
                delta_max=histo[i];
        }
		histo[array_size-1]=delta_sum*1000;
		delta_sum=0;
	}

	const ImGuiWindowFlags flags =
            ImGuiWindowFlags_MenuBar |
            //ImGuiWindowFlags_NoDecoration |
			//ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoTitleBar |
            //ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings;
	// Beginning of frame: update Renderer + Platform backend, start Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

    //ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

	if(1)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
	}


	if (ImGui::Begin("This text should not be displayed!", &show_window, flags))
	{
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Examples"))
            {
                ImGui::MenuItem("(demo menu)", NULL, false, false);
                ImGui::MenuItem("show button", NULL, &show_btn);
                ImGui::MenuItem("check", NULL, &checkbox1);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

		const ImVec4 color = ImVec4(0.9f,0.9f,0.9f,0.9f);
		ImGui::TextColored(color,"text");
		ImGui::Text("text2");
		if (ImGui::Button("Exit"))
		{
			show_window=false;
		}
		ImGui::SameLine();
        ImGui::Text("Application average %.5f (%.5g) ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().DeltaTime, ImGui::GetIO().Framerate);
		ImGui::Text("Using work area instead of main area");
		ImGui::SameLine();
		ImGui::TextDisabled("(i)");
        ImGui::BulletText("text1");
        ImGui::BulletText("text2");
        ImGui::Separator();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::PlotHistogram("", histo.data(), histo.size(), 0, "30 frame render time (in ms)",0,delta_max,ImVec2(0,100));
		if(show_btn){
            //ImGui::ShowDemoWindow(&show_btn);
            ImGui::Indent();
			if(ImGui::Button("Hide this button"))
				show_btn=false;
            ImGui::Unindent();
		}
	}
	ImGui::End();


	ImGui::Render();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if(!show_window)
		WindowManager::Exit();
	return 0;
}





// EventLoop, misnamed
int WindowManager::Event()
{
	DeltaTime.init();
	CurrentContext();
	glwInitialize(0x300);

	// Create a Dear ImGui context, setup some options
	ImGui::CreateContext();
	ImGuiIO& io=ImGui::GetIO();
	bd = IM_NEW(ImGui_ImplMy_Data)();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable some options
	IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
	io.BackendPlatformUserData = (void*)bd;
	io.BackendPlatformName = "imgui_impl_my";
#if IS_WINDOWS_OS
	ImGui::GetMainViewport()->PlatformHandleRaw = (void*)hWnd;
#elif IS_LINUX_OS
	ImGui::GetMainViewport()->PlatformHandleRaw = (void*)win;
#endif

	ImGui_ImplOpenGL3_Init();

	#if IS_WINDOWS_OS
    {
        RECT rect = { 0, 0, 0, 0 };
        ::GetClientRect(hWnd, &rect);
        int width=rect.right-rect.left;
        int height=rect.bottom-rect.top;
        io.DisplaySize = ImVec2((float)width, (float)height);
        Console::log(Console::debug,"size: (%ld,%ld)",width,height);
    }
	#endif
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsDark();

#ifdef _WIN32
	MSG msg;
#else
	int xpending;
	XEvent xev;
#endif // _WIN32
	// to avoid bad 'dtime'
	DeltaTime.start_point();

	while (isRunning)
	{
	#if IS_WINDOWS_OS
		/* check for messages in the queue */
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			// WM_KEYDOWN and WM_KEYUP ==> WM_CHAR, wont replace but add new message to the queue
			TranslateMessage(&msg);
			// pass the message to the WndProc
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) isRunning = 0;
		}
		// dont render until no event remaind
		else{
			io.DeltaTime = DeltaTime.dtime/1000;

			RedrawWindow(WindowManager::hWnd, NULL, NULL, RDW_INTERNALPAINT);

			DeltaTime.end_point();
			if(DeltaTime.dtime < SleepLenght){
				Sleep ((int) ((SleepLenght - DeltaTime.dtime)));
				DeltaTime.dtime = SleepLenght;
			}
			// encounter event proccessing
			DeltaTime.start_point();
		}
	#else
		if((xpending = XPending(display)) != 0)
		{
			if (xpending > 0)
			{
				XNextEvent(display, &xev);
				WndProc(&xev);
			}
			else if(xpending < 0)
			{
				Console::log(Console::crit,"XPending returned %d\n",xpending);
				isRunning = 0;
				goto brk;
			}
		}
		// dont render until no event remaind
		else{
            // deltatime in second
			io.DeltaTime = DeltaTime.dtime/1000.0;

			WindowManager::OnDraw();
			WindowManager::SwapBuffer();

			DeltaTime.end_point();
			renderTime.end_point();

			if(DeltaTime.dtime < SleepLenght){
				Sleep ((int) ((SleepLenght - DeltaTime.dtime)));
				DeltaTime.dtime = SleepLenght;
			}
			// encounter event proccessing
			DeltaTime.start_point();
			renderTime.start_point();
		}
	#endif
	}
    #if IS_LINUX_OS
	brk:
    #endif
	// Shutdown
	ImGui_ImplOpenGL3_Shutdown();
	IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
	io.BackendPlatformName = nullptr;
	io.BackendPlatformUserData = nullptr;
	IM_DELETE(bd);
	bd=nullptr;
	ImGui::DestroyContext();
	glwDestroy();
	return 0;
}

#if IS_WINDOWS_OS
#define IM_VK_KEYPAD_ENTER      (VK_RETURN + 256)
ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam);
static void ImGui_ImplWin32_AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode = -1)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddKeyEvent(key, down);
	// To support legacy indexing (<1.87 user code)
	io.SetKeyEventNativeData(key, native_keycode, native_scancode);
	IM_UNUSED(native_scancode);
}


void printEvent(UINT msg)
{
// Tiz::Console(Tiz::Logger::debug,"Event: %s\n",#X);
#define print_event(X) case X: break;
	switch (msg)
	{
//	print_event(WM_CREATE)
//	//print_event(WA_INACTIVE)
//	//print_event(WM_ACTIVATE)delta_max
//	//print_event(WA_CLICKACTIVE)
//	//print_event(WM_SETFOCUS)
//	print_event(WM_MOVE)
//	print_event(WM_CLOSE)
//	//print_event(WM_ACTIVATEAPP)
//	print_event(WM_KILLFOCUS)
//	print_event(WM_SHOWWINDOW)
//	print_event(WM_IME_SETCONTEXT)
//	print_event(WM_IME_NOTIFY)
//	print_event(WM_GETICON)
	print_event(WM_PAINT)
//	print_event(WM_CAPTURECHANGED)
//	print_event(WM_SYSCOMMAND)
	print_event(WM_ERASEBKGND)
//	print_event(512)
//	print_event(132)
//	print_event(160)
//	print_event(32)
//	print_event(71)
//	print_event(70)
	default:
		Console::log(Console::debug,"Event: %u\n",msg);
		break;
	}
#undef print_event
}

LRESULT CALLBACK WindowManager::WndProc
(HWND _hWnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	//printEvent(message);
	if(ImGui::GetCurrentContext())
	{
		ImGuiIO& io=ImGui::GetIO();
		switch (message)
		{
		case WM_SIZE://WM_SIZING
		{
			RECT rect = { 0, 0, 0, 0 };
			::GetClientRect(hWnd, &rect);
			int width=rect.right-rect.left;
			int height=rect.bottom-rect.top;
			io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
			break;
		}
		case WM_MOUSEMOVE:
			// We need to call TrackMouseEvent in order to receive WM_MOUSELEAVE events
			if (!WindowManager::MouseTracked)
			{
				TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, WindowManager::hWnd, 0 };
				::TrackMouseEvent(&tme);
				WindowManager::MouseTracked = true;
			}
			io.AddMousePosEvent((float)LOWORD(lParam),(float)HIWORD(lParam));
			break;
		case WM_MOUSELEAVE:
			WindowManager::MouseTracked=false;
			io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			break;
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		{
			int button = 0;
			if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) { button = 0; }
			else if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) { button = 1; }
			else if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) { button = 2; }
			else if (message == WM_XBUTTONDOWN || message == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
			if (WindowManager::MouseButtonsDown == 0 && ::GetCapture() == nullptr)
				::SetCapture(WindowManager::hWnd);
			WindowManager::MouseButtonsDown |= 1 << button;
			io.AddMouseButtonEvent(button, true);
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			int button = 0;
			if (message == WM_LBUTTONUP) { button = 0; }
			else if (message == WM_RBUTTONUP) { button = 1; }
			else if (message == WM_MBUTTONUP) { button = 2; }
			else if (message == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
			WindowManager::MouseButtonsDown &= ~(1 << button);
			if (WindowManager::MouseButtonsDown == 0 && ::GetCapture() == WindowManager::hWnd)
				::ReleaseCapture();
			io.AddMouseButtonEvent(button, false);
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
		    printf("WM_KEY: %u lparam=%lld wparam=%lld\n",message,lParam,wParam);
			if(wParam < 256)
			{
				io.AddKeyEvent(ImGuiMod_Ctrl, GetKeyState(VK_CONTROL) & 0x8000);
				io.AddKeyEvent(ImGuiMod_Shift, GetKeyState(VK_SHIFT) & 0x8000);
				io.AddKeyEvent(ImGuiMod_Alt, GetKeyState(VK_MENU) & 0x8000);
				io.AddKeyEvent(ImGuiMod_Super, GetKeyState(VK_APPS) & 0x8000);

				if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
					wParam = IM_VK_KEYPAD_ENTER;
				// Submit key event
				const bool is_key_down = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
				const ImGuiKey key = ImGui_ImplWin32_VirtualKeyToImGuiKey(wParam);
				const int scancode = (int)LOBYTE(HIWORD(lParam));
				if (key != ImGuiKey_None)
					ImGui_ImplWin32_AddKeyEvent(key, is_key_down, wParam, scancode);
			}
			break;
		}
		case WM_CHAR:
			if (IsWindowUnicode(_hWnd))
			{
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacterUTF16((unsigned short)wParam);
			}
			else
			{
				wchar_t wch = 0;
				::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
				io.AddInputCharacter(wch);
			}
			break;
		case WM_MOUSEWHEEL:
			io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
			break;
		case WM_MOUSEHWHEEL:
			io.AddMouseWheelEvent((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);
			break;
		// return if it was an user input event, countinue to process if is system event.
		default: goto sys;
		}
		return 0;
		sys: switch (message)
		{
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			io.AddFocusEvent(message == WM_SETFOCUS);
			break;
		}
	}

	// switch(message)
	// {
	// case WM_SETCURSOR:
	// case WM_NCMOUSEMOVE:
	// case WM_NCLBUTTONDOWN:
	// case WM_NCHITTEST:
	// case WTS_REMOTE_CONNECT:
	// case WM_MOVING:
	// case WM_WINDOWPOSCHANGING:
	// case WM_WINDOWPOSCHANGED:
	// case 49409:
	// case WM_HSCROLL:
	// case WM_NCACTIVATE:
	// case WM_NCCALCSIZE:
	// case WM_MENUSELECT:
	// case WM_MENUCHAR:
	// case WM_ENTERIDLE:
	// case WM_MENURBUTTONUP:
	// 	return DefWindowProc (_hWnd, message, wParam, lParam);
	// }
	;

	switch (message)
	{
	// case WM_CREATE:
	case WM_PAINT:
		//if drawing context is initialized
		if(ImGui::GetCurrentContext())
		{
			WindowManager::OnDraw();
			WindowManager::SwapBuffer();
		}
		break;
	case WM_ERASEBKGND:
		break;
	case WM_QUIT:
		isRunning = 0;
		break;
	case WM_DESTROY:
		break;
	case WM_CLOSE:
		// like PostMessage,
		PostQuitMessage (0);
		break;

	default:
		return DefWindowProc (_hWnd, message, wParam, lParam);
	}
	return 0;
}
#else
ImGuiKey ImGui_ImplLinux_VirtualKeyToImGuiKey(unsigned int);
ImGuiKey ImGui_ImplLinux_VirtualKeyToImGuiModKey(unsigned int keycode);
int WindowManager::WndProc (XEvent *event)
{
	//if(event->type != 22)
	// Console(Logger::developer,"Event: %d\n",event->type);

	if(ImGui::GetCurrentContext())
	{
		ImGuiIO& io=ImGui::GetIO();
		switch (event->type)
		{
		case ConfigureNotify:
			io.DisplaySize = ImVec2((float)event->xconfigure.width, (float)event->xconfigure.height);
            //Console(Tiz::Logger::developer,"Resize: %d %d\n",event->xconfigure.width,event->xconfigure.height);
			break;
		case KeyPress:
		{
			// Tiz::Console(Tiz::Logger::debug,"press: %d\n",event->xkey.keycode);
			io.AddKeyEvent(ImGui_ImplLinux_VirtualKeyToImGuiModKey(event->xkey.keycode), 1);
			io.AddKeyEvent(ImGui_ImplLinux_VirtualKeyToImGuiKey(event->xkey.keycode), true);
			KeySym unic=XLookupKeysym(&event->xkey, 0);

			//putchar(unic);Console.flush();
			if ((0x20<=unic && unic<=0x7e)
			|| (0xa0<=unic && unic<=0xff)
			|| (0x1a1<=unic && unic<=0x1ff)
			|| (0x2a1<=unic && unic<=0x2fe)
			){
				io.AddInputCharacter(unic);
			}
		}
			break;
		case KeyRelease:
			//XMaskEvent()
			io.AddKeyEvent(ImGui_ImplLinux_VirtualKeyToImGuiModKey(event->xkey.keycode), 0);
			io.AddKeyEvent(ImGui_ImplLinux_VirtualKeyToImGuiKey(event->xkey.keycode), false);
			break;
		/*
			1 = left button
			2 = middle button (pressing the scroll wheel)
			3 = right button
			4 = turn scroll wheel up
			5 = turn scroll wheel down
			6 = push scroll wheel left
			7 = push scroll wheel right
			8 = 4th button (aka browser backward button)
			9 = 5th button (aka browser forward button)
		*/
		case ButtonPress:
			if (Button1==event->xbutton.button)
				io.AddMouseButtonEvent(0, true);
            if (Button2==event->xbutton.button)
				io.AddMouseButtonEvent(2, true);
            if (Button3==event->xbutton.button)
				io.AddMouseButtonEvent(1, true);
			else if (event->xbutton.button==Button4)
				io.AddMouseWheelEvent(0.0f, 0.5);
			else if (event->xbutton.button==Button5)
				io.AddMouseWheelEvent(0.0f,-0.5);
			break;
		case ButtonRelease:
			if (Button1==event->xbutton.button)
				io.AddMouseButtonEvent(0, false);
            if (Button2==event->xbutton.button)
				io.AddMouseButtonEvent(2, false);
            if (Button3==event->xbutton.button)
				io.AddMouseButtonEvent(1, false);
			else if (event->xbutton.button==Button4)
				io.AddMouseWheelEvent(0.0f, 0.5);
			else if (event->xbutton.button==Button5)
				io.AddMouseWheelEvent(0.0f,-0.5);
			break;
		case MotionNotify:
			io.AddMousePosEvent((float)event->xmotion.x,(float)event->xmotion.y);
			break;
		}
	}

	switch (event->type)
	{
	case Expose:
		break;
	case ClientMessage:
		if ((unsigned)(event->xclient.data.l[0]) == wmDeleteMessage)
			isRunning = false;
		break;
	default:
		return 0;
	}
	return 1;
}
#endif





#if IS_WINDOWS_OS
// Map VK_xxx to ImGuiKey_xxx.
ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
{
	switch (wParam)
	{
		case VK_TAB: return ImGuiKey_Tab;
		case VK_LEFT: return ImGuiKey_LeftArrow;
		case VK_RIGHT: return ImGuiKey_RightArrow;
		case VK_UP: return ImGuiKey_UpArrow;
		case VK_DOWN: return ImGuiKey_DownArrow;
		case VK_PRIOR: return ImGuiKey_PageUp;
		case VK_NEXT: return ImGuiKey_PageDown;
		case VK_HOME: return ImGuiKey_Home;
		case VK_END: return ImGuiKey_End;
		case VK_INSERT: return ImGuiKey_Insert;
		case VK_DELETE: return ImGuiKey_Delete;
		case VK_BACK: return ImGuiKey_Backspace;
		case VK_SPACE: return ImGuiKey_Space;
		case VK_RETURN: return ImGuiKey_Enter;
		case VK_ESCAPE: return ImGuiKey_Escape;
		case VK_OEM_7: return ImGuiKey_Apostrophe;
		case VK_OEM_COMMA: return ImGuiKey_Comma;
		case VK_OEM_MINUS: return ImGuiKey_Minus;
		case VK_OEM_PERIOD: return ImGuiKey_Period;
		case VK_OEM_2: return ImGuiKey_Slash;
		case VK_OEM_1: return ImGuiKey_Semicolon;
		case VK_OEM_PLUS: return ImGuiKey_Equal;
		case VK_OEM_4: return ImGuiKey_LeftBracket;
		case VK_OEM_5: return ImGuiKey_Backslash;
		case VK_OEM_6: return ImGuiKey_RightBracket;
		case VK_OEM_3: return ImGuiKey_GraveAccent;
		case VK_CAPITAL: return ImGuiKey_CapsLock;
		case VK_SCROLL: return ImGuiKey_ScrollLock;
		case VK_NUMLOCK: return ImGuiKey_NumLock;
		case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
		case VK_PAUSE: return ImGuiKey_Pause;
		case VK_NUMPAD0: return ImGuiKey_Keypad0;
		case VK_NUMPAD1: return ImGuiKey_Keypad1;
		case VK_NUMPAD2: return ImGuiKey_Keypad2;
		case VK_NUMPAD3: return ImGuiKey_Keypad3;
		case VK_NUMPAD4: return ImGuiKey_Keypad4;
		case VK_NUMPAD5: return ImGuiKey_Keypad5;
		case VK_NUMPAD6: return ImGuiKey_Keypad6;
		case VK_NUMPAD7: return ImGuiKey_Keypad7;
		case VK_NUMPAD8: return ImGuiKey_Keypad8;
		case VK_NUMPAD9: return ImGuiKey_Keypad9;
		case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
		case VK_DIVIDE: return ImGuiKey_KeypadDivide;
		case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case VK_ADD: return ImGuiKey_KeypadAdd;
		case IM_VK_KEYPAD_ENTER: return ImGuiKey_KeypadEnter;
		case VK_LSHIFT: return ImGuiKey_LeftShift;
		case VK_LCONTROL: return ImGuiKey_LeftCtrl;
		case VK_LMENU: return ImGuiKey_LeftAlt;
		case VK_LWIN: return ImGuiKey_LeftSuper;
		case VK_RSHIFT: return ImGuiKey_RightShift;
		case VK_RCONTROL: return ImGuiKey_RightCtrl;
		case VK_RMENU: return ImGuiKey_RightAlt;
		case VK_RWIN: return ImGuiKey_RightSuper;
		case VK_APPS: return ImGuiKey_Menu;
		case '0': return ImGuiKey_0;
		case '1': return ImGuiKey_1;
		case '2': return ImGuiKey_2;
		case '3': return ImGuiKey_3;
		case '4': return ImGuiKey_4;
		case '5': return ImGuiKey_5;
		case '6': return ImGuiKey_6;
		case '7': return ImGuiKey_7;
		case '8': return ImGuiKey_8;
		case '9': return ImGuiKey_9;
		case 'A': return ImGuiKey_A;
		case 'B': return ImGuiKey_B;
		case 'C': return ImGuiKey_C;
		case 'D': return ImGuiKey_D;
		case 'E': return ImGuiKey_E;
		case 'F': return ImGuiKey_F;
		case 'G': return ImGuiKey_G;
		case 'H': return ImGuiKey_H;
		case 'I': return ImGuiKey_I;
		case 'J': return ImGuiKey_J;
		case 'K': return ImGuiKey_K;
		case 'L': return ImGuiKey_L;
		case 'M': return ImGuiKey_M;
		case 'N': return ImGuiKey_N;
		case 'O': return ImGuiKey_O;
		case 'P': return ImGuiKey_P;
		case 'Q': return ImGuiKey_Q;
		case 'R': return ImGuiKey_R;
		case 'S': return ImGuiKey_S;
		case 'T': return ImGuiKey_T;
		case 'U': return ImGuiKey_U;
		case 'V': return ImGuiKey_V;
		case 'W': return ImGuiKey_W;
		case 'X': return ImGuiKey_X;
		case 'Y': return ImGuiKey_Y;
		case 'Z': return ImGuiKey_Z;
		case VK_F1: return ImGuiKey_F1;
		case VK_F2: return ImGuiKey_F2;
		case VK_F3: return ImGuiKey_F3;
		case VK_F4: return ImGuiKey_F4;
		case VK_F5: return ImGuiKey_F5;
		case VK_F6: return ImGuiKey_F6;
		case VK_F7: return ImGuiKey_F7;
		case VK_F8: return ImGuiKey_F8;
		case VK_F9: return ImGuiKey_F9;
		case VK_F10: return ImGuiKey_F10;
		case VK_F11: return ImGuiKey_F11;
		case VK_F12: return ImGuiKey_F12;
		default: return ImGuiKey_None;
	}
}
#else
ImGuiKey ImGui_ImplLinux_VirtualKeyToImGuiModKey(unsigned int keycode)
{
    switch (keycode)
	{
	case 37:
	case 105:
        return ImGuiMod_Ctrl;
	case 64:
	case 108:
        return ImGuiMod_Alt;
	case 50:
	case 62:
        return ImGuiMod_Shift;
    default:
        return ImGuiKey_None;
    }
}
ImGuiKey ImGui_ImplLinux_VirtualKeyToImGuiKey(unsigned int keycode)
{
	switch (keycode)
	{
	//ImGuiKey_0
	//case XK_0 ... XK_9: return (ImGuiKey)key+488;


	case 9: return ImGuiKey_Escape;
	case 10 ... 18: return (ImGuiKey)(ImGuiKey_1+(keycode-10));
	case 19: return ImGuiKey_0;
	case 20: return ImGuiKey_Minus;
	case 21: return ImGuiKey_Equal;
	case 22: return ImGuiKey_Backspace;


	case 23: return ImGuiKey_Tab;
	case 24: return ImGuiKey_Q;
	case 25: return ImGuiKey_W;
	case 26: return ImGuiKey_E;
	case 27: return ImGuiKey_R;
	case 28: return ImGuiKey_T;
	case 29: return ImGuiKey_Y;
	case 30: return ImGuiKey_U;
	case 31: return ImGuiKey_I;
	case 32: return ImGuiKey_O;
	case 33: return ImGuiKey_P;
	case 34: return ImGuiKey_LeftBracket;
	case 35: return ImGuiKey_RightBracket;

	case 36: return ImGuiKey_Enter;
	case 37: return ImGuiKey_LeftCtrl;
	case 38: return ImGuiKey_A;
	case 39: return ImGuiKey_S;
	case 40: return ImGuiKey_D;
	case 41: return ImGuiKey_F;
	case 42: return ImGuiKey_G;
	case 43: return ImGuiKey_H;
	case 44: return ImGuiKey_J;
	case 45: return ImGuiKey_K;
	case 46: return ImGuiKey_L;
	case 47: return ImGuiKey_Semicolon;
	case 48: return ImGuiKey_Comma;
	case 49: return ImGuiKey_GraveAccent;

	case 50: return ImGuiKey_LeftShift;
	case 51: return ImGuiKey_Backslash;
	case 52: return ImGuiKey_Z;
	case 53: return ImGuiKey_X;
	case 54: return ImGuiKey_C;
	case 55: return ImGuiKey_V;
	case 56: return ImGuiKey_B;
	case 57: return ImGuiKey_N;
	case 58: return ImGuiKey_M;
	case 59: return ImGuiKey_Apostrophe;
	case 60: return ImGuiKey_Period;
	case 61: return ImGuiKey_Slash;
	case 62: return ImGuiKey_RightShift;
	case 63: return ImGuiKey_KeypadMultiply;
	case 64: return ImGuiKey_LeftAlt;
	case 65: return ImGuiKey_Space;
	case 66: return ImGuiKey_CapsLock;

	case 67 ... 76: return (ImGuiKey)(ImGuiKey_F1+(keycode-67));


	case 77:        return ImGuiKey_NumLock;
	case 79 ... 81: return (ImGuiKey)(ImGuiKey_Keypad7+(keycode-79));
	case 82:        return ImGuiKey_KeypadSubtract;
	case 83 ... 85: return (ImGuiKey)(ImGuiKey_Keypad4+(keycode-83));
	case 86:        return ImGuiKey_KeypadAdd;
	case 87 ... 89: return (ImGuiKey)(ImGuiKey_Keypad1+(keycode-87));
	case 90:        return ImGuiKey_Keypad0;
	case 91:        return ImGuiKey_KeypadDecimal;


	case 95: return ImGuiKey_F11;
	case 96: return ImGuiKey_F12;

	case 104: return ImGuiKey_KeypadEnter;
	case 105: return ImGuiKey_RightCtrl;
	case 106: return ImGuiKey_KeypadDivide;
	case 108: return ImGuiKey_RightAlt;

	case 110: return ImGuiKey_Home;
	case 111: return ImGuiKey_UpArrow;
	case 113: return ImGuiKey_LeftArrow;
	case 112: return ImGuiKey_PageUp;
	case 114: return ImGuiKey_RightArrow;
	case 115: return ImGuiKey_End;
	case 116: return ImGuiKey_DownArrow;
	case 117: return ImGuiKey_PageDown;
	case 118: return ImGuiKey_Insert;
	case 119: return ImGuiKey_Delete;
	default:
	//printf("key: %i\n",keycode);
	return ImGuiKey_None;
	}
}
#endif
