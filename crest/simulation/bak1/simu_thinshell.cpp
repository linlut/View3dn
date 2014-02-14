//FILE: simu_thinshell.cpp

#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_thinshell.h"
#include <ztime.h>

inline int _searchEdgeInPolygon(const int tri[], const int nv_per_elm, const int x, const int y)
{
	for (int i=0; i<nv_per_elm; i++){
		if ((x==tri[i]) && (y==tri[i+1]))
			return i;
	}
	return -1;
}

//input: quad.x quad.y have been known, we now only need to determine quad.z and quad.w 
//static 
void _decideTriangleRemainingNodes(const int *pelm, const int elm0, const int elm1, Vector8i& quad)
{
	const int nv_per_elm = 3;
	int buff0[8], buff1[8]; 
	const Vector3i *ptri = (const Vector3i*)pelm;
	const Vector3i tri0 = ptri[elm0];
	const Vector3i tri1 = ptri[elm1];
	Vector3i *p = (Vector3i*)(&buff0[0]); 
	p[0] = p[1] = tri0;
	Vector3i *q = (Vector3i*)(&buff1[0]); 
	q[0] = q[1] = tri1;

	//============search==============================
	int ipos = _searchEdgeInPolygon(buff0, nv_per_elm, quad.x, quad.y);
	if (ipos>=0){
		quad.z = buff0[ipos+2];
		ipos = _searchEdgeInPolygon(buff1, nv_per_elm, quad.y, quad.x);
		ASSERT0(ipos>=0);
		quad.w = buff1[ipos+2];
		return;
	}
	ipos = _searchEdgeInPolygon(buff1, nv_per_elm, quad.x, quad.y);
	if (ipos>=0){
		quad.z = buff1[ipos+2];
		ipos = _searchEdgeInPolygon(buff0, nv_per_elm, quad.y, quad.x);
		ASSERT0(ipos>=0);
		quad.w = buff0[ipos+2];
		return;
	}

	assert(0);
}

//input: quad.x quad.y have been known, we now only need to determine quad.z and quad.w 
//static 
void _decideQuadRemainingNodes(const int *pelm, const int elm0, const int elm1, Vector8i& quad)
{
	const int nv_per_elm = 4;
	int buff0[8], buff1[8]; 
	const Vector4i *ptri = (const Vector4i*)pelm;
	const Vector4i tri0 = ptri[elm0];
	const Vector4i tri1 = ptri[elm1];
	Vector4i *p = (Vector4i*)(&buff0[0]); 
	p[0] = p[1] = tri0;
	Vector4i *q = (Vector4i*)(&buff1[0]); 
	q[0] = q[1] = tri1;

	//============search==============================
	int ipos = _searchEdgeInPolygon(buff0, nv_per_elm, quad.x, quad.y);
	if (ipos>=0){
		quad.z = buff0[ipos+2];
		quad.w = buff0[ipos+3];
		ipos = _searchEdgeInPolygon(buff1, nv_per_elm, quad.y, quad.x);
		ASSERT0(ipos>=0);
		quad.x1 = buff1[ipos+2];
		quad.y1 = buff1[ipos+3];
		return;
	}
	ipos = _searchEdgeInPolygon(buff1, nv_per_elm, quad.x, quad.y);
	if (ipos>=0){
		quad.z = buff1[ipos+2];
		quad.w = buff1[ipos+3];
		ipos = _searchEdgeInPolygon(buff0, nv_per_elm, quad.y, quad.x);
		ASSERT0(ipos>=0);
		quad.x1 = buff0[ipos+2];
		quad.y1 = buff0[ipos+3];
		return;
	}

	assert(0);
}

/*
static int test_func(void)
{
	CMeMaterialProperty mtl;
	const int nv=6;
	Vector3d pVertex[nv]={
		Vector3d(0, 0, 0),
		Vector3d(1, 0, 0),
		Vector3d(0, 1, 0),
		Vector3d(1, 1, 0),
		Vector3d(0.5, 1.5, 0),
		Vector3d(0.5, -0.5, 0)
	};
		
	Vector3d init_velocity(0,0,1);
	const int nelm=4;
	Vector3i element[nelm]={
		Vector3i(0, 1, 3), 
		Vector3i(0, 3, 2),
		Vector3i(2, 3, 4),
		Vector3i(0, 5, 1)
	};
	const int nv_per_elm=3;
	const double shell_thickness=0.001;
	CSimuThinshell s(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, 2,  //element information
		shell_thickness,
		mtl							//material property
		);

	const int N=100000;
	const int timerid=0;

	startFastTimer(timerid);
	for (int i=0; i<N; i++){
		//e.computeNodalForce(p0, p1, mat, Force, NULL);
		s.computeElasticForces(false);
	}
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "TEMPLATE elm run time is ");
	printf("Run simu_thinshell, Run number is %d, time is %lg ms\n", N, t);

	
	//set constraints
	CVertexConstraints constraints;
	constraints.setFixedPositionXYZ();
	s.setConstraints(0, constraints);
	s.setConstraints(3, constraints);
	s.setGravityAccelertion(Vector3d(0,0,-20));

	//velocity verlet integration method
	const double dt=0.004;		//1MS
	for (int i=0; i<500; i++){
		s.dumpVertexPosition(2, i, stdout);
		//s.velocityVerletIntegration(dt);
	}
	
	return 1;
}
*/


static int test_func(void)
{
	CMeMaterialProperty mtl;
	const int nv=6;
    Vector3d p0(0, 0, 0);
    Vector3d p1(0,1,0);
    Vector3d p2(-1, 1, 0);
    Vector3d p3(-1, 0, 0);
    Vector3d p4(1, 0, 0);
    Vector3d p5(1, 1, 0);
	Vector3d pVertex[nv]={
		p0, p1, p2, p3, p4, p5
	};
		
	Vector3d init_velocity(0,0,1);
	const int nelm=2;
	Vector4i element[nelm]={
		Vector4i(0, 1, 2, 3), 
		Vector4i(0, 4, 5, 1)
	};
	const int nv_per_elm=4;
	const double shell_thickness=0.001;
	CSimuThinshell s(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, nelm,//element information
		shell_thickness,
		mtl							    //material property
		);

	const int N=100000;
	const int timerid=0;

	startFastTimer(timerid);
	for (int i=0; i<N; i++){
		//e.computeNodalForce(p0, p1, mat, Force, NULL);
		s.computeElasticForces(false);
	}
	stopFastTimer(timerid);
	const double t = getTimeDifference(timerid);
	printf("Run simu_thinshell, Run number is %d, time is %lg ms\n", N, t);
	return 1;
}



static int test = test_func();
