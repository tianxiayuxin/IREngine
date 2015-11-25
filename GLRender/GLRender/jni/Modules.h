#ifndef _MODULES_H
#define _MODULES_H

#include "IrPlatform.h"
#include "Logger.h"


namespace IRBox{

struct LogTypes{
	enum List{
		INFO,
		WARNING,
		ERROR
	};
};


//class SceneManager;
class ResourceManager;
class RenderBase;
class Render;

//===================================================
//Modules
//====================================================
class Modules
{
public:
	static bool init();
	static void release();

	static void log(int logType, const std::string &name);

	static ResourceManager  &resMan(){return *_resourceManager;}
	static Render &render(){return *_render;}
private:
	static ResourceManager            *_resourceManager;
	static RenderBase                 *_renderBase;
	static Render                     *_render;
};

}

#endif
