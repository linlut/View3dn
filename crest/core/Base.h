//FILE: base.h
#ifndef CIA3D_CORE_BASE_H
#define CIA3D_CORE_BASE_H

#include <string>
#include <assert.h>
#include <crest/core/Field.h>
#include <crest/core/DataField.h>
#include <crest/core/BaseObjectDescription.h>

namespace cia3d
{
namespace core
{

/// Base class for everything
class Base
{

public:
    Base();
    virtual ~Base();
    
    DataField<std::string> name;

    std::string getName() const
	{
	    return name.getValue();
	}

    void setName(const std::string& na)
	{
	    name.setValue(na);
	}

    virtual std::string getTypeName() const
    {
        return decodeTypeName(typeid(*this));
    }
    
    virtual std::string getClassName() const
    {
        return decodeClassName(typeid(*this));
    }
    
    virtual std::string getTemplateName() const
    {
        return decodeTemplateName(typeid(*this));
    }
    
    /// Decode the type's name to a more readable form if possible
    static std::string decodeTypeName(const std::type_info& t);
    
    /// Extract the class name (removing namespaces and templates)
    static std::string decodeClassName(const std::type_info& t);
    
    /// Extract the namespace (removing class name and templates)
    static std::string decodeNamespaceName(const std::type_info& t);
    
    /// Extract the template name (removing namespaces and class name)
    static std::string decodeTemplateName(const std::type_info& t);
    
    template<class T>
    static std::string typeName(const T* = NULL)
    {
        return decodeTypeName(typeid(T));
    }
    
    template<class T>
    static std::string className(const T* = NULL)
    {
        return decodeClassName(typeid(T));
    }
    
    template<class T>
    static std::string namespaceName(const T* = NULL)
    {
        return decodeNamespaceName(typeid(T));
    }
    
    template<class T>
    static std::string templateName(const T* = NULL)
    {
        return decodeTemplateName(typeid(T));
    }

    void parseFields ( std::list<std::string> str );
    virtual void parseFields ( const std::map<std::string,std::string*>& str );
    void writeFields (std::map<std::string,std::string*>& str);
    void writeFields (std::ostream& out);
    void xmlWriteFields (std::ostream& out, unsigned level);
    
    template<class T>
    DataField<T> dataField( DataField<T>* field, char* name, char* help )
    {
        std::string ln(name);
        if( ln.size()>0 && m_fieldMap.find(ln) != m_fieldMap.end() )
        {
            std::cerr << "field name " << ln << " already used in this class or in a parent class !...aborting" << std::endl;
            exit( 1 );
        }
        //field = tmp;
        m_fieldMap[name] = field;
        return DataField<T>(help);
    }
    
    template<class T>
    DataField<T> dataField( DataField<T>* field, const T& value, char* name, char* help )
    {
        std::string ln(name);
        if( ln.size()>0 && m_fieldMap.find(ln) != m_fieldMap.end() )
        {
            std::cerr << "field name " << ln << " already used in this class or in a parent class !...aborting" << std::endl;
            exit( 1 );
        }
        //field = tmp;
        m_fieldMap[name] = field;
        return DataField<T>(value,help);
    }
    
    template<class T>
    Field<T> field( Field<T>* field, T* ptr, char* name, char* help )
    {
        std::string ln(name);
        if( ln.size()>0 && m_fieldMap.find(ln) != m_fieldMap.end() )
        {
            std::cerr << "field name " << ln << " already used in this class or in a parent class !...aborting" << std::endl;
            exit( 1 );
        }
        //field = tmp;
        m_fieldMap[name] = field;
        return Field<T>(ptr,help);
    }

    virtual void parse ( BaseObjectDescription* arg )
    {
        //this->parseFields ( arg->getAttributeMap() );
		assert(0);
    }

    virtual void parse ( const char * arg )
    {
        //this->parseFields ( arg->getAttributeMap() );
		std::cout<<"Base::parse() not implemented!\n";
		//assert(0);
    }

	std::map< std::string, FieldBase* > getFields() { return m_fieldMap; }

protected:
    /// name -> Field object
    std::map< std::string, FieldBase* > m_fieldMap;
        
    void addField( FieldBase* f, char* name )
    {
      std::string ln(name);
      if( ln.size()>0 && m_fieldMap.find(ln) != m_fieldMap.end() )
      {
        std::cerr << "field name " << ln << " already used in this class or in a parent class !...aborting" << std::endl;
        exit( 1 );
      }
      m_fieldMap[name] = f;
    }
	
};

} // namespace core

} // namespace cia3d

#endif

