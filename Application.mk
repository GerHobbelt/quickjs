NDK_TOOLCHAIN_VERSION := 4.9
APP_OPTIM := release
# APP_STL gnustl_static is no longer supported. Please switch to either c++_static or c++_shared.
APP_STL := c++_shared
APP_ABI := armeabi-v7a 
# try 'readelf -h' to get architecture of libraries :)
# armeabi armeabi-v7a arm64-v8a x86 x86_64 mips mips64, all, all64,
APP_PLATFORM := android-24
APP_CPPFLAGS := -fno-rtti -fexceptions
APP_BUILD_SCRIPT := ./Android.mk
APP_MODULES := hello-jni
