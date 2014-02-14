//FILE: Factory.cpp

#include <typeinfo>
#include <crest/core/Factory.inl>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace cia3d{
  namespace core{

/// Decode the type's name to a more readable form if possible
std::string gettypename(const std::type_info& t)
{
	std::string name = t.name();
#ifdef __GNUC__
	char* realname = NULL;
	int status;
	realname = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
	if (realname!=NULL)
	{
		name = realname;
		free(realname);
	}
#endif
	// Remove namespaces
	for(;;)
	{
		std::string::size_type pos = name.find("::");
		if (pos == std::string::npos) break;
		std::string::size_type first = name.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_",pos-1);
		if (first == std::string::npos) first = 0;
		else first++;
		name.erase(first,pos-first+2);
	}
	//Remove "class "
	for(;;)
	{
		std::string::size_type pos = name.find("class ");
		if (pos == std::string::npos) break;
		name.erase(pos,6);
	}
	return name;
}

//static std::string factoryLog;
static std::string& getFactoryLog()
{
	static std::string s;
	return s;
}

/// Log classes registered in the factory
void logFactoryRegister(std::string baseclass, std::string classname, std::string key, bool multi)
{
	/*
	std::string cs = (multi?" template class ":" class ");
	std::string rs(" registered as "); 
	std::string s = baseclass + cs + classname + rs + key;
	getFactoryLog() += s;
	*/
}

/// Print factory log
void printFactoryLog(std::ostream& out)
{
	//out << getFactoryLog();
}


  } // namespace zcore
} // namespace cia3d

