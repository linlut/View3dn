//FILE: element_base.h
//the base for all physically based simulation elements

#ifndef _INC_ELEMENTBASE_ZN_20091028H
#define _INC_ELEMENTBASE_ZN_20091028H

#include <sysconf.h>
#include <matrix3x3.h>

class CSparseMatrix33;

class CElementBase
{
public:
	CElementBase(void){}

	~CElementBase(){}

	void saveStiffness(CSparseMatrix33 *p, const double3x3 jac[]){ assert(0); }

	void updateState(void){}

	static bool hasConstantStiffness(void)
	{ 
		return false; 
	}

	static bool isStiffnessMatrix3X3(void)
	{ 
		return true; 
	}


};

#endif