//FILE: etkQuaternion.cu  
// I try to make this part compatible with NVIDIA CUDA and NVIDIA CG

/// Normalize a quaternion
__device__ inline float4 normalize4(const float4 quat)
{
	QFLOAT x = quat.x;
	QFLOAT y = quat.y;
	QFLOAT z = quat.z;
	QFLOAT w = quat.w;
	QFLOAT dNorm = sqrt(w*w + x*x + y*y + z*z);
	if (dNorm == 0){
		x = y = z = 0, w=1;
	}
	else{
		QFLOAT n1 = 1.0/dNorm;
		w *= n1;
		x *= n1;
		y *= n1;
		z *= n1;
	}
	//Avoid zero norm
	return make_float4(x,y,z,w);
}


/// Transform a quaternion to a 3x3 (rotation) matrix
__device__ inline matrix3x3 quaterionToMatrix33(const float4 quat)
{
	matrix3x3 mat33;
	const QFLOAT dX = quat.x;
	const QFLOAT dY = quat.y;
	const QFLOAT dZ = quat.z;
	const QFLOAT dW = quat.w;
	const QFLOAT dX2 = dX + dX;
	const QFLOAT dY2 = dY + dY;
	const QFLOAT dZ2 = dZ + dZ;
	const QFLOAT dXX = dX * dX2,
				 dXY = dX * dY2,
				 dXZ = dX * dZ2,
				 dYY = dY * dY2,
				 dYZ = dY * dZ2,
				 dZZ = dZ * dZ2,
				 dWX = dW * dX2,
				 dWY = dW * dY2,
				 dWZ = dW * dZ2;
	mat33.x[0] = 1 - (dYY + dZZ);
	mat33.x[1] = dXY + dWZ;
	mat33.x[2] = dXZ - dWY;
	mat33.x[3] = dXY - dWZ;
	mat33.x[4] = 1 - (dXX + dZZ);
	mat33.x[5] = dYZ + dWX;
	mat33.x[6] = dXZ + dWY;
	mat33.x[7] = dYZ - dWX;
	mat33.x[8] = 1 - (dXX + dYY);
	return mat33;
 }


__device__ inline matrix3x3 quaterionToMatrix33_v2(const float4 pquat)
{
	matrix3x3 mat33;
	const QFLOAT dX = pquat.x;
	const QFLOAT dY = pquat.y;
	const QFLOAT dZ = pquat.z;
	const QFLOAT dW = pquat.w;

	const QFLOAT dX2 = dX + dX;
	const QFLOAT dY2 = dY + dY;
	const QFLOAT dZ2 = dZ + dZ;
	const QFLOAT dXX = dX * dX2,
				 dXY = dX * dY2,
				 dXZ = dX * dZ2,
				 dYY = dY * dY2,
				 dYZ = dY * dZ2,
				 dZZ = dZ * dZ2,
				 dWX = dW * dX2,
				 dWY = dW * dY2,
				 dWZ = dW * dZ2;

	mat33.x[0] = 1 - (dYY + dZZ);
	mat33.x[3] = dXY + dWZ;
	mat33.x[6] = dXZ - dWY;
	mat33.x[1] = dXY - dWZ;
	mat33.x[4] = 1 - (dXX + dZZ);
	mat33.x[7] = dYZ + dWX;
	mat33.x[2] = dXZ + dWY;
	mat33.x[5] = dYZ - dWX;
	mat33.x[8] = 1 - (dXX + dYY);
	return mat33;
  }
