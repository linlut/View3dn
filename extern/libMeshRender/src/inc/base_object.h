#ifndef INC_OBJECTBASE_H_
#define INC_OBJECTBASE_H_

#include <typeinfo> 
#include <string>
#include <sysconf.h>

const int OBJECTBASE_NAMELEN = 24;

class CObjectBase
{
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

private:
	char m_sObjectName[OBJECTBASE_NAMELEN];

};

#endif
