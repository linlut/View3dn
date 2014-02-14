/////////////////////////////////////////////////////////////////////////////
/// Collision Detection
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////
#ifndef __MATHDEFS_H__
#define __MATHDEFS_H__

#define M_PI        3.14159265358979323846f
#define HALF_PI	    1.57079632679489661923f

/// Trig Macros ///////////////////////////////////////////////////////////////
#define DEGTORAD(A)	((A * M_PI) / 180.0f)
#define RADTODEG(A)	((A * 180.0f) / M_PI)
///////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "defines.h"


typedef struct
{
	float m[16];
} tMatrix;


#define MAKEVECTOR(a,vx,vy,vz)	{a.x = vx; a.y = vy; a.z = vz;}

//void	MultVectorByMatrix(tMatrix *mat, vertex *v,vertex *result);
//inline void	MultVectorByMatrix(float m[], vertex *v,vertex *result);
//inline double VectorSquaredLength(vertex *v); 
//inline double VectorLength(vertex *v); 
//inline void	NormalizeVector(vertex *v); 
//inline double DotProduct(vertex *v1, vertex *v2);
//inline void	CrossProduct(vertex *v1, vertex *v2, vertex *result);
//inline double VectorSquaredDistance(vertex *v1, vertex *v2);
//inline void	ScaleVector(vertex *v, float scale, vertex *result);
//inline void	VectorSum(vertex *v1, vertex *v2, vertex *result);
//inline void	VectorDifference(vertex *v1, vertex *v2, vertex *result);
//inline void	SetIdentityMatrix(float* m);
//inline void	Multiply44(float* a, float* b);
//inline void	Transform(float* m, vertex* v0, vertex* v, long nLength);
//inline void	Transforms(float* m, vertex* v, long nLength);
//inline void	SetTranslateMatrix(float* m, vertex t);
//inline void	SetTranslateMatrix2(float* m, float x, float y, float z);
//inline void	SetScaleMatrix(float* m, float x, float y, float z, vertex center);
//inline void	SetScaleMatrix(float* m, float x, float y, float z);
//inline void	SetRotateMatrix(float* m, vertex p1, vertex p2, float radians);
//inline void	SetOriginRotateMatrix(float* m, vertex radians);
//inline vertex*	CalcNormal(vertex* v1, vertex* v2, vertex* v3, vertex* norm, int iNormalize=1);
//inline vertex*	Norm(vertex* vector);
//inline float TriArea(vertex v1, vertex v2, vertex v3);
//inline float TetVolume(float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3,float x4,float y4,float z4);

void	MultVectorByMatrix(float m[], vertex *v,vertex *result);
double VectorSquaredLength(vertex *v); 
double VectorLength(vertex *v); 
void	NormalizeVector(vertex *v); 
double DotProduct(vertex *v1, vertex *v2);
void	CrossProduct(vertex *v1, vertex *v2, vertex *result);
double VectorSquaredDistance(vertex *v1, vertex *v2);
void	ScaleVector(vertex *v, float scale, vertex *result);
void	VectorSum(vertex *v1, vertex *v2, vertex *result);
void	VectorDifference(vertex *v1, vertex *v2, vertex *result);
void	SetIdentityMatrix(float* m);
void	Multiply44(float* a, float* b);
void	Transform(float* m, vertex* v0, vertex* v, long nLength);
void	Transforms(float* m, vertex* v, long nLength);
void	SetTranslateMatrix(float* m, vertex t);
void	SetTranslateMatrix2(float* m, float x, float y, float z);
void	SetScaleMatrix(float* m, float x, float y, float z, vertex center);
void	SetScaleMatrix(float* m, float x, float y, float z);
void	SetRotateMatrix(float* m, vertex p1, vertex p2, float radians);
void	SetOriginRotateMatrix(float* m, vertex radians);
vertex*	CalcNormal(vertex* v1, vertex* v2, vertex* v3, vertex* norm, int iNormalize=1);
vertex*	Norm(vertex* vector);
inline float TriArea(vertex v1, vertex v2, vertex v3);
inline float TetVolume(float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3,float x4,float y4,float z4);
/////////////////////////////////////////////////////////////////////////////
typedef double lreal;
typedef float  real;
typedef unsigned long uint32;
typedef long int32;

const lreal _double2fixmagic = 68719476736.0*1.5;     //2^36 * 1.5,  (52-_shiftamt=36) uses limited precisicion to floor
const int32 _shiftamt        = 16;                    //16.16 fixed point representation,

#if BigEndian_
	#define iexp_				0
	#define iman_				1
#else
	#define iexp_				1
	#define iman_				0
#endif //BigEndian_

// ==========================================================================
// Real2Int
// ==========================================================================
inline int32 Real2Int(lreal val)
{
#if DEFAULT_CONVERSION
	return val;
#else
	val		= val + _double2fixmagic;
	return ((int32*)&val)[iman_] >> _shiftamt; 
#endif
}

// ==========================================================================
// Real2Int
// ==========================================================================
inline int32 Real2Int(real val)
{
#if DEFAULT_CONVERSION
	return val;
#else
	return Real2Int ((lreal)val);
#endif
}
/////////////////////////////////////////////////////////////////////////////

inline float TetVolume(float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3,float x4,float y4,float z4)
{ 
	float A1,A2,A3,A4,value;
	A1=x2*(y3*z4-y4*z3)-x3*(y2*z4-y4*z2)+x4*(y2*z3-y3*z2);
	A2=x1*(y3*z4-y4*z3)-x3*(y1*z4-y4*z1)+x4*(y1*z3-y3*z1);
	A3=x1*(y2*z4-y4*z2)-x2*(y1*z4-y4*z1)+x4*(y1*z2-y2*z1);
	A4=x1*(y2*z3-y3*z2)-x2*(y1*z3-y3*z1)+x3*(y1*z2-y2*z1);
	return (A1-A2+A3-A4)/6.0f;
}

inline float TriArea(vertex v1, vertex v2, vertex v3)
{
	vertex a, b, s;
	VectorDifference(&v1,&v2,&a);
	VectorDifference(&v3,&v2,&b);
	CrossProduct(&a,&b,&s);
	return sqrt(s.x*s.x+s.y*s.y+s.z*s.z)/2.0;
}

/////////////////////////////////////////////////////////////////////////////
#endif