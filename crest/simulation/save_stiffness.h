//FILE: save_stiffness.h

#ifndef _INCLUDE_SAVE_STIFFNESS_ZN2009_H_
#define _INCLUDE_SAVE_STIFFNESS_ZN2009_H_

#include <sysconf.h>
#include "zsparse_matrix.h"


//save stiffness for thin shell triangle pair structure
inline void SaveVertSitffMatrixIntoSparseMatrix(
	CSparseMatrix33 *p, const bool FASTSTIFF,
	const int vi, const int vj, const int vk, const double3x3 stiffmat[3])
{
	if (FASTSTIFF){
		//store order of the stiff matrix is (22 33 23)
		double s11, s12, s13, s21, s22, s23, s31, s32, s33;
		s22 = stiffmat[0].x[0];
		s23 = s32 = stiffmat[1].x[0];
		s33 = stiffmat[2].x[0];
		s12 = s21 = -(s22+s32);
		s13 = s31 = -(s23+s33);
		s11 = -(s12+s13);
		p->addWithMatrixElement(vi, vi, s11);
		p->addWithMatrixElement(vi, vj, s12);
		p->addWithMatrixElement(vi, vk, s13);
		p->addWithMatrixElement(vj, vi, s21);
		p->addWithMatrixElement(vj, vj, s22);
		p->addWithMatrixElement(vj, vk, s23);
		p->addWithMatrixElement(vk, vi, s31);
		p->addWithMatrixElement(vk, vj, s32);
		p->addWithMatrixElement(vk, vk, s33);
	}
	else{
		const double3x3& s22 = stiffmat[0];
		const double3x3& s23 = stiffmat[1];
		const double3x3& s33 = stiffmat[2];
		const double3x3 s12 = -(s22+s23);
		const double3x3 s13 = -(s23+s33);
		const double3x3 s21 = s12;
		const double3x3 s31 = s13;
		const double3x3 s32 = s23;
		const double3x3 s11 = -(s12+s13);
		p->addWithMatrixElement(vi, vi, s11);
		p->addWithMatrixElement(vi, vj, s12);
		p->addWithMatrixElement(vi, vk, s13);
		p->addWithMatrixElement(vj, vi, s21);
		p->addWithMatrixElement(vj, vj, s22);
		p->addWithMatrixElement(vj, vk, s23);
		p->addWithMatrixElement(vk, vi, s31);
		p->addWithMatrixElement(vk, vj, s32);
		p->addWithMatrixElement(vk, vk, s33);
	}
}


//save stiffness for thin shell triangle pair structure
inline void SaveVertSitffMatrixIntoSparseMatrix(
	CSparseMatrix33 *p, const int v1, const int v2, const int v3, const double stiffmat[3])
{
	//store order of the stiff matrix is (22 33 23)
	double s11, s12, s13, s21, s22, s23, s31, s32, s33;
	s22 = stiffmat[0];
	s23 = s32 = stiffmat[1];
	s33 = stiffmat[2];
	s12 = s21 = -(s22+s32);
	s13 = s31 = -(s23+s33);
	s11 = -(s12+s13);
	p->addWithMatrixElement(v1, v1, s11);
	p->addWithMatrixElement(v1, v2, s12);
	p->addWithMatrixElement(v1, v3, s13);
	p->addWithMatrixElement(v2, v1, s21);
	p->addWithMatrixElement(v2, v2, s22);
	p->addWithMatrixElement(v2, v3, s23);
	p->addWithMatrixElement(v3, v1, s31);
	p->addWithMatrixElement(v3, v2, s32);
	p->addWithMatrixElement(v3, v3, s33);
}


//save stiffness for zero length spring 
inline void SaveVertSitffMatrixIntoSparseMatrix(
	CSparseMatrix33 *p, const int vi, const int vj, const double& stiffness)
{
	p->addWithMatrixElement(vi, vi, stiffness);
	p->addWithMatrixElement(vj, vj, stiffness);
	p->addWithMatrixElement(vi, vj, -stiffness);
	p->addWithMatrixElement(vj, vi, -stiffness);
}


#endif