//FILE: trusslink.cpp
#include <qmessagebox.h>

#include <vector>
#include <set>
#include <edgetable.h>
#include <hexobj.h>
#include <tetraobj.h>
#include <vectorall.h>
#include "trusslink.h"

using namespace std;

#define EPSILON 1e-6

#define CROSS(dest,v1,v2)                  \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUBSTRACT(dest,v1,v2)  \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

static inline int intersect_triangle(const double orig[3], const double dir[3],
                   double vert0[3], double vert1[3], double vert2[3],
                   double *t, double *u, double *v)
{
   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUBSTRACT(edge1, vert1, vert0);
   SUBSTRACT(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUBSTRACT(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0 / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
#else                    /* the non-culling branch */
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0 / det;

   /* calculate distance from vert0 to ray origin */
   SUBSTRACT(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec) * inv_det;
   if (*u < 0.0 || *u > 1.0)
     return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec) * inv_det;
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   /* calculate t, ray intersects triangle */
   *t = DOT(edge2, qvec) * inv_det;
#endif
   return 1;
}


static inline bool pointInTet(CTetraObj *pobj, const Vector3d &p, const Vector3d& dir)
{
	Vector3i *ptri = (Vector3i*)pobj->m_pPolygon;

	//count number of intersection points
	int c = 0;
	double t, u, v;
	for (int i=0; i<pobj->m_nPolygonCount; i++){
		Vector3i tri = ptri[i];
		Vector3d vert0 = pobj->m_pVertex[tri.x];
		Vector3d vert1 = pobj->m_pVertex[tri.y];
		Vector3d vert2 = pobj->m_pVertex[tri.z];
		int f = intersect_triangle(&p.x, &dir.x, &vert0.x, &vert1.x, &vert2.x, &t, &u, &v);
		if (f && (t<0)) 
			f=0;
		c+=f;
	}
	return (c&0x1);
}


static inline bool pointInTet(CTetraObj *pobj, const Vector3d &v)
{
	Vector3d x(1,0,0);
	Vector3d y(0,1,0);
	Vector3d z(0,0,1);
	Vector3d p = v + Vector3d(1.073453463e-6, 1.926332e-6, 1.09882362e-6);

	int c1 = pointInTet(pobj, p, x);
	int c2 = pointInTet(pobj, p, y);
	int c3 = pointInTet(pobj, p, z);
	int c=c1+c2+c3;
	if (c>=2) return true;
	return false;
}



//#ifndef INT64
//#define INT64  _int64
//#endif

static void
_genTruss(const int x, const int y, vector<CTruss> &truss, set<INT64>& truss_set)
{
	CTruss t;
	if (x==y) return;
	const int xmin = _MIN_(x, y);
	const int xmax = _MAX_(x, y);
	t.ix = xmin, 
	t.iy = xmax;
	INT64 key= xmin + ((INT64)xmax << 32);
	set<INT64>::iterator it = truss_set.find(key);
	if (it==truss_set.end()){
		truss.push_back(t);
		truss_set.insert(key);
	}
}


static void 
_computeNodeDegree(const vector<CTruss>& truss, const int nv, vector<unsigned short>& nodedgree)
{
	int i;
	nodedgree.clear();
	nodedgree.resize(nv);
	for (i=0; i<nv; i++) nodedgree[i]=0;
	for (i=0; i<truss.size(); i++){
		const CTruss *ptruss = &truss[i];
		const int x = ptruss->ix;
		const int y = ptruss->iy;
		ASSERT0(x>=0 && x<nv);
		ASSERT0(y>=0 && y<nv);
		nodedgree[x]++;
		nodedgree[y]++;
	}
}


static void 
_computeTrussEdgeInfoCase01(CPolyObj *pobj, vector<CTruss>& truss)
{
	int i;

	//Compute node degree 
	const int nv = pobj->m_nVertexCount;
	vector<unsigned short> nodedgree;
	_computeNodeDegree(truss, nv, nodedgree);

	//Then, compute per vertex volume (==>mass)
	double *pVertexVol = pobj->computeVertexVolumes();
	assert(pVertexVol!=NULL);
	const Vector3d *pVertex = pobj->m_pVertex;

	//compute total volume
	double totalvol=0;
	double massprojX=0, massprojY=0, massprojZ=0;
	for (i=0; i<pobj->m_nVertexCount; i++){
		totalvol += pVertexVol[i];
	}

	//compute total truss edge length
	double tlength = 0;
	for (i=0; i<truss.size(); i++){
		CTruss *ptruss = &truss[i];
		tlength+= Distance(pVertex[ptruss->ix], pVertex[ptruss->iy]);
	}
	//output average truss edge crosssection area
	double crossarea = totalvol/tlength;
	printf("Truss edge total length %lg, avg cross area %lg m2\n", tlength, crossarea);

	for (i=0; i<truss.size(); i++){
		CTruss *ptruss = &truss[i];
		const int x = ptruss->ix;
		const int y = ptruss->iy;
		const int degx = nodedgree[x];
		const int degy = nodedgree[y];
		//compute truss edge's mass
		const double volx = pVertexVol[x]/degx;
		const double voly = pVertexVol[y]/degy;
		ptruss->mass1 = volx;
		ptruss->mass2 = voly;
		ptruss->tvol = volx + voly;
		//compute truss edge's direction vec and length
		Vector3d difvec = pVertex[x]- pVertex[y];
		const double veclen = sqrt(DotProd(difvec, difvec));
		ptruss->tlen = veclen;
		ptruss->len0 = veclen;
		ptruss->len0_inv = 1.0/veclen;
		//compute projected mass on X,Y,Z axes;
		difvec *= (1.0/veclen); //Normalize the difvec;
		difvec.x *= difvec.x; 
		difvec.y *= difvec.y; 
		difvec.z *= difvec.z;
		difvec *= ptruss->tvol;
		massprojX += fabs(difvec.x);
		massprojY += fabs(difvec.y);
		massprojZ += fabs(difvec.z);
	}

	SafeDeleteArray(pVertexVol);

	//output info
	printf("total object volume is %lg.\n", totalvol);
	printf("Projected volumes are %lg, %lg, %lg.\n", massprojX, massprojY, massprojZ);
	printf("Projected volume scaling factors are %lg, %lg, %lg.\n", totalvol/massprojX, totalvol/massprojY, totalvol/massprojZ);

}


static void
_computeTrussEdgeInfo(CPolyObj *pobj, const int nSpringType, vector<CTruss>& truss)
{
	switch (nSpringType){
	case 0:	//adaptive, non-adaptive
	case 1:
		_computeTrussEdgeInfoCase01(pobj, truss);
		break;
	case 2: //G's method       ---could only be used on tet mesh
		{


		}
		break;
	case 3: //TVCG 2007 method ---could only be used on tet mesh
		{

		}
		break;
	default:
		break;
	};

}


static void 
checkAngle(const Vector3d& v0, const Vector3d* pVertex, set<int>& nbset, set<int>&nbset2, const double ERRTOL, int *ring3, int& n3)
{
	Vector3d norm[100];
	int i, j;

	set<int>::iterator it;
	const int ns = nbset.size();
	ASSERT0(ns <= 100);
	n3 = 0;

	for (it=nbset.begin(), i=0; it!=nbset.end(); it++, i++){
		const int k = *it;
		norm[i] = pVertex[k] - v0;
		norm[i].normalize();
	}

	for (it=nbset2.begin(); it!=nbset2.end(); it++){
		const int k = *it;
		Vector3d dist = pVertex[k] - v0;
		dist.normalize();
		bool skip =false;
		for (j=0; j<ns; j++){
			const double dotval = DotProd(dist, norm[j]);
			if (dotval > 1-ERRTOL){ //The two vectors are two close to each other in direction
				skip = true;
				break;
			}
		}
		if (!skip)
			ring3[n3++] = k;
	}
}


static float *getVertexSupportSizeSquare(Vector3d *pVertex, const int nv, Vector4i* ptet, const int ntet)
{
	int i;
	const float SIZETOL = 1.25;

	float * pSize = new float [nv];
	assert(pSize!=NULL);
	for (i=0; i<nv; i++) pSize[i]=0;
	for (i=0; i<ntet; i++){
		Vector4i tet = ptet[i];
		const float s0 = Distance2(pVertex[tet.x], pVertex[tet.y]);
		const float s1 = Distance2(pVertex[tet.x], pVertex[tet.z]);
		const float s2 = Distance2(pVertex[tet.x], pVertex[tet.w]);
		const float s3 = Distance2(pVertex[tet.y], pVertex[tet.z]);
		const float s4 = Distance2(pVertex[tet.y], pVertex[tet.w]);
		const float s5 = Distance2(pVertex[tet.z], pVertex[tet.w]);
		float s = _MAX_(s0, s1);
		s = _MAX_(s, s2);
		s = _MAX_(s, s3);
		s = _MAX_(s, s4);
		s = _MAX_(s, s5);
		pSize[tet.x] = _MAX_(pSize[tet.x], s);
		pSize[tet.y] = _MAX_(pSize[tet.y], s);
		pSize[tet.z] = _MAX_(pSize[tet.z], s);
		pSize[tet.w] = _MAX_(pSize[tet.w], s);
	}

	for (i=0; i<nv; i++)
		pSize[i]*=(SIZETOL*SIZETOL);
	return pSize;
}


static inline void 
get1RingVertices(Vector3d *pVertex, EdgeTable *et, const int i, set<int> &nbset)
{
	int neighbours[100];
	nbset.clear();

	const int n1 = et->getNeighbourVerices(i, neighbours);
	for (int k=0; k<n1; k++){
		const int vv = neighbours[k];
		nbset.insert(vv);
	}
}


static inline void 
get2RingVertices(const Vector3d *pVertex, EdgeTable *et, const int rootvert, const float supportSize2, set<int> &nbset, set<int> &nbset2)
{
	int neighbours[100];
	nbset2.clear();
	for (set<int>::iterator itr=nbset.begin(); itr!=nbset.end(); itr++){
		const int vv = *itr;
		const int n2 = et->getNeighbourVerices(vv, neighbours);
		for (int j=0; j<n2; j++){
			const int x = neighbours[j];
			if (x!=rootvert){
				if (nbset.find(x)==nbset.end() && nbset2.find(x)==nbset2.end()){
					float r2 = Distance2(pVertex[rootvert], pVertex[x]);
					if (r2<supportSize2)
						nbset2.insert(x);
				}
			}
		}
	}
}


static inline void 
get3RingVertices(const Vector3d *pVertex, EdgeTable *et, const int rootvert, const float supportSize2, set<int> &nbset, set<int> &nbset2)
{
	int neighbours[100], tmpv[100], cc=0;
	for (set<int>::iterator itr=nbset2.begin(); itr!=nbset2.end(); itr++){
		tmpv[cc]=*itr;
		cc++;
	}

	for(int i=0; i<cc; i++){
		const int vv = tmpv[i];
		const int n2 = et->getNeighbourVerices(vv, neighbours);
		for (int j=0; j<n2; j++){
			const int x = neighbours[j];
			if (x!=rootvert){
				if (nbset.find(x)==nbset.end() && nbset2.find(x)==nbset2.end()){
					float r2 = Distance2(pVertex[rootvert], pVertex[x]);
					if (r2<supportSize2)
						nbset2.insert(x);
				}
			}
		}
	}
}



void buildTrussForTet_case0(CTetraObj *pobj, vector<CTruss> &truss)
{
	truss.clear();
	Vector3d *pVertex = pobj->m_pVertex; 
	EdgeTable *et = NULL;
	const double ERRTOL = 1.0 - cos(PI/2.0*0.075);
	set<INT64> truss_set;

	//compute boundary info
	bool * boundaryflag = pobj->getBoundaryVertexFlags();
	assert(boundaryflag!=NULL);

	//get support 
	const int nv = pobj->m_nVertexCount;
	const int nelm = pobj->m_nElementCount;
	float *vertSupportSize2 = getVertexSupportSizeSquare(pVertex, nv, (Vector4i*)pobj->m_pElement, nelm);

	{	
		const int nvpt = pobj->m_nElmVertexCount;
		const bool refinemesh=false;
		const int isSolid = true;
		et = new EdgeTable(isSolid, NULL, nv, pobj->m_pElement, nelm, nvpt, refinemesh);
		assert(et!=NULL);
		et->buildEdgeTable();
	}

	{//Find the 1-ring and 2-ring neighbour vertices
		set<int> nbset, nbset2;
		for (int i=0; i<pobj->m_nVertexCount; i++){
			const bool b1 = boundaryflag[i];
			const float radius2 = vertSupportSize2[i];
			get1RingVertices(pVertex, et, i, nbset);
			get2RingVertices(pVertex, et, i, radius2, nbset, nbset2);
			get3RingVertices(pVertex, et, i, radius2, nbset, nbset2);

			//finally, check the angles with the 1-ring edges;
			int ring3[100], n3;
			checkAngle(pVertex[i], pVertex, nbset, nbset2, ERRTOL, ring3, n3);

			//Build truss edges
			for (set<int>::iterator it=nbset.begin(); it!=nbset.end(); it++){
				const int x = *it;
				_genTruss(i, x, truss, truss_set);
			}
			for (int k=0; k<n3; k++){
				const int x = ring3[k];
				bool b2 = boundaryflag[x];
				if (b1 && b2){ 
				//if both are on the boundray, then we need to check the midpoint
				//make sure that it is inside the object
					const Vector3d midp = (pobj->m_pVertex[i] + pobj->m_pVertex[x])*0.5;
					if (!pointInTet(pobj, midp))
						continue;

				}
				_genTruss(i, x, truss, truss_set);
			}
		}
	}

	SafeDeleteArray(boundaryflag);
	SafeDelete(et);
	SafeDeleteArray(vertSupportSize2);
}


void buildTrussForTet_case1(CTetraObj *pobj, vector<CTruss> &truss)
{
	truss.clear();

	{//Build truss edges
		set<INT64> truss_set;
		Vector4i* pelm = (Vector4i*)pobj->m_pElement;
		for (int i=0; i<pobj->m_nElementCount; i++, pelm++){
			_genTruss(pelm->x, pelm->y, truss, truss_set);
			_genTruss(pelm->x, pelm->z, truss, truss_set);
			_genTruss(pelm->x, pelm->w, truss, truss_set);
			_genTruss(pelm->y, pelm->z, truss, truss_set);
			_genTruss(pelm->y, pelm->w, truss, truss_set);
			_genTruss(pelm->z, pelm->w, truss, truss_set);		
		}
	}
}


void buildTrussForHex(CHexObj *pobj, vector<CTruss> &truss)
{
	const int nv = pobj->m_nVertexCount;
	const Vector3d *pVertex = pobj->m_pVertex;
	truss.clear();

	{//First, build truss edges
		set<INT64> truss_set;
		Vector8i* pelm = (Vector8i*)pobj->m_pElement;
		for (int i=0; i<pobj->m_nElementCount; i++, pelm++){
			int *p = &pelm->x;
			for (int j=0; j<7; j++){
				for (int k=j+1; k<8; k++){
					_genTruss(p[j], p[k], truss, truss_set);
				}
			}
		}
	}
}


//--------------------------------------------------------------------
// If the input object has no element, e.g. a surface shell, confirm
// whether a shell needs to be constructed
//--------------------------------------------------------------------
static void 
_buildTrussForTriangles(CTriangleObj * pobj, const int nSpringType, vector<CTruss>& truss)
{
	if (pobj==NULL){
		printf("Input object pointer is NULL!\n");
		return;
	}

	set<INT64> truss_set;
	Vector3i *ptri = (Vector3i*)pobj->m_pPolygon;
	assert(ptri!=NULL);
	for (int i=0; i<pobj->m_nPolygonCount; i++){
		Vector3i& tri = ptri[i];
		_genTruss(tri.x, tri.y, truss, truss_set);
		_genTruss(tri.x, tri.z, truss, truss_set);
		_genTruss(tri.y, tri.z, truss, truss_set);
	}

}

static void buildTrussShell(CPolyObj * pobj, const int nSpringType, vector<CTruss>& truss)
{
	assert(pobj->m_pElement==NULL);
	switch(pobj->m_nPlyVertexCount){
	case 2:{

			break;
		   }
	case 3:{
			CTriangleObj * p = dynamic_cast<CTriangleObj*>(pobj);
			_buildTrussForTriangles(p, nSpringType, truss);
			break;
		   }
	case 4:
		break;
	default:
		break;
	}

}


//--------------------------------------------------------------------
//Build the truss structures for elements
// nSpringType;			
// 0: Adaptive, meshfree type of truss
// 1: Nonadaptive truss, using the element edges without auxiliary shearing edges 
// 2: Gelder's method  ---could only be used on tet mesh
//       Approximate Simulation of Elastic Membranes by Trianglated Spring Meshes, J. Graphics Tools 98
//       A. van Gelder
// 3: TVCG 2007 method ---could only be used on tet mesh
//       Identification of Spring Parametrs for Deformable Object Simulation
//       Bryn A. LIoyd et al.
//--------------------------------------------------------------------
void buildTruss(CPolyObj * pobj, const int nSpringType, vector<CTruss>& truss)
{
	truss.clear();
	if (pobj->m_pElement==NULL){
		const int answer = QMessageBox::warning(
						NULL, "Warning", "No element found. Do you want to use the surface only\\to build a shell?", 
						QMessageBox::Ok, QMessageBox::Cancel);
		if (answer!=QMessageBox::Ok)
			return;
		buildTrussShell(pobj, nSpringType, truss);
		return;
	}

	//hexhedral elements, we build in one way
	if (pobj->m_nElmVertexCount==8){
		CHexObj *phexobj = dynamic_cast<CHexObj*>(pobj);
		assert(phexobj!=NULL);
		buildTrussForHex(phexobj, truss);
	}
	else{
		//tetrahedral elements, build in another method
		CTetraObj *ptetobj = dynamic_cast<CTetraObj*>(pobj);
		assert(ptetobj!=NULL);
		switch(nSpringType){
		case 0: //Adaptive, meshfree type of truss
			buildTrussForTet_case0(ptetobj, truss);
			break;
		case 1: //Non-adaptive method, use the existing element edges to 
				//build the truss edges
		case 2:
		case 3:
			buildTrussForTet_case1(ptetobj, truss);
			break;
		default:
			fprintf(stderr, "Error: unknown spring type.\n");
			assert(0);
			break;
		}
	}

	//compute truss edge's data
	_computeTrussEdgeInfo(pobj, nSpringType, truss);
}



void exportTrussPov(FILE *fp, Vector3d *pVertex, const int nv, const double *matrix, vector<CTruss>& truss)
{

	int i;
	//print vertices
	for (i=0; i<nv; i++){
		Vector3d q;
		TransformVertex3fToVertex3d(pVertex[i], matrix, q);
		const double x0 = q.x;
		const double y0 = q.y;
		const double z0 = -q.z;		//POVRAY use left-hand coor.
		fprintf(fp, "#declare V%d = <%lg, %lg, %lg>;\n", i, x0, y0, z0);
	}
	fprintf(fp, "union{\n");
	for (i=0; i<nv; i++){
		fprintf(fp, "  sphere {V%d, RR}\n", i);
	}
	fprintf(fp, "  pigment{ color rgb<1,0,0> }\n");
	fprintf(fp, "}\n");

	//print edges
	fprintf(fp, "union{\n");
	const int nsize = truss.size();
	for (i=0; i<nsize; i++){
		const int x = truss[i].ix;
		const int y = truss[i].iy;
		fprintf(fp, "  cylinder{V%d, V%d, RL}\n", x, y);
	}
	fprintf(fp, "  pigment{ color rgb<0.6,1,0> }\n");
	fprintf(fp, "}\n");
}
