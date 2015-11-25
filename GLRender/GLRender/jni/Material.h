//===========================================
// can 2015/11/21
//==========================================

#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "IrPlatform.h"
#include "Resource.h"
#include "Shader.h"

namespace IRBox{

//=================================================================
//Material Resource
//======================================================================

struct MaterialResData
{
	enum List
	{
		MaterialElem = 400,
		SamplerElem,
		UniformElem,
		MatClassStr,
		MatLinkI,
		MatShaderI,
		SampNameStr,
		SampTexResI,
		UnifNameStr,
		UnifValueF4
	};
};


struct MatUniform
{
	std::string  name;
	float        values[4];

	MatUniform()
	{
		values[0] = 0; values[1] =0; values[2] = 0; values[3] = 0;
	}
};

//======================================================================
class MaterialResource;
typedef SmartResPtr<MaterialResource> PMaterialResource;

class MaterialResource : public Resource
{
public:
	static Resource *factoryFunc(const std::string &name, int flags)
	{
		return new MaterialResource(name, flags);
	}

	MaterialResource(const std::string name, int flags);
	~MaterialResource();
//  Resource *clone();

	void intDefault();
	void release();
	void load(const char *data, int size);
	bool setUniform(const std::string &name, float a, float b, float c, float d);

	//TODO:  some methods should be specified

private:
	PShaderResource           _shaderRes;
	uint32                    _combMask;
	std::vector<MatUniform>   _uniforms;
	std::vector<std::string>  _shaderFlags;

	PMaterialResource         _matLink;

	friend class              ResourceManager;
	friend class              Render;
};

}

#endif
