/*! \file etkQuaternion.hpp
    \brief  Quaternion basic operations.
 */

#ifndef etkQuaternion_hpp
#define etkQuaternion_hpp 1

  #include "etkMatrix.hpp"

  /// Quaternion structure
  struct etkQuaternion 
  {	
	double dX;
	double dY;
	double dZ;
	double dW;
  };

  /// Create and initialize a quaternion
  inline void etkCreateQuaternion (etkQuaternion* pQuat, const double& dW, const double& dX, const double& dY, const double& dZ)
  {
	pQuat->dW = dW;
	pQuat->dX = dX;
	pQuat->dY = dY;
	pQuat->dZ = dZ;
  }

  /// Normalize a quaternion
  inline void etkNormalizeQuaterion (etkQuaternion* pQuat)
  {
	double& w = pQuat->dW;
	double& x = pQuat->dX;
	double& y = pQuat->dY;
	double& z = pQuat->dZ;
    const double dNorm = sqrt(w*w + x*x + y*y + z*z);
    const double n1 = 1.0/dNorm;
    w *= n1;
    x *= n1;
	y *= n1;
	z *= n1;
  }

  /// Transform a quaternion to a 3x3 (rotation) matrix
  inline void etkQuaterionToMatrix33 (const etkQuaternion* pquat, matrix* pMat33)
  {
	const double dX = pquat->dX;
	const double dY = pquat->dY;
	const double dZ = pquat->dZ;
	const double dW = pquat->dW;

	const double dX2 = dX + dX;
	const double dY2 = dY + dY;
	const double dZ2 = dZ + dZ;
	const double dXX = dX * dX2,
				 dXY = dX * dY2,
				 dXZ = dX * dZ2,
				 dYY = dY * dY2,
				 dYZ = dY * dZ2,
				 dZZ = dZ * dZ2,
				 dWX = dW * dX2,
				 dWY = dW * dY2,
				 dWZ = dW * dZ2;

	pMat33->block [0] = 1.0 - (dYY + dZZ);
	pMat33->block [1] = dXY + dWZ;
	pMat33->block [2] = dXZ - dWY;
	pMat33->block [3] = dXY - dWZ;
	pMat33->block [4] = 1.0 - (dXX + dZZ);
	pMat33->block [5] = dYZ + dWX;
	pMat33->block [6] = dXZ + dWY;
	pMat33->block [7] = dYZ - dWX;
	pMat33->block [8] = 1.0 - (dXX + dYY);
  }

  inline void etkQuaterionToMatrix33_v2 (const etkQuaternion* pquat, matrix* pMat33)
  {
	const double dX = pquat->dX;
	const double dY = pquat->dY;
	const double dZ = pquat->dZ;
	const double dW = pquat->dW;

	const double dX2 = dX + dX;
	const double dY2 = dY + dY;
	const double dZ2 = dZ + dZ;
	const double dXX = dX * dX2,
				 dXY = dX * dY2,
				 dXZ = dX * dZ2,
				 dYY = dY * dY2,
				 dYZ = dY * dZ2,
				 dZZ = dZ * dZ2,
				 dWX = dW * dX2,
				 dWY = dW * dY2,
				 dWZ = dW * dZ2;

	pMat33->block [0] = 1.0 - (dYY + dZZ);
	pMat33->block [3] = dXY + dWZ;
	pMat33->block [6] = dXZ - dWY;
	pMat33->block [1] = dXY - dWZ;
	pMat33->block [4] = 1.0 - (dXX + dZZ);
	pMat33->block [7] = dYZ + dWX;
	pMat33->block [2] = dXZ + dWY;
	pMat33->block [5] = dYZ - dWX;
	pMat33->block [8] = 1.0 - (dXX + dYY);
  }


#endif
