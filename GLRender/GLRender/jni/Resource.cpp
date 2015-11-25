#include "Resource.h"


namespace IRBox{
using namespace std;
//*********************************************************
//Class Resource
//*********************************************************

Resource::Resource(int type, const string &name, int flags)
{
	_type = type;
	_name = name;
	_handle = 0;
	_loaded = false;
	_refCount = 0;
	_userRefCount = 0;
	_flags = flags;

	if((flags & ResourceFlags::NoQuery)==ResourceFlags::NoQuery)_noQuery =  true;
	else _noQuery = false;
}

Resource::~Resource(){

}

void Resource::initDefault()
{

}

void Resource::release()
{

}

bool Resource::load(const char *data, int size)
{
	//Resource can only be loaded once
	if(_loaded) return false;

	// A NULL pointer can be used if the file could not be loaded
	if(data == 0x0 || size <0)
	{
		//Log
		_noQuery = true;
		return false;
	}

	_loaded = true;

	return true;
}

void Resource::unload()
{
	release();
	initDefault();
	_loaded = false;
}


//***********************************************************
//Class ResourceManager
//**********************************************************

ResourceManager::ResourceManager()
{
	_resources.reserve(100);
}


ResourceManager::~ResourceManager()
{
	clear();

	//Release resource types
	map<int, ResourceRegEntry>::const_iterator itr = _registry.begin();
	while(itr !=_registry.end())
	{
		if(itr->second.releaseFunc != 0x0)
		{
			(*itr->second.releaseFunc)();
		}
		++itr;
	}
}



void ResourceManager::registerResType(int resType, const string &typeString, ResTypeInitializationFunc inf,
		ResTypeReleaseFunc rf, ResTypeFactoryFunc ff)
{
	ResourceRegEntry entry;
	entry.typeString = typeString;
	entry.initializationFunc = inf;
	entry.releaseFunc = rf;
	entry.factoryFunc = ff;
	_registry[resType] = entry;

	//Initialize resource type
	if(inf != 0)(*inf)();
}


ResHandle ResourceManager::addResource(Resource &resource)
{
	// Try to insert resource in free slot
	for(uint32 i= 0; i<_resources.size(); ++i)
	{
		if(_resources[i]==0x0)
		{
			resource._handle = i+1;
			_resources[i] = &resource;
			return i+1;
		}
	}

	// If there is no free slot, add Resource to end
	resource._handle = (ResHandle)_resources.size() +1;
	_resources.push_back(&resource);
	return resource._handle;
}


ResHandle ResourceManager::addResource(int type, const string &name, int flags, bool userCall)
{
	if(name == "")
	{
		//TODO:  log
		return 0;
	}

	//Check if resource is already in list and return index
	for(uint32 i = 0; i < _resources.size(); ++i)
	{
		if(_resources[i]!=0x0 && _resources[i]->_name == name)
		{
			if(_resources[i]->_type == type)
			{
				if(userCall)++_resources[i]->_userRefCount;
				return i+1;
			}
		}
	}

	//Create resource
	Resource *resource = 0x0;
	map<int, ResourceRegEntry>::iterator itr = _registry.find(type);
	if(itr!= _registry.end())resource = (*itr->second.factoryFunc)(name, flags);
	if(resource == 0x0) return 0;

	if(userCall) resource->_userRefCount = 1;
	return addResource(*resource);
}

int ResourceManager::removeResource(Resource &resource, bool userCall)
{
	// Decrease reference count
	if(userCall && resource._userRefCount>0)--resource._userRefCount;
	return (signed)resource._userRefCount;
}

void ResourceManager::clear()
{
	//Release resources and remove dependencies
	for(uint32 i = 0; i < _resources.size();  ++i)
	{
		if(_resources[i]!=0x0)_resources[i]->release();
	}

	//Delete resources
	for(uint32 i = 0; i<_resources.size(); i++)
	{
		if(_resources[i]!=0x0)
		{
			delete _resources[i]; _resources[i]=0x0;
		}
	}
}

ResHandle  ResourceManager::queryUnloadedResource(int index)
{
	int j = 0;
	for(uint32 i = 0; i<_resources.size(); i++)
	{
		if(_resources[i]!= 0x0 && !_resources[i]->_loaded && !_resources[i]->_noQuery)
		{
			if(j == index) return _resources[i]->_handle;
			else ++j;
		}
	}
	return 0;
}




}
