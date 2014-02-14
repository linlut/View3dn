//File: factory.inl

#ifndef _CIA3D_FACTORY_INL
#define _CIA3D_FACTORY_INL

#include <iostream>
#include <typeinfo>
#include <string>
#include <crest/core/Factory.h>

namespace cia3d{
namespace core{

template <typename TKey, class TObject, typename TArgument>
TObject* Factory<TKey, TObject, TArgument>::createObject(Key key, Argument arg)
{
	Object* object;
	Creator* creator;
	typename std::multimap<Key, Creator*>::iterator it = registry.lower_bound(key);
	typename std::multimap<Key, Creator*>::iterator end = registry.upper_bound(key);
	while (it != end)
	{
		creator = (*it).second;
		object = creator->createInstance(arg);
		if (object != NULL)
		{
			//std::cout<<"Object type "<<key<<" created: "<<gettypename(typeid(*object))<<std::endl;
			return object;
			}
		++it;
	}
	std::cerr<<"Object type "<<key<<" creation failed."<<std::endl;
	return NULL;
}

template <typename TKey, class TObject, typename TArgument>
Factory<TKey, TObject, TArgument>* Factory<TKey, TObject, TArgument>::getInstance()
{
	static Factory<Key, Object, Argument> instance;
	return &instance;
}

} // namespace core
} // namespace cia3d

#endif
