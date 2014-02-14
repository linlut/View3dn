
//File: ObjectLib
#ifndef _INC_OBJECTLIB_H_
#define _INC_OBJECTLIB_H_

#include <vector>
#include <crest/core/BaseObject.h>

namespace cia3d{
	namespace core{
//=================================================
class ObjectLib
{

private:
	vector<CObjectBase*> m_objects;
	
public:
	
	ObjectLib(void)
	{
		m_objects.clear();
	}

	void clear(void)
	{
		const int nsize = m_objects.size();
		for (int i=0; i<nsize; i++){
			CObjectBase *p = m_objects[i];
			delete p;
			m_objects[i]=NULL;
		}
		m_objects.clear();
	}

	~ObjectLib(void)
	{
		clear();
	}
	
	static ObjectLib * getInstance(void)
	{
		static ObjectLib lib;
		return &lib;
	}

	void push_back(CObjectBase * p)
	{
		m_objects.push_back(p);
	}
	
	CObjectBase *find_object(const char *name)
	{
		const int nsize = m_objects.size();
		for (int i=0; i<nsize; i++){
			CObjectBase *p = m_objects[i];
			if (p->CompareObjectName(name)==0)
				return p;
		}
		return NULL;
	}

};

//=================================================
	}//name core
}//name cia3d


#endif