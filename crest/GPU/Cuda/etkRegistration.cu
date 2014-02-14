//---------------------------------------------------------------------------
// PRIVATE: determinant of a 3x3 matrix
// ----------------------------------------------------------------------------

__device__ inline 
QFLOAT etkDet33 (const QFLOAT a1, const QFLOAT b1, const QFLOAT c1,
                 const QFLOAT a2, const QFLOAT b2, const QFLOAT c2,
                 const QFLOAT a3, const QFLOAT b3, const QFLOAT c3)
{
	QFLOAT r= a1*b2*c3 + b1*c2*a3 + c1*a2*b3 - a3*b2*c1 - b3*c2*a1 - c3*a2*b1;
	return r;
}

//---------------------------------------------------------------------------
// PRIVATE: comatrix of a 4x4 matrix
// ----------------------------------------------------------------------------
struct mat4x4{
	QFLOAT a1, b1, c1, d1;
	QFLOAT a2, b2, c2, d2;
	QFLOAT a3, b3, c3, d3;
	QFLOAT a4, b4, c4, d4;
};

__device__ inline mat4x4
etkComatrix44 (QFLOAT a1, QFLOAT b1, QFLOAT c1, QFLOAT d1,
               QFLOAT a2, QFLOAT b2, QFLOAT c2, QFLOAT d2,
               QFLOAT a3, QFLOAT b3, QFLOAT c3, QFLOAT d3,
               QFLOAT a4, QFLOAT b4, QFLOAT c4, QFLOAT d4)
{
	QFLOAT coffa1, coffa2, coffa3, coffa4, coffb1, coffb2, coffb3, coffb4,
		   coffc1, coffc2, coffc3, coffc4, coffd1, coffd2, coffd3, coffd4;
	mat4x4 m;

	coffa1 =  etkDet33(b2, c2, d2, b3, c3, d3, b4, c4, d4);
	coffb1 = -etkDet33(a2, c2, d2, a3, c3, d3, a4, c4, d4);
	coffc1 =  etkDet33(a2, b2, d2, a3, b3, d3, a4, b4, d4);
	coffd1 = -etkDet33(a2, b2, c2, a3, b3, c3, a4, b4, c4);
	coffa2 = -etkDet33(b1, c1, d1, b3, c3, d3, b4, c4, d4);
	coffb2 =  etkDet33(a1, c1, d1, a3, c3, d3, a4, c4, d4);
	coffc2 = -etkDet33(a1, b1, d1,a3, b3, d3, a4, b4, d4);
	coffd2 =  etkDet33(a1, b1, c1,a3, b3, c3, a4, b4, c4);
	coffa3 =  etkDet33(b1, c1, d1,b2, c2, d2, b4, c4, d4);
	coffb3 = -etkDet33(a1, c1, d1,a2, c2, d2, a4, c4, d4);
	coffc3 =  etkDet33(a1, b1, d1,a2, b2, d2, a4, b4, d4);
	coffd3 = -etkDet33(a1, b1, c1,a2, b2, c2, a4, b4, c4);
	coffa4 = -etkDet33(b1, c1, d1, b2, c2, d2, b3, c3, d3);
	coffb4 =  etkDet33(a1, c1, d1, a2, c2, d2, a3, c3, d3);
	coffc4 = -etkDet33(a1, b1, d1, a2, b2, d2, a3, b3, d3);
	coffd4 =  etkDet33(a1, b1, c1, a2, b2, c2, a3, b3, c3);
	m.a1 = coffa1;  m.a2 = coffa2;  m.a3 = coffa3;  m.a4 = coffa4;
	m.b1 = coffb1;  m.b2 = coffb2;  m.b3 = coffb3;  m.b4 = coffb4;
	m.c1 = coffc1;  m.c2 = coffc2;  m.c3 = coffc3;  m.c4 = coffc4;
	m.d1 = coffd1;  m.d2 = coffd2;  m.d3 = coffd3;  m.d4 = coffd4;
	return m;
}

// ----------------------------------------------------------------------------
// PRIVATE: eigen value decomposition of a 4x4 matrix
// ----------------------------------------------------------------------------
__device__ inline float4 
etkRegisterMatrix(
			QFLOAT dN11, QFLOAT dN12, QFLOAT dN13, QFLOAT dN14,
			QFLOAT dN21, QFLOAT dN22, QFLOAT dN23, QFLOAT dN24,
			QFLOAT dN31, QFLOAT dN32, QFLOAT dN33, QFLOAT dN34,
			QFLOAT dN41, QFLOAT dN42, QFLOAT dN43, QFLOAT dN44)
{
	// Determinant of N - y*I
	QFLOAT dByyy, dCyy, dDy, dE;
	int i;

	// Ay^4 + By^3 + Cy^2 + Dy + E = 0;
	// ---- First row of the determinant ----
	// 1.1
	dByyy = - dN11 - dN22 - dN33 - dN44;
	dCyy  =   dN11*dN22 + dN11*dN33 + dN11*dN44 + dN22*dN33 + dN22*dN44 + dN33*dN44;
	dDy	  = - dN11*dN22*dN33 - dN11*dN22*dN44 - dN11*dN33*dN44 - dN22*dN33*dN44;
	dE	  =   dN11*dN22*dN33*dN44;
	// 1.2
	dDy  -=	dN23*dN34*dN42;
	dE   +=	dN11*dN23*dN34*dN42;
	// 1.3
	dDy  -=	dN24*dN32*dN43;
	dE   +=	dN11*dN24*dN32*dN43;
	// 1.4
	dCyy -=   dN34*dN43;
	dDy  +=	(dN11 + dN22)*dN34*dN43;
	dE   -=	dN11*dN22*dN34*dN43;
	// 1.5
	dCyy -=	dN23*dN32;
	dDy  +=	(dN11 + dN44)*dN23*dN32;
	dE   -=	dN11*dN23*dN32*dN44;
	// 1.6
	dCyy -=	dN24*dN42;
	dDy  +=	(dN11 + dN33)*dN24*dN42;
	dE   -=	dN11*dN24*dN33*dN42;

	// ---- Second row of the determinant ----
	// 2.1
	dCyy -= dN21*dN12;
	dDy  += dN21*dN12*(dN33 + dN44);
	dE   -= dN21*dN12*dN33*dN44;
	// 2.2
	dE  -= dN21*dN13*dN34*dN42;
	// 2.3
	dE  -= dN21*dN14*dN32*dN43;
	// 2.4
	dE += dN21*dN12*dN34*dN43;
	// 2.5
	dDy -= dN21*dN13*dN32;
	dE += dN21*dN13*dN32*dN44;
	// 2.6
	dDy -= dN21*dN14*dN42;
	dE  += dN21*dN14*dN33*dN42;

	// ---- Third row of the determinant ----
	// 3.1
	dDy -= dN31*dN12*dN23;
	dE += dN31*dN12*dN23*dN44;
	// 3.2
	dE += dN31*dN13*dN24*dN42;
	// 3.3
	dDy -= dN31*dN14*dN43;
	dE += dN31*dN14*dN22*dN43;
	// 3.4
	dE -= dN31*dN12*dN24*dN43;
	// 3.5
	dCyy -= dN31*dN13;
	dDy += dN31*dN13*(dN22 + dN44);
	dE -= dN31*dN13*dN22*dN44;
	// 3.6
	dE -= dN31*dN14*dN23*dN42;

	// ---- Forth row of the determinant ----
	// 4.1
	dE -= dN41*dN12*dN23*dN34;
	// 4.2
	dE -= dN41*dN13*dN24*dN32;
	// 4.3
	dCyy -= dN41*dN14;
	dDy += dN41*dN14*(dN22 + dN33);
	dE -= dN41*dN14*dN22*dN33;
	// 4.4
	dDy -= dN41*dN12*dN24;
	dE += dN41*dN12*dN24*dN33;
	// 4.5
	dDy -= dN41*dN13*dN34;
	dE += dN41*dN13*dN34*dN22;
	// 4.6
	dE += dN41*dN14*dN23*dN32;

	//Solving the quartic equation to obtain the eigenvalues
	/*{
	QFLOAT Amax = fabs(dByyy);
	Amax = max(Amax, fabs(dCyy));
	Amax = max(Amax, fabs(dDy));
	Amax = max(Amax, fabs(dE));
	QFLOAT Amax1 = 1.0/Amax;
	dByyy *= Amax1;
	dCyy *= Amax1;
	dDy *= Amax1;
	dE *= Amax1;
	const QFLOAT TOL=1e-20;
	if (fabs(dByyy)<TOL) dByyy = 0;
	if (fabs(dCyy)<TOL) dCyy = 0;
	if (fabs(dDy)<TOL) dDy = 0;
	if (fabs(dE)<TOL) dE = 0;
	}*/
	QuarticSolverResult result = etkQuartic(dByyy, dCyy, dDy, dE);
	const int iNbRoots = result.num;
	const QFLOAT adRoots[4]={result.roots[0], result.roots[1], result.roots[2], result.roots[3]};

	// Finding the  argest eigenvalue
#ifdef __CUDACC__
	QFLOAT dMaxEigenValue = max(adRoots[0], adRoots[1]);
	dMaxEigenValue = max(dMaxEigenValue, adRoots[2]);
	dMaxEigenValue = max(dMaxEigenValue, adRoots[3]);
#else
	QFLOAT dMaxEigenValue = adRoots[0];
	for (i=1; i<iNbRoots; i++){
		if (adRoots[i] > dMaxEigenValue)
			dMaxEigenValue = adRoots[i];
	}
#endif

	QFLOAT dN11bis = dN11 - dMaxEigenValue;
	QFLOAT dN22bis = dN22 - dMaxEigenValue;
	QFLOAT dN33bis = dN33 - dMaxEigenValue;
	QFLOAT dN44bis = dN44 - dMaxEigenValue;
	mat4x4 m = etkComatrix44(
					dN11bis, dN12, dN13, dN14,
					dN21, dN22bis, dN23, dN24,
					dN31, dN32, dN33bis, dN34,
					dN41, dN42, dN43, dN44bis);
	dN11bis=m.a1, dN12=m.b1, dN13=m.c1, dN14=m.d1;
	dN21=m.a2, dN22bis=m.b2, dN23=m.c2, dN24=m.d2;
	dN31=m.a3, dN32=m.b3, dN33bis=m.c3, dN34=m.d3;
	dN41=m.a4, dN42=m.b4, dN43=m.c4, dN44bis=m.d4;
	QFLOAT qx=dN12+dN22bis+dN32+dN42;
	QFLOAT qy=dN13+dN23+dN33bis+dN43;
	QFLOAT qz=dN14+dN24+dN34+dN44bis;
	QFLOAT qw=dN11bis+dN21+dN31+dN41;	
	float4 quat = make_float4(qx, qy, qz, qw);
	return normalize4(quat);
}


// ----------------------------------------------------------------------------
// Register the two point sets.
// - Rotation matrix is stored in p33Rot,
// - Translation vector is stored in pTrans
// - Return error matrix
// ----------------------------------------------------------------------------

__device__ inline float3 weightedSum(const float3 p[4], const QFLOAT w[4], const int n)
{
	float3 sum = p[0]*w[0];
   #ifdef __CUDACC__
    sum+= p[1]*w[1];
    sum+= p[2]*w[2];
    sum+= p[3]*w[3];
   #else	
	for (int i=1; i<n; i++)
		sum += p[i]*w[i];
   #endif
	return sum;
}


__device__ inline void _etkRegister(
	const int nPoints, const float3 _adModelPoints[4], const float3 _adSensorPoints[4], const QFLOAT weights[4], //Input parameters
    float4 result_quat[1], matrix3x3 result_rot[1], float3 result_trans[1])  //return parameters
{
	int u;
	float3 adModelPoints[16];
	float3 adSensorPoints[16];

	// 1. Find centroid
	const float3 adModelCentroid = weightedSum(_adModelPoints, weights, nPoints);
	const float3 adSensorCentroid = weightedSum(_adSensorPoints, weights, nPoints);

	// 2. Recenter points;
   #ifdef __CUDACC__
        u = 0;
		adModelPoints[u]=_adModelPoints[u]-adModelCentroid;
		adSensorPoints[u]=_adSensorPoints[u]-adSensorCentroid;
        u = 1;
		adModelPoints[u]=_adModelPoints[u]-adModelCentroid;
		adSensorPoints[u]=_adSensorPoints[u]-adSensorCentroid;
        u = 2;
		adModelPoints[u]=_adModelPoints[u]-adModelCentroid;
		adSensorPoints[u]=_adSensorPoints[u]-adSensorCentroid;
        u = 3;
		adModelPoints[u]=_adModelPoints[u]-adModelCentroid;
		adSensorPoints[u]=_adSensorPoints[u]-adSensorCentroid;	
   #else
	for (u = 0; u < nPoints; u++){
		adModelPoints[u]=_adModelPoints[u]-adModelCentroid;
		adSensorPoints[u]=_adSensorPoints[u]-adSensorCentroid;
	}	   
   #endif
   	
	// 3. Compute momentum
	QFLOAT dSxx, dSxy, dSxz, dSyx, dSyy, dSyz, dSzx, dSzy, dSzz;
	dSxx=dSxy=dSxz=dSyx=dSyy=dSyz=dSzx=dSzy=dSzz=0;
	for (u = 0; u < nPoints; u++){
		const float3 v1 = adModelPoints[u];
		const float3 v2 = adSensorPoints[u];
		const QFLOAT w = weights[u];
		const QFLOAT p0=v1.x, p1=v1.y, p2=v1.z;
		const QFLOAT q0=v2.x*w, q1=v2.y*w, q2=v2.z*w;	
		dSxx += p0 * q0; dSxy += p0 * q1;
		dSxz += p0 * q2; dSyx += p1 * q0;
		dSyy += p1 * q1; dSyz += p1 * q2;
		dSzx += p2 * q0; dSzy += p2 * q1;
		dSzz += p2 * q2;
	}

	// N Symmetric Matrix
	QFLOAT dN11 = dSxx + dSyy + dSzz;
	QFLOAT dN12 = dSyz - dSzy;
	QFLOAT dN13 = dSzx - dSxz;
	QFLOAT dN14 = dSxy - dSyx;
	QFLOAT dN21 = dN12;
	QFLOAT dN22 = dSxx - dSyy - dSzz;
	QFLOAT dN23 = dSxy + dSyx;
	QFLOAT dN24 = dSzx + dSxz;
	QFLOAT dN31 = dN13;
	QFLOAT dN32 = dN23;
	QFLOAT dN33 = -dSxx + dSyy - dSzz;
	QFLOAT dN34 = dSyz + dSzy;
	QFLOAT dN41 = dN14;
	QFLOAT dN42 = dN24;
	QFLOAT dN43 = dN34;
	QFLOAT dN44 = -dSxx - dSyy + dSzz;

	//compute rotation
	result_quat[0] = etkRegisterMatrix(dN11, dN12, dN13, dN14, dN21, dN22, dN23, dN24, dN31, dN32, dN33, dN34, dN41, dN42, dN43, dN44);		
	
	//vector is column vector to mult the matrix, therefore, we use y=Ax
	result_rot[0] = quaterionToMatrix33_v2(result_quat[0]);	
	
    //compute translation
  #ifdef __CUDACC__
    //GPU implementation
	result_trans[0]= adSensorCentroid - mat_float3_mult(result_rot[0], adModelCentroid);
  #else 
  #ifdef __cplusplus
    //C++ CPU implementation
	result_trans[0] = adSensorCentroid - result_rot[0] * adModelCentroid;
  #else 
	//Cg language
	result_trans[0] = adSensorCentroid - mul(result_rot[0], adModelCentroid);
  #endif
  #endif
}


__device__ inline void EtkRegister(
	const int nPoints, 
	const float3 _adModelPoints[], 
	const float3 _adSensorPoints[], 
	const QFLOAT weights[],				//Input parameters
    float4 result_quat[1])				//return parameters
{
	const float3* adModelPoints = _adModelPoints;
	const float3* adSensorPoints = _adSensorPoints;
   	
	// 3. Compute momentum
	QFLOAT dSxx, dSxy, dSxz, dSyx, dSyy, dSyz, dSzx, dSzy, dSzz;
	dSxx=dSxy=dSxz=dSyx=dSyy=dSyz=dSzx=dSzy=dSzz=0;
	for (int u = 0; u < nPoints; u++){
		const float3& v1 = adModelPoints[u];
		const float3& v2 = adSensorPoints[u];
		const QFLOAT& w = weights[u];
		const QFLOAT p0=v1.x, p1=v1.y, p2=v1.z;
		const QFLOAT q0=v2.x*w, q1=v2.y*w, q2=v2.z*w;	
		dSxx += p0 * q0; dSxy += p0 * q1;
		dSxz += p0 * q2; dSyx += p1 * q0;
		dSyy += p1 * q1; dSyz += p1 * q2;
		dSzx += p2 * q0; dSzy += p2 * q1;
		dSzz += p2 * q2;
	}

	// N Symmetric Matrix
	QFLOAT dN11 = dSxx + dSyy + dSzz;
	QFLOAT dN12 = dSyz - dSzy;
	QFLOAT dN13 = dSzx - dSxz;
	QFLOAT dN14 = dSxy - dSyx;
	QFLOAT dN21 = dN12;
	QFLOAT dN22 = dSxx - dSyy - dSzz;
	QFLOAT dN23 = dSxy + dSyx;
	QFLOAT dN24 = dSzx + dSxz;
	QFLOAT dN31 = dN13;
	QFLOAT dN32 = dN23;
	QFLOAT dN33 = -dSxx + dSyy - dSzz;
	QFLOAT dN34 = dSyz + dSzy;
	QFLOAT dN41 = dN14;
	QFLOAT dN42 = dN24;
	QFLOAT dN43 = dN34;
	QFLOAT dN44 = -dSxx - dSyy + dSzz;

	//compute rotation
	result_quat[0] = etkRegisterMatrix(dN11, dN12, dN13, dN14, dN21, dN22, dN23, dN24, dN31, dN32, dN33, dN34, dN41, dN42, dN43, dN44);	
}


