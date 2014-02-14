//FILE: base_object.h
//Nan Zhang
//Nov 11, 2007

#ifndef _ZINC_OBJECTBASE_H_
#define _ZINC_OBJECTBASE_H_

//#include <malloc.h>
#include <typeinfo> 
#include <string>
#include <sysconf.h>

namespace cia3d{
	namespace core{
		class BaseContext;
		class ZBaseObjectDescription;
	}
};

class QDomElement;
#define OBJECTBASE_NAMELEN 24
class CObjectBase
{

private:
	char m_sObjectName[OBJECTBASE_NAMELEN];

	/// Extract the class name (removing namespaces and templates)
	static std::string decodeClassName(const std::type_info& t)
	{
		std::string name = t.name();
		const char* realname = NULL;
		char* allocname = strdup(name.c_str());
	#ifdef __GNUC__
		int status;
        #ifndef __APPLE__
		//realname = allocname = abi::__cxa_demangle(allocname, 0, 0, &status);
        #endif
		if (realname==NULL)
	#endif
			realname = allocname;
		int len = strlen(realname);
		char* newname = (char*)malloc(len+1);
		int start = 0;
		int dest = 0;
		int i;
		char cprev = '\0';
		//std::cout << "name = "<<realname<<std::endl;
		for (i=0; i<len; i++)
		{
			char c = realname[i];
			if (c == '<') break;
			if (c == ':') // && cprev == ':')
			{
				start = i+1;
			}
			else if (c == ' ' && i >= 5 && realname[i-5] == 'c' && realname[i-4] == 'l' && realname[i-3] == 'a' && realname[i-2] == 's' && realname[i-1] == 's')
			{
				start = i+1;
			}
			else if (c != ':' && c != '_' && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z'))
			{
				// write result
				while (start < i)
				{
					newname[dest++] = realname[start++];
					newname[dest] = 0;
				}
			}
			cprev = c;
			//std::cout << "i = "<<i<<" start = "<<start<<" dest = "<<dest<<" newname = "<<newname<<std::endl;
		}
		while (start < i)
		{
			newname[dest++] = realname[start++];
			newname[dest] = 0;
		}
		newname[dest] = '\0';
		//std::cout << "newname = "<<newname<<std::endl;
		name = newname;
		free(newname);
		//if (allocname)
		//    free(allocname);
		return name;
	}

	/// Decode the template name (removing namespaces and class name)
	static std::string decodeTemplateName(const std::type_info& t)
	{
		std::string name = t.name();
		const char* realname = NULL;
		char* allocname = strdup(name.c_str());
	#ifdef __GNUC__
		int status;
        #ifndef __APPLE__
		//realname = allocname = abi::__cxa_demangle(allocname, 0, 0, &status);
        #endif
		if (realname==NULL)
	#endif
			realname = allocname;
		int len = strlen(realname);
		char* newname = (char*)malloc(len+1);
		newname[0] = '\0';
		int start = 0;
		int dest = 0;
		int i = 0;
		char cprev = '\0';
		//std::cout << "name = "<<realname<<std::endl;
		while (i < len && realname[i]!='<')
			++i;
		start = i+1; ++i;
		for (; i<len; i++)
		{
			char c = realname[i];
			//if (c == '<') break;
			if (c == ':') // && cprev == ':')
			{
				start = i+1;
			}
			else if (c == ' ' && i >= 5 && realname[i-5] == 'c' && realname[i-4] == 'l' && realname[i-3] == 'a' && realname[i-2] == 's' && realname[i-1] == 's')
			{
				start = i+1;
			}
			else if (c != ':' && c != '_' && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z'))
			{
				// write result
				while (start <= i)
				{
					newname[dest++] = realname[start++];
					newname[dest] = 0;
				}
			}
			cprev = c;
			//std::cout << "i = "<<i<<" start = "<<start<<" dest = "<<dest<<" newname = "<<newname<<std::endl;
		}
		while (start < i)
		{
			newname[dest++] = realname[start++];
			newname[dest] = 0;
		}
		newname[dest] = '\0';
		//std::cout << "newname = "<<newname<<std::endl;
		name = newname;
		free(newname);
		//if (allocname)
		//    free(allocname);
		return name;
	/*
		std::string name = decodeTypeName(t);
		// Find template
		std::string::size_type pos = name.find("<");
		if (pos != std::string::npos)
		{
			name = name.substr(pos+1,name.length()-pos-2);
		}
		else
		{
			name = "";
		}
		//std::cout << "TEMPLATE NAME="<<name<<std::endl;
		return name;
	*/
	}

public:
	CObjectBase()
	{
		m_sObjectName[0] = 0;
	}

	~CObjectBase()
	{
		m_sObjectName[0]=0;
	}

	void SetObjectName(const char *pname)
	{
		const int MAX_LENGTH = OBJECTBASE_NAMELEN-1;
		int len = strlen(pname);
		if (len>MAX_LENGTH) len=MAX_LENGTH;
		for (int i=0; i<len; i++)
			m_sObjectName[i] = pname[i];
		m_sObjectName[len]=0;
	}

	const char* GetObjectName(void) const
	{
		return m_sObjectName;
	}

	int CompareObjectName(const char *in) const 
	{
		int inlen = strlen(in);
		if (inlen>=OBJECTBASE_NAMELEN) 
			inlen = OBJECTBASE_NAMELEN - 1;
		const int strlength = strlen(m_sObjectName);
		const int len = _MAX_(inlen, strlength);
		return strncmp(in, m_sObjectName, len);
	}

    virtual void parse(QDomElement* arg ){}

    template<class T>
	static bool canCreate(T*&, const cia3d::core::BaseContext*, QDomElement*)
	{
		return true;
	}

    template<class T>
	static void createInstance(T*& obj, const cia3d::core::BaseContext*, QDomElement* arg)
	{
		 obj = new T();
		 if (arg) obj->parse(arg);
	}

    template<class T>
	static void create(T*& obj, const cia3d::core::BaseContext*, QDomElement*arg)
	{
		obj = new T();
		if (arg) obj->parse(arg);
	}

    virtual std::string getClassName() 
    {
        return decodeClassName(typeid(*this));
    }
    
    virtual std::string getTemplateName() 
    {
        return decodeTemplateName(typeid(*this));
    }

    template<class T>
    static std::string className(const T* = NULL)
    {
        return decodeClassName(typeid(T));
    }
        
    template<class T>
    static std::string templateName(const T* = NULL)
    {
        return decodeTemplateName(typeid(T));
    }

};

#endif
