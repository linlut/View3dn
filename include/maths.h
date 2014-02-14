/**************************************************************
  maths lib head file 
  for vector, matrix , and some other functions;
  Zhang Nan
  Nov 10th,1994
***************************************************************/
//#ifdef __cplusplus
//extern "C"{
//#endif


#ifndef __INC_ZMATHS321_H__
#define __INC_ZMATHS321_H__

#include <math.h>
#include <memory.h>
#include "typedef.h"



int SamePoint(REAL p1[3],REAL p2[3]);


inline int VecNormalize(REAL n[3])
{
	double a=n[0], b=n[1], c=n[2];
	const double TOLRR = 1e-40;
	double t = a*a+b*b+c*c;
	if (t<TOLRR){
		n[0]=n[1]=0.0; n[2]=1.0;
		return 0;
    }
	t=sqrt(t), t=1/t;
	n[0]=(REAL)(a*t), n[1]=(REAL)(b*t),	n[2]=(REAL)(c*t);
	return 1;
}


/****Copy vector b to vector a ****/
inline void VecAssign(double a[3], const double b[3]) 
{
	a[0]=b[0], a[1]=b[1], a[2]=b[2];
}

/****Add two vectors a and b to vector c ****/
inline void VecAdd(const double a[3], const double b[3], double c[3]) 
{
	c[0]=a[0] + b[0],    
	c[1]=a[1] + b[1],    
	c[2]=a[2] + b[2];
}

inline void VecMinus(const double a[3], const double b[3], double c[3]) 
{
	c[0]=a[0] - b[0],    
	c[1]=a[1] - b[1],    
	c[2]=a[2] - b[2];
}

inline void vxvProduct(const REAL m1[3], const REAL m2[3],REAL res[3])
{
	res[0]=m1[1]*m2[2]-m1[2]*m2[1];
	res[1]=m1[2]*m2[0]-m1[0]*m2[2];
	res[2]=m1[0]*m2[1]-m1[1]*m2[0];
}


inline void vvProduct(const REAL v1[], const REAL v2[], REAL &r) 
{
	r= v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}


inline void TransposMatrix(const REAL m1[4][4], REAL m2[4][4])
{
	int i, j; 
	for (i=0; i<4; i++){
		for (j=0; j<4; j++){
			m2[i][j]=m1[j][i];
		}
	}
}


inline void vmCopy(const double v[16], double m[4][4])
{
	memcpy(m, v, sizeof(double)*16);
}


inline void mvCopy(const double m[4][4], double v[16])
{
	memcpy(v, m, sizeof(double)*16);
}


inline void pmMult(const double p[3], const double m[4][4], double w[3])
{
	double t;
	w[0]=p[0]*m[0][0]+p[1]*m[1][0]+p[2]*m[2][0]+m[3][0];
	w[1]=p[0]*m[0][1]+p[1]*m[1][1]+p[2]*m[2][1]+m[3][1];
	w[2]=p[0]*m[0][2]+p[1]*m[1][2]+p[2]*m[2][2]+m[3][2];
	t= p[0]*m[0][3]+p[1]*m[1][3]+p[2]*m[2][3]+m[3][3];
	if (t!=1.0){
		t=1.0/t;
		w[0]*=t, w[1]*=t, w[2]*=t;
	}
}

inline void mTranspose(const double m1[4][4], double m2[4][4])
{
	int i, j;
	double r[4][4];
	for (i=0; i<4; i++){
		for (j=0; j<4; j++){
			r[i][j]=m1[j][i];
		}
	}
	for (i=0; i<4; i++){
		for (j=0; j<4; j++){
			m2[i][j]=r[i][j];
		}
	}
}


inline void unitMatrix(double m[4][4])
{ 
	for (int i=0; i<4; i++){
		for(int j=0;j<4;j++){
			m[i][j]=(i==j);
		}
	}
}



REAL square_distance(REAL p1[],REAL p2[3]);
int  InMid(REAL up,REAL down,REAL mid);
REAL Distance(REAL p1[3],REAL p2[3]);

/********About normal vector  **************/
void AverageNormal(REAL norm[][3],int n, REAL anorm[3]);
void GetTriPlaneNormal(REAL p1[3],REAL p2[3],REAL p3[3],REAL normal[3]);

/********About matrix operations************/
void mmMult(REAL m1[4][4],REAL m2[4][4],REAL m[4][4]);

inline void matrixAssign(REAL m1[4][4], const REAL m2[4][4])
{
	REAL *p1 = (REAL*)m1;;
	const REAL *p2 = (const REAL *)m2;
	for (int i=0; i<16; i++)
		p1[i]=p2[i];
}


inline void vmMult(const REAL w[3], REAL m[4][4], REAL r[3])
{
	for (int i=0; i<3; i++){
		r[0]=w[0]*m[0][0]+w[1]*m[1][0]+w[2]*m[2][0];
		r[1]=w[0]*m[0][1]+w[1]*m[1][1]+w[2]*m[2][1];
		r[2]=w[0]*m[0][2]+w[1]*m[1][2]+w[2]*m[2][2];
	}
}



void ScaleMatrix(REAL,REAL,REAL,REAL m[4][4]);
void RotateY(REAL deg, REAL m[4][4]);
void RotateX(REAL deg, REAL m[4][4]);
void RotateZ(REAL deg, REAL m[4][4]);
void MatrixMove(REAL p[3],REAL m[4][4]);
int  invertmat(REAL from[4][4],REAL to[4][4]);


/********About view transform *************/
void perspect_trans(REAL,REAL Tp[4][4],REAL Tpr[4][4]);

void world_view_trans(REAL view[3], REAL look_at[3], REAL T[4][4]);

void screen_trans(REAL,REAL,REAL Mps[4][4],REAL Msp[4][4]);

void local_world_trans(REAL*,REAL*,REAL Tlw[4][4],REAL Twl[4][4]);

void GetPlaneNormal(REAL vertex[4][3],REAL parm[3]);

void GetPlaneParms(REAL vertex[4][3],REAL parm[4]);

int collinear_points(REAL MINDIS, REAL p1[3], REAL p2[3], REAL p3[3]);


//1995/02/16;
void viewpoint_persp_trans(
		     REAL view[3],
		     REAL obj[3],
		     short screen_width,  //picture screen resolution
		     short screen_height,
		     short elm_width,     //elm file resolution
		     short elm_height,
		     REAL hh,            //perspective distance
		     REAL vpd8,
		     REAL vpd9,
		     double Twe[4][4],    //transform matrix,
		     double TMws[4][4]);   

int line_plane_intersection(REAL p1[3], REAL dir[3], REAL p2[3], REAL norm[3], REAL *t);
REAL point_line_distance(REAL point[3], REAL linepoint[3], REAL linedir[3]);
REAL point_line_distance2(REAL point[3], REAL linepoint[3], REAL linedir[3]);


//#ifdef __cplusplus
//}
//#endif

#endif
