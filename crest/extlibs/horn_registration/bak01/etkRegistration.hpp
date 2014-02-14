
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
#include <vector3d.h>
#include <vector3f.h>
#include <matrix3x3.h>
#include "etkQuaternion.hpp"


//Two functions for weight computation
inline void computeSquareDistanceWeights(const Vector3d& center, Vector3d refVertex[], const int nv, double weights[])
{
	double sum_w = 0;
	for (int i=0; i<nv; i++){
		const double dist2 = Distance2(center, refVertex[i]) + 1e-16;
		const double w = 1.0 / dist2;
		sum_w += w;
		weights[i] = w;
	}
	double inv_sum_w = 1.0/sum_w;
	for (int i=0; i<nv; i++)
		weights[i] *= inv_sum_w;
}

inline void computeSquareDistanceWeights(const Vector3f& center, Vector3f refVertex[], const int nv, float weights[])
{
	double sum_w = 0;
	for (int i=0; i<nv; i++){
		const double dist2 = Distance2(center, refVertex[i]) + 1e-16;
		const double w = 1.0 / dist2;
		sum_w += w;
		weights[i] = w;
	}
	float inv_sum_w = (float)(1.0/sum_w);
	for (int i=0; i<nv; i++)
		weights[i] *= inv_sum_w;
}


  /// Maximum number of points during the registration  
  #define REGISTRATION_MAX_POINTS 40

 
  /// Registration structure
  class etkRegistration
  {
  private:
    /// Number of points to be registered (minimum 3)
    int uNbPoints;

    /// Model points
    Vector3d adModelPoints[REGISTRATION_MAX_POINTS];

    /// Sensor points
    Vector3d adSensorPoints[REGISTRATION_MAX_POINTS];

	double weights[REGISTRATION_MAX_POINTS];

  private:

	// p: the model points
	// q: the sensor points
	// weights: the weights for points, can be NULL ( all weights 1)
	inline void _addPoints(const Vector3d *p, const Vector3d* q, const double *W, const int n)
	{
		int i;

		if (W==NULL){
			const double n1 = 1.0f/n;
			for (i=0; i<n; i++) weights[i] = n1;
		}
		else{
			for (i=0; i<n; i++) weights[i] = W[i];
		}

		//use the weights to scale the points
		for (i=0; i<n; i++){
			adModelPoints[i] = p[i];
			adSensorPoints[i] = q[i];
		}
		uNbPoints = n;
	}

	inline void _addPoints(const Vector3f *p, const Vector3f* q, const float *W, const int n)
	{
		Vector3d dp[30], dq[30];

		ASSERT0(n<30);		
		for (int i=0; i<n; i++){
			const Vector3f *pi = &p[i];
			Vector3d *dpi = &dp[i];
			const Vector3f *qi = &q[i];
			Vector3d *dqi = &dq[i];
			dpi->x = p->x, dpi->y = p->y, dpi->z = p->z;
			dqi->x = q->x, dqi->y = q->y, dqi->z = q->z;
		}
		if (W){
			double dw[30];
			for (int i=0; i<n; i++) dw[i]=W[i];
			_addPoints(dp, dq, dw, n);
		}
		else
			_addPoints(dp, dq, NULL, n);
	}

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
	double etkRegistration::_etkRegister(etkQuaternion* pQuat, double* radRot33, double* radTrans3, const bool getRMS);

  public:

	etkRegistration(void)
	{
		uNbPoints = 0;
	}

	~etkRegistration(void)
	{
	}

	// p: the model points
	// q: the sensor points
	// w: weights
	inline double hornRegistration(const Vector3f *p, const Vector3f* q, const float *w, const int n, etkQuaternion* pQuat, double rot9[9], Vector3f& trans, float &scale, const bool getRMS=false)
	{
		Vector3d _transd;

		_addPoints(p, q, w, n);
		const double dError= _etkRegister(pQuat, rot9, &_transd.x, getRMS);
		trans.x = _transd.x, trans.y = _transd.y, trans.z = _transd.z;
		return dError;
	}

	inline void hornRegistration(const Vector3d *p, const Vector3d* q, const double *w, const int n, etkQuaternion& etkquat, double3x3& rot, Vector3d& trans)
	{
		double* rot9d= &rot.x[0][0];
		_addPoints(p, q, w, n);
		const double dError= _etkRegister(&etkquat, rot9d, &trans.x, false);
	}

	inline void hornRegistration(const Vector3f *p, const Vector3f* q, const float *w, const int n, Vector4f & quat, float3x3& rot, Vector3f& trans)
	{
		Vector3d _transd;
		double rot9d[9];
		etkQuaternion etkquat;

		_addPoints(p, q, w, n);
		const double dError= _etkRegister(&etkquat, rot9d, &_transd.x, false);
		trans.x = _transd.x, trans.y = _transd.y, trans.z = _transd.z;
		quat.x = etkquat.dX, quat.y = etkquat.dY, quat.z = etkquat.dZ, quat.w = etkquat.dW;
		float *pm = &rot.x[0][0];
		pm[0] = rot9d[0], pm[1] = rot9d[1], pm[2] = rot9d[2],
		pm[3] = rot9d[3], pm[4] = rot9d[4], pm[5] = rot9d[5],
		pm[6] = rot9d[6], pm[7] = rot9d[7], pm[8] = rot9d[8];
	}

  };
  

#endif
