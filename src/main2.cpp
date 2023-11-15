
#ifndef __cplusplus
#error
#endif



#include "uv.h"
#include "quickjs/cutils.h"
#include "quickjs/quickjs-libc.h"
#include "serverWrapper.hpp"
#include <string.h>
#include <stdint.h>
//#include "Log.hpp"


#ifdef __ANDROID__

#include <android/ndk-version.h>
#include <string.h>
#include <jni.h>
#if __NDK_MAJOR__ <= 17
#error "you ndk does not meet the required version(18)"
#endif



extern "C"
{
	JNIEXPORT jstring JNICALL Java_com_mycompany_myapp_jniWrapper_onFuck(JNIEnv* env, jobject thiz)
	{
	    return env->NewStringUTF("Hello from JNI !");
	}
	JNIEXPORT void JNICALL Java_com_mycompany_myapp_jniWrapper_onCreate(JNIEnv* env, jobject thiz,jobject assetManager){
        // TizEngine::Resource::staticAssetManager = AAssetManager_fromJava(env,assetManager);
	}
	JNIEXPORT void JNICALL Java_com_mycompany_myapp_jniWrapper_onInit(JNIEnv* env, jobject thiz){
	}
	JNIEXPORT void JNICALL Java_com_mycompany_myapp_jniWrapper_onChange(JNIEnv* env, jobject thiz,jint width,jint height){
	}
	JNIEXPORT jboolean JNICALL Java_com_mycompany_myapp_jniWrapper_onDraw(JNIEnv* env, jobject thiz,jfloat DTime)
	{
        return 1;
	}
}

#else //! __ANDROID__

#error

#endif // __ANDROID__
