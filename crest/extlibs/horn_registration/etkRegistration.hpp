
//File: etkRegistration.hpp

/*! \file etkRegistration.hpp
    \brief 3D-3D point registration using the Horn method.
    
    Click \htmlonly <a href="../Horn1987.pdf">here</a> \endhtmlonly
    to read the original Horn paper: "Closed-form solution of 
    absolute orientation using unit quaternions" (1987)
    
*/

#ifndef etkRegistration_hpp
#define etkRegistration_hpp

#include <assert.h>
#include <sysconf.h>
#include <matrix3x3.h>
#include "etkTypes.h"
#include "etkQuaternion.hpp"



//A function do the basic thing
/*
struct RegistrationResult
{
	float4 quat;	//Rotation quaternion
	matrix3x3 rot;	//Rotation matrix
	float3 trans;	//Translation vector
};
*/

void _etkRegister(
	const int uNbPoints, const float3 adModelPoints[4], 
	const float3 adSensorPoints[4], const QFLOAT weights[4],
 	float4& quat, matrix3x3& radRot33, float3& radTrans3);


void _etkRegister(
	const int uNbPoints, const float3 adModelPoints[4], 
	const float3 adSensorPoints[4], const QFLOAT weights[4], 
    float4 result_quat[1], matrix3x3 result_rot[1], float3 result_trans[1]);  //return parameters


//Two functions for weight computation
inline void computeSquareDistanceWeights(const float3& center, const float3 refVertex[4], const int nv, QFLOAT weights[4])
{
	const QFLOAT WEIGHTMAX=0.90f;
	const QFLOAT WEIGHTMIN=(1.0-WEIGHTMAX)/3;
	int i;

	QFLOAT sum_w = 0;
	for (i=0; i<nv; i++){
		const QFLOAT dist2 = Distance2(center, refVertex[i]) + 1e-6;
		const QFLOAT w = 1.0 / dist2;
		sum_w += w;
		weights[i] = w;
	}
	QFLOAT inv_sum_w = 1.0/sum_w;
	for (i=0; i<nv; i++){
		weights[i] *= inv_sum_w;
		if (weights[i]>WEIGHTMAX)
			weights[i]=WEIGHTMAX;
		else if (weights[i]<WEIGHTMIN)
			weights[i]=WEIGHTMIN;
	}
}


//Two functions for weight computation
inline void computeSquareDistanceWeights(const Vector3d& center, const Vector3d refVertex[4], const int nv, const double &damp, double weights[4])
{
	const double WEIGHTMAX=0.90;
	const double WEIGHTMIN=(1.0-WEIGHTMAX)/3;
	double sum_w = 0;
	int i;

	for (i=0; i<nv; i++){
		const double dist2 = Distance2(center, refVertex[i]) + damp;
		const double w = 1.0 / dist2;
		sum_w += w;
		weights[i] = w;
	}
	const double inv_sum_w = 1.0/sum_w;
	for (i=0; i<nv; i++){
		weights[i] *= inv_sum_w;
	}
}


//Two functions for weight computation
inline void computeSquareDistanceWeightsNoDamp(const Vector3d& center, const Vector3d refVertex[4], const int nv, double weights[4])
{
	double sum_w = 0;
	int i;

	for (i=0; i<nv; i++){
		const double dist2 = Distance2(center, refVertex[i]) + 1e-16;
		const double w = 1.0 / dist2;
		sum_w += w;
		weights[i] = w;
	}
	const double inv_sum_w = 1.0/sum_w;
	for (i=0; i<nv; i++){
		weights[i] *= inv_sum_w;
	}
}


/// Maximum number of points during the registration  
#define REGISTRATION_MAX_POINTS 20

 
/// Registration structure
class etkRegistration
  {
  private:
	/// Register two datasets using the Horn method. 
	/// Return Return mean error or < 0 in case of an error.
	/// - Rotation matrix is stored in p33Rot 
	///
	///   | a0 a1 a2 |   <====Wrong, stored as COLUMN major, be careful!
	///   | a3 a4 a5 |
	///   | a6 a7 a8 |
	///
	/// - Translation vector is stored in pTrans 
	///
	///   | b0 |
	///   | b1 |
	///   | b2 |
	//QFLOAT etkRegistration::_etkRegister(float4* pQuat, QFLOAT* radRot33, QFLOAT* radTrans3, const bool getRMS);

  public:

	etkRegistration(void)
	{
	}

	~etkRegistration(void)
	{
	}

#ifndef USE_ETK_FLOAT
	#define FLOAT_TP float
	#define VECT_TP  Vector3f
#else
	#define FLOAT_TP double
	#define VECT_TP  Vector3d
#endif

	inline void hornRegistration(const VECT_TP p[], const VECT_TP q[], const FLOAT_TP w[], const int n, float4& quat, matrix3x3& rot, float3& trans)
	{
		ASSERT0(n<REGISTRATION_MAX_POINTS);
		float3 adModelPoints[REGISTRATION_MAX_POINTS];
		float3 adSensorPoints[REGISTRATION_MAX_POINTS];
		QFLOAT weights[REGISTRATION_MAX_POINTS];
		for (int i=0; i<n; i++){
			adModelPoints[i] = float3(p[i].x, p[i].y, p[i].z);
			adSensorPoints[i] = float3(q[i].x, q[i].y, q[i].z);
			weights[i] = w[i];
		}
		_etkRegister(n, adModelPoints, adSensorPoints, weights, &quat, &rot, &trans);
	}

	// p: the model points
	// q: the sensor points
	// w: weights
	inline void hornRegistration(const float3 p[], const float3 q[], const QFLOAT w[], const int n, float4& quat, matrix3x3& rot, float3& trans)
	{
		_etkRegister(n, p, q, w, &quat, &rot, &trans);
	}
	
};


#endif
