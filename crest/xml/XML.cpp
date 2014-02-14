#include <string>
#include <typeinfo>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sofa/simulation/tree/xml/XML.h>

/* For loading the scene */


namespace cia3d
{
namespace xml
{

using std::cout;
using std::endl;


#define is(n1, n2) (! xmlStrcmp((const xmlChar*)n1,(const xmlChar*)n2)) 
#define getProp(n) ( xmlGetProp(cur, (const xmlChar*)n) )

BaseElement* createNode(xmlNodePtr root)
{
	if (!xmlStrcmp(root->name,(const xmlChar*)"text")) return NULL;

	std::string name, type;
	
	xmlChar *pname = xmlGetProp(root, (const xmlChar*) "name");
	xmlChar *ptype = xmlGetProp(root, (const xmlChar*) "type");
	
	if (pname != NULL)
	{
		name = (const char*)pname;
		xmlFree(pname);
	}
	else
	{
		name = "default";
		static int num = 0;
		char buf[16];
		// added by Sylvere F.
		sprintf(buf, "%d", num);
		// snprintf(buf,sizeof(buf),"%d",num);
		++num;
		name += buf;
	}
	if (ptype != NULL)
	{
		type = (const char*)ptype;
		xmlFree(ptype);
	}
	else
	{
		type = "default";
	}
	
	BaseElement* node = BaseElement::Create((const char*)root->name,name,type);
	if (node == NULL)
	{
		std::cerr << "Node "<<root->name<<" name "<<name<<" type "<<type<<" creation failed.\n";
		return NULL;
	}
	
	//std::cout << "Node "<<root->name<<" name "<<name<<" type "<<type<<" created.\n";
	
	// List attributes
	for (xmlAttrPtr attr = root->properties; attr!=NULL; attr = attr->next)
	{
		if (attr->children==NULL) continue;
		if (!xmlStrcmp(attr->name,(const xmlChar*)"name")) continue;
		if (!xmlStrcmp(attr->name,(const xmlChar*)"type")) continue;
		//std::cout << attr->name << " = " << attr->children->content << std::endl;
		node->setAttribute((const char*)attr->name, (const char*)attr->children->content);
	}
	
	for (xmlNodePtr child = root->xmlChildrenNode; child != NULL; child = child->next)
	{
		BaseElement* childnode = createNode(child);
		if (childnode != NULL)
		{
			if (!node->addChild(childnode))
			{
				std::cerr << "Node "<<childnode->getClass()<<" name "<<childnode->getName()<<" type "<<childnode->getType()
					<<" cannot be a child of node "<<node->getClass()<<" name "<<node->getName()<<" type "<<node->getType()<<std::endl;
				delete childnode;
			}
		}
	}
	return node;
}

static void dumpNode(BaseElement* node, std::string prefix0="==", std::string prefix="  ")
{
	std::cout << prefix0;
	std::cout << node->getClass()<<" name "<<node->getName()<<" type "<<node->getType()<<std::endl;
	BaseElement::child_iterator<> it = node->begin();
	BaseElement::child_iterator<> end = node->end();
	while (it != end)
	{
		BaseElement::child_iterator<> next = it;
		++next;
		if (next==end) dumpNode(it, prefix+"\\-", prefix+"  ");
		else           dumpNode(it, prefix+"+-", prefix+"| ");
		it = next;
	}
}

BaseElement* load(const char *filename)
{
     //
     // this initialize the library and check potential ABI mismatches
     // between the version it was compiled for and the actual shared
     // library used.
     //
    LIBXML_TEST_VERSION

	xmlDocPtr doc; // the resulting document tree 
	xmlNodePtr root;

    doc = xmlParseFile(filename);
    if (doc == NULL) {
		std::cerr << "Failed to open " << filename << std::endl;
		return NULL;
    }
	
	root = xmlDocGetRootElement(doc);
	
	if (root == NULL) {
		std::cerr << "empty document" << std::endl;
		xmlFreeDoc(doc);
		return NULL;
	}

	//std::cout << "Creating XML graph"<<std::endl;
	BaseElement* graph = createNode(root);
	//std::cout << "XML Graph created"<<std::endl;
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	
	if (graph == NULL)
	{
		std::cerr << "XML Graph creation failed."<<std::endl;
		return NULL;
	}
	
	dumpNode(graph);

	return graph;
}

} // namespace xml

} // namespace sofa

