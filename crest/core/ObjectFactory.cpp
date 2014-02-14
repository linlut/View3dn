/**********************************************
FILE: objectfactory.cpp


**********************************************/

#include <crest/io/nametableitem.h>
#include <crest/core/ObjectFactory.h>
#include <qdom.h>

using namespace cia3d::io;
using namespace std;

namespace cia3d{
namespace core{

//Add an alias for class, where 
//  name : the alias name 
//  result: the official class name
//  force: force the aliase to be created even though it already exists.
bool ObjectFactory::addAlias(std::string name, std::string result, bool force)
{
    std::map<std::string, ClassEntry*>::iterator it = registry.find(result);
    if (it == registry.end()){
        std::cerr << "ERROR: ObjectFactory: cannot create alias "<<name<<" to unknown class " << result << ".\n";
        return false;
    }
    ClassEntry* entry = it->second;
    ClassEntry*& p = registry[name];
    if (p!=NULL && !force){
        std::cerr << "ERROR: ObjectFactory: cannot create alias "<<name<<" as a class with this name already exists.\n";
        return false;
    }
    else{
        if (p!=NULL)
            p->aliases.erase(name);
        p = entry;
        entry->aliases.insert(name);
    }
    return true;
}
    
ZBaseObject* ObjectFactory::createObject(BaseContext* context, ZBaseObjectDescription* arg)
{
    ZBaseObject* object = NULL;
    std::vector<Creator*> creators;
    string classname = arg->attribute( "class", "");
    string templatename = arg->attribute( "template", "");
    map<std::string, ClassEntry*>::iterator it = registry.find(classname);
	
    if (it == registry.end()){
        std::cout << "ObjectFactory: class "<<classname<<" NOT FOUND."<<std::endl;
    }
	else{
        std::cout << "ObjectFactory: class "<<classname<<" FOUND."<<std::endl;
        ClassEntry* entry = it->second;
		if(templatename.empty()) templatename = entry->defaultTemplate;
		std::map<std::string, Creator*>::iterator it2 = entry->creatorMap.find(templatename);
        if (it2 != entry->creatorMap.end()){
            std::cout << "ObjectFactory: template "<<templatename<<" FOUND."<<std::endl;
            Creator* c = it2->second;
            if (c->canCreate(context, arg))
				creators.push_back(c);
        }
        else{
            std::cout << "ObjectFactory: template "<<templatename<<" NOT FOUND."<<std::endl;
            std::list< std::pair< std::string, Creator*> >::iterator it3;
            for (it3 = entry->creatorList.begin(); it3 != entry->creatorList.end(); ++it3)
            {
                Creator* c = it3->second;
                if (c->canCreate(context, arg))
					creators.push_back(c);
            }
        }
    }
    if (creators.empty()){
        std::cerr<<"ERROR: ObjectFactory: Object type "<<classname<<"<"<<templatename<<"> creation failed."<<std::endl;
    }
    else{
        if (creators.size()>1)
            std::cerr<<"WARNING: ObjectFactory: Several possibilities found for type "<<classname<<"<"<<templatename<<">."<<std::endl;
        object = creators[0]->createInstance(context, arg);
    }
	
    return object;
}
    
        
void ObjectFactory::dumpXML(std::ostream& out)
{
	map<std::string, ClassEntry*>::iterator it, itend;
    for (it = registry.begin(), itend = registry.end(); it != itend; ++it){
        ClassEntry* entry = it->second;
        if (entry->className != it->first) continue;
        out << "<class name=\"" << xmlencode(entry->className) <<"\">\n";
        out << "<class name=\"" << (entry->className) <<"\">\n";
		{
        for (std::set<std::string>::iterator it = entry->aliases.begin(), itend = entry->aliases.end(); it != itend; ++it)
            out << "<alias>" << xmlencode(*it) << "</alias>\n";
		}
		{
        for (std::set<std::string>::iterator it = entry->baseClasses.begin(), itend = entry->baseClasses.end(); it != itend; ++it)
            out << "<base>" << *it << "</base>\n";
		}
        if (!entry->description.empty())
            out << "<description>"<<entry->description<<"</description>\n";
        if (!entry->authors.empty())
            out << "<authors>"<<entry->authors<<"</authors>\n";
        if (!entry->license.empty())
            out << "<license>"<<entry->license<<"</license>\n";
        for (std::list< std::pair< std::string, Creator* > >::iterator itc = entry->creatorList.begin(), itcend = entry->creatorList.end(); itc != itcend; ++itc)
        {
            out << "<creator";
            if (!itc->first.empty()) 
				out << " template=\"" << xmlencode(itc->first) << "\"";
            out << "/>\n";
        }
        out << "</class>\n";
    }
}


RegisterObject::RegisterObject(const std::string& description)
{
  if (!description.empty()){
    //std::cerr<<"description.size() = "<<description.size()<<", value = "<<description<<std::endl;
        addDescription(description);
  }
}

RegisterObject& RegisterObject::addAlias(std::string val)
{
    entry.aliases.insert(val);
    return *this;
}

RegisterObject& RegisterObject::addDescription(std::string val)
{
    //std::cout << "ObjectFactory: add description "<<val<<std::endl;
    val += '\n';
    entry.description += val;
    return *this;
}

RegisterObject& RegisterObject::addAuthor(std::string val)
{
    //std::cout << "ObjectFactory: add author "<<val<<std::endl;
    val += ' ';
    entry.authors += val;
    return *this;
}

RegisterObject& RegisterObject::addLicense(std::string val)
{
    //std::cout << "ObjectFactory: add license "<<val<<std::endl;
    entry.license += val;
    return *this;
}

RegisterObject& RegisterObject::addCreator(std::string classname, std::string templatename, ObjectFactory::Creator* creator)
{
    //std::cout << "ObjectFactory: add creator "<<classname<<" with template "<<templatename<<std::endl;
    if (!entry.className.empty() && entry.className != classname)
    {
        std::cerr << "ERROR: ObjectFactory: all templated class should have the same base classname ("<<entry.className<<"!="<<classname<<")\n";
    }
    else if (entry.creatorMap.find(templatename) != entry.creatorMap.end())
    {
        std::cerr << "ERROR: ObjectFactory: class "<<classname<<"<"<<templatename<<"> already registered\n";
    }
    else
    {
        entry.className = classname;
        entry.creatorMap.insert(std::make_pair(templatename, creator));
        entry.creatorList.push_back(std::make_pair(templatename, creator));
    }
    return *this;
}

RegisterObject::operator int()
{
    if (entry.className.empty()){
        return 0;
    }

	std::cout << entry.className << "==>ObjectFactory.\n";
    {
        //std::cout << "ObjectFactory: commit"<<std::endl;
        ObjectFactory::ClassEntry* reg = ObjectFactory::getInstance()->getEntry(entry.className);
        reg->description += entry.description;
        reg->authors += entry.authors;
        reg->license += entry.license;
		if (!entry.defaultTemplate.empty())
		{
			if (!reg->defaultTemplate.empty())
			{
	            std::cerr << "ERROR: ObjectFactory: default template for class "<<entry.className<<" already registered <"<<reg->defaultTemplate<<">, do not register <"<<entry.defaultTemplate<<"> as default.\n";
			}
			else
			{
				reg->defaultTemplate = entry.defaultTemplate;
			}
		}
        for (std::list< std::pair< std::string, ObjectFactory::Creator* > >::iterator itc = entry.creatorList.begin(), itcend = entry.creatorList.end(); itc != itcend; ++itc)
        //for (std::map<std::string, ObjectFactory::Creator*>::iterator itc = entry.creators.begin(), itcend = entry.creators.end(); itc != itcend; ++itc)
        {
            if (reg->creatorMap.find(itc->first) != reg->creatorMap.end())
            {
                std::cerr << "ERROR: ObjectFactory: class "<<entry.className<<"<"<<itc->first<<"> already registered\n";
            }
            else
            {
                reg->creatorMap.insert(*itc);
                reg->creatorList.push_back(*itc);
            }
        }
        for (std::set<std::string>::iterator it = entry.aliases.begin(), itend = entry.aliases.end(); it != itend; ++it)
        {
            if (reg->aliases.find(*it) == reg->aliases.end())
            {
                ObjectFactory::getInstance()->addAlias(*it,entry.className);
            }
        }
        for (std::set<std::string>::iterator it = entry.baseClasses.begin(), itend = entry.baseClasses.end(); it != itend; ++it)
        {
            if (reg->baseClasses.find(*it) == reg->baseClasses.end())
            {
                reg->baseClasses.insert(*it);
            }
        }
        //std::cout << "ObjectFactory: commit end"<<std::endl;
        return 1;
    }
}


}//namespace core
}//namespace cia3d
