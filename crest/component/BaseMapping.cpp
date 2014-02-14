//File: RigidMapping.cpp

#include <crest/core/Config.h>
#include <crest/component/BaseMapping.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>

namespace cia3d{
namespace component{

using namespace cia3d::core;

void BaseMapping::topologyChange(void)
{
}

void BaseMapping::parse(ZBaseObjectDescription* arg )
{
	//set name
	std::string objname = arg->attribute("name");
	this->SetObjectName(objname.c_str());

	//set parameters
	ObjectLib * objlib = ObjectLib::getInstance();	
	std::string srcobjname = arg->attribute("source", "");
	std::string dstobjname = arg->attribute("destination", "");
	CObjectBase *psrcobj = objlib->find_object(srcobjname.c_str());
	CObjectBase *pdstobj = objlib->find_object(dstobjname.c_str());
	if (psrcobj==NULL){
		fprintf(stderr, "Object %s not found.\n", srcobjname.c_str());
		return;
	}
	if (pdstobj==NULL){
		fprintf(stderr, "Object %s not found.\n", dstobjname.c_str());
		return;
	}
	CPolyObj *psrc = dynamic_cast<CPolyObj*>(psrcobj);
	CPolyObj *pdst = dynamic_cast<CPolyObj*>(pdstobj);
	if (psrc==NULL || pdst==NULL){
		fprintf(stderr, "Object type mis-match while parsing %s\n", this->GetObjectName());
		return;
	}

	//assign the pointers, plus init the mapping things
	this->init(psrc, pdst);
}

} // namespace component
} // namespace cia3d
