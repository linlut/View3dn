#include <xml/NodeElement.h>
#include <xml/Element.inl>

namespace cia3d
{
namespace xml
{

//using namespace sofa::defaulttype;

//template class Factory< std::string, xml::BaseElement, Node<xml::BaseElement*>* >;

NodeElement::NodeElement(const std::string& name, const std::string& type, BaseElement* parent)
: Element<core::objectmodel::BaseNode>(name, type, parent)
{
}

NodeElement::~NodeElement()
{
}

bool NodeElement::setParent(BaseElement* newParent)
{
	if (newParent != NULL && dynamic_cast<NodeElement*>(newParent)==NULL)
		return false;
	else
		return Element<core::objectmodel::BaseNode>::setParent(newParent);
}

bool NodeElement::initNode()
{
	if (!Element<core::objectmodel::BaseNode>::initNode()) return false;
	if (getTypedObject()!=NULL && getParentElement()!=NULL && dynamic_cast<core::objectmodel::BaseNode*>(getParentElement()->getObject())!=NULL)
	{
		std::cout << "Adding Child "<<getName()<<" to "<<getParentElement()->getName()<<std::endl;
		dynamic_cast<core::objectmodel::BaseNode*>(getParentElement()->getObject())->addChild(getTypedObject());
	}
	return true;
}

bool NodeElement::init()
{
	bool res = Element<core::objectmodel::BaseNode>::init();
	/*
	if (getTypedObject()!=NULL)
	{
		for (child_iterator<> it = begin();
					it != end(); ++it)
		{
			objectmodel::BaseObject* obj = dynamic_cast<objectmodel::BaseObject*>(it->getTypedObject());
			if (obj!=NULL)
			{
				std::cout << "Adding Object "<<it->getName()<<" to "<<getName()<<std::endl;
				getTypedObject()->addObject(obj);
			}
		}
	}
	*/
	return res;
}

SOFA_DECL_CLASS(Node)

helper::Creator<BaseElement::NodeFactory, NodeElement> NodeNodeClass("Node");
//helper::Creator<BaseElement::NodeFactory, NodeElement> NodeBodyClass("Body");
//helper::Creator<BaseElement::NodeFactory, NodeElement> NodeGClass("G");

const char* NodeElement::getClass() const
{
	return NodeNodeClass.c_str();
}


} // namespace xml
} // namespace sofa

