#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "IrPlatform.h"
#include "Resource.h"
#include "RenderBase.h"


namespace IRBox{

//=========================================================
//Texture Resource
//=============================================================

struct TextureResData
{
	enum List{
		TextureElem = 700,
		ImageElem,
		TexFormatI,
		TexSliceCountI,
		ImgWidthI,
		ImgHeightI,
		ImgPixelStream,
		TexNativeRefI
	};
};

class TextureResource : public Resource
{
public:
	static void initializationFunc();
	static void releaseFunc();
	static Resource *factoryFunc(const std::string &name, int flags)
	{
		return new TextureResource(name, flags);
	}

	TextureResource(const std::string &name, int flags);
	TextureResource(const std::string &name, uint32 width, uint32 height, uint32 depth,
			TextureFormats::List fmt, int flags);
	~TextureResource();

	void initDefault();
	void release();
	bool load(const char *data, int size);

public:
	static uint32 defTex2DObject;
	static uint32 defTex3DObject;
	static uint32 defTexCubeObject;

protected:
	bool loadDDS(const char *data, int size);

};


}










#endif
