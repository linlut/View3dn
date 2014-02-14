#ifndef _CIA3D_XML_ELEMENT_H
#define _CIA3D_XML_ELEMENT_H

#include <vector>
#include <core/baseelement.h>
#include <core/factory.h>

namespace cia3d
{
namespace xml
{


template<class Object>
class Element : public BaseElement
{
private:
    Object* object;
public:
    Element(const std::string& name, const std::string& type, BaseElement* newParent=NULL);
	
    virtual ~Element();

    Object* getTypedObject();

    virtual void setObject(Object* newObject);

    /// Get the associated object
    virtual core::objectmodel::Base* getObject();

    virtual bool initNode();

    typedef helper::Factory< std::string, Object, Element<Object>* > Factory;

};

template<class Object>
void createWithFilename(Object*& obj, BaseElement* arg)
{
	const char* filename = arg->getAttribute("filename");
	if (!filename)
	{
		std::cerr << arg->getType() << " requires a filename attribute\n";
		obj = NULL;
	}
	else
		obj = new Object(filename);
}

template<class Object, class ParentObject>
void createWithParentAndFilename(Object*& obj, BaseElement* arg)
{
	obj = NULL;
	const char* filename = arg->getAttribute("filename");
	if (!filename || arg->getParent()==NULL)
	{
		std::cerr << arg->getType() << " requires a filename attribute and a parent node\n";
		return;
	}
	ParentObject* object = dynamic_cast<ParentObject*>(arg->getParent()->getObject());
	if (object==NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(arg->getParent()->getObject());
		if (ctx!=NULL)
			object = dynamic_cast<ParentObject*>(ctx->getMechanicalState());
	}
	if (object==NULL) return;
	obj = new Object(object, filename);
}

template<class Object, class ParentObject>
void createWithParent(Object*& obj, BaseElement* arg)
{
	obj = NULL;
	ParentObject* object = dynamic_cast<ParentObject*>(arg->getParent()->getObject());
	if (object==NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(arg->getParent()->getObject());
		if (ctx!=NULL)
			object = dynamic_cast<ParentObject*>(ctx->getMechanicalState());
	}
	if (object==NULL) return;
	obj = new Object(object);
}

template<class Object, class Object1, class Object2>
void createWith2ObjectsAndFilename(Object*& obj, BaseElement* arg)
{
	obj = NULL;
	const char* filename = arg->getAttribute("filename");
	const char* object1 = arg->getAttribute("object1","../..");
	const char* object2 = arg->getAttribute("object2","..");
	if (!filename || !object1 || !object2)
	{
		std::cerr << arg->getType()<< " requires filename, object1 and object2 attributes\n";
		return;
	}
	core::objectmodel::Base* pbase1 = arg->findObject(object1);
	if (pbase1==NULL)
	{
		std::cerr << arg->getType()<< " object1 \""<<object1<<"\" not found\n";
		return;
	}
	core::objectmodel::Base* pbase2 = arg->findObject(object2);
	if (pbase2==NULL)
	{
		std::cerr << arg->getType()<< " object2 \""<<object2<<"\" not found\n";
		return;
	}
	Object1* pobject1 = dynamic_cast<Object1*>(pbase1);
	if (pobject1==NULL && pbase1!=NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(pbase1);
		if (ctx!=NULL)
			pobject1 = dynamic_cast<Object1*>(ctx->getMechanicalState());
	}
	Object2* pobject2 = dynamic_cast<Object2*>(pbase2);
	if (pobject2==NULL && pbase2!=NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(pbase2);
		if (ctx!=NULL)
			pobject2 = dynamic_cast<Object2*>(ctx->getMechanicalState());
	}
	if (pobject1==NULL || pobject2==NULL)
	{
		//std::cerr << arg->getType()<<": object1 "<<(pobject1?"OK":arg->findObject(object1)?"INVALID":"NULL")
		//                           <<", object2 "<<(pobject2?"OK":arg->findObject(object2)?"INVALID":"NULL")<<std::endl;
		return;
	}
	obj = new Object(pobject1, pobject2, filename);
}

template<class Object, class Object1, class Object2>
void createWith2Objects(Object*& obj, BaseElement* arg)
{
	obj = NULL;
	const char* object1 = arg->getAttribute("object1","../..");
	const char* object2 = arg->getAttribute("object2","..");
	if (!object1 || !object2)
	{
		std::cerr << arg->getType()<< " requires object1 and object2 attributes\n";
		return;
	}
	core::objectmodel::Base* pbase1 = arg->findObject(object1);
	if (pbase1==NULL)
	{
		std::cerr << arg->getType()<< " object1 \""<<object1<<"\" not found\n";
		return;
	}
	core::objectmodel::Base* pbase2 = arg->findObject(object2);
	if (pbase2==NULL)
	{
		std::cerr << arg->getType()<< " object2 \""<<object2<<"\" not found\n";
		return;
	}
	Object1* pobject1 = dynamic_cast<Object1*>(pbase1);
	if (pobject1==NULL && pbase1!=NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(pbase1);
		if (ctx!=NULL)
			pobject1 = dynamic_cast<Object1*>(ctx->getMechanicalState());
	}
	Object2* pobject2 = dynamic_cast<Object2*>(pbase2);
	if (pobject2==NULL && pbase2!=NULL)
	{ // look for mechanicalmodel
		core::objectmodel::BaseContext* ctx = dynamic_cast<core::objectmodel::BaseContext*>(pbase2);
		if (ctx!=NULL)
			pobject2 = dynamic_cast<Object2*>(ctx->getMechanicalState());
	}
	if (pobject1==NULL || pobject2==NULL)
	{
		//std::cerr << arg->getType()<<": object1 "<<(pobject1?"OK":arg->findObject(object1)?"INVALID":"NULL")
		//                           <<", object2 "<<(pobject2?"OK":arg->findObject(object2)?"INVALID":"NULL")<<std::endl;
		return;
	}
	obj = new Object(pobject1, pobject2);
}

} // namespace xml
} // namespace sofa

#endif
