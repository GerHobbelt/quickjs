/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief the job of main thread is creating an OpenGL window and creating the Tiz engine Core object
 *  and handling input events and initializing things
 * it is OS dependent, means must write low level access code for each OS
 * @version 0.1
 * @date 2022-06-9
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Window.hpp"
#include "Log.hpp"
#include <math.h>
#include "Timer.hpp"

//"/usr/include/c++/9",
//"/usr/include/"

#ifdef _WIN32
int WINAPI WinMain (
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        const int iCmdShow)
#else
int main(int argc,char *argv[])
#endif
{

#ifdef _WIN32
	(void)hInstance;(void)hPrevInstance;(void)lpCmdLine;
#else
	(void)argc;(void)argv;
#endif

    WindowManager::MultiThreadAccess();
	if(!WindowManager::InitWindow(
#ifdef _WIN32
		hInstance
#endif
	)){
    #ifdef _WIN32
        iCmdShow?WindowManager::ShowConsole():WindowManager::HideConsole();
    #endif
		if(!WindowManager::RetrieveFunctions())
		{
            if(!WindowManager::CreateFramebuffer())
            {
                WindowManager::SetTitle("Test");
                if(!WindowManager::CreateContext())
                {
                    WindowManager::Event();
                }
                WindowManager::DestroyContext();
                WindowManager::DestroyFramebuffer();
            }
		}
		WindowManager::DestroyWindow();
	}
	else
	{
//		Console.log(Console::error,"WindowManager::InitWindow failed\n");
	}
    exit(0);
    return 0;
}
