
#include <vector.h>
#include <stdio.h>
#include <zntoolbox.h>
#include "etkRegistration.hpp"
#include <QGLViewer/quaternion.h>


using namespace qglviewer;



// ----------------------------------------------------------------------------
/*
#define NB_PTS 4
static double weights[NB_PTS]={0.3, 0.4, 0.4, 0.5};
static double adModelPoints  [NB_PTS][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.5, 1, 0}};
static double adSensorPoints  [NB_PTS][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.5, 1, 0}};
//double adSensorPoints [NB_PTS][3] = {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0.5, 1.0}};
*/
#define NB_PTS 5
static double adModelPoints  [NB_PTS][3] = {{0, 0, 0}, {64, 0, 0}, {0, 64, 0}, {64, 64, 0}, {32, 32, 64}};
static double adSensorPoints [NB_PTS][3] = {{0, 0, 0}, {64, 0, 0}, {0, 64, 0}, {64, 64, 0}, {32, 32, 64}};
static double weights[NB_PTS]={1, 1, 1, 1, 100};



//---------------------------------------------------------------------------
//main func for test
/*
int mainxx(int argc, char* argv[])
{
	printf ("Horn Registration Test\n\n");

	Vector3d *p = (Vector3d*)&adModelPoints[0][0];
	Vector3d *q = (Vector3d*)&adSensorPoints[0][0];
	double adRot[9];
	Vector3d adTrans;
	double err;
	float s;

	double t=GetClockInSecond();
	const int NN=1;
	for (int i=0; i<NN; i++){
		etkRegistration reg;
		etkQuaternion quat;
		err = reg.hornRegistration(p, q, weights, NB_PTS, &quat, adRot, adTrans,s, true);
	}
	t = GetClockInSecond()-t;
	printf("N is %d, Time is %lg, avg time per op is %lg.\n", NN, t, t/NN);
	printf("Trans vector is <%lg, %lg, %lg>\n", adTrans.x, adTrans.y, adTrans.z);
	printf("Error is %lg\n", err);
	printf("Rot matrix is:\n");
	printf("\t %lg %lg %lg\n", adRot[0], adRot[1], adRot[2]);
	printf("\t %lg %lg %lg\n", adRot[3], adRot[4], adRot[5]);
	printf("\t %lg %lg %lg\n", adRot[6], adRot[7], adRot[8]);
	return 0;
}

*/

void deformOneVertex(const Vector3f& pos, Vector3f& deformedpos, Vector3f& translation, Vector4f &quatf4, Vector3f pdv[3], float & detf);

int mainyy(int argc, char**argv)
{
/*
	Vector3f pos(63,63,0), deformedpos, trans, pde[3];
	Vector4f quat;
	float sc=1, detf=1;

	//deformOneVertex(pos, deformedpos, trans, quat, pde, detf);
	
	Quaternion q(quat.x, quat.y, quat.z, quat.w);
	q.normalize();

	Vec v(deformedpos.x, deformedpos.y, deformedpos.z);
	Vec t(trans.x, trans.y, trans.z);
	Vec r = q.inverse()*(v -t);
//	printf("Inverse transform Diff (%f %f %f)\n", r.x-pos.x, r.y-pos.y, r.z-pos.z);
//	printf("Inverse transform Diff (%f %f %f)\n", r.x-pos.x, r.y-pos.y, r.z-pos.z);

	Vec s=(q*Vec(pos.x,pos.y,pos.z))*sc+t;
	printf("Position is  (%f %f %f)\n", pos.x, pos.y, pos.z);
	printf("Transformed  (%f %f %f)\n", s.x, s.y, s.z);
*/
	return 1;
}