//===================================================================
//FILE: pntriangle.h
//Generate a Bezier patch for a triangle. Based on the paper
//"Cuvrved PN Triangles" by Alex Vlachos et al in I3D 2001 proceedings
//===================================================================

#ifndef _INC_PNTRIANGLE_H_
#define _INC_PNTRIANGLE_H_

#include <sysconf.h>
#include <vectorall.h>


inline void computePositionCoefficientsPN(
	const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, //input tri. vertex positions
	const Vector3f& N1, const Vector3f& N2, const Vector3f& N3, //input tri. vertex normals
	Vector3f& b210, Vector3f& b120, Vector3f& b021, Vector3f& b012, Vector3f& b102, Vector3f& b201, Vector3f& b111)
{
	const float onethird = 0.3333333333333333333f;
	const Vector3f P2_P1 = P2-P1;
	const Vector3f P3_P1 = P3-P1;
	const Vector3f P3_P2 = P3-P2;
	const float w12 = DotProd(P2_P1, N1);
	const float w13 = DotProd(P3_P1, N1);
	const float w21 = -DotProd(P2_P1, N2);
	const float w23 = DotProd(P3_P2, N2);
	const float w31 = -DotProd(P3_P1, N3);
	const float w32 = -DotProd(P3_P2, N3);
	b210 = (2*P1 + P2 - w12*N1)*onethird;
	b120 = (2*P2 + P1 - w21*N2)*onethird;
	b021 = (2*P2 + P3 - w23*N2)*onethird;
	b012 = (2*P3 + P2 - w32*N3)*onethird;
	b102 = (2*P3 + P1 - w31*N3)*onethird;
	b201 = (2*P1 + P3 - w13*N1)*onethird;
	
	const Vector3f E = (b210+b120+b021+b012+b102+b201)*0.166666666666666667f;
	const Vector3f V = (P1+P2+P3)*onethird;
	b111 = E+(E-V)*0.5f;
}


inline void computeNormalCoefficientsPN(
	const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, //input tri. vertex positions
	const Vector3f& N1, const Vector3f& N2, const Vector3f& N3, //input tri. vertex normals
	Vector3f& n110, Vector3f& n011, Vector3f& n101)
{
	const Vector3f &n200 = N1;
	const Vector3f &n020 = N2;
	const Vector3f &n002 = N3;
	const Vector3f P2_P1 = P2 - P1;
	const float v12 = 2*(DotProd(P2_P1, N1+N2)/DotProd(P2_P1, P2_P1));
	const Vector3f P3_P2 = P3 - P2;
	const float v23 = 2*(DotProd(P3_P2, N2+N3)/DotProd(P3_P2, P3_P2));
	const Vector3f P1_P3 = P1 - P3;
	const float v31 = 2*(DotProd(P1_P3, N1+N3)/DotProd(P1_P3, P1_P3));
	
	n110 = N1+N2-v12*(P2_P1); n110.normalize();
	n011 = N2+N3-v23*(P3_P2); n011.normalize();
	n101 = N3+N1-v31*(P1_P3); n101.normalize();
}

inline void bezierPatchCoefficients(
	const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, //input tri. vertex positions
	const Vector3f& N1, const Vector3f& N2, const Vector3f& N3, //input tri. vertex normals
	const int nvbase,
	Vector3f v[], Vector3f n[], int& nv, Vector3i tri[], int &ntri)
{
	//vertices
	v[0]=P1; v[1]=P2; v[2]=P3;
	Vector3f& b210 = v[3];
	Vector3f& b201 = v[4];
	Vector3f& b120 = v[5];
	Vector3f& b111 = v[6];
	Vector3f& b102 = v[7];
	Vector3f& b021 = v[8];
	Vector3f& b012 = v[9];
	nv = 10;
	ntri = 9;
	computePositionCoefficientsPN(P1, P2, P3, N1, N2, N3, b210, b120, b021, b012, b102, b201, b111);
	//normals
	n[0] = N1; n[1] = N2; n[2] = N3;
	Vector3f n110, n011, n101;
	computeNormalCoefficientsPN(P1, P2, P3, N1, N2, N3, n110, n011, n101);
	n[3]=(N1+2*n110); n[3].normalize();
	n[4]=(N1+2*n101); n[4].normalize();
	n[5]=(N2+2*n110); n[5].normalize();
	n[6]=(N1+N2+N3);  n[6].normalize();
	n[7]=(N3+2*n101); n[7].normalize();
	n[8]=(N2+2*n011); n[8].normalize();
	n[9]=(N3+2*n011); n[9].normalize();
	
	//triangles
	Vector3i vbase(nvbase, nvbase, nvbase);
	tri[0] = Vector3i(0, 3, 4) + vbase;
	tri[1] = Vector3i(3, 5, 6) + vbase;
	tri[2] = Vector3i(4, 3, 6) + vbase;
	tri[3] = Vector3i(4, 6, 7) + vbase;
	tri[4] = Vector3i(5, 1, 8) + vbase;
	tri[5] = Vector3i(5, 8, 6) + vbase;
	tri[6] = Vector3i(6, 8, 9) + vbase;
	tri[7] = Vector3i(7, 6, 9) + vbase;
	tri[8] = Vector3i(7, 9, 2) + vbase;
}


inline void bezierPatchCoefficients(
	const Vector3d& _P1, const Vector3d& _P2, const Vector3d& _P3, //input tri. vertex positions
	const Vector3f& N1, const Vector3f& N2, const Vector3f& N3, //input tri. vertex normals
	const int nvbase,
	Vector3f v[], Vector3f n[], int& nv, Vector3i tri[], int &ntri)
{
	Vector3f P1(_P1.x, _P1.y, _P1.z);
	Vector3f P2(_P2.x, _P2.y, _P2.z);
	Vector3f P3(_P3.x, _P3.y, _P3.z);
	bezierPatchCoefficients(P1, P2, P3, N1, N2, N3, nvbase, v, n, nv, tri, ntri);
}

#endif