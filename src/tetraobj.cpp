//tetraobj.cpp

#include "tetraobj.h"
#include "volumemeshio.h"



class CQsortTetCell
{
public:
	Vector3d m_vertex;
	int m_nID;

public: 
	void Assign(const Vector3d &vertex, const int id)
	{
		m_vertex = vertex;
		m_nID = id;
	}
};


inline int dictionary_compare(const Vector3d &v1, const Vector3d &v2, const float errorbound)
{
	if (fabs(v1.x - v2.x)< errorbound){
		if (fabs(v1.y - v2.y)< errorbound){
			if (fabs(v1.z - v2.z)< errorbound){
				return 0;
			}
			else{
				if (v1.z < v2.z)
					return -1;
				else 
					return 1;
			}
		}
		else{
			if (v1.y<v2.y)
				return -1;
			else
				return 1;
		}
	}
	else{
		if (v1.x < v2.x) 
			return -1;
		else
			return 1;
	}
	return 1;
}


static double SAMEPOINT_TOLERANCE;
static int compare(const void *e1, const void *e2)
{
	CQsortTetCell *p1, *p2;
	
	p1 = (CQsortTetCell *)e1;
	p2 = (CQsortTetCell *)e2;

	return 
		dictionary_compare(p1->m_vertex, p2->m_vertex, SAMEPOINT_TOLERANCE);
}



void CTetraObj::OptimizeMesh(Vector3d* pVertex, const double& th)
{
	int i;
	AxisAlignedBox box;

	//compute error tolerance
	box.minp = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	box.maxp = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	for (i=0; i<m_nVertexCount; i++){
		const Vector3d& v=pVertex[i];
		Minimize(box.minp, v);
		Maximize(box.maxp, v);
	}

	//set the object bounding box;
	SetBoundingBox(box);
	Vector3d dist= box.maxp-box.minp;
	const double T=(dist.x+dist.y+dist.z)*(0.333*th);

	//compute duplicated vertices
	int *pIDBuffer= new int[m_nVertexCount];
	assert(pIDBuffer!=NULL);
	OptimizeVertices(pVertex, T, pIDBuffer);
	OptimizeConnectivity(pIDBuffer);
	delete [] pIDBuffer;
}


void CTetraObj::OptimizeVertices(Vector3d* pVertex, const double& th, int *pIDBuffer)
{
	int i, count;

	if (m_pVertex==NULL) 
		m_pVertex = new Vector3d[m_nVertexCount];
	assert(m_pVertex!=NULL);
	CQsortTetCell* pBuffer = new CQsortTetCell[m_nVertexCount];
	assert(pBuffer!=NULL);
	for (i=0; i<m_nVertexCount; i++)
		pBuffer[i].Assign(pVertex[i], i);

	//do the sorting;
	SAMEPOINT_TOLERANCE = th;
	qsort(pBuffer, m_nVertexCount, sizeof(CQsortTetCell), compare);

	//assign new vertex positions to the array;
	pIDBuffer[pBuffer[0].m_nID] = 0;
	count = 0;
	m_pVertex[0].x = pBuffer[0].m_vertex.x;
	m_pVertex[0].y = pBuffer[0].m_vertex.y;
	m_pVertex[0].z = pBuffer[0].m_vertex.z;

	for (i=1; i<m_nVertexCount; i++){
		if (compare(&pBuffer[i-1], &pBuffer[i])!=0){
			count++;
			m_pVertex[count].x = pBuffer[i].m_vertex.x;
			m_pVertex[count].y = pBuffer[i].m_vertex.y;
			m_pVertex[count].z = pBuffer[i].m_vertex.z;
		}
		pIDBuffer[pBuffer[i].m_nID] = count;
	}

	count++;
	if (m_nVertexCount != count)
		printf("Orignal vertex count is %d, new count is %d\n", m_nVertexCount, count);
	m_nVertexCount = count;
	delete []pBuffer;
}


void CTetraObj::OptimizeConnectivity(int *pIDBuffer)
{
	int i, count;
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;

	//assign new node indices;
	for (i=0; i<m_nPolygonCount; i++){
		Vector3i & t = m_pTriangle[i];
		t.x = pIDBuffer[t.x];
		t.y = pIDBuffer[t.y];
		t.z = pIDBuffer[t.z];
	}
	
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;
	for (i=0; i<m_nElementCount; i++){
		Vector4i & t = m_pTetra[i];
		t.x = pIDBuffer[t.x];
		t.y = pIDBuffer[t.y];
		t.z = pIDBuffer[t.z];
		t.w = pIDBuffer[t.w];
	}

	//check degenerated triangles;
	count = 0; 
	for (i=0; i<m_nPolygonCount; i++){
		Vector3i & t = m_pTriangle[i];
		if (t.x==t.y || t.x==t.z || t.y==t.z){
			//skip this tri;
		}
		else{
			m_pTriangle[count] = t;
			count++;
		}
	}
	if (m_nPolygonCount != count)
		printf("Orignal face count is %d, new count is %d\n", m_nPolygonCount, count);
	m_nPolygonCount = count;

	//check degenerated elements;
	count = 0; 
	for (i=0; i<m_nElementCount; i++){
		Vector4i & t = m_pTetra[i];
		if (t.x==t.y || t.x==t.z || t.x==t.w ||
			t.y==t.z || t.y==t.w || t.z==t.w){
			//skip this tri;
		}
		else{
			m_pTetra[count] = t;
			count++;
		}
	}
	if (m_nElementCount != count)
		printf("Orignal tetrahedron count is %d, new count is %d\n", m_nElementCount, count);
	m_nElementCount = count;
}



inline void _drawElementWire(const Vector4i &e, Vector3d *pVertex)
{
	const int v0=e.x;
	const int v1=e.y;
	const int v2=e.z;
	const int v3=e.w;
	//============================
	glVertex3dv(&pVertex[v0].x);
	glVertex3dv(&pVertex[v1].x);
	glVertex3dv(&pVertex[v1].x);
	glVertex3dv(&pVertex[v2].x);
	glVertex3dv(&pVertex[v2].x);
	glVertex3dv(&pVertex[v0].x);
	//============================
	glVertex3dv(&pVertex[v3].x);
	glVertex3dv(&pVertex[v0].x);
	glVertex3dv(&pVertex[v3].x);
	glVertex3dv(&pVertex[v1].x);
	glVertex3dv(&pVertex[v3].x);
	glVertex3dv(&pVertex[v2].x);
}

void CTetraObj::_drawSolidElementLine(void)
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
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;
	glBegin(GL_LINES);
		for (i=0; i<m_nElementCount; i++){
			_drawElementWire(m_pTetra[i], m_pVertex);
		}
	glEnd();
	
}


void CTetraObj::glDraw(const CGLDrawParms &dparms)
{
	//printf("Not finished!\n");
	//assert(0);

	if (dparms.m_nDrawType!=CGLDrawParms::DRAW_MESH_LINE){
		CPolyObj::glDraw(dparms);
		return;
	}

	m_pDrawParms = (CGLDrawParms*)(&dparms);
	glShadeModel(GL_SMOOTH);

	_drawSolidElementLine();
		
	//draw the vertices;
	if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
		if (m_fGivenRad<1e-12f) 
			m_fGivenRad=estimatedVertexRadius();
		glDrawVertices(dparms.m_cVertexColor);
	}
}


static bool output_hypermesh_file(const char *fname, 
								  int nv, Vector3d *pVertex, 
								  int nface, Vector3i *pFace, 
								  int nTet, Vector4i *pTet)
{

	return true;
}



//==========================================================================


int CTetraObj::LoadFile(const char *fname)
{
	Vector3d* pVertex;
	int nv, ntri, ntet, meshtype;
	int *pFace, *pTet;

	if (!loadNeutralMeshFile(fname, pVertex, nv, pFace, ntri, pTet, ntet, meshtype)){
		fprintf(stderr, "Loading file error: %s", fname);
		return false;
	}
	assert(meshtype==1);
	
	//copy data
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	m_nVertexCount = nv;	
	m_nPolygonCount = ntri;	
	m_nElementCount = ntet;
	m_pVertex = pVertex;
	m_pTriangle = (Vector3i*)pFace;
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;
	m_pTetra = (Vector4i*)pTet; 

	//bounding box and normal;
	AxisAlignedBox box;
	ComputeBoundingBox(box.minp, box.maxp);
	SetBoundingBox(box);
	return 1;
}


int CTetraObj::exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer)
{
	const int TYPEID=1;
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;
	for (int i=0; i<m_nElementCount; i++){
		int idx = tetbaseno+i;
		Vector4i *pt = &m_pTetra[i];
		int vx = vbaseno + pt->x;
		int vy = vbaseno + pt->y;
		int vz = vbaseno + pt->z;
		int vw = vbaseno + pt->w;
		fprintf(fp, "%d %d %d %d %d %d %d\n", 
					idx, TYPEID, objsetid, vx, vy, vz, vw);
	}
	return m_nElementCount;
}


void CTetraObj::exportNeutralMeshFile(FILE *fp, const double *matrix)
{
	int i;
	Vector3i *& m_pTriangle = (Vector3i*&)m_pPolygon;
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;

	//write vertex;
	fprintf(fp, "%d\n", m_nVertexCount);
	for (i=0; i<m_nVertexCount; i++){
		double v[3];
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, v);
		fprintf(fp, "%.14lf %.14lf %.14lf\n", v[0], v[1], v[2]);
	}

	fprintf(fp, "%d\n", m_nElementCount);
	for (i=0; i<m_nElementCount; i++){
		Vector4i * p = &m_pTetra[i];
		int x = p->x+1;
		int y = p->y+1;
		int z = p->z+1;
		int w = p->w+1;
		fprintf(fp, "1 %d %d %d %d\n", x, y, z, w);
	}

	fprintf(fp, "%d\n", m_nPolygonCount);
	for (i=0; i<m_nPolygonCount; i++){
		Vector3i * p = &m_pTriangle[i];
		int x = p->x+1;
		int y = p->y+1;
		int z = p->z+1;
		fprintf(fp, "1 %d %d %d\n", x, y, z);
	}
}


void CTetraObj::exportFile(FILE *fp, const char *format, const double *matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0){
		exportPovrayFile(fp, matrix);
	}
	else if (strcmp(format, ".mesh")==0 || strcmp(format, ".MESH")==0){
		exportNeutralMeshFile(fp, matrix);
	}
}


void CTetraObj::_setData(Vector3d *pVertex, const int nVertex, 
        Vector4i *pElement, const int nElement, 
	    Vector3i *pTri, const int nTri, const bool allocbuffer)
{
	Vector3i *& m_pTriangle = (Vector3i*&)m_pPolygon;
	Vector4i *&m_pTetra = (Vector4i*&)m_pElement;
	m_nVertexCount = nVertex;	//total vertex count
	m_nPolygonCount = nTri;		//total polygon count
	m_nElementCount = nElement;
	if (!allocbuffer){
	    m_pVertex = pVertex;		//the vertex array
	    m_pTriangle = pTri;			//triangle array
	    m_pTetra = pElement;
        return;
    }
	//copy the buffers;
	m_pVertex = new Vector3d[nVertex];
	assert(m_pVertex!=NULL);
	memcpy(m_pVertex, pVertex, nVertex*sizeof(Vector3d));
	m_pTriangle = new Vector3i[nTri];
	assert(m_pTriangle!=NULL);
	memcpy(m_pTriangle, pTri, sizeof(Vector3i)*nTri);
	m_pTetra = new Vector4i[nElement];
	assert(m_pTetra!=NULL);
	memcpy(m_pTetra, pElement, sizeof(Vector4i)*nElement);
}

	
	
CTetraObj::CTetraObj(Vector3d *pVertex, const int nVertex, Vector4i *pElement, 
	    const int nElement, Vector3i *pTri, const int nTri, const bool allocbuffer)
        :CTriangleObj()
{
    m_nElmVertexCount = 4;
    _setData(pVertex, nVertex, pElement, nElement, pTri, nTri, allocbuffer);

	//bounding box and normal;
	AxisAlignedBox box;
	ComputeBoundingBox(box.minp, box.maxp);
	SetBoundingBox(box);
}



double* CTetraObj::computeElementVolumes(void)
{
	const Vector4i* pelm = (const Vector4i*)m_pElement;
	double *pvol = new double[this->m_nElementCount];
	if (pelm==NULL || pvol==NULL) 
		return NULL;

	for (int i=0; i<this->m_nElementCount; i++){
		const Vector4i t = pelm[i];
		const Vector3d& a = m_pVertex[t.x];
		const Vector3d& b = m_pVertex[t.y];
		const Vector3d& c = m_pVertex[t.z];
		const Vector3d& d = m_pVertex[t.w];
		pvol[i] = computeTetrahedronVolume(a, b, c, d);
	}
	return pvol;
}


double* CTetraObj::computeVertexVolumes(void)
{
	int i, j;
	double* pElmVol=NULL, *pVertVol=NULL;

	//get per element volume
	pElmVol = computeElementVolumes();
	if (pElmVol==NULL) goto EXITPNT;

	//div 4 first
	for (i=0; i<m_nElementCount; i++) 
		pElmVol[i]*=0.250;

	//alloc vertex buffer
	pVertVol = new double[this->m_nVertexCount];
	if (pVertVol==NULL) goto EXITPNT;
	for (i=0; i<m_nVertexCount; i++) 
		pVertVol[i] = 0;

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
	}

EXITPNT:
	SafeDeleteArray(pElmVol);
	return pVertVol;
}


void CTetraObj::drawVolumetricElements(void)
{
	extern CObjectBase* runGPUTetIsoRender(CObjectBase *pRenderingObj, CPolyObj *pobj, CGLDrawParms *pdrawparm);
	CObjectBase* p;

	p = runGPUTetIsoRender(m_pRenderingObject, this, m_pDrawParms);
	if (p!=NULL)
		m_pRenderingObject = p;
}


