//================================================================

/*! \file etkRegistration.cpp
    \brief 3D-3D point registration using the Horn method.
 */

//=========in C++ environment===================
#ifdef __cplusplus
	#include <sysconf.h> 
	#include <vectorall.h>
	#include <matrix3x3.h>
	#include <memory.h>
	#include <string.h>
	#include "etkTypes.h"
	#include "etkQuartic.hpp"
	#include "etkRegistration.hpp"
#else
//in Cg, we have all the classes: float3 float4, matrix3x3 defined
	#define QFLOAT float
	#define matrix3x3 float3x3

	struct RegistrationResult
	{
		float4 quat;	//Rotation quaternion
		matrix3x3 rot;	//Rotation matrix
		float3 trans;	//Translation vector
	};

	struct QuarticSolverResult
	{
		int num;			//number of roots
		QFLOAT roots[4];	//the four roots
	};

	#include "etkQuaternion.hpp"
	#include "etkQuartic.cpp"
#endif


inline float3 cpu_mat_float3_mult(const matrix3x3 rot, const float3 t)
{
	float3 r;
	r.x = rot.x[0]*t.x + rot.x[1]*t.y + rot.x[2]*t.z;
	r.y = rot.x[3]*t.x + rot.x[4]*t.y + rot.x[5]*t.z;
	r.z = rot.x[6]*t.x + rot.x[7]*t.y + rot.x[8]*t.z;
	return r;
}


#include <crest/GPU/Cuda/etkRegistration.cu>
