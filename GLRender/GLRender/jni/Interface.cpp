#include <jni.h>

#include"Logger.h"
#include"IRBox.h"
#include "Interface.h"
#include <android/native_window_jni.h>

#define LOG_TAG "EglSample"


static ANativeWindow *window = 0 ;
static IRBox *irbox = 0;

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnStart(JNIEnv* jenv, jclass jclazz)
{
	LOG_INFO("nativeOnStart");
	irbox = new IRBox();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnResume(JNIEnv* jenv, jclass jclazz)
{
	LOG_INFO("nativeOnResume");
	irbox->start();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnPause(JNIEnv* jenv, jclass jclazz)
{
    LOG_INFO("nativeOnPause");
    irbox->stop();
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeOnStop(JNIEnv* jenv, jclass jclazz)
{
    LOG_INFO("nativeOnStop");
    delete irbox;
    irbox = 0;
    return;
}

JNIEXPORT void JNICALL Java_com_example_glrender_MainActivity_nativeSetSurface(JNIEnv* jenv, jclass jclazz, jobject surface)
{
	if(surface != 0)
	{
		window = ANativeWindow_fromSurface(jenv, surface);
		irbox->setWindow(window);
	}else{
		ANativeWindow_release(window);
	}

    return;
}
