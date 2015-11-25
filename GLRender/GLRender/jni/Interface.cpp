#include <jni.h>

#include"Logger.h"
#include"IRApplication.h"
#include "Interface.h"
#include <android/native_window_jni.h>




static ANativeWindow *window = 0 ;
static IRApplication *irApp = 0;

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeSetApkPath(JNIEnv* jenv, jclass jclazz, jstring jpath)
{
	LOG_INFO("Interface", "nativeOnStart");
	irApp = new IRApplication();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnStart(JNIEnv* jenv, jclass jclazz)
{
	LOG_INFO("Interface", "nativeOnStart");
	irApp = new IRApplication();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnResume(JNIEnv* jenv, jclass jclazz)
{
	LOG_INFO("Interface", "nativeOnResume");
	irApp->start();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnPause(JNIEnv* jenv, jclass jclazz)
{
    LOG_INFO("Interface", "nativeOnPause");
    irApp->stop();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnStop(JNIEnv* jenv, jclass jclazz)
{
    LOG_INFO("Interface","nativeOnStop");
    delete irApp;
    irApp = 0;
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeSetSurface(JNIEnv* jenv, jclass jclazz, jobject surface)
{
	if(surface != 0)
	{
		window = ANativeWindow_fromSurface(jenv, surface);
		irApp->setWindow(window);
	}else{
		ANativeWindow_release(window);
	}

    return;
}
