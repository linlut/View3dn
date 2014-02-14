#include "BaseElement.h"
#include <core/factory.inl>

namespace cia3d
{
namespace xml
{

BaseElement::BaseElement(const std::string& name, const std::string& type, BaseElement* newParent)
: name(name), type(type), parent(NULL)
{
	if (newParent!=NULL) newParent->addChild(this);
	attributes["name"]=&this->name;
	attributes["type"]=&this->type;
}

BaseElement::~BaseElement()
{
	attributes.erase("name");
	attributes.erase("type");
	for (std::map<std::string,std::string*>::iterator it = attributes.begin();
			it != attributes.end(); ++it)
	{
		delete it->second;
	}
	attributes.clear();
	for (ChildList::iterator it = children.begin();
			it != children.end(); ++it)
	{
		delete *it;
	}
	children.clear();
}

const std::map<std::string,std::string*>& BaseElement::getAttributeMap() const
{
	return attributes;
}

std::map<std::string,std::string*>& BaseElement::getAttributeMap()
{
	return attributes;
}

/// Get an attribute given its name (return defaultVal if not present)
const char* BaseElement::getAttribute(const std::string& attr, const char* defaultVal)
{
	std::map<std::string,std::string*>::iterator it = attributes.find(attr);
	if (it == attributes.end())
		return defaultVal;
	else
		return it->second->c_str();
}

/// Set an attribute. Override any existing value
void BaseElement::setAttribute(const std::string& attr, const char* val)
{
	std::map<std::string,std::string*>::iterator it = attributes.find(attr);
	if (it != attributes.end())
		*(it->second) = val;
	else
		attributes[attr] = new std::string(val);
}

/// Remove an attribute. Fails if this attribute is "name" or "type"
bool BaseElement::removeAttribute(const std::string& attr)
{
	std::map<std::string,std::string*>::iterator it = attributes.find(attr);
	if (it == attributes.end())
		return false;
	if (it->second == &name)
		return false;
	if (it->second == &type)
		return false;
	delete it->second;
	attributes.erase(it);
	return true;
}
	
bool BaseElement::addChild(BaseElement* child)
{
	if (child->getParent()==this) return false;
	BaseElement* oldParent =  child->getParentElement();
	if (!child->setParent(this)) return false;
	if (oldParent != NULL)
	{
		oldParent->removeChild(child);
	}
	children.push_back(child);
	return true;
}

bool BaseElement::removeChild(BaseElement* child)
{
	if (child->getParent()!=this) return false;
	ChildList::iterator it = children.begin();
	while (it!=children.end())
	{
		if (*it == child)
		{
			child->setParent(NULL);
			children.erase(it);
			return true;
		}
	}
	return false;
}

bool BaseElement::init()
{
	bool res = initNode();
	for (child_iterator<> it = begin();
				it != end(); ++it)
	{
		res &= it->init();
	}
	return res;
}

BaseElement* BaseElement::Create(const std::string& nodeClass, const std::string& name, const std::string& type)
{
	return NodeFactory::CreateObject(nodeClass, std::pair<std::string,std::string>(name, type));
}

/// Find a node given its name
BaseElement* BaseElement::findNode(const char* nodeName, bool absolute)
{
	if (nodeName == NULL) return NULL;
	if (nodeName[0]=='\\' || nodeName[0]=='/')
	{
		if (!absolute && getParentElement()!=NULL)
			return getParentElement()->findNode(nodeName);
		else
		{ ++nodeName; absolute = true; }
	}
	if (nodeName[0]=='\0')
	{
		if (absolute) return this;
		else return NULL;
	}
	const char* sep = nodeName;
	while (*sep!='\0' && *sep!='\\' && *sep!='/')
		++sep;
	if (!strncmp(nodeName,".",sep-nodeName))
		return findNode(sep, true);
	if (!strncmp(nodeName,"..",sep-nodeName))
	{
		if (getParentElement()==NULL) return NULL;
		else return getParentElement()->findNode(sep,true);
	}
	for (child_iterator<> it = begin(); it != end(); ++it)
	{
		if (it->getName().length() == (unsigned)(sep-nodeName) && !strncmp(it->getName().c_str(), nodeName, sep-nodeName))
		{
			BaseElement* res = it->findNode(sep,true);
			if (res!=NULL) return res;
		}
	}
	if (!absolute && getParentElement()!=NULL)
		return getParentElement()->findNode(nodeName);
	else
		return NULL;
}

} // namespace xml
} // namespace sofa
