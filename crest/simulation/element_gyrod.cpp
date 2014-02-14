//FILE: element_gyrod.cpp
#include <math.h>
#include <vector>
#include <ztime.h>
#include "element_gyrod.h"

const int LUT_BETAANGLE_SIZE = 90;
CGyrodLookupTable* CGyrodElement::m_pLookupTable = new CGyrodLookupTable(LUT_BETAANGLE_SIZE);

void CGyrodElement::_init(const Vector3d& p, const double3x3& mat, const double &vol)
{
	Vector3d p0 = mat * p;   //rotate the point
	const double ll = Magnitude(p0);
	Vector3d n0 = p0 /ll;    //Normalized vector
	m_len0 = ll;
	
	//compute a second rotation to rotate p0 as the Z-axis
	Vector3d& X = *((Vector3d*)(&m_R.x[0]));
	Vector3d& Y = *((Vector3d*)(&m_R.x[3]));
	Vector3d& Z = *((Vector3d*)(&m_R.x[6]));
	Z = n0;
	double maxval = fabs(n0.x);
	int ix=1;
	if (maxval<fabs(n0.y)) maxval=fabs(n0.y), ix=2;
	if (maxval<fabs(n0.z)) ix=0;
	X = Vector3d(0,0,0); X[ix]=1;
	Y = CrossProd(Z, X); Y.normalize();
	X = CrossProd(Y, Z); X.normalize();

	//init the shear elm
	const Vector3d q0 = Vector3d(0,0,1)*m_len0;
	const Vector3d q1(0,0,0);
	static CMeMaterialProperty mtl;
	m_shearelm.init(mtl, q0, q1, mat, vol);
}

void CGyrodElement::initLookupTable(void)
{
	SafeDelete(m_pLookupTable);
	m_pLookupTable = new CGyrodLookupTable(LUT_BETAANGLE_SIZE);
	assert(m_pLookupTable!=NULL);
}

//=============================================================
static int test_func(void)
{
	DGCubeMap cube;
	cube.testFunc();
	CGyrodElement::initLookupTable();

	CMeMaterialProperty mat;
	mat.setPoisson(0.30);
	mat.setDamping(0.0, 0.0);
	//--------------------------------
	const double vol=0.0001;
	Vector3d p0(0,0,1);
	const Vector3d p1(0,0,0);
	CGyrodElement e(p0, p1, vol);
	//CShearElement e(p0, p1, vol);
	//--------------------------------
	const int N=360;
	const double K = 360.0/N;
	const double A1 = (PI*2.0)*K/360.0;
	double3x3 stiffness(0.0);

	printf("Angle\t\tForce\t\t\tNorm\tEng\n");
	double z=1, x;
	for (int i=0; i<=N; i++){
		if (i>=220) break;
		if (i>=135){
			int yyy=1;
		}
		double angle = +i*A1;
		double R=1;
		Vector3d Force;
		z = R*cos(angle);
		x = R*sin(angle);
		//x = 1e-2; 
		p0=Vector3d(x, 0, z);
		//z = z-0.01;
		if (z<0.001) z=0.001;
		e.computeNodalForce(p0, p1, mat, Force, &stiffness);
		const double f2 = Magnitude(Force);
		const double engr = 0; //e.computeElementEnergy(mat);
		printf("%lg, %lg, %lg, %lg, %lg, %lg, %lg\n", i*K, Force.x, Force.y, Force.z, f2, engr, stiffness.x[0]);
		if (i*K>0){
			//Tensor3x3 F = e.getDeformationGradient();
			//printf("%lg, %lg, %lg\n", F.x[0], F.x[1], F.x[2]);
			//printf("%lg, %lg, %lg\n", F.x[3], F.x[4], F.x[5]);
			//printf("%lg, %lg, %lg\n", F.x[6], F.x[7], F.x[8]);
		}
	}
	exit(0);
	return 1;
}

//static int test = test_func();

/*
int testsph(void)
{
	DGCubeMap cube;
	cube.testFunc();
	CGyrodElement::initLookupTable();

	SphericalQuadTree *p = SphericalQuadTree::getInstance();
	//p->exportMesh("./unitsphere.txt");

	Vector3d dir0(0, 0, 1); dir0.Normalize();
	p->findLookupTableElement(dir0);
	Vector3d dir1(-1, -1, -1); 
	p->findLookupTableElement(dir1);
	int i;
	const int timerid = 0;
	const int N=1000000;
	startFastTimer(timerid);
	for (i=0; i<N; i++){
		p->findLookupTableElement(dir1);
	}
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Searching HTM run time:");

	CLookupTableElement *pelm;
	CGyrodLookupTable* ptab = CGyrodElement::getLookupTable();
	startFastTimer(timerid);
	Vector3d dirx(1,1,1);
	const double ZERO=0;
	for (i=0; i<N; i++){
		pelm = (CLookupTableElement *)ptab->findLookupTableElement(dirx);
		pelm->m_dir.x+=ZERO;
	}
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Searching ATAN run time:");

	dirx = Vector3d(1,1,1);
	startFastTimer(timerid);
	for (i=0; i<N; i++){
		pelm = (CLookupTableElement *)ptab->findLookupTableElementCubeMap(dirx);
		pelm->m_dir.x+=ZERO;
	}
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Searching ATAN CUBEMAP run time:");
	exit(0);
	return 1;
}
*/
//static int xxx = testsph();
