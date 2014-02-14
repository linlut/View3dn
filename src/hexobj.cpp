//hexobj.cpp

#include <vector>
#include <windows.h>
#include <GL/glu.h>
#include "hexobj.h"
#include "maths.h"
#include "pointorgnizer.h"


int CHexObj::LoadFile(const char *fname)
{
	Vector3d* pVertex;
	int nv, nquad, nelm, meshtype;
	int *pFace, *pCube;
	extern bool loadNeutralMeshFile(const char *fname, Vector3d *& pVertex, int &nv, int *& pFace, int &ntri, int *& pTet, int &ntet, int &meshtype);

	if (!loadNeutralMeshFile(fname, pVertex, nv, pFace, nquad, pCube, nelm, meshtype)){
		fprintf(stderr, "Loading file error: %s", fname);
		return 0;
	}
	assert(meshtype==2);
	
	//copy data
	Vector8i*& m_pHex = (Vector8i*&)m_pElement;
	m_nVertexCount = nv;	
	m_nPolygonCount = nquad;	
	m_nElementCount = nelm;
	m_pVertex = pVertex;
	m_pPolygon = (int*)pFace;
	m_pHex = (Vector8i*)pCube; 

	//bounding box and normal;
	AxisAlignedBox box;
	ComputeBoundingBox(box.minp, box.maxp);
	SetBoundingBox(box);

	return 1;
}


int CHexObj::exportElemConnectivity(
	FILE *fp, const int objid, const int objsetid, 
	const int tetbaseno, const int vbaseno, const int *vbasebuffer)
{
	const int TYPEID = 1;
	Vector8i*& m_pHex = (Vector8i*&)m_pElement;
	for (int i=0; i<m_nElementCount; i++){
		const int idx = tetbaseno+i;
		const Vector8i *pt = &m_pHex[i];
		const int vx = vbaseno + pt->x;
		const int vy = vbaseno + pt->y;
		const int vz = vbaseno + pt->z;
		const int vw = vbaseno + pt->w;
		const int vx1 = vbaseno + pt->x1;
		const int vy1 = vbaseno + pt->y1;
		const int vz1 = vbaseno + pt->z1;
		const int vw1 = vbaseno + pt->w1;
		fprintf(fp, "%d %d %d %d %d %d %d %d %d %d %d\n", 
					idx, TYPEID, objsetid,
					vx, vy, vz, vw, vx1, vy1, vz1, vw1);
	}
	return m_nElementCount;
}


void CHexObj::exportNeutralMeshFile(FILE *fp, const double *matrix)
{
	int i;
	Vector8i*& m_pHex = (Vector8i*&)m_pElement;

	//write vertex;
	double m2[4][4];
	double u[3], v[3];
	if (matrix) vmCopy(matrix, m2);
	fprintf(fp, "%d\n", m_nVertexCount);
	for (i=0; i<m_nVertexCount; i++){
		Vector3d * p = &m_pVertex[i];
		if (matrix==NULL){
			v[0] = p->x;
			v[1] = p->y;
			v[2] = p->z;
		}else{
			u[0] = p->x;
			u[1] = p->y;
			u[2] = p->z;
			pmMult(u, m2, v);
		}
		fprintf(fp, "%.14lg %.14lg %.14lg\n", v[0], v[1], v[2]);
	}

	fprintf(fp, "%d\n", m_nElementCount);
	for (i=0; i<m_nElementCount; i++){
		Vector8i * p = &m_pHex[i];
		int x = p->x+1;
		int y = p->y+1;
		int z = p->z+1;
		int w = p->w+1;
		int x1 = p->x1+1;
		int y1 = p->y1+1;
		int z1 = p->z1+1;
		int w1 = p->w1+1;
		fprintf(fp, "2 %d %d %d %d %d %d %d %d\n", x, y, z, w, x1, y1, z1, w1);
	}

	fprintf(fp, "%d\n", m_nPolygonCount);
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (i=0; i<m_nPolygonCount; i++){
		Vector4i * p = &m_pQuad[i];
		int x = p->x+1;
		int y = p->y+1;
		int z = p->z+1;
		int w = p->w+1;
		fprintf(fp, "2 %d %d %d %d\n", x, y, z, w);
	}
}


void CHexObj::exportFile(FILE *fp, const char *format, const double *matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0){
		exportPovrayFile(fp, matrix);
	}
	else if (strcmp(format, ".mesh")==0 || strcmp(format, ".MESH")==0){
		exportNeutralMeshFile(fp, matrix);
	}
}


void CHexObj::_setData(Vector3d *pVertex, const int nVertex, Vector8i *pElement, const int nElement, Vector4i *pQuad, const int nQuad, const bool allocbuffer)
{
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	Vector8i *& m_pHex = (Vector8i*&)m_pElement;

	//if new buffer space is required, we make a copy of them;
    m_nVertexCount = nVertex;	//total vertex count
	m_nPolygonCount = nQuad;	//total polygon count
	m_nElementCount = nElement;
	if (!allocbuffer){
		m_pVertex = pVertex;		//the vertex array
		m_pQuad = pQuad;			//triangle array
		m_pHex = pElement;
		return;
	}
	//copy the buffers;
	m_pVertex = new Vector3d[nVertex];
	assert(m_pVertex!=NULL);
	memcpy(m_pVertex, pVertex, nVertex*sizeof(Vector3d));
	m_pQuad = new Vector4i[nQuad];
	assert(m_pQuad!=NULL);
	memcpy(m_pQuad, pQuad, sizeof(Vector4i)*nQuad);
	m_pHex = new Vector8i[nElement];
	assert(m_pHex!=NULL);
	memcpy(m_pHex, pElement, sizeof(Vector8i)*nElement);
}


static inline void 
_drawElementWire(const Vector8i &e, Vector3d *pVertex)
{
	const int v0=e.x;
	const int v1=e.y;
	const int v2=e.z;
	const int v3=e.w;
	const int v4=e.x1;
	const int v5=e.y1;
	const int v6=e.z1;
	const int v7=e.w1;
	//============================
		glVertex3dv(&pVertex[v0].x);
		glVertex3dv(&pVertex[v1].x);
		glVertex3dv(&pVertex[v1].x);
		glVertex3dv(&pVertex[v2].x);
		glVertex3dv(&pVertex[v2].x);
		glVertex3dv(&pVertex[v3].x);
		glVertex3dv(&pVertex[v3].x);
		glVertex3dv(&pVertex[v0].x);
	//============================
		glVertex3dv(&pVertex[v4].x);
		glVertex3dv(&pVertex[v5].x);
		glVertex3dv(&pVertex[v5].x);
		glVertex3dv(&pVertex[v6].x);
		glVertex3dv(&pVertex[v6].x);
		glVertex3dv(&pVertex[v7].x);
		glVertex3dv(&pVertex[v7].x);
		glVertex3dv(&pVertex[v4].x);
	//============================
		glVertex3dv(&pVertex[v0].x);
		glVertex3dv(&pVertex[v4].x);
		glVertex3dv(&pVertex[v1].x);
		glVertex3dv(&pVertex[v5].x);
		glVertex3dv(&pVertex[v2].x);
		glVertex3dv(&pVertex[v6].x);
		glVertex3dv(&pVertex[v3].x);
		glVertex3dv(&pVertex[v7].x);
}

void CHexObj::_drawSolidElementLine(void)
{
	int i;
	bool needtex = false;
	
	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
    glDisable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glColor3fv((float*)&m_pDrawParms->m_cHLineColor);
	glLineWidth(m_pDrawParms->m_fLineWidth);
	Vector8i*& m_pHex = (Vector8i*&)m_pElement;
	glBegin(GL_LINES);
		for (i=0; i<m_nElementCount; i++){
			_drawElementWire(m_pHex[i], m_pVertex);
		}
	glEnd();
	
}


void CHexObj::glDraw(const CGLDrawParms &dparms)
{
	if (dparms.m_nDrawType!=CGLDrawParms::DRAW_MESH_LINE){
		CPolyObj::glDraw(dparms);
		return;
	}

	m_pDrawParms = (CGLDrawParms*)(&dparms);
	glShadeModel(GL_SMOOTH);

	_drawSolidElementLine();
		
	//draw the vertices;
	if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
		if (m_fGivenRad<1e-20) 
			m_fGivenRad=estimatedVertexRadius();
		glDrawVertices(dparms.m_cVertexColor);
	}
}


int *CHexObj::getBoundaryVeritces(int &nBoundVert)
{
	int i, j, c; 
	int *pBoundVert=NULL;
	nBoundVert = 0;

	if (m_nVertexCount==0) return NULL;
	unsigned char * pbuff = new unsigned char [m_nVertexCount];
	assert(pbuff!=NULL);
	for (i=0; i<m_nVertexCount; i++) pbuff[i]=0;
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (i=0; i<m_nPolygonCount; i++){
		const Vector4i *p = &m_pQuad[i];
		pbuff[p->x]=1;
		pbuff[p->y]=1;
		pbuff[p->z]=1;
		pbuff[p->w]=1;
	}

	//check the boundary vertex number
	for (i=c=0; i<m_nVertexCount; i++)
		c+=pbuff[i];
	if (c>0){
		nBoundVert = c;
		pBoundVert= new int [c];
		assert(pBoundVert!=NULL);
		for (i=j=0; i<m_nVertexCount; i++)
			if (pbuff[i]) pBoundVert[j++]=i;
	}

	delete [] pbuff;
	return pBoundVert;
}


void CHexObj::mirrorXYPlane(void)
{
	//reverse the z value using CPolyObj func
	CQuadObj::mirrorXYPlane();

	//aslo, we need to reverse the element orientations
	int i;
	Vector8i*& m_pHex = (Vector8i*&)m_pElement;
    for (i=0; i<m_nElementCount; i++){
		int* pquad = &m_pHex[i].x;
		int _tmp = pquad[1];
		pquad[1] = pquad[3];
		pquad[3] = _tmp;

		pquad = &m_pHex[i].x1;
		_tmp = pquad[1];
		pquad[1] = pquad[3];
		pquad[3] = _tmp;
	}
}


static double _computeMergeVertexErrorTol(const CHexObj& a)
{
	const int n = a.m_nElementCount;
	const int K=4;
	const int n4 = n/K;
	const Vector3d *p = a.m_pVertex;
	double sum=0, sumx=0, sumy=0, sumz=0;
	Vector8i*& pHexA = (Vector8i*&)a.m_pElement;

	for (int i=0; i<K; i++){	
		const Vector8i& e = pHexA[i*n4];
		sumx += Distance2(p[e.x], p[e.x1]);
		sumx += Distance2(p[e.y], p[e.y1]);
		sumx += Distance2(p[e.z], p[e.z1]);
		sumx += Distance2(p[e.w], p[e.w1]);

        sumy += Distance2(p[e.x], p[e.y]);
        sumy += Distance2(p[e.z], p[e.w]);
        sumy += Distance2(p[e.x1], p[e.y1]);
        sumy += Distance2(p[e.z1], p[e.w1]);

        sumz += Distance2(p[e.y], p[e.z]);
        sumz += Distance2(p[e.x], p[e.w]);
        sumz += Distance2(p[e.y1], p[e.z1]);
        sumz += Distance2(p[e.x1], p[e.w1]);
	}

    const double s1=_MAX3_(sumx, sumy, sumz);
    const double s2=_MIN3_(sumx, sumy, sumz) + 1e-9;
    const double ratio=s1/s2;
    if (ratio>9)
        sum= s2;
    else if (ratio<0.11111)
        sum= s1;
    else    
		sum=(sumx+sumy+sumz)*0.333333;
	sum*=1.0/(4*K);
	return sum;
}


double CHexObj::estimatedVertexRadius(void)
{
    double r2=_computeMergeVertexErrorTol(*this);
    double r = sqrt(r2);
    r *= 0.10;
	return r;
}


double* CHexObj::computeElementVolumes(void)
{
	const int nelm = this->m_nElementCount;
	const Vector8i *pHex = (const Vector8i*)this->m_pElement;
	if (pHex==NULL || nelm==0) 
		return NULL;

	double *pvol = new double[this->m_nElementCount];
	if (pvol==NULL) return NULL;
	
	for (int i=0; i<nelm; i++){
		double r = computeHexahedronVolume(this->m_pVertex, pHex[i]);
		pvol[i]=fabs(r);
	}
	return pvol;
}


double* CHexObj::computeVertexVolumes(void)
{
	int i, j;
	double* pElmVol=NULL, *pVertVol=NULL;

	//get per element volume
	pElmVol = computeElementVolumes();
	if (pElmVol==NULL) goto EXITPNT;

	//div by 8 first
	for (i=0; i<m_nElementCount; i++) 
		pElmVol[i] *= 0.1250;

	//alloc vertex vol buffer
	pVertVol = new double[this->m_nVertexCount];
	if (pVertVol==NULL) goto EXITPNT;
	for (i=0; i<m_nVertexCount; i++) pVertVol[i] = 0;

	//compute per vertex volume
	for (i=j=0; i<this->m_nElementCount; i++){
		const double qart_vol = pElmVol[i];

		int id = this->m_pElement[j++];
		pVertVol[id] += qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;

		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
		id = this->m_pElement[j++];
		pVertVol[id]+=qart_vol;
	}

EXITPNT:
	SafeDeleteArray(pElmVol);
	return pVertVol;
}


//=======================================================================
void doMeshBrick(const Vector3d& lowleft, const Vector3d& upright, const int nx, const int ny, const int nz, 
		Vector3d *& m_pVertex, Vector4i *&m_pQuad, Vector8i *&m_pHex, 
		int& m_nVertexCount, int& m_nPolygonCount, int& m_nElementCount)
{
	int i, j, k, count;
	const int NX=nx+1;
	const int NY=ny+1;
	const int NZ=nz+1;
	assert(nx>0 && ny>0 && nz>0);
	m_nVertexCount = NX*NY*NZ;
	m_pVertex = new Vector3d [m_nVertexCount];
	assert(m_pVertex!=NULL);

	//init the vertices;
	double ddx = double(upright.x-lowleft.x)/nx;
	double ddy = double(upright.y-lowleft.y)/ny;
	double ddz = double(upright.z-lowleft.z)/nz;
	for (k=count=0; k<=nz; k++){
		double dz = k*ddz;
		for (j=0; j<=ny; j++){
			double dy = j*ddy;
			for (i=0; i<=nx; i++,count++){
				double dx = i*ddx;
				Vector3d& v = m_pVertex[count];
				v.x = lowleft.x+dx;
				v.y = lowleft.y+dy;
				v.z = lowleft.z+dz;
			}
		}
	}

	//get the elements and boundary surface
	getBrickElements(nx, ny, nz, m_pHex, m_nElementCount);
	getBrickBounday(nx, ny, nz, m_pQuad, m_nPolygonCount);
}


static inline int 
ARRAY3DINDEX(const int x, const int y, const int z, const int nx, const int nxny)
{ return (x+y*nx+z*nxny); }

void getBrickElements(const int nx, const int ny, const int nz, Vector8i *&m_pHex, int& m_nElementCount)
{
	int i, j, k, count;
	const int NX=nx+1;
	const int NY=ny+1;
	const int NZ=nz+1;
    const int NXNY=NX*NY;
	assert(nx>0 && ny>0 && nz>0);

	//init elements;
	m_nElementCount = nx*ny*nz;
	m_pHex = new Vector8i[m_nElementCount];
	assert(m_pHex!=NULL);

	for (k=count=0; k<nz; k++){
        const int k1=k+1;
		for (j=0; j<ny; j++){
            const int j1=j+1;
			for (i=0; i<nx; i++,count++){
				Vector8i *p = &m_pHex[count];
                const int i1=i+1;
				p->x = ARRAY3DINDEX(i,    j,   k,   NX, NXNY);;
				p->y = ARRAY3DINDEX(i1,  j,   k,   NX, NXNY);
				p->z = ARRAY3DINDEX(i1,  j1, k,   NX, NXNY);
				p->w = ARRAY3DINDEX(i,    j1, k,   NX, NXNY);
				p->x1 = ARRAY3DINDEX(i,   j,   k1, NX, NXNY);
				p->y1 = ARRAY3DINDEX(i1, j,   k1, NX, NXNY);
				p->z1 = ARRAY3DINDEX(i1, j1, k1, NX, NXNY);
				p->w1 = ARRAY3DINDEX(i,   j1, k1, NX, NXNY);
			}
		}
	}
}

void getBrickBounday(const int nx, const int ny, const int nz, Vector4i *&m_pQuad, int& m_nPolygonCount)
{
	int i, j, k, count;
	const int NX=nx+1;
	const int NY=ny+1;
	const int NZ=nz+1;
    const int NXNY = NX*NY;
	assert(nx>0 && ny>0 && nz>0);

	//boundary quads
	m_nPolygonCount = (nx*ny+nx*nz+ny*nz)*2;
	m_pQuad = new Vector4i[m_nPolygonCount]; 
	assert(m_pQuad!=NULL);
	count = 0;
	for (j=0, count=0; j<ny; j++){   //z=0
        const int j1=j+1;
		for (i=0; i<nx; i++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(i,   j,   0, NX, NXNY);
			pquad->y = ARRAY3DINDEX(i,   j1, 0, NX, NXNY);
			pquad->z = ARRAY3DINDEX(i+1, j1, 0, NX, NXNY);
			pquad->w = ARRAY3DINDEX(i+1, j,   0, NX, NXNY);
		}
	}
	for (j=0; j<ny; j++){			//z=nz;
        const int j1=j+1;
		for (i=0; i<nx; i++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(i,   j,   nz, NX, NXNY);
			pquad->y = ARRAY3DINDEX(i+1, j,   nz, NX, NXNY);
			pquad->z = ARRAY3DINDEX(i+1, j1, nz, NX, NXNY);
			pquad->w = ARRAY3DINDEX(i,   j1, nz, NX, NXNY);
		}
	}
	for (k=0; k<nz; k++){			//x=0
        const int k1=k+1;
		for (j=0; j<ny; j++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(0, j,   k,   NX, NXNY);
			pquad->y = ARRAY3DINDEX(0, j,   k1, NX, NXNY);
			pquad->z = ARRAY3DINDEX(0, j+1, k1, NX, NXNY);
			pquad->w = ARRAY3DINDEX(0, j+1, k,   NX, NXNY);
		}
	}
	for (k=0; k<nz; k++){			//x=nx;
        const int k1=k+1;
		for (j=0; j<ny; j++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(nx, j,   k,   NX, NXNY);
			pquad->y = ARRAY3DINDEX(nx, j+1, k,   NX, NXNY);
			pquad->z = ARRAY3DINDEX(nx, j+1, k1, NX, NXNY);
			pquad->w = ARRAY3DINDEX(nx, j,   k1, NX, NXNY);
		}
	}
	for (i=0; i<nx; i++){			//y=0;
        const int i1=i+1;
		for (k=0; k<nz; k++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(i,   0, k,   NX, NXNY);
			pquad->y = ARRAY3DINDEX(i1, 0, k,   NX, NXNY);
			pquad->z = ARRAY3DINDEX(i1, 0, k+1, NX, NXNY);
			pquad->w = ARRAY3DINDEX(i,   0, k+1, NX, NXNY);
		}
	}
	for (i=0; i<nx; i++){			//y=ny;
        const int i1=i+1;
		for (k=0; k<nz; k++, count++){
			Vector4i *pquad = &m_pQuad[count];
			pquad->x = ARRAY3DINDEX(i,   ny, k,   NX, NXNY);
			pquad->y = ARRAY3DINDEX(i,   ny, k+1, NX, NXNY);
			pquad->z = ARRAY3DINDEX(i1, ny, k+1, NX, NXNY);
			pquad->w = ARRAY3DINDEX(i1, ny, k,   NX, NXNY);
		}
	}
	assert(count==m_nPolygonCount);
}


//====================================================================================
static inline int 
_searchDupVertexInArray(const Vector3d& p, const int *bvert, const Vector3d *a, const int len, const double TOLSQRDIS)
{
	for (int i=0; i<len; i++){
		const int j=bvert[i];
		if (Distance2(p, a[j])<TOLSQRDIS)
			return j;
	}
	return -1;
}


static inline double 
_computeDupVertexErrorTol(const CHexObj& a, const CHexObj& b, const double &zerotol)
{
	const double t1 = _computeMergeVertexErrorTol(a);   //square distance
	const double t2 = _computeMergeVertexErrorTol(b);   //square distance
	const double t = _MIN_(t1, t2);
	return t*zerotol;
}

static inline int
_quadDupVertexCountA(const Vector4i& quad, const int *pindex)
{
	int c=0;
	if (pindex[quad.x]>=0) c++;
	if (pindex[quad.y]>=0) c++;
	if (pindex[quad.z]>=0) c++;
	if (pindex[quad.w]>=0) c++;
	return c;
}

static inline int
_quadDupVertexCountB(const Vector4i& quad, const int *pindex, const int nbase)
{
	int c=0;
	if (pindex[quad.x]<nbase) c++;
	if (pindex[quad.y]<nbase) c++;
	if (pindex[quad.z]<nbase) c++;
	if (pindex[quad.w]<nbase) c++;
	return c;
}


static void __mergeHexObj(CHexObj & obja, CHexObj & objb, CHexObj &objc, const double errscale)
{

	Vector4i * &pQuadA = (Vector4i * &)obja.m_pPolygon;
	Vector4i * &pQuadB = (Vector4i * &)objb.m_pPolygon;
	Vector4i * &pQuadC = (Vector4i * &)objc.m_pPolygon;
	int i, c;
	const Vector3d *pva = obja.m_pVertex;
	const Vector3d *pvb = objb.m_pVertex;
	const int nva = obja.m_nVertexCount;
	const int nvb = objb.m_nVertexCount;
	int *pVertIndexA = new int[nva];
	int *pVertIndexB = new int[nvb];
	assert(pVertIndexA!=NULL && pVertIndexB!=NULL);
	for (i=0; i<nva; i++) pVertIndexA[i]=-1;	
	for (i=0; i<nvb; i++) pVertIndexB[i]=-1;	

	//get the dup vertex tolerance 
    const double ERRSCALE2 = errscale*errscale*3;
	const double TOLSQRDIS=_computeDupVertexErrorTol(obja, objb, ERRSCALE2);

	//get the bounday vertices of the two objects
	int nBoundVertA, nBoundVertB;
	int *pBufA = obja.getBoundaryVeritces(nBoundVertA);
	int *pBufB = objb.getBoundaryVeritces(nBoundVertB);
	assert(pBufA!=NULL && pBufB!=NULL);

	//find the dup vertices in object b
    if (nBoundVertA<50){
	    for (i=0; i<nBoundVertB; i++){	//find ref vertices for array B
		    const int j=pBufB[i];
		    const int pos=_searchDupVertexInArray(pvb[j], pBufA, pva, nBoundVertA, TOLSQRDIS);
		    pVertIndexB[j] = pos;
	    }
    }
    else{//for large array A, we do spatial sorting before the search 
        CPointOrgnizer3D pointset(pva, pBufA, nBoundVertA);
	    for (i=0; i<nBoundVertB; i++){
		    const int j=pBufB[i];
		    const int pos=pointset.searchPoint(pvb[j], TOLSQRDIS);
            //naive code to verify the result
		    //const int pos2=_searchDupVertexInArray(pvb[j], pBufA, pva, nBoundVertA, TOLSQRDIS);
            //if (pos!=pos2) assert(0);
		    pVertIndexB[j] = pos;
	    }
    }
	for (i=0; i<nvb; i++){			//cross refernce for vertices of array A
		const int j=pVertIndexB[i];
		if (j>=0) pVertIndexA[j]=i;
	}
	for (i=0, c=nva; i<nvb; i++){	//fill the rest of the buffer
		if (pVertIndexB[i]<0)
			pVertIndexB[i] = c++;
	}
	objc.m_nVertexCount = c;
	delete [] pBufA;
	delete [] pBufB;

	//copy the final boundary surface
	Vector4i* pQuad = new Vector4i[obja.m_nPolygonCount+objb.m_nPolygonCount];
	assert(pQuad!=NULL);
	pQuadC = pQuad;
	for (i=c=0; i<obja.m_nPolygonCount; i++){
		Vector4i & quad = pQuadA[i];
		const int ndup=_quadDupVertexCountA(quad, pVertIndexA);
		if (ndup<=3)
			pQuad[c++]=quad;
	}
	for (i=0; i<objb.m_nPolygonCount; i++){
		Vector4i & quad = pQuadB[i];
		const int ndup=_quadDupVertexCountB(quad, pVertIndexB, nva);
		if (ndup<=3){
			const int n0=pVertIndexB[quad.x];
			const int n1=pVertIndexB[quad.y];
			const int n2=pVertIndexB[quad.z];
			const int n3=pVertIndexB[quad.w];
			pQuad[c++]=Vector4i(n0,n1,n2,n3);
		}
	}
	objc.m_nPolygonCount = c;

	//prepare the new hex obj vertices;
	objc.m_nVertexCount = objc.m_nVertexCount;
	Vector3d *pVertex = objc.m_pVertex = new Vector3d[objc.m_nVertexCount];
	assert(pVertex!=NULL);
	memcpy(pVertex, pva, sizeof(Vector3d)*nva);
	for (i=0; i<nvb; i++){
		const int j = pVertIndexB[i];
		if (j>=nva) pVertex[j]=pvb[i];
	}

	//copy the elements for obj c
	objc.m_nElementCount = obja.m_nElementCount + objb.m_nElementCount;
	Vector8i *pHex = new Vector8i[objc.m_nElementCount];
	objc.m_pElement = (int*)pHex;
	assert(pHex!=NULL);
	memcpy(pHex, obja.m_pElement, sizeof(Vector8i)*obja.m_nElementCount);

	Vector8i*& pHexB = (Vector8i*&)objb.m_pElement;
	Vector8i*& pHexC = (Vector8i*&)objc.m_pElement;
	for (i=0, c=obja.m_nElementCount; i<objb.m_nElementCount; i++,c++){
		Vector8i e = pHexB[i];
		e.x = pVertIndexB[e.x];
		e.y = pVertIndexB[e.y];
		e.z = pVertIndexB[e.z];
		e.w = pVertIndexB[e.w];
		e.x1 = pVertIndexB[e.x1];
		e.y1 = pVertIndexB[e.y1];
		e.z1 = pVertIndexB[e.z1];
		e.w1 = pVertIndexB[e.w1];
		pHexC[c]=e;
	}

	delete [] pVertIndexA;
	delete [] pVertIndexB;
}



void MergeHexObj(CHexObj & obja, CHexObj & objb, CHexObj &objc, const double errscale)
{
    if (obja.m_nVertexCount>objb.m_nVertexCount)
        __mergeHexObj(obja, objb, objc, errscale);
    else
        __mergeHexObj(objb, obja, objc, errscale);
}


