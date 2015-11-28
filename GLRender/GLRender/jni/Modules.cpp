//*****************************************************
//
//can 2015/11/20
//
//******************************************************

#include "Resource.h"
#include "RenderBase.h"
#include "Render.h"
#include "Modules.h"


namespace IRBox{

ResourceManager       *Modules::_resourceManager = 0x0;
RenderBase            *Modules::_renderBase = 0x0;
Render                *Modules::_render = 0x0;

RenderBase *gRB = 0x0;

bool Modules::init()
{
	if(_resourceManager == 0x0) _resourceManager = new ResourceManager();
	if(_renderBase == 0x0) _renderBase = new RenderBase();
	gRB = _renderBase;
	if(_render == 0x0) _render = new Render();

	//Init modules
	if(!render().init())return false;

	//Register resource types
/*	resMan().registerResType(ResourceTypes::SceneGraph, 0x0, 0x0,
			)*/

	resMan().registerResType(ResourceTypes::Material, "Material", 0x0, 0x0, MaterialResource::factoryFunc);



}

void Modules::log(int logType, const char* moduleName, ...)
{
	if(logType == LogTypes::INFO)
	{
		va_list args;
		va_start(args, msg);
		__android_log_vprint(ANDROID_LOG_INFO, "Modules", moduleName, args);
		va_end(args);
	}else if(logType == LogTypes::WARNING)
	{
		va_list args;
		va_start(args, msg);
		__android_log_vprint(ANDROID_LOG_WARN, "Modules", moduleName, args);
		va_end(args);
	}else if(logType == LogTypes::ERROR)
	{
		va_list args;
		va_start(args, msg);
		__android_log_vprint(ANDROID_LOG_ERROR, "Modules", moduleName, args);
		va_end(args);
	}
}


void Modules::release()
{
	//Remove overlays sincce they reference resources and resource manager is removed before renderer
	if(_render)_render->clearOverlays();

	//Order of destruction is important
	delete _resourceManager; _resourceManager = 0x0;
	delete _render;  _render = 0x0;
	delete _renderBase; _renderBase = 0x0;
	gRB = 0x0;
}



}
