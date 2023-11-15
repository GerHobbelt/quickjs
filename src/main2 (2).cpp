

#ifdef __ANDROID__
#include <android/ndk-version.h>
#if __NDK_MAJOR__ <= 17
#error "you ndk does not meet the required version(18)"
#endif
#include <jni.h>
#include "TizEngine.hpp"
#include "ECS.h"
using namespace TizEngine;



extern "C"
{
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onCreate(JNIEnv* env, jobject thiz,jobject assetManager){
        TizEngine::Resource::staticAssetManager = AAssetManager_fromJava(env,assetManager);
        Core::onCreate(2);
        //
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onInit(JNIEnv* env, jobject thiz){
       Core::onGLContext();
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onStart(JNIEnv* env, jobject thiz){
        Core::onStart();
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onStop(JNIEnv* env, jobject thiz){
        Core::onStop();
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onDestroy(JNIEnv* env, jobject thiz){
        //Core::onDestroy();
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onChange(JNIEnv* env, jobject thiz,jint width,jint height){
        Screen::onChange(width,height);
        glViewport(0, 0, Screen::width, Screen::height);
	}
	JNIEXPORT jboolean JNICALL Java_com_zgames_mygame_jniWrapper_onDraw(JNIEnv* env, jobject thiz,jfloat DTime)
    {
        Core::time_info::render_dtime = DTime;
        Core::onRender();
        Input::update();
        return 1;
	}
	JNIEXPORT void JNICALL Java_com_zgames_mygame_jniWrapper_onTouchS(JNIEnv* env, jobject thiz,jdouble x,jdouble y,jint status){
		Input::updateCursor(x,y,status,0,0);
	}
	JNIEXPORT jint JNICALL Java_com_zgames_mygame_jniWrapper_getTX(JNIEnv* env, jobject thiz){
		return 50;// (Screen::touchx );
	}
	JNIEXPORT jint JNICALL Java_com_zgames_mygame_jniWrapper_getTY(JNIEnv* env, jobject thiz){
		return 50;// (Screen::touchy );
	}
}

#else //! __ANDROID__
#include <iostream>
#include "TizEngine.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "ECS.h"
using namespace std;
using namespace TizEngine;

int objs = 0;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}
static void framebuffer_resize_callback(GLFWwindow*w,int width,int height)
{
    Screen::onChange(width,height);
    glViewport(0, 0, Screen::width, Screen::height);
}
static void key_callback(GLFWwindow* window, int input, int scancode, int action, int mods)
{
    //GLFW_PRESS | GLFW_RELEASE
    if (input == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (input == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        objs++;

    }

}
int main()
{
    if (!glfwInit())
    {
        assert(0 && "Initialization failed");
    }
    glfwSetErrorCallback(error_callback);
    //OpenGL 3.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_REFRESH_RATE,31);
    Core::window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    glfwSetKeyCallback(Core::window, key_callback);
    glfwSetFramebufferSizeCallback(Core::window,framebuffer_resize_callback);
    if (!Core::window)
    {
        glfwTerminate();
        assert(0 && "glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(Core::window);
    GLenum *buff = (GLenum*)malloc(sizeof(GLenum));
    *buff = glewInit();
    if(*buff != GLEW_NO_ERROR)
    {
        printf("GLEW ERROR: %s\n",glewGetErrorString (*buff) );
        glfwDestroyWindow(Core::window);
        glfwTerminate();
        assert(0 && "glfwMakeContextCurrent failed");
    }free(buff);
    //glewIsSupported (name);
    glfwSwapInterval(1);

    Core::onCreate(2);
    Core::onGLContext();

    Timer render_sleep;
    render_sleep.init();
    uint32_t fps_sum=0;
    float render_time_sum=0;



    Core::onStart();
    framebuffer_resize_callback(Core::window,640,480);
    while (!glfwWindowShouldClose(Core::window))
    {
        render_sleep.start_point();////////////////////////////////////////////////////////////////////////////////////////////////////

        render_time_sum += render_sleep.dtime;
        Core::time_info::render_dtime = render_sleep.dtime;
        if( render_time_sum > 1000.0f)
        {
            Core::time_info::fps = fps_sum;
            fps_sum = 0;
            render_time_sum -= 1000.0f;
        }
        fps_sum++;


        Core::onRender();

        Input::update();

        glfwMakeContextCurrent(Core::window);
        glfwSwapBuffers(Core::window);
        glfwPollEvents();

        if( glfwGetWindowAttrib(Core::window, GLFW_FOCUSED) != 0 )
        {
            if(window_status == 0)
                Core::onStart();
            // also
            double buffer3,buffer4;
            glfwGetCursorPos(Core::window, &(buffer3), &(buffer4));
            Input::cursor_pos[0].cursor_pos_x = buffer3;
            Input::cursor_pos[0].cursor_pos_y = buffer4;
            for(int i = 0;i < ENGINE_MAX_MBTN; i++)
                Input::cursor_pos[0].cursor_status[i] = (glfwGetMouseButton(Core::window, i) != 0) << 1; // 1 => 2, 0 => 0
        }
        else
            if(TizEngine::window_status != 0)
                TizEngine::Core::onStop();

        render_sleep.end_point();////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    Core::onDestroy();
    glfwDestroyWindow(Core::window);
    glfwTerminate();
    return 0;
}
#endif // __ANDROID__
