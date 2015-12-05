// ===============================================================
// can 2015/11/21
//=================================================================

#ifndef  _SHADER_H
#define  _SHADER_H

#include "IrPlatform.h"
#include "Resource.h"
#include <set>

namespace IRBox{

//==================================================================
//Code Resource
//==================================================================

class CodeResource;
typedef SmartResPtr<CodeResource> PCodeResource;

class  CodeResource : public Resource
{
public:
	static Resource *factoryFunc(const std::string &name, int flags)
	{
		return new CodeResource(name, flags);
	}



	CodeResource(const std::string &name, int flags);
	~CodeResource();

	void initDefault();
	void release();
	bool load(const char *data, int size);

	bool hasDependency(CodeResource *codeRes);
	bool tryLinking(uint32 *flagMask);
	std::string assembleCode();

private:
	bool raiseError(const std::string &msg);
	void updateShaders();

private:
	uint32                 _flagMask;
	std::string            _code;
	std::vector<std::pair<PCodeResource, size_t> > _includes; //Pair: Included res and location in -code


	friend class Render;
};

//==============================================================
//Shader Resource
//==================================================================
struct ShaderResData
{
	enum List
	{
		ContextElem = 600,
		UniformElem,
		UnifNameStr,
		UnifSizeI,
		UnifDefValueF4
	};
};

struct TestModes
{
	enum List
	{
		LessEqual,
		Less,
		Equal,
		Greater,
		GreaterEqual,
		Always
	};
};

struct ShaderCombination
{
	uint32              combMask;

	uint32              shaderObj;
	uint32              lastUpdateStamp;

	// Engine uniforms
	int                 uni_frameBufSize;
	int                 uni_viewMat, uni_viewMatInv, uni_projMat, uni_viewProjMat, uni_viewProjMatInv, uni_viewerPos;
	int                 uni_worldMat, uni_worldNormalMat, uni_nodeId, uni_customInstData;
	int                 uni_skinMatRows;
	int                 uni_lightPos, uni_lightDir, uni_lightColor;
	int                 uni_shadowSplitDists, uni_shadowMats, uni_shadowMapSize, uni_shadowBias;
	int                 uni_parPosArray, uni_parSizeAndRotArray, uni_parColorArray;
	int                 uni_olayColor;

	std::vector< int >  customSamplers;
	std::vector< int >  customUniforms;


	ShaderCombination() :
		combMask( 0 ), shaderObj( 0 ), lastUpdateStamp( 0 )
	{
	}
};

struct ShaderContext
{
	std::string                       id;
	uint32                            flagMask;

	// RenderConfig
//	BlendModes::List                  blendMode;
	TestModes::List                   depthFunc;
//	CullModes::List                   cullMode;
	bool                              depthTest;
	bool                              writeDepth;
	bool                              alphaToCoverage;

	// Shaders
	std::vector< ShaderCombination >  shaderCombs;
	int                               vertCodeIdx, fragCodeIdx;
	bool                              compiled;


	ShaderContext() :
		/*blendMode( BlendModes::Replace ),*/ depthFunc( TestModes::LessEqual ),
		/*cullMode( CullModes::Back ),*/ depthTest( true ), writeDepth( true ), alphaToCoverage( false ),
		vertCodeIdx( -1 ), fragCodeIdx( -1 ), compiled( false )
	{
	}
};

//====================================================================================
struct ShaderSampler
{
	std::string          id;
	TextureTypes::List   type;
//	PTextureResource     defTex;
	int                  texUnit;
	uint32               sampState;
};


struct ShaderUniform
{
	std::string   id;
	float         defValues[4];
	unsigned char size;
};

class ShaderResource :  public Resource
{
public:
	static Resource *factoryFunc(const std::string &name, int flags)
	{
		return new ShaderResource(name, flags);
	}


	static uint32 calcCombMask(const std::vector<std::string> &flags);

	ShaderResource(const std::string &name, int flags);
	~ShaderResource();

	void initDefault();
	void release();
	bool load(const char *data, int size);
	void preLoadCombination(uint32 combMask);
	void compileContexts();
	ShaderCombination *getCombination(ShaderContext &context, uint32 combMask);

	std::vector<ShaderContext> &getContexts(){return _contexts;}
	CodeResource *getCode(uint32 index){return &_codeSections[index];}

private:
	bool raiseError(const std::string &msg, int line = -1);
	bool parseFXSection(char *data);
	void compileCombination(ShaderContext &context, ShaderCombination &sc);

private:
	static std::string           _vertPreamble, _fragPreamble;
	static std::string           _tmpCode0,  _tmpCode1;

	std::vector<ShaderContext>   _contexts;
	std::vector<ShaderUniform>   _uniforms;
	std::vector<CodeResource>    _codeSections;
	std::set<uint32>             _preLoadList;

	friend class Render;
};

typedef SmartResPtr<ShaderResource> PShaderResource;
}

#endif
