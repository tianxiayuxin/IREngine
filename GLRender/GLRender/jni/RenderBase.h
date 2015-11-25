//*******************************
//can 2015/11/17
//
//*******************************

#ifndef _RENDERBASE_H
#define _RENDERBASE_H
#include "IrPlatform.h"
#include "Logger.h"
#include <vector>



namespace IRBox{
const uint32 MaxNumVertexLayouts = 16;

//------------------------------------------------------
//General
//------------------------------------------------------

template< class T > class RBObjects
{
public:

	uint32 add( const T &obj )
	{
		if( !_freeList.empty() )
		{
			uint32 index = _freeList.back();
			_freeList.pop_back();
			_objects[index] = obj;
			return index + 1;
		}
		else
		{
			_objects.push_back( obj );
			return (uint32)_objects.size();
		}
	}

	void remove( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );

		_objects[handle - 1] = T();  // Destruct and replace with default object
		_freeList.push_back( handle - 1 );
	}

	T &getRef( uint32 handle )
	{
		ASSERT( handle > 0 && handle <= _objects.size() );

		return _objects[handle - 1];
	}

	friend class RenderBase;

private:
	std::vector< T >       _objects;
	std::vector< uint32 >  _freeList;
};



struct DeviceCaps
{
	bool texFloat;
	bool texNPOT;
	bool rtMultisampling;
};



//----------------------------------------------------------
//Buffers
//-----------------------------------------------------------
struct RBBuffer
{
	uint32 type;
	uint32 glObj;
	uint32 size;
};

struct RBVertBufSlot
{
	uint32 vbObj;
	uint32 offset;
	uint32 stride;

	RBVertBufSlot(): vbObj(0), offset(0), stride(0){}
	RBVertBufSlot( uint32 vbObj, uint32 offset, uint32 stride ) :
			vbObj( vbObj ), offset( offset ), stride( stride ) {}
};


//----------------------------------------------------------
//Textures
//----------------------------------------------------------
struct TextureTypes
{
	enum List{
		Tex2D = GL_TEXTURE_2D,
		Tex3D = GL_TEXTURE_3D,
		TexCube = GL_TEXTURE_CUBE_MAP

	};
};


struct TextureFormats
{
	enum List
	{
		Unknown,
		BGRA8,
		DXT1,
		DXT3,
		DXT5,
		RGBA16F,
		RGBA32F,
		DEPTH
	};
};


struct RBTexture
{
	uint32               glObj;
	uint32               glFmt;
	int                  type;
	TextureFormats::List format;
	int                  width, height, depth;
	int                  memSize;
	uint32               samplerState;
	bool                 sRGB;
	bool                 hasMips, genMips;
};

struct RBTexSlot
{
	uint32  texObj;
	uint32  samplerState;

	RBTexSlot() : texObj( 0 ), samplerState( 0 ) {}
	RBTexSlot( uint32 texObj, uint32 samplerState ) :
		texObj( texObj ), samplerState( samplerState ) {}
};



//------------------------------------------------
//Shaders
//-------------------------------------------------
enum RBShaderConstType
{
	CONST_FLOAT,
	CONST_FLOAT2,
	CONST_FLOAT3,
	CONST_FLOAT4,
	CONST_FLOAT44,
	CONST_FLOAT33
};

struct RBInputLayout
{
	bool valid;
	int8 attribIndices[16];
};

struct RBShader
{
	uint32              oglProgramObj;
	RBInputLayout       inputLayouts[MaxNumVertexLayouts];
};


//---------------------------------------------
//Render buffers
//--------------------------------------------
struct RBRenderBuffer
{
	static const uint32 MaxColorAttachmentCount = 4;
	uint32 fbo, fboMS; // fboMS: Multisampled FBO used when samples > 0
	uint32 width, height;
	uint32 samples;

	uint32 depthTex, colTexs[MaxColorAttachmentCount];
	uint32 depthBuf, colBufs[MaxColorAttachmentCount];

	RBRenderBuffer() : fbo( 0 ), fboMS( 0 ), width( 0 ), height( 0 ), depthTex( 0 ), depthBuf( 0 )
	{
		for( uint32 i = 0; i < MaxColorAttachmentCount; ++i ) colTexs[i] = colBufs[i] = 0;
	}
};


//-------------------------------------------
//Render states
//-------------------------------------------
enum RBDepthFunc
{
	DSS_DEPTHFUNC_LESS_EQUAL = 0,
	DSS_DEPTHFUNC_LESS,
	DSS_DEPTHFUNC_EQUAL,
	DSS_DEPTHFUNC_GREATER,
	DSS_DEPTHFUNC_GREATER_EQUAL,
	DSS_DPETHFUNC_ALWAYS
};

struct RBDepthStencilState
{
	union
	{
		uint32 hash;
		struct
		{
			uint32 depthWriteMask : 1;
			uint32 depthEnable : 1;
			uint32 depthFunc : 4; // RBDepthFunc
		};
	};
};


//--------------------------------------
//Draw calls and clears
//-------------------------------------
enum RBIndexFormat
{
	IDXFMT_16 = GL_UNSIGNED_SHORT,
	IDXFMT_32 = GL_UNSIGNED_INT
};


class RenderBase{
public:
	RenderBase();
	~RenderBase();

	void initStates();
	bool init();
//--------------------------------------------------------------
//Resources
//--------------------------------------------------------------
	//Buffers
	void beginRendering();

	uint32 createVertexBuffer(uint32 size, const void *data);
	uint32 createIndexBuffer(uint32 size, const void *data);
	void destroyBuffer(uint32 bufObj);
	void  updateBufferData(uint32 bufObj, uint32 offset, uint32 size, void *data);
//	uint32 getBufferMem(){
//
//	}


	//Textures
	uint32 calcTextureSize(TextureFormats::List format, int width, int height, int depth);
	uint32 createTexture(TextureTypes::List type, int width, int height, int depth, TextureFormats::List format,
			bool hasMips, bool genMips, bool compress, bool sRGB);
	void uploadTextureData(uint32 texObj, int slice, int mipLevel, const void *pixels);
	void destroyTexture(uint32 texObj);



	//Shaders
	uint32 createShader(const char *vertexShaderSrc, const char *fragmentShaderSrc);
	void destroyShader(uint32 shaderId);
	void bindShader(uint32 shaderId);
	//std::string &getShaderLog(){};
	int getShaderConstLoc(uint32 shaderId, const char *name);
	int getShaderSamplerLoc(uint32 shaderId, const char *name);
	void setShaderConst(int loc, RBShaderConstType type, void *values, uint32 count = 1);
	void setShaderSampler(int loc, uint32 texUnit);
	const char *getDefaultVSCode();
	const char *getDefaultFSCode();

	//Renderbuffers
	uint32 createRenderBuffer(uint32 width, uint32 height, TextureFormats::List format,
			bool depth, uint32 numColBufs, uint32 samples);
	void destroyRenderBuffer(uint32 rbObj);
	uint32 getRenderBufferTex(uint32 rbObj, uint32 bufIndex);
	void setRenderBuffer(uint32 rbObj);


//----------------------------------------------------
//Commands
//----------------------------------------------------
	void setViewport(int x, int y, int width, int height)
	{
		_vpX = x; _vpY = y; _vpWidth = width; _vpHeight = height; _pendingMask |= PM_VIEWPORT;
	}

//	void setScissorRect(int x, int y, int width, int height);

	void setIndexBuffer(uint32 bufObj, RBIndexFormat idxFmt)
	{
		_indexFormat = (uint32)idxFmt; _newIndexBuf = bufObj;_pendingMask |= PM_INDEXBUF;
	}

	void setVertexBuffer(uint32 slot, uint32 vbObj, uint32 offset, uint32 stride)
	{
		ASSERT(slot <16);_vertBufSlots[slot] = RBVertBufSlot( vbObj, offset, stride );
	      _pendingMask |= PM_VERTLAYOUT;

	}

	void setTexture(uint32 slot, uint32 texObj, uint16 samplerState)
	{
		ASSERT( slot < 16 ); _texSlots[slot] = RBTexSlot( texObj, samplerState );
			      _pendingMask |= PM_TEXTURES;
	}

	//Render states
	void resetStates();

	void setDepthMask( bool enabled )
	{ _newDepthStencilState.depthWriteMask = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthMask( bool &enabled )
	{ enabled = _newDepthStencilState.depthWriteMask; }
	void setDepthTest( bool enabled )
	{ _newDepthStencilState.depthEnable = enabled; _pendingMask |= PM_RENDERSTATES; }
	void getDepthTest( bool &enabled )
	{ enabled = _newDepthStencilState.depthEnable; }
	void setDepthFunc( RBDepthFunc depthFunc )
	{ _newDepthStencilState.depthFunc = depthFunc; _pendingMask |= PM_RENDERSTATES; }
	void getDepthFunc( RBDepthFunc &depthFunc )
	{ depthFunc = (RBDepthFunc)_newDepthStencilState.depthFunc; }


protected:
	enum RDIPendingMask
	{
		PM_VIEWPORT      = 0x00000001,
		PM_INDEXBUF      = 0x00000002,
		PM_VERTLAYOUT    = 0x00000004,
		PM_TEXTURES      = 0x00000008,
		PM_SCISSOR       = 0x00000010,
		PM_RENDERSTATES  = 0x00000020
	};

protected:
	uint32 createShaderProgram( const char *vertexShaderSrc, const char *fragmentShaderSrc );
	bool linkShaderProgram( uint32 programObj );

	void checkGLError();



protected:
    DeviceCaps    _caps;

	uint32        _depthFormat;
	int           _vpX, _vpY, _vpWidth, _vpHeight;
//	int           _scX, _scY, _scWidth, _scHeight;
	int           _fbWidth, _fbHeight;
	std::string   _shaderLog;
	uint32        _curRendBuf;
	int           _outputBufferIndex;  // Left and right eye for stereo rendering
	uint32        _textureMem, _bufferMem;

	int                            _defaultFBO;
	uint32                         _numVertexLayouts;
//	RDIVertexLayout                _vertexLayouts[MaxNumVertexLayouts];
	RBObjects< RBBuffer >        _buffers;
	RBObjects< RBTexture >       _textures;
	RBObjects< RBShader >        _shaders;
//	RBObjects< RBRenderBuffer >  _rendBufs;

	RBVertBufSlot        _vertBufSlots[16];
	RBTexSlot            _texSlots[16];
//	RDIRasterState        _curRasterState, _newRasterState;
//	RDIBlendState         _curBlendState, _newBlendState;
	RBDepthStencilState  _curDepthStencilState, _newDepthStencilState;
	uint32                _prevShaderId, _curShaderId;
	uint32                _curVertLayout, _newVertLayout;
	uint32                _curIndexBuf, _newIndexBuf;
	uint32                _indexFormat;
	uint32                _activeVertexAttribsMask;
	uint32                _pendingMask;

};
}



#endif// RENDERBASE_H
