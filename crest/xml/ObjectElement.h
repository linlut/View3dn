//FILE: ObjectElement.h
#ifndef _CIA3D_XML_OBJECTELEMENT_H
#define _CIA3D_XML_OBJECTELEMENT_H

#include <xml/Element.h>
#include <core/BaseObject.h>

namespace cia3d
{
namespace xml
{

class ObjectElement : public Element<zcore::BaseObject>
{
public:
	ObjectElement(const std::string& name, const std::string& type, BaseElement* parent=NULL);
	
	virtual ~ObjectElement();

	virtual bool initNode();
	
	virtual const char* getClass() const;
};


} // namespace xml
} // namespace sofa

#endif
