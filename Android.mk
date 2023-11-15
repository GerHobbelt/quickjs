LOCAL_PATH := $(call my-dir)

#The command include $(CLEAR_VARS)
# clears all the NDK built-in variables starting with LOCAL_,
# such as LOCAL_SRC_FILES, LOCAL_C_INCLUDES, LOCAL_CFLAGS, LOCAL_LDFLAGS, LOCAL_LDLIBS  and so on, 
# except for the LOCAL_PATH.
include $(CLEAR_VARS)
LOCAL_MODULE := hello-jni
#The build system, when it generates the final shared-library file,
# automatically adds the proper prefix and suffix to the name that you assign to LOCAL_MODULE.
# For example, the example that appears above results in generation of a library called libhello-jni.so.
# LOCAL_STATIC_LIBRARIES := 
LOCAL_CPP_EXTENSION := .cpp .hpp .cxx
#LOCAL_SRC_FILES := src/main2.cpp
LOCAL_SRC_FILES := src/serverWrapper.cpp src/main.cpp
LOCAL_SRC_FILES += src/uv/unix/async.c src/uv/unix/core.c src/uv/unix/dl.c src/uv/unix/epoll.c src/uv/unix/fs.c src/uv/unix/fsevents.c src/uv/unix/linux-core.c src/uv/unix/linux-inotify.c src/uv/unix/linux-syscalls.c src/uv/unix/loop-watcher.c src/uv/unix/loop.c src/uv/unix/pipe.c src/uv/unix/poll.c src/uv/unix/process.c src/uv/unix/procfs-exepath.c src/uv/unix/proctitle.c src/uv/unix/pthread-fixes.c src/uv/unix/signal.c src/uv/unix/stream.c src/uv/unix/tcp.c src/uv/unix/thread.c src/uv/unix/udp.c
LOCAL_SRC_FILES += src/uv/fs-poll.c src/uv/idna.c src/uv/inet.c src/uv/strscpy.c src/uv/strtok.c  src/uv/threadpool.c src/uv/timer.c src/uv/uv-common.c src/uv/uv-data-getter-setters.c src/uv/version.c
LOCAL_SRC_FILES += src/quickjs/cutils.c src/quickjs/libbf.c src/quickjs/libregexp.c src/quickjs/libunicode.c src/quickjs/quickjs.c src/quickjs/quickjs-libc.c
LOCAL_C_INCLUDES := include
LOCAL_EXPORT_C_INCLUDE_DIRS := include
LOCAL_LDLIBS := -lGLESv3 -landroid -lEGL -llog
#LOCAL_SHARED_LIBRARIES := libs/libreactphysics3d.a
LOCAL_CPPFLAGS += -std=c++14 -DUSING_UV_SHARED -DDUMP_LEAKS -DJS_STATIC_LIBRARY -DCONFIG_BIGNUM
LOCAL_CFLAGS += -std=c11 -DUSING_UV_SHARED -DDUMP_LEAKS -DJS_STATIC_LIBRARY -DCONFIG_BIGNUM
LOCAL_CFLAGS += -O3
include $(BUILD_SHARED_LIBRARY)



########################
########################
#PREBUILT_SHARED_LIBRARY:
#   include compiled library files with LOCAL_SRC_FILES := libraryFile.so
#   no need to define header files
#    ( to use this code, include shared libraries like this: LOCAL_SHARED_LIBRARIES := moduleName)
########################
#PREBUILT_STATIC_LIBRARY:
#   include compiled library files with LOCAL_SRC_FILES := libraryFile.a
#   no need to define header files
#    ( to use this code, include shared libraries like this: LOCAL_STATIC_LIBRARIES := moduleName)
########################
#BUILD_SHARED_LIBRARY: 
#   share the compiled library file to java code
#   and create a wrapper to use your created functions in java codes
#   in java:
#        class jniWrapper { 
#              public native void func1();
#              public native String func2(int par1);
#              static {  System.loadLibrary("moduleName");  }
#        }
#    in cpp file:
#         #include <jni.h>
#         JNIEXPORT jstring JNICALL Java_com_mycompany_myApp_jniWrapper_func2(JNIEnv* env, jobject thiz,jint par1){
#               return env->NewStringUTF("test" + par1);
#          }
#          JNIEXPORT void JNICALL Java_com_mycompany_myApp_jniWrapper_func1(JNIEnv* env, jobject thiz){
#               // ...
#          }
########################
#BUILD_STATIC_LIBRARY: 
#    compile this library and use it in other libraries
#    to include a whole library (or only cpp) files you have to way:
#        1- LOCAL_CPP_EXTENSION := .cpp .cxx
#             LOCAL_CPP_INCLUDES := libraryDirectoryAddress
#             #LOCAL_C_INCLUDES := ~
#        2- LOCAL_SRC_FILES := libraryDirectoryAddress
#    ( use this code to include static libraries LOCAL_STATIC_LIBRARIES:= moduleName)
########################
#BUILD_EXECUTABLE:
#     compile to an "muduleName.so" that can be executed in java
#     may you know it must contain "main" function, 'so' file starts from main function
#     String nativeDir = getApplicationInfo().nativeLibraryDir;
#     final Process process = new ProcessBuilder().redirectErrorStream(true).command(nativeDir + "/muduleName.so").start();
#     final BufferedReader processReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
#     processReader.readLine()
########################
########################