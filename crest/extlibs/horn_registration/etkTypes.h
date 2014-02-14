
/*=============================================
  File etkTypes.h
    Various data types


===============================================*/

#ifndef ETK_TYPES_ZN12222007_H
#define ETK_TYPES_ZN12222007_H 

#include <vectorall.h>

//#define USE_ETK_FLOAT 

#ifdef USE_ETK_FLOAT
	#define QFLOAT float
	#define matrix3x3 float3x3
	#define float3 Vector3f
#else
	#define QFLOAT double
	#define matrix3x3 double3x3
	#define float3 Vector3d
#endif

//Compatible with Nvidia CUDA
#define make_float3(x,y,z) float3(x,y,z)
#define make_float4(x,y,z,w) float4(x,y,z,w)
//#define make_float3(x) float4(x,x,x)
//#define make_float4(x) float4(x,x,x,x)

#define __device__
#define __global__

//Quaternion structure
struct float4{	
public:
	QFLOAT x;
	QFLOAT y;
	QFLOAT z;
	QFLOAT w;
public:
	float4(void){}

	float4(const QFLOAT dX, const QFLOAT dY, const QFLOAT dZ, const QFLOAT dW)
	{
		float4 &quat = *this;
		quat.w = dW;
		quat.x = dX;
		quat.y = dY;
		quat.z = dZ;
	}
};


#endif