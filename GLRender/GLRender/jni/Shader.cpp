// =====================================================
//can 2015/11/21
// =========================================================

#include "Shader.h"
#include "Render.h"
#include "Logger.h"
#include <cstring>
#include <string>
#include <string.h>



namespace IRBox{

using namespace std;
//=============================================================
//Code Resource
//=============================================================

CodeResource::CodeResource(const string &name, int flags):
		Resource(ResourceTypes::Code,  name, flags)
{
	initDefault();
}

CodeResource::~CodeResource()
{
	release();
}

//       clone

void CodeResource::initDefault()
{
	_flagMask = 0;
	_code.clear();
}

void CodeResource::release()
{
	for(uint32 i = 0; i<_includes.size(); ++i)
	{
		_includes[i].first = 0x0;
	}
	_includes.clear();
}


bool CodeResource::raiseError(const std::string &msg)
{
	//Reset
	release();
	initDefault();

	Modules::log(LogTypes::ERROR, "CodeResource", "'%s':%s", _name.c_str(), msg.c_str());
	return false;
}

bool CodeResource::load(const char *data, int size)
{
	if(!Resource::load(data, size)) return false;

	char *code = new char[size+1];
	char *pCode = code;
	const char *pData = data;
	const char *eof = data+size;

	bool lineComment = false, blockComment = false;

	//Parse code
	while(pData < eof)
	{
		// Check for begin of comment
		if(pData < eof -1 && !lineComment && !blockComment)
		{
			if(*pData == '/' &&*(pData+1)=='/')
				lineComment = true;
			else if(*pData == '/'&&*(pData+1)=='*')
				blockComment = true;
		}

		//Check for end of comment
		if(lineComment && (*pData == '\n'||*pData == '\r'))
			lineComment = false;
		else if(blockComment&& pData< eof-1 &&*pData == '*'&& *(pData+1)=='/')
			blockComment = false;

		//Check for includes
		if(!lineComment && !blockComment && pData < eof -7)
		{
			if( *pData == '#' && *(pData+1) == 'i' && *(pData+2) == 'n' && *(pData+3) == 'c' &&
			  *(pData+4) == 'l' && *(pData+5) == 'u' && *(pData+6) == 'd' && *(pData+7) == 'e' )
			{
				pData +=6;

				//Parse resource name
				const char *nameBegin = 0x0, *nameEnd = 0x0;

				while(++pData < eof)
				{
					if(*pData == '"')
					{
						if(nameBegin == 0x0)
							nameBegin = pData +1;
						else
							nameEnd = pData;
					}
					else if(*pData == '\n' || *pData == '\r')break;
				}

				if(nameBegin != 0x0 && nameEnd != 0x0)
				{
					std::string resName(nameBegin, nameEnd);

					ResHandle res = Modules::resMan().addResource(
							ResourceTypes::Code, resName, 0, false);
					CodeResource *codeRes = (CodeResource *)Modules::resMan().resolveResHandle(res);
					_includes.push_back(std::pair<PCodeResource, size_t>(codeRes, pCode-code));
				}else
				{
					delete[] code;
					return raiseError("Invalid #include syntax");
				}
			}
		}

		//Check for flags
		if(!lineComment && !blockComment && pData < eof-4)
		{
			if( *pData == '_' && *(pData+1) == 'F' && *(pData+4) == '_' &&
			   *(pData+2) >= 48 && *(pData+2) <= 57 && *(pData+3) >= 48 && *(pData+3) <= 57 )
			{
				//Set flag
				uint32 num = (*(pData+2)-48)*10 + (*(pData+3)-48);
				_flagMask |= 1 <<(num-1);

				for(uint32 i = 0; i <5; ++i)*pCode++ = *pData++;

				//Ignore rest of name
				while(pData < eof && *pData != ' '&&*pData!='\t'&& *pData!='\n'&&*pData != '\r')
				{
					++pData;
				}
			}
		}

		*pCode++ = *pData++;
	}
	*pCode = '\0';
	_code = code;
	delete[] code;

	//Compile shaders that require this code block
	updateShaders();

	return true;
}

bool CodeResource::hasDependency(CodeResource *codeRes)
{
	//Note: There is no check for  cycles

	if(codeRes ==  this) return true;

	for(uint32 i = 0; i < _includes.size(); ++i)
	{
		if(_includes[i].first->hasDependency(codeRes)) return true;
	}

	return false;
}


bool CodeResource::tryLinking(uint32 *flagMask)
{
	if(!_loaded)return false;
	if(flagMask!=0x0)*flagMask |= _flagMask;

	for(uint32 i=0; i<_includes.size();++i)
	{
		if(!_includes[i].first->tryLinking(flagMask))return false;
	}
	return true;
}

void CodeResource::updateShaders()
{
	for(uint32 i = 0; i < Modules::resMan().getResource().size(); ++i)
	{
		Resource *res = Modules::resMan().getResource()[i];

		if(res != 0x0 && res->getType()==ResourceTypes::Shader)
		{
			ShaderResource *shaderRes = (ShaderResource *)res;

			//Mark shaders using this code as uncompiled
			for(uint32 j = 0; j<shaderRes->getContexts().size();  ++j)
			{
				ShaderContext &context = shaderRes->getContexts()[j];

				if(shaderRes->getCode(context.vertCodeIdx)->hasDependency(this)||
						shaderRes->getCode(context.fragCodeIdx)->hasDependency(this))
				{
					context.compiled = false;
				}
			}

			//Recompile shaders
			shaderRes->compileContexts();
		}
	}
}


//============================================================
//Shader  Resource
//=============================================================
class Tokenizer
{
public:
	Tokenizer(const char *data):_p(data), _line(1){getNextToken();}

	int getLine(){return _line;}

	bool hasToken(){return _token[0]!='\0';}

	bool checkToken(const char *token, bool peekOnly = false)
	{
		if(strcasecmp(_token, token)==0)
		{
			if(!peekOnly)getNextToken();
			return true;
		}
		return false;
	}

	const char *getToken(const char *charset)
	{
		if(charset)
		{
			//Validate token
			const char *p = _token;
			while(*p)
			{
				if(strchr(charset, *p++)==0x0)
				{
					_prevToken[0] = '\0';
					return _prevToken;
				}
			}
		}

		memcpy(_prevToken, _token, tokenSize);
		getNextToken();
		return _prevToken;
	}


	bool seekToken(const char *token)
	{
		while(strcasecmp(getToken(0x0), token)!= 0)
		{
			if(!hasToken())return false;
		}
		return true;
	}

protected:

	void checkLineChange()
	{
		if(*_p == '\r'&&*(_p+1)=='\n')
		{
			++_p;
			++_line;
		}
		else if(*_p =='\r' || *_p == '\n')++_line;
	}


	void skip(const char *chars)
	{
		while(*_p)
		{
			if(!strchr(chars, *_p))break;
			checkLineChange();
			++_p;
		}
	}

	bool seekChar(const char *chars)
	{
		while(*_p)
		{
			if(strchr(chars, *_p))break;
			checkLineChange();
			++_p;
		}
		return *_p != '\0';
	}

	void getNextToken()
	{
		//Skip whitespace
		skip(" \t\n\r");

		//Parse token
		const char *p0 = _p, *p1 = _p;
		if(*_p == '"') //Handle string
		{
			++_p; ++p0;
			if(seekChar("\"\n\r")) p1 = _p++;
		}else{
			seekChar(" \t\n\r{}()<>=,;"); //Advance until whitespace or special char found
			if(_p == p0 && *_p != '\0')++_p; //Handle special char
			p1 = _p;
		}
		memcpy(_token, p0, std::min((ptrdiff_t)(p1-p0), tokenSize-1));
		_token[std::min((ptrdiff_t)(p1-p0), tokenSize-1)]='\0';
	}


protected:

	static const ptrdiff_t  tokenSize = 128;

	char         _token[tokenSize], _prevToken[tokenSize];
	const char   *_p;
	int          _line;
};

//=========================================================================

string ShaderResource::_vertPreamble = "";
string ShaderResource::_fragPreamble = "";
string ShaderResource::_tmpCode0 = "";
string ShaderResource::_tmpCode1 = "";


ShaderResource::ShaderResource(const string &name, int flags):
		Resource(ResourceTypes::Shader, name, flags)
{
	initDefault();
}

ShaderResource::~ShaderResource()
{
	release();
}

void ShaderResource::initDefault()
{

}



void ShaderResource::release()
{
	for(uint32 i = 0; i <_contexts.size(); ++i)
	{
		for(uint32 j = 0; j <_contexts[i].shaderCombs.size(); ++j)
		{
			// call RenderBaseµÄ destroyShader
		}
	}

	_contexts.clear();
	_uniforms.clear();

	_codeSections.clear();
}

bool ShaderResource::raiseError(const string &msg, int line)
{
	//Reset
	release();
	initDefault();

	LOG_INFO("Shader", "Shader resource '%s':%s(line  %i)", _name.c_str(), msg.c_str(), line);

	if(line <0)
		LOG_ERROR("Shader", "Shader resource '%s':%s", _name.c_str(), msg.c_str());
	else
		LOG_ERROR("Shader", "Shader resource '%s':%s(line  %i)", _name.c_str(), msg.c_str(), line);

	return false;
}


bool ShaderResource::parseFXSection(char *data)
{
	//Pre-processing: Replace comments with whitespace
	char *p = data;
	while(*p)
	{
		if(*p == '/'&& *(p+1)=='/')
		{
			while(*p&&*p!='\n'&&*p!='\r')
				*p++ =' ';
			if(*p =='\0')break;
		}else if(*p == '/'&&*(p+1)=='*')
		{
			*p++=' '; *p++ = ' ';
			while(*p&&(*p != '*'||*(p+1)!='/'))
				*p++ =' ';
			if(*p == '\0')return raiseError("FX: Expected */");
			*p++=' '; *p++=' ';
		}
		++p;
	}

	//Parsing
	const char *identifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	const char *intnum = "+-0123456789";
	const char *floatnum = "+-0123456789.eE";

	bool unitFree[12] =  {true,  true, true, true, true, true, true, true, true, true, true, true};
	Tokenizer tok(data);

	while(tok.hasToken())
	{
		if(tok.checkToken("float"))
		{
			ShaderUniform uniform;
			uniform.size = 1;
			uniform.id = tok.getToken(identifier);
			if(uniform.id == "") return raiseError("FX:Invalid identifier", tok.getLine());
			uniform.defValues[0] = uniform.defValues[1] = uniform.defValues[2]=uniform.defValues[3]=0.0;

			//Skip annotations
			if(tok.checkToken("<"))
				if(!tok.seekToken(">"))return raiseError("FX: expected '>'", tok.getLine());

			if(tok.checkToken("="))
				uniform.defValues[0] = (float)atof(tok.getToken(floatnum));
			if(!tok.checkToken(";"))return raiseError("FX: expected ';'", tok.getLine());

			_uniforms.push_back(uniform);
		}
		else if(tok.checkToken("float4"))
		{
			ShaderUniform uniform;
			uniform.size = 4;
			uniform.id = tok.getToken( identifier );
			if( uniform.id == "" ) return raiseError( "FX: Invalid identifier", tok.getLine() );
			uniform.defValues[0] = uniform.defValues[1] = uniform.defValues[2] = uniform.defValues[3] = 0.0f;

			// Skip annotations
			if( tok.checkToken( "<" ) )
			if( !tok.seekToken( ">" ) ) return raiseError( "FX: expected '>'", tok.getLine() );

			if( tok.checkToken( "=" ) )
			{
				if( !tok.checkToken( "{" ) ) return raiseError( "FX: expected '{'", tok.getLine() );
				uniform.defValues[0] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[1] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[2] = (float)atof( tok.getToken( floatnum ) );
				if( tok.checkToken( "," ) ) uniform.defValues[3] = (float)atof( tok.getToken( floatnum ) );
				if( !tok.checkToken( "}" ) ) return raiseError( "FX: expected '}'", tok.getLine() );
			}
			if( !tok.checkToken( ";" ) ) return raiseError( "FX: expected ';'", tok.getLine() );

			_uniforms.push_back( uniform );
		}else if(tok.checkToken("sampler2D", true)||tok.checkToken("samplerCube", true)||
				tok.checkToken("sampler3D", true))
		{
			ShaderSampler sampler;
			sampler.sampState = SS_FILTER_TRILINEAR | SS_ANISO8 | SS_ADDR_WRAP;

			if(tok.checkToken("sampler2D"))
			{
				sampler.type = TextureTypes::Tex2D;
//				sampler.defTex = (TextureResource *)
			}
//          TODO: somthing to do with texture
		}else if(tok.checkToken("context"))
		{
			ShaderContext context;
			_tmpCod0 = tmpCode1 = "";
			context.id = tok.getToken(identifier);
			if(context.id == "")return raiseError("FX:Invalid identifier", tok.getLine());

			//Skip annotations
			if(tok.checkToken("<"))
				if(!tok.seekToken(">"))return raiseError("FX: expected '>'", tok.getLine());

			if(!tok.checkToken("{"))return raiseError("FX:expected '{'", tok.getLine());
			while(true)
			{
				if(!tok.hasToken())
					return raiseError("FX: expected '}'", tok.getLine());
				else if(tok.checkToken("}"))
					break;
				else if(tok.checkToken("ZWriteEnable"))
				{
					if(!tok.checkToken("=")) return raiseError("FX: expected '='", tok.getLine());
					if(tok.checkToken("true")) context.writeDepth = true;
					else if(tok.checkToken("false"))context.writeDepth = false;
					else return raiseError("FX: invalid bool value",  tok.getLine());
				}else if(tok.checkToken("ZEnable"))
				{
					if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
					if( tok.checkToken( "true" ) ) context.depthTest = true;
					else if( tok.checkToken( "false" ) ) context.depthTest = false;
					else return raiseError( "FX: invalid bool value", tok.getLine() );
				}else if(tok.checkToken("ZFunc"))
				{
					if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
					if( tok.checkToken( "LessEqual" ) ) context.depthFunc = TestModes::LessEqual;
					else if( tok.checkToken( "Always" ) ) context.depthFunc = TestModes::Always;
					else if( tok.checkToken( "Equal" ) ) context.depthFunc = TestModes::Equal;
					else if( tok.checkToken( "Less" ) ) context.depthFunc = TestModes::Less;
					else if( tok.checkToken( "Greater" ) ) context.depthFunc = TestModes::Greater;
					else if( tok.checkToken( "GreaterEqual" ) ) context.depthFunc = TestModes::GreaterEqual;
					else return raiseError( "FX: invalid enum value", tok.getLine() );
				}else if(tok.checkToken("BlendMode"))
				{
//
				}else if(tok.checkToken("CullMode"))
				{
//
				}else if(tok.checkToken("AlphaToCoverage"))
				{
					if( !tok.checkToken( "=" ) ) return raiseError( "FX: expected '='", tok.getLine() );
					if( tok.checkToken( "true" ) || tok.checkToken( "1" ) ) context.alphaToCoverage = true;
					else if( tok.checkToken( "false" ) || tok.checkToken( "1" ) ) context.alphaToCoverage = false;
					else return raiseError( "FX: invalid bool value", tok.getLine() );
				}else if(tok.checkToken("VertexShader"))
				{
					if( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
					return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
					_tmpCode0 = tok.getToken( identifier );
					if( _tmpCode0 == "" ) return raiseError( "FX: Invalid name", tok.getLine() );

				}else if(tok.checkToken("PixelShader"))
				{
					if( !tok.checkToken( "=" ) || !tok.checkToken( "compile" ) || !tok.checkToken( "GLSL" ) )
					return raiseError( "FX: expected '= compile GLSL'", tok.getLine() );
					_tmpCode1 = tok.getToken( identifier );
					if( _tmpCode1 == "" ) return raiseError( "FX: Invalid name", tok.getLine() );
				}else
					return raiseError("FX: unexpected token", tok.getLine());
				if(!tok.checkToken(";"))return raiseError("FX: expected ';'", tok.getLine());
			}

			//Handle shaders
			for(uint32 i = 0; i <_codeSections.size(); ++i)
			{
				if(_codeSections[i].getName()==_tmpCode0)context.vertCodeIdx = i;
				if(_codeSections[i].getName()==_tempCode1)context.fragCodeIdx = i;
			}

			if(context.vertCodeIdx <0)
				return raiseError("FX: Vertex shader referenced by context '" +context.id + "not found");
			if( context.fragCodeIdx < 0 )
				return raiseError( "FX: Pixel shader referenced by context '" + context.id + "' not found" );

			_contexts.push_back(context);
		}else
		{
			return raiseError("FX: unexpected token", tok.getLine());
		}

		// Automatic texture unit assignment
		//TODO:
	}
	return true;
}


bool ShaderResource::load(const char *data, int size)
{
	if(!Resource::load(data, size)) return false;

	//Parse sections
	const char *pData = data;
	const char *eof = data+size;

	char *fxCode = 0x0;

	while( pData < eof )
	{
		if( pData < eof-1 && *pData == '[' && *(pData+1) == '[' )
		{
			pData += 2;

			// Parse section name
			const char *sectionNameStart = pData;
			while( pData < eof && *pData != ']' && *pData != '\n' && *pData != '\r' ) ++pData;
			const char *sectionNameEnd = pData++;

			// Check for correct closing of name
			if( pData >= eof || *pData++ != ']' ) return raiseError( "Error in section name" );

			// Parse content
			const char *sectionContentStart = pData;
			while( (pData < eof && *pData != '[') || (pData < eof-1 && *(pData+1) != '[') ) ++pData;
			const char *sectionContentEnd = pData;

			if( sectionNameEnd - sectionNameStart == 2 &&
			    *sectionNameStart == 'F' && *(sectionNameStart+1) == 'X' )
			{
				// FX section
				if( fxCode != 0x0 ) return raiseError( "More than one FX section" );
				fxCode = new char[sectionContentEnd - sectionContentStart + 1];
				memcpy( fxCode, sectionContentStart, sectionContentEnd - sectionContentStart );
				fxCode[sectionContentEnd - sectionContentStart] = '\0';
			}
			else
			{
				// Add section as private code resource which is not managed by resource manager
				_tmpCode0.assign( sectionNameStart, sectionNameEnd );
				_codeSections.push_back( CodeResource( _tmpCode0, 0 ) );
				_tmpCode0.assign( sectionContentStart, sectionContentEnd );
				_codeSections.back().load( _tmpCode0.c_str(), (uint32)_tmpCode0.length() );
			}
		}
		else
			++pData;
	}

	if(fxCode == 0x0) return raiseError("Missing FX Section");
	bool result = parseFXSection(fxCode);
	delete[] fxCode; fxCode = 0x0;
	if(!result)return false;

	compileContexts();

	return true;
}

void ShaderResource::compileContexts()
{
	for(uint32 i = 0; i <_contexts.size(); ++i)
	{
		ShaderContext &context = _contexts[i];
		if(!context.compiled)
		{
			context.flagMask = 0;
			if(!getCode(context.vertCodeIdx)->tryLinking(&context.flagMask)||
					!getCode(context.fragCodeIdx)->tryLinking(&context.flagMask))
			{
				continue;
			}


			//Add preloaded combinations
//			for(std::set<uint32>::iterator itr = _preLoadList.begin(); itr!=_preLoadList.end(); ++itr)
//			{
//				uint32 combMask = *itr&context.flagMask;
//			}

		}


	}
}



}
