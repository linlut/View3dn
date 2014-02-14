#ifndef SOFA_SIMULATION_TREE_XML_ELEMENT_INL
#define SOFA_SIMULATION_TREE_XML_ELEMENT_INL

#include "Element.h"
#include <core/factory.inl>

namespace cia3d
{
namespace xml
{


template<class Object>
Element<Object>::Element(const std::string& name, const std::string& type, BaseElement* newParent)
: BaseElement(name, type, newParent), object(NULL)
{
}

template<class Object>
Element<Object>::~Element()
{
}

template<class Object>
Object* Element<Object>::getTypedObject()
{
    return object;
}

template<class Object>
void Element<Object>::setObject(Object* newObject)
{
    object = newObject;
}

/// Get the associated object
template<class Object>
core::objectmodel::Base* Element<Object>::getObject()
{
    return object;
}

template<class Object>
bool Element<Object>::initNode()
{
	Object *obj = Factory::CreateObject(this->getType(), this);
	if (obj != NULL)
	{
		setObject(obj);
		obj->setName(getName());
		return true;
	}
	else return false;
}

//template<class Object> class Factory< std::string, Object, Node<Object>* >;


} // namespace xml
} // namespace sofa

#endif
