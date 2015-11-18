//***********************************************************
//can 2015/11//18
//
//*********************************************************

#include "RenderBase.h"



namespace IRBox{
static const char *defaultShaderVS =
	"uniform mat4 viewProjMat;\n"
	"uniform mat4 worldMat;\n"
	"attribute vec3 vertPos;\n"
	"void main() {\n"
	"	gl_Position = viewProjMat * worldMat * vec4( vertPos, 1.0 );\n"
	"}\n";

static const char *defaultShaderFS =
	"uniform vec4 color;\n"
	"void main() {\n"
	"	gl_FragColor = color;\n"
	"}\n";


//==============================================
//RenderDevice
//=============================================



RenderBase::RenderBase()
{
	_vpX = 0; _vpY = 0; _vpWidth = 320; _vpHeight = 240;
	_prevShaderId = _curShaderId = 0;
	_curRendBuf = 0; _outputBufferIndex = 0;
	_textureMem = 0; _bufferMem = 0;
	_curDepthStencilState.hash = _newDepthStencilState.hash = 0;
	_curIndexBuf = _newIndexBuf = 0;
	_defaultFBO = 0;
	_indexFormat = (uint32)IDXFMT_16;
	_pendingMask = 0;
}

RenderBase::~RenderBase()
{

}

void RenderBase::initStates()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

bool RenderBase::init()
{
	initStates();
	return true;
}

void RenderBase::beginRendering()
{
	//Get the currently bound frame buffer object
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_defaultFBO);
}


void RenderBase::resetStates()
{
	_curIndexBuf = 1; _newIndexBuf = 0;
	_curDepthStencilState.hash = 0xFFFFFFFF; _newDepthStencilState.hash = 0;

	for(uint32 i = 0; i <16; i++)
	{
		setTexture(i,  0, 0);
	}
	_pendingMask = 0xFFFFFFFF;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);

}

//uint32 RenderBase::createVertexBuffer(uint32 size, const void *data)
//{
//
//}


//===================================================================
//Shaders
//====================================================================
uint32 RenderBase::createShaderProgram(const char *vertexShaderSrc, const char *fragmentShaderSrc)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";

	//Vertex shader
	uint32 vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShaderSrc, 0x0);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if(!status)
	{
		//Get info
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH,  &infologLength);
		if(infologLength >1)
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog(vs, infologLength, &charsWritten, infoLog);
			_shaderLog = _shaderLog + "[Vertex Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader(vs);
		return 0;
	}

	// Fragment shader
	uint32 fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragmentShaderSrc, 0x0 );
	glCompileShader( fs );
	glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
	if( !status )
	{
		glGetShaderiv( fs, GL_INFO_LOG_LENGTH, &infologLength );
		if( infologLength > 1 )
		{
			infoLog = new char[infologLength];
			glGetShaderInfoLog( fs, infologLength, &charsWritten, infoLog );
			_shaderLog = _shaderLog + "[Fragment Shader]\n" + infoLog;
			delete[] infoLog; infoLog = 0x0;
		}

		glDeleteShader( vs );
		glDeleteShader( fs );
		return 0;
	}

	//Shader program
	uint32 program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}


bool RenderBase::linkShaderProgram(uint32 programObj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog = 0x0;
	int status;

	_shaderLog = "";

	glLinkProgram( programObj );
	glGetProgramiv( programObj, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[infologLength];
		glGetProgramInfoLog( programObj, infologLength, &charsWritten, infoLog );
		_shaderLog = _shaderLog + "[Linking]\n" + infoLog;
		delete[] infoLog; infoLog = 0x0;
	}

	glGetProgramiv( programObj, GL_LINK_STATUS, &status );
	if( !status ) return false;

	return true;
}

//======================================================================
//Internal state management
//======================================================================
void RenderBase::checkGLError()
{
	uint32 error = glGetError();
	ASSERT( error != GL_INVALID_ENUM );
	ASSERT( error != GL_INVALID_VALUE );
	ASSERT( error != GL_INVALID_OPERATION );
	ASSERT( error != GL_OUT_OF_MEMORY );
	//ASSERT( error != GL_STACK_OVERFLOW && error != GL_STACK_UNDERFLOW );
}


}



