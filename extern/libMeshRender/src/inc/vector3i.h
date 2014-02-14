//FILE: vector3i.h

#ifndef __VECTOR3I__
#define __VECTOR3I__

//use template class 
#include <vector3d.h>
typedef _Vector3T<int> Vector3i;
typedef Vector3i Vector3I;

//short triple
typedef _Vector3T<short> Vector3s;

inline int _vec3i_compare (const void * a, const void * b)
{
	Vector3I * p1 = (Vector3I*) a;
	Vector3I * p2 = (Vector3I*) b;
	int s0 = (p1->x - p2->x);
	if (s0!=0) return s0;
	int s1 = (p1->y - p2->y);
	if (s1!=0) return s1;
	int s2 = (p1->z - p2->z);
	return s2;
}

#endif
