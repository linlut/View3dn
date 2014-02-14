/***********************************************************
FILE: ObjectFactory.h
Author: Nan Zhang
Date: Aug 20, 2007
***********************************************************/

#ifndef __INC_OBJECT_FACTORY_H__
#define __INC_OBJECT_FACTORY_H__

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <typeinfo>
#include <crest/core/BaseObject.h>
#include <crest/core/BaseObjectDescription.h>
#include <crest/core/BaseContext.h>


namespace cia3d{
namespace core{

using namespace std;

class ObjectFactory
{
public:

    class Creator
    {
    public:
		Creator(){}
        virtual ~Creator(){}
        virtual bool canCreate(const BaseContext* context, ZBaseObjectDescription* arg)=0;
        virtual ZBaseObject* createInstance(const BaseContext* context, ZBaseObjectDescription* arg)=0;
        virtual const std::type_info& type()
		{
			assert(0);
			return typeid(*this); 
		}
    };

    class ClassEntry
    {
    public:
        std::string className;
        std::set<std::string> baseClasses;
        std::set<std::string> aliases;
        std::string description;
        std::string authors;
        std::string license;
        std::string defaultTemplate;								//Default template name
        std::map<std::string, Creator*> creatorMap;					//Template based creation, preferred first
        std::list< std::pair<std::string, Creator*> > creatorList;	//A list of creators. If no template, then use
    };

protected:
    map<std::string, ClassEntry*> registry;
    
public:

	//Constructor
	ObjectFactory(void): registry()
	{
		registry.clear();
	}

	//Destructor
	~ObjectFactory(void)
	{
		registry.clear();
	}

	//For a given class name, return the ptr to the first class entry record
    ClassEntry* getEntry(const std::string& classname)
    {
	    ClassEntry*& p = registry[classname];
	    if (p==NULL){
	        p = new ClassEntry;
	        p->className = classname;
	    }
	    return p;
    }
    
	//Return an array of all the class entries
    void getAllEntries(std::vector<ClassEntry*>& result)
    {
	    result.clear();
		std::map<std::string, ClassEntry*>::iterator it, itend;
	    for (it = registry.begin(), itend = registry.end(); it != itend; ++it){
	        ClassEntry* entry = it->second;
	        if (entry->className != it->first) continue;
			result.push_back(entry);    	
		}
    }
    
    static ObjectFactory* getInstance()
    {
	    static ObjectFactory instance;
	    return &instance;    	
    }

    bool addAlias(std::string name, std::string result, bool force=false);
            
    static bool AddAlias(std::string name, std::string result, bool force=false)
    {
        return getInstance()->addAlias(name, result, force);
    }

	bool addEntry(const std::string& classname, ClassEntry * classentry)
	{
		registry[classname] = classentry;
		return true;
	}

    static bool AddEntry(const std::string& classname, ClassEntry * classentry)
    {
        bool r=getInstance()->addEntry(classname, classentry);
		return r;
    }    

    ZBaseObject* createObject(BaseContext *context, ZBaseObjectDescription* arg);

	static ZBaseObject* CreateObject(BaseContext* context, ZBaseObjectDescription* arg)
    {
        return getInstance()->createObject(context, arg);
    }

    void dump(std::ostream& out = std::cout);
    
    void dumpXML(std::ostream& out = std::cout);
};


template<class RealObject>
class ObjectCreator : public ObjectFactory::Creator
{
public:
    virtual bool canCreate(const BaseContext* context, ZBaseObjectDescription* arg)
    {
        RealObject* instance = NULL;
        return RealObject::canCreate(instance, context, arg);
    }

    virtual ZBaseObject *createInstance(const BaseContext* context, ZBaseObjectDescription* arg)
    {
        RealObject* instance = NULL;
        RealObject::create(instance, context, arg);
        return instance;
    }

    virtual const std::type_info& type()
	{ 
		return typeid(RealObject); 
	}
};


class RegisterObject
{
protected:
    ObjectFactory::ClassEntry entry;

public:
    
    RegisterObject(const std::string& description);
    
    RegisterObject& addAlias(std::string val);
    
    RegisterObject& addDescription(std::string val);
    
    RegisterObject& addAuthor(std::string val);
    
    RegisterObject& addLicense(std::string val);
    
    RegisterObject& addCreator(std::string classname, std::string templatename, ObjectFactory::Creator* creator);
    
    // test whether T* converts to U*,
    // that is, if T is derived from U
    // taken from Modern C++ Design
    template <class T, class U>
    class Conversion
    {
        typedef char Small;
        class Big {char dummy[2];};
        static Small Test(U*);
        static Big Test(...);
        static T* MakeT();
    public:
        enum { exists = sizeof(Test(MakeT())) == sizeof(Small) };
        static int Exists() { return exists; }
    };
    
    template<class RealClass, class BaseClass>
    bool implements()
    {
        bool res = Conversion<RealClass, BaseClass>::exists;
        //RealClass* p1=NULL;
        //BaseClass* p2=NULL;
        //if (res)
        //    std::cout << "class "<<RealClass::typeName(p1)<<" implements "<<BaseClass::typeName(p2)<<std::endl;
        //else
        //    std::cout << "class "<<RealClass::typeName(p1)<<" does not implement "<<BaseClass::typeName(p2)<<std::endl;
        return res;
    }
    
    template<class RealObject>
    RegisterObject& add(bool defaultTemplate=false)
    {
        RealObject* p = NULL;
        std::string classname = RealObject::className(p);
        std::string templatename = RealObject::templateName(p);

		if (defaultTemplate)
			entry.defaultTemplate = templatename;
/*
        // This is the only place where we can test which base classes are implemented by this particular object, without having to create any instance
        // Unfortunately, we have to enumerate all classes we are interested in...
        if (implements<RealObject,objectmodel::ContextObject>())
            entry.baseClasses.insert("ContextObject");
        if (implements<RealObject,VisualModel>())
            entry.baseClasses.insert("VisualModel");
        if (implements<RealObject,BehaviorModel>())
            entry.baseClasses.insert("BehaviorModel");
        if (implements<RealObject,CollisionModel>())
            entry.baseClasses.insert("CollisionModel");
        if (implements<RealObject,core::componentmodel::behavior::BaseMechanicalState>())
            entry.baseClasses.insert("MechanicalState");
        if (implements<RealObject,core::componentmodel::behavior::BaseForceField>())
            entry.baseClasses.insert("ForceField");
        if (implements<RealObject,core::componentmodel::behavior::InteractionForceField>())
            entry.baseClasses.insert("InteractionForceField");
        if (implements<RealObject,core::componentmodel::behavior::BaseConstraint>())
            entry.baseClasses.insert("Constraint");
        if (implements<RealObject,core::BaseMapping>())
            entry.baseClasses.insert("Mapping");
        if (implements<RealObject,core::componentmodel::behavior::BaseMechanicalMapping>())
            entry.baseClasses.insert("MechanicalMapping");
        if (implements<RealObject,core::componentmodel::behavior::BaseMass>())
            entry.baseClasses.insert("Mass");
        if (implements<RealObject,core::componentmodel::behavior::OdeSolver>())
            entry.baseClasses.insert("OdeSolver");
        if (implements<RealObject,core::componentmodel::topology::Topology>())
            entry.baseClasses.insert("Topology");
	*/	

		ObjectCreator<RealObject> *mycreator =  new ObjectCreator<RealObject>;
        return addCreator(classname, templatename, mycreator);
    }
    
    /// Convert to an int
    /// This is the final operation that will actually commit the additions to the factory
    operator int();
};


}//namespace core
}//namespace cia3d


#endif