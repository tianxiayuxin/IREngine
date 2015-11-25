//
// can 2015/11/12
//
//Introduction£º
//
//IRApplication is intended to introduce a interactive reality engine for the future
// Augmented reality App which focuses on User Interaction.
// IRApplication is fully open for everyone who interests in this intension to  create
// and improve this engine together.
//
//Functions:
// 1 Initialize the EGLWindow and Create a EGLSurfae with a EGLContext
// 2 create a GL thread for rendering
// 3 destroy GL thread and EGLWindow
//

#ifndef         IRApplication_H
#define        IRApplication_H

#include <android/native_window.h>
#include <pthread.h>
#include <EGL/egl.h>
#include "Assimp/include/assimp/Importer.hpp"
#include "Assimp/include/assimp/postprocess.h"

#include "IrPlatform.h"


class IRApplication{
public:
	IRApplication();
	~IRApplication();

	void setWindow(ANativeWindow *window);
    void start();
    void stop();

private:
    enum ConfigFlag{
        	CONFIG_RGB = 0,
        	CONFIG_ALPHA  = 1,
        	CONFIG_DEPTH  = 2,
        	CONFIG_STENCIL = 4,
        	CONFIG_MULTISAMPLE = 8
        };

        enum ConfigFlag  _flag;


    void renderLoop();
    bool initialize(ConfigFlag flag);
    void drawFrame();
    void destroy();

private:
    pthread_t  _threadId;
    pthread_mutex_t _mutex;

    enum LoopMessage{
    	MSG_NONE = 0,
    	MSG_SET_WINDOW,
    	MSG_LOOP_EXIT
    };

    enum LoopMessage _msg;

    static void *threadStartCallback(void *it);

private:
    ANativeWindow *_window;

    EGLDisplay    _display;
    EGLSurface    _surface;
    EGLContext   _context;

    int width, height;

    EGLint getContextRenderableType(EGLDisplay eglDisplay);
};

#endif

