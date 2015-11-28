//********************************************
//
//can 2015/11/27
//
//**********************************************

#include "Material.h"
#include "Modules.h"
#include "XML.h"

namespace IRBox{

using namespace std;

MaterialResource::MaterialResource(const string &name, int  flags):
		Resource(ResourceTypes::Material, name, flags)
{
	initDefault();
}


MaterialResource::~MaterialResource()
{
	release();
}

void MaterialResource::initDefault()
{
	_shaderRes = 0x0;
	_combMask = 0;
	_matLink = 0x0;
//	_class = "";
}


void MaterialResource::release()
{
	_shaderRes = 0x0;
	_matLink = 0x0;
//	for(uint32 i = 0; i<_samplers)

	_uniforms.clear();
}

bool MaterialResource::raiseError(const string &msg, int line)
{
	//Reset
	release();
	initDefault();

	if(line < 0)
		Modules::log(LogTypes::ERROR, "Material", "Material resource '%s':%s", _name.c_str(), msg.c_str());
	else
		Modules::log(LogTypes::ERROR, "Material", "Material resource '%s'in line %i: %s", _name.c_str(),line,  msg.c_str());
	return false;
}

bool MaterialResource::load(const char *data, int size)
{
	if(!Resource::load(data, size)) return false;

	XMLDoc doc;
	doc.parseBuffer(data, size);
	if(doc.hasError())
		return raiseError("XML parsing error");

	XMLNode rootNode = doc.getRootNode();
	if(strcmp(rootNode.getName(), "Material")!=0)
		return raiseError("Not a material resource file");

	//Class
	_class =  rootNode.getAttribute("class", "");

	//Link
	if(strcmp(rootNode.getAttribute("link", ""), "")!=0)
	{
		uint32 mat = Modules::resMan().addResource(
				ResourceTypes::Material, rootNode.getAttribute("link"), 0, false);
		_matLink = (MaterialResource *)Modules::resMan().resolveResHandle(mat);
		if(_matLink == this)
			return raiseError("Illegal self link in material, causing infinite link loop");
	}

	//Shader Flags
	XMLNode nodel = rootNode.getFirstChild("ShaderFlag");
	while(!nodel.isEmpty())
	{
		if(nodel.getAttribute("name")==0x0) return raiseError("Missing ShaderFlag attribute 'name'");

		_shaderFlags.push_back(nodel.getAttribute("name"));

		nodel= nodel.getNextSibling("ShaderFlag");
	}

	//Shader



}







}
