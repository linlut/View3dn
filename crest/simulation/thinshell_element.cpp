//FILE: thinsehll_element.cpp
#include <geomath.h>
#include <ztime.h>
#include "thinshell_element.h"
#include "simu_entity.h"


static void _printAllVertices(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5)
{
	Vector3d v;
	v = p0; printf("0: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p1; printf("1: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p2; printf("2: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p3; printf("3: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p4; printf("4: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p5; printf("5: %10lg %10lg %10lg\n", v.x, v.y, v.z);
}

//===========================================================================
static int test_func(void)
{
    CMeMaterialProperty mtl;
    const int timerid = 0;
    const int N=100000;
    CThinshellElement shell;
	const int NJ=4;
    Vector3d p0(0, -1, 0);
    Vector3d p1(0, 1, 0);
    Vector3d p2(0,0,1);
    Vector3d p3(1, 0, 0);
	Vector3d vbuffer[4]={p0, p1, p2, p3};
    Vector3d F[4];
    double3x3 jac[NJ], *ppJacobian[NJ];
    double t=0.01;
    int quad[4]={0,1,2,3};
    shell.init(mtl, 0, quad, p0, p1, p2, p3, t);
	for (int i=0; i<NJ; i++) jac[i].setZeroMatrix(), ppJacobian[i]=&jac[i];

    startFastTimer(timerid);
    for (int i=0; i<N; i++){
		F[0]=F[1]=F[2]=F[3]=Vector3d(0);
        //const double k = 1.0*i/N;
        //double a = 180.0*k;
        //double t = PI*k;
        //double z = 1;
        //double x = 0;
        //p2 = Vector3d(x, 0, z);
		const int *vid = shell.getElementNodeBuffer();
		const int i0 = vid[0];
		const int i1 = vid[1];
		const int i2 = vid[2];
		const int i3 = vid[3];
		Vector3d &x0 = vbuffer[i0];
		Vector3d &x1 = vbuffer[i1];
		Vector3d &x2 = vbuffer[i2];
		Vector3d &x3 = vbuffer[i3];
        shell.computeForce(x0, x1, x2, x3, 0, mtl, F, ppJacobian);
        //printf("Angel %3lg, Force is %8lg %8lg %8lg\n", a, F[j].x, F[j].y, F[j].z);
    }
    stopFastTimer(timerid);
    reportTimeDifference(timerid, "Shell template elm run time:");

    return 1;
}

/*
void test_thinshellelm(void * pshell, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d&p2, const Vector3d&p3, const Vector3d&p4, const Vector3d&p5
	)
{
	CMeMaterialProperty mtl;
	Vector3d pp0 = p0;
	Vector3d pp1 = p1;
	Vector3d pp2 = p2;
	Vector3d pp3 = p3;
	Vector3d pp4 = p4;
	Vector3d pp5 = p5;
	Vector3d ZERO(1e-30);
	CThinshellElement *tshell = (CThinshellElement *)pshell;
	//CThinshellElement shell = *tshell;
    double3x3 *ppJacobian[6]={NULL, NULL, NULL, NULL, NULL, NULL};
    Vector3d F[6];
    const int timerid = 0;
    const int N=1000000;
	const Vector3d *v=&p0;
	//printf("p0 %lg %lg %lg, ", v->x, v->y, v->z);
	v=&p1;
	//printf("p1 %lg %lg %lg, ", v->x, v->y, v->z);
	v=&p2;
	//printf("p2 %lg %lg %lg\n", v->x, v->y, v->z);
	v=&p3;
	//printf("p3 %lg %lg %lg, ", v->x, v->y, v->z);
	v=&p4;
	//printf("p4 %lg %lg %lg, ", v->x, v->y, v->z);
	v=&p5;
	//printf("p5 %lg %lg %lg\n", v->x, v->y, v->z);
    startFastTimer(timerid);
    for (int i=0; i<N; i++){
		pp0+=ZERO;
		//tshell->computeForceBridson(pp0, pp1, pp2, pp3, 0, mtl, F, ppJacobian);
		tshell->computeForce(pp0, pp1, pp2, pp3, pp4, pp5, 0, mtl, F, ppJacobian);
    }
    stopFastTimer(timerid);
    const double t = getTimeDifference(timerid);
	printf("Shell Template Element run time:%lg, Looping %d\n", t, N);
}

void computeForceNoTemp(void *shell, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d&p2, const Vector3d&p3, const Vector3d&p4, const Vector3d&p5, 
		const int isstatic,
		CMeMaterialProperty&mtl, 
		Vector3d *force, 
		double3x3 *ppjac[6])
{
	CThinshellElement *tshell = (CThinshellElement *)shell;
	tshell->computeForce(p0, p1, p2, p3, p4, p5, isstatic, mtl, force, ppjac);
}

/*
int test_shellelmfunc(void)
{
	int i;
	const int NSHELL=300;
    CThinshellElement shell[NSHELL];
	char buff[20481];
	CSimuEntity::VertexInfo p[36];
    CMeMaterialProperty mtl;
    Vector3d p0(0, 0, 0);
    Vector3d p1(0,1,0);
    Vector3d p2(-1, 1, 0);
    Vector3d p3(-1, 0, 0);
    Vector3d p4(1, 0, 0);
    Vector3d p5(1, 1, 0);
	p[0].m_pos = p0;
	p[1].m_pos = p1;
	p[2].m_pos = p2;
	p[3].m_pos = p3;
	p[4].m_pos = p4;
	p[5].m_pos = p5;
    double t=0.01;
    int quad[6]={0,1,2,3, 4, 5};
	for (i=0; i<NSHELL; i++)
		shell[i].init(mtl, 0, quad, p[0].m_pos, p[1].m_pos, p[2].m_pos, p[3].m_pos, p[4].m_pos, p[5].m_pos, t);
	test_thinshellelm(&shell[0], p[0].m_pos, p[1].m_pos, p[2].m_pos, p[3].m_pos, p[4].m_pos, p[5].m_pos);
    return 1;
}
*/

static int ntest = test_func();

