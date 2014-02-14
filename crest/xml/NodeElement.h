//FILE: NodeElement.h
#ifndef _CIA3D_XML_NODEELEMENT_H
#define _CIA3D_XML_NODEELEMENT_H

#include <xml/Element.h>
#include <xml/BaseElement.h>
#include <core/BaseNode.h>

namespace cia3d
{
namespace xml
{

class NodeElement : public Element<core::objectmodel::BaseNode>
{
public:
	NodeElement(const std::string& name, const std::string& type, BaseElement* parent=NULL);
	
	virtual ~NodeElement();
	
	virtual bool setParent(BaseElement* newParent);
	
	virtual bool initNode();
	
	virtual bool init();
	
	virtual const char* getClass() const;
};

} // namespace xml

} // namespace sofa

#endif
