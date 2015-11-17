#include "IRBox.h"
#include <GLES3/gl3.h>


IRBox::IRBox()
:_msg(MSG_NONE), _flag(CONFIG_RGB), _window(0),
 _context(0), _display(0), _surface(0),_threadId(0),
 height(0), width(0)
{
	pthread_mutex_init(&_mutex, 0);
	return ;
}

IRBox::~IRBox()
{
	pthread_mutex_destroy(&_mutex);
	return;
}

void IRBox::start()
{
	pthread_create(&_threadId, 0, threadStartCallback, this);
	return;
}

void IRBox::stop()
{
	pthread_mutex_lock(&_mutex);
	_msg = MSG_LOOP_EXIT;
	pthread_mutex_unlock(&_mutex);

	pthread_join(_threadId, 0);
	return;
}


void IRBox::renderLoop()
{
	bool renderingEnabled = true;

	while(renderingEnabled)
	{
		pthread_mutex_lock(&_mutex);

		//process incoming messages
		switch(_msg)
		{
		case MSG_SET_WINDOW:
			initialize(CONFIG_RGB);
			break;
		case MSG_LOOP_EXIT:
			renderingEnabled = false;
			destroy();
			break;
		default:
			break;
		}

		_msg = MSG_NONE;

		if(_display)
		{
			drawFrame();
			if(!eglSwapBuffers(_display, _surface))
			{
				// do something
			}
		}

		pthread_mutex_unlock(&_mutex);
	}

	return;

}


void IRBox::setWindow(ANativeWindow *window)
{
	pthread_mutex_lock(&_mutex);
	_msg = MSG_SET_WINDOW;
	_window = window;
	pthread_mutex_unlock(&_mutex);
	return;
}

bool IRBox::initialize(ConfigFlag flags)
{
	//EGL Version vars for EGL initialize
	EGLint majorVersion;
	EGLint minorVersion;

	//Get a EGLDisplay collection
	_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(_display == EGL_NO_DISPLAY)
	{
		return false;
	}

	//Initialize EGL
	if(!eglInitialize(_display, &majorVersion, &minorVersion))
	{
		return false;
	}

	// Determining the available Surface Configurations
	EGLConfig config;
	//Specify the surface attribList
	{
		GLint numConfigs = 0;
		EGLint attribList[] = {
				EGL_RED_SIZE,             5,
				EGL_GREEN_SIZE,        6,
				EGL_BLUE_SIZE,            5,
				EGL_ALPHA_SIZE,			(flags&CONFIG_ALPHA)?8:EGL_DONT_CARE,
				EGL_DEPTH_SIZE, 		(flags&CONFIG_DEPTH)?8:EGL_DONT_CARE,
				EGL_STENCIL_SIZE, 		(flags&CONFIG_STENCIL)?8:EGL_DONT_CARE,
				EGL_SAMPLE_BUFFERS,  (flags&CONFIG_MULTISAMPLE)?1:0,
				EGL_RENDERABLE_TYPE, getContextRenderableType(_display),
				EGL_NONE
		};

		//Choose config
		if(!eglChooseConfig(_display, attribList, &config, 1, &numConfigs))
		{
			return false;
		}

		if(numConfigs<1)
		{
			return false;
		}
	}

	//need to get the EGL_NATIVE_VISUAL_ID and
	//set it using ANativeWindow_setBuffersGeometry
	{
		EGLint format = 0;
		eglGetConfigAttrib(_display, config, EGL_NATIVE_VISUAL_ID, &format);
		ANativeWindow_setBuffersGeometry(_window, 0, 0, format);
	}

	//create a surface
	EGLint winAttribList[] =  {
				EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
				EGL_NONE
		};
	_surface = eglCreateWindowSurface(_display, config, (EGLNativeWindowType) _window, winAttribList);
	if(_surface == EGL_NO_SURFACE)
	{
		return false;
	}



	//create a GL context
	EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
	_context = eglCreateContext(_display, config, EGL_NO_CONTEXT, contextAttribs);
	if(_context == EGL_NO_CONTEXT)
	{
		return false;
	}

	//make the context current
	if(!eglMakeCurrent(_display, _surface, _surface,  _context))
	{
		return false;
	}

	//Query width and height
		if(!eglQuerySurface(_display,  _surface,  EGL_WIDTH,  &width)
				|| !eglQuerySurface(_display, _surface, EGL_HEIGHT, &height))
		{
			destroy();
			return false;
		}

		glDisable(GL_DITHER);
		glHint(GL_PRIMITIVE_RESTART_FIXED_INDEX, GL_FASTEST);
		glClearColor(0, 0, 0, 0);
		glEnable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);


		Assimp::Importer Importer;
		const aiScene *pScene = Importer.ReadFile(" ", aiProcess_Triangulate|aiProcess_GenSmoothNormals);

	return true;
}


void IRBox::destroy()
{
	eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(_display, _context);
	eglDestroySurface(_display, _surface);
	eglTerminate(_display);

	_display = EGL_NO_DISPLAY;
	_surface = EGL_NO_SURFACE;
	_context = EGL_NO_CONTEXT;
	return ;
}


void IRBox::drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

EGLint IRBox::getContextRenderableType(EGLDisplay eglDisplay)
{
#ifdef EGL_KHR_create_context
	const char *extensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);
	//Check whether EGL_KHR_create_context is in the extension string
	if(extensions != NULL && strstr(extensions, "EGL_KHR_create_context"))
	{
		//extension is supported
		return EGL_OPENGL_ES3_BIT_KHR;
	}
#endif
	// extension is not supported
	return EGL_OPENGL_ES2_BIT;
}


void *IRBox::threadStartCallback(void *it)
{
	IRBox *ir = (IRBox*)it;
	ir->renderLoop();
	pthread_exit(0);
	return 0;
}

