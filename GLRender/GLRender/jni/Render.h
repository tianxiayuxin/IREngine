//*************************************************************
// can 2015/11/18
//
//
//***********************************************************

#ifndef _RENDER_H
#define _RENDER_H

#include "RenderBase.h"

namespace IRBox{


//==============================================
struct OverlayBatch
{
	PMaterialResource  materialRes;
	uint32             firstVert, vertCount;
	float              colRGBA[4];
	int                flags;

	OverlayBatch() {}

	OverlayBatch( uint32 firstVert, uint32 vertCount, float *col, MaterialResource *materialRes, int flags ) :
		materialRes( materialRes ), firstVert( firstVert ), vertCount( vertCount ), flags( flags )
	{
		colRGBA[0] = col[0]; colRGBA[1] = col[1]; colRGBA[2] = col[2]; colRGBA[3] = col[3];
	}
};


struct OverlayVert
{
	float x, y;//Position
	float u, v; //Texture coordinates
};


class Render
{
public :
	Render();
	~Render();

	bool init();
	void initStates();

//=====================================
// overlays
//====================================
	void showOverlays(const float *verts, uint32 vertCount, float *colRGBA, MaterialResource *matRes, int flags);
	void clearOverlays();

protected:
	void drawOverlays(const std::string &shaderContext);

protected:


	std::vector<OverlayBatch>       _overlayBatches;
	OverlayVert                     *_overlayVerts;
	uint32                          _overlayVB;

	uint32                          _quadIdxBuf;


	ShaderCombination           *curShader;

};


}



#endif// _RENDER_H
