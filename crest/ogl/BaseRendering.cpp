//File: BaseRendering.cpp

#include <crest/core/Config.h>
#include <crest/ogl/BaseRendering.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>

namespace cia3d{
namespace ogl{

using namespace cia3d::core;


void BaseRendering::parse(ZBaseObjectDescription* arg )
{
	std::string objname = arg->attribute("name");
	this->SetObjectName(objname.c_str());

	ObjectLib * objlib = ObjectLib::getInstance();	
	std::string srcobjname = arg->attribute("source", "");
	std::string mappingname = arg->attribute("mapping", "");
	CObjectBase *psrcobj = objlib->find_object(srcobjname.c_str());
	CObjectBase *pmappingobj = objlib->find_object(mappingname.c_str());
	if (psrcobj==NULL){
		fprintf(stderr, "Object %s not found.\n", srcobjname.c_str());
		return;
	}
	if (pmappingobj==NULL)
		fprintf(stderr, "Object %s not found.\n", mappingname.c_str());

	CPolyObj *psrc = dynamic_cast<CPolyObj*>(psrcobj);
	CBaseMapping *pmapping = dynamic_cast<CBaseMapping*>(pmappingobj);
	if (psrc==NULL){
		fprintf(stderr, "Object type mis-match while parsing %s\n", this->GetObjectName());
		return;
	}

	//assign the pointers, plus init the mapping things
	this->_init(psrc, pmapping);
}

} // namespace ogl
} // namespace cia3d
