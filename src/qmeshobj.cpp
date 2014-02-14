//qmeshobj.cpp

#include "qmeshobj.h"
#include <GL/glu.h>
#include <geomath.h>
#include "pointorgnizer.h"

//#define TVCG_DEMO
static void setUserNormals(CPolyObj *pobj)
{
	int vid1=1045;
	int vid2=1055;
	//pobj->ComputeVertexNormals();
	for (int i=vid1; i<=vid2; i++){
		pobj->m_pVertexNorm[i]=Vector3f(0,1,0);
	}
}


int CQuadObj::exportElemConnectivity(FILE *fp, const int objid, const int tetbaseno, const int vbaseno)
{
    const int ELMCLASSID =3;   //shell element
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (int i=0; i<m_nPolygonCount; i++){
		const int idx = tetbaseno+i;
		const Vector4i *pt = &m_pQuad[i];
		const int vx = vbaseno + pt->x;
		const int vy = vbaseno + pt->y;
		const int vz = vbaseno + pt->z;
		const int vw = vbaseno + pt->w;
		fprintf(fp, "%d %d %d %d %d %d %d\n", 
					idx, ELMCLASSID, objid,
					vx, vy, vz, vw);
	}
	return m_nPolygonCount;
}


int CQuadObj::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double thickness)
{
    extern void exportPlyVertices(FILE *fp, Vector3d *pVertex, const int nv, double _th, double *pThick, const int baseno, const double *pmatrix);
    exportPlyVertices(fp, m_pVertex, m_nVertexCount, thickness, NULL, baseno, pmatrix);
    return m_nVertexCount;
}


int CQuadObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nTotalAttrib)
{
	//read vertices;
	m_nVertexCount = nv;
	LoadPltVertices(fp, nv, nTotalAttrib);

	//read the quads;
	m_nPolygonCount = nf;
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	m_pQuad	= new Vector4i[m_nPolygonCount];
	assert(m_pQuad!=NULL);

	for (int i=0; i<nf; i++){
		int n0, n1, n2, n3;
		int count= fscanf(fp, "%d %d %d %d", &n0, &n1, &n2, &n3);
		n0--, n1--, n2--, n3--;
		if (count!=4 || n0<0 || n1<0 || n2<0 || n3<0){
			fprintf(stderr, "Error reading triangle %d!\n", i);
			exit(0);
		}
		m_pQuad[i] = Vector4i(n0, n1, n2, n3);
	}

	return 1;
}


int CQuadObj::LoadFile(FILE *fp, const char *ftype)
{
	int status, nvert, nface, nfattrib;
	float radius;
	char chunktype[64];

	if (strcmp(ftype, ".PLT")==0 || strcmp(ftype, ".plt")==0){
		status = CPolyObj::ReadPltFileHeader(fp, chunktype, nvert, nface, radius, nfattrib, m_strFVarNames, m_strIVarNames);
		if (status == 0)
			return 0;
		status = LoadPltFileWithoutHeader(fp, nvert, nface, nfattrib);
		if (status == 0)
			return 0;
	}
	else
		return 0;

	return 1;
}


int CQuadObj::SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
{
	int i;
	Vector3d lowleft, upright;

	for (i=0; i<bufflen; i++){
		CQuadObj *p = new CQuadObj;
		assert(p!=NULL);
		_copyToObject(*p, objidbuff[i]);
		p->ComputeBoundingBox(lowleft, upright);
		p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
		obj[i] = p;
	}
	return bufflen;
}


int CQuadObj::saveSTLFile(const char *fname, const double* pmatrix)
{
	assert(0);
	return 1;
}


int CQuadObj::saveTXTFile(const char *fname, const double* pmatrix)
{
    int i;
	FILE *fp=fopen(fname, _WA_);
	if (fp==NULL) return 0;
    
	fprintf(fp, "%d %d\n1\n", m_nVertexCount, m_nPolygonCount*2);
	for (i=0; i<m_nVertexCount; i++){
        const Vector3d *_p= &m_pVertex[i];
        Vector3d p;
		TransformVertex3dToVertex3d(*_p, pmatrix, &p.x);
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
    for (i=0; i<m_nPolygonCount; i++){
		const Vector4i quad = m_pQuad[i];
		fprintf(fp, "%d %d %d\n", quad.x, quad.y, quad.z);
		fprintf(fp, "%d %d %d\n", quad.x, quad.z, quad.w);
	}
	fclose(fp);
	return 1;
}


void CQuadObj::_copyToObject(CQuadObj & aobj, const int objid)
{
	int i, k, c, nply, *vindex;
	float* pid = GetVertexObjectIDPointer();
	assert(pid!=NULL);
	assert(objid>=0);

	//count how many polygons;
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (i=nply=0; i<m_nPolygonCount; i++){
		Vector4i& quad = m_pQuad[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid) nply++;
	}
	assert(nply>0);
	aobj.m_nPolygonCount = nply;
	aobj.m_pPolygon = (int*)(new Vector4i[nply]);
	assert(aobj.m_pPolygon!=NULL);

	//copy the connectivity;
	Vector4i * &pQuad2 = (Vector4i * &)aobj.m_pPolygon;
	for (i=c=0; i<m_nPolygonCount; i++){
		Vector4i& quad = m_pQuad[i];
		const int idx = (int)(pid[quad.x]);
		if (idx==objid)
			pQuad2[c++] = quad;
	}
	assert(c==nply);

	//decide the new vertex buffer;
	vindex = new int [m_nVertexCount];
	assert(vindex!=NULL);
	for (i=0; i<m_nVertexCount; i++) vindex[i]=-1;
	for (i=c=0; i<nply; i++){
		Vector4i& quad = pQuad2[i];
		k = quad.x;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.y;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.z;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.w;
		if (vindex[k]==-1) vindex[k]= c++;
	}

	//copy vertex buffer;
	aobj.m_nVertexCount = c;
	aobj.m_pVertex = new Vector3d[c];
	assert(aobj.m_pVertex!=NULL);
	for (i=0; i<m_nVertexCount; i++){
		k = vindex[i];
		if (k>=0)
			aobj.m_pVertex[k] = m_pVertex[i];
	}

	//fix the connectivity;
	for (i=c=0; i<nply; i++){
		Vector4i& quad = pQuad2[i];
		quad.x = vindex[quad.x];
		quad.y = vindex[quad.y];
		quad.z = vindex[quad.z];
		quad.w = vindex[quad.w];
		if (quad.x==-1||quad.y==-1||quad.z==-1||quad.w==-1)
			assert(0);
	}

	//copy other polyobj class attributes;
	CPolyObj::CopyAttributes(aobj, vindex);
	delete []vindex;
}


void CQuadObj::glDrawElementFaceGoround(const int eid, void * pMirrorTag)
{
	//first, setup the normal
	Vector3f nn=m_pPolyNorm[eid];
	if (pMirrorTag) nn = -nn;
	glNormal3fv(&nn.x);

	//get the vertices
	Vector4i* m_pQuad = (Vector4i *)m_pPolygon;
	const Vector4i quad = m_pQuad[eid];
	Vector3d *v0= &m_pVertex[quad.x];
	Vector3d *v1= &m_pVertex[quad.y];
	Vector3d *v2= &m_pVertex[quad.z];
	Vector3d *v3= &m_pVertex[quad.w];

	if (this->NeedTexture1D() && m_pVertexTexCoor1D){
		const float tx = m_pVertexTexCoor1D[quad.x];
		const float ty = m_pVertexTexCoor1D[quad.y];
		const float tz = m_pVertexTexCoor1D[quad.z];
		const float tw = m_pVertexTexCoor1D[quad.w];
	    glTexCoord1f(tx);
		glVertex3dv(&v0->x);
	    glTexCoord1f(ty);
		glVertex3dv(&v1->x);
	    glTexCoord1f(tz);
		glVertex3dv(&v2->x);
	    glTexCoord1f(tw);
		glVertex3dv(&v3->x);
	}
	else{
		glVertex3dv(&v0->x);
		glVertex3dv(&v1->x);
		glVertex3dv(&v2->x);
		glVertex3dv(&v3->x);
	}
}


void CQuadObj::glDrawElementFacePhong(const int eid, void *pvoid)
{
	assert(0);
}


/*
void CQuadObj::glDrawElementSphere(const int eid, void *p)
{
	ASSERT0(p!=NULL);
	ASSERT0(eid>=0 && eid<m_nVertexCount);
	GLUquadric * quad = (GLUquadric*)p;
	Vector3f& v = m_pVertex[eid];

	if (this->NeedTexture1D()){
		float tx = m_pVertexTexCoor1D[eid];
		glTexCoord1f(tx);
	}

	glPushMatrix();
	glTranslatef(v.x, v.y, v.z);
	float r = m_fGivenRad;
	gluSphere(quad, r, 15, 7);
	glPopMatrix();
}
*/

void CQuadObj::glDrawElementLine(const int eid, const float offset, const bool needtex)
{
	Vector3d p0, p1, p2, p3, *vt = m_pVertex;
	Vector3d displacement(0,0,0);
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	Vector4i &quad = m_pQuad[eid];
	int v0=quad.x, v1=quad.y, v2=quad.z, v3=quad.w;

	if (fabs(offset)>1E-12){
		Vector3f &norm = m_pPolyNorm[eid];
		displacement.x = offset*norm.x;
		displacement.y = offset*norm.y;
		displacement.z = offset*norm.z;
		p0 = vt[v0] + displacement;
		p1 = vt[v1] + displacement;
		p2 = vt[v2] + displacement;
		p3 = vt[v3] + displacement;
	}
	else{
		p0 = vt[v0];
		p1 = vt[v1];
		p2 = vt[v2];
		p3 = vt[v3];
	}
	glBegin(GL_LINE_LOOP);
        if (needtex && m_pVertexTexCoor1D){
		    glTexCoord1f(m_pVertexTexCoor1D[v0]);			
		    glVertex3dv(&p0.x);
		    glTexCoord1f(m_pVertexTexCoor1D[v1]);			
		    glVertex3dv(&p1.x);
		    glTexCoord1f(m_pVertexTexCoor1D[v2]);			
		    glVertex3dv(&p2.x);
		    glTexCoord1f(m_pVertexTexCoor1D[v3]);			
		    glVertex3dv(&p3.x);
        }
        else{
		    glVertex3dv(&p0.x);
		    glVertex3dv(&p1.x);
		    glVertex3dv(&p2.x);
		    glVertex3dv(&p3.x);
        }
    glEnd();
}

void CQuadObj::ComputeVertexNormals(void)
{
    int i;
	Vector3f zero(0,0,0);
	Vector4i* m_pTriangle = (Vector4i*)m_pPolygon;

	if (m_pVertexNorm==NULL){
		m_pVertexNorm = new Vector3f[m_nVertexCount];
		assert(m_pVertexNorm!=NULL);
	}
    for (i=0; i<m_nVertexCount; i++) m_pVertexNorm[i]=zero;

    //need the polygon normals
	ComputePolygonNormals();
 
	for (i=0; i<m_nPolygonCount; i++){
	    const Vector4i t = m_pTriangle[i];
        const Vector3f n = m_pPolyNorm[i];
        m_pVertexNorm[t.x]+= n;
        m_pVertexNorm[t.y]+= n;
        m_pVertexNorm[t.z]+= n;
        m_pVertexNorm[t.w]+= n;
	}

    for (i=0; i<m_nVertexCount; i++)
        m_pVertexNorm[i]=Normalize(m_pVertexNorm[i]);

#ifdef TVCG_DEMO
	setUserNormals(this);
#endif

}

void CQuadObj::ComputePolygonNormals(void)
{
	if (m_pPolyNorm==NULL){ 
		//this is modified for deformable surface
		const int NSIZE = DETERMINE_DEFORMABLE_POLYSIZE(m_nPolygonCount);	
		m_pPolyNorm = new Vector3f[NSIZE];
		assert(m_pPolyNorm != NULL);
	}
	Vector4i *m_pQuad = (Vector4i *)m_pPolygon;
	for (int f=0; f<m_nPolygonCount; f++){
		Vector4i &g = m_pQuad[f];
		Vector3d n = compute_quad_normal(m_pVertex[g.x], m_pVertex[g.y], m_pVertex[g.z], m_pVertex[g.w]);
        m_pPolyNorm[f] = Vector3f(n.x, n.y, n.z);
	}
}


//======================PICKING==========================

void CQuadObj::DrawPickingObject(const int objid)
{
	int i, name = objid;
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;

	SETUP_PICKING_GLENV();
    glPushName(name);
	glBegin(GL_QUADS);
	for (i=0; i<m_nPolygonCount; i++, name++){
		for (int t=0; t<4; t++){
			int p = m_pQuad[i][t];
			glVertex3dv(&m_pVertex[p].x);
		}
	}
	glEnd();
    glPopName();
}

void CQuadObj::DrawPickingObjectFace(const int objid)
{
	int i, name = objid;
	Vector4i *quad;
	Vector3d *v;
	SETUP_PICKING_GLENV();

	//draw the triangles first;
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (i=0; i<m_nPolygonCount; i++, name++){
		quad = &m_pQuad[i];
	    glPushName(name);
		glBegin(GL_QUADS);
			v = &m_pVertex[quad->x];
			glVertex3dv(&v->x);
			v = &m_pVertex[quad->y];
			glVertex3dv(&v->x);
			v = &m_pVertex[quad->z];
			glVertex3dv(&v->x);
			v = &m_pVertex[quad->w];
			glVertex3dv(&v->x);
		glEnd();
	    glPopName();
	}
}


inline void 
DrawPickingQuadEdge(Vector3d vt[], const Vector3d& disp, 
    const int v0, const int v1, const int v2, const int v3, const int Name)
{
	Vector3d p0 = vt[v0] + disp;
	Vector3d p1 = vt[v1] + disp;
	Vector3d p2 = vt[v2] + disp;
	Vector3d p3 = vt[v3] + disp;

    glPushName(Name);
	glBegin(GL_LINES);
		glVertex3dv(&p0.x);
		glVertex3dv(&p1.x);
	glEnd();
    glPopName();

    glPushName(Name+1);
	glBegin(GL_LINES);
		glVertex3dv(&p1.x);
		glVertex3dv(&p2.x);
	glEnd();
    glPopName();

    glPushName(Name+2);
	glBegin(GL_LINES);
		glVertex3dv(&p2.x);
		glVertex3dv(&p3.x);
	glEnd();
    glPopName();

    glPushName(Name+3);
	glBegin(GL_LINES);
		glVertex3dv(&p3.x);
		glVertex3dv(&p0.x);
	glEnd();
    glPopName();
}


void CQuadObj::DrawPickingObjectLine(const int objid)
{
	int i, name = objid;
	SETUP_PICKING_GLENV();
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;

	//draw the triangles first;
    glPushName(-1);
	glBegin(GL_QUADS);
	for (i=0; i<m_nPolygonCount; i++){
		for (int t=0; t<4; t++){
			int p = m_pQuad[i][t];
			glVertex3dv(&m_pVertex[p].x);
		}
	}
	glEnd();
    glPopName();

	//then, draw the lines
	if (this->m_pPolyNorm==NULL)
		this->ComputePolygonNormals();
	const float offset = this->GetHiddenLineOffset();
    glLineWidth(2);
	for (i=0; i<m_nPolygonCount; i++, name+=4){
		Vector3f &norm = m_pPolyNorm[i];
		Vector3d disp;
        disp.x= norm.x * offset; 
        disp.y= norm.y * offset; 
        disp.z= norm.z * offset; 
		Vector4I & quad = m_pQuad[i];
		DrawPickingQuadEdge(m_pVertex, disp, quad.x, quad.y, quad.z, quad.w, name);
	}
}


void CQuadObj::GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
{
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	const int EDGECOUNT = 4;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	int t = eid / EDGECOUNT;
	int e = eid % EDGECOUNT;
	Vector4i tri = m_pQuad[t];
	
	int e1 = (e+1)%EDGECOUNT;
	int x = tri[e];
	int y = tri[e1];
	v0 = m_pVertex[x];
	v1 = m_pVertex[y];
}


void CQuadObj::GetPickedLine(const int eid, int & v0, int &v1)
{
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	const int EDGECOUNT = 4;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	const int t = eid / EDGECOUNT;
	const int e = eid % EDGECOUNT;
	const Vector4i tri = m_pQuad[t];
	const int e1 = (e+1)%EDGECOUNT;
	v0 = tri[e];
	v1 = tri[e1];
}


void CQuadObj::GetPickedPlane(const int eid, Vector3d v[4])
{
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	assert(eid>=0 && eid<m_nPolygonCount);
	Vector4i tri = m_pQuad[eid];
	v[0] = m_pVertex[tri.x];
	v[1] = m_pVertex[tri.y];
	v[2] = m_pVertex[tri.z];
	v[3] = m_pVertex[tri.w];
}


static double _computeMergeVertexErrorTol(const CQuadObj& a)
{
	const int n = a.m_nPolygonCount;
	const int K=4;
	const int n4 = n/K;
	const Vector3d *p = a.m_pVertex;
	double sum=0, sumy=0, sumz=0;
	Vector4i * &pQuadA = (Vector4i * &)a.m_pPolygon;
	for (int i=0; i<K; i++){	
		const Vector4i& e = pQuadA[i*n4];
        sumy += Distance2(p[e.x], p[e.y]);
        sumy += Distance2(p[e.z], p[e.w]);
        sumz += Distance2(p[e.y], p[e.z]);
        sumz += Distance2(p[e.x], p[e.w]);
	}

    const double s1=_MAX_(sumy, sumz);
    const double s2=_MIN_(sumy, sumz) + 1e-9;
    const double ratio=s1/s2;
    if (ratio>4)
        sum= s2;
    else if (ratio<0.25)
        sum= s1;
    else    
		sum=(sumy+sumz)*0.5;
	sum*=1.0/(2*K);
	return sum;
}


double CQuadObj::estimatedVertexRadius(void)
{
    double r2=_computeMergeVertexErrorTol(*this);
    double r = sqrt(r2);
    r *= 0.10;
	return r;
}


//Save the mesh into POVRAY's mesh2 format;
int CQuadObj::exportPovrayFile(FILE *fp, const double *matrix)
{
	Vector4i *t;
	double v[3];
	int i;

	//output vertices;
	fprintf(fp, "mesh2 {\n");
	fprintf(fp, "  vertex_vectors {\n");
	fprintf(fp, "\t%d\n", m_nVertexCount); 
	for (i=0; i<m_nVertexCount-1; i++){
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, v);
		fprintf(fp, "\t<%lf,%lf,%lf>,\n", v[0], v[1], -v[2]);
	}
	TransformVertex3dToVertex3d(m_pVertex[m_nVertexCount-1], matrix, v);
	fprintf(fp, "\t<%lf,%lf,%lf>\n  }\n", v[0], v[1], -v[2]);

	//output triangles
	fprintf(fp, "  face_indices {\n");
	fprintf(fp, "\t%d\n", m_nPolygonCount*2); 

	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
	for (i=0; i<m_nPolygonCount-1; i++){
		t = &m_pQuad[i];
		fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->y, t->z);
		fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->z, t->w);
	}
	t = &m_pQuad[m_nPolygonCount-1];
	fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->y, t->z);
	fprintf(fp, "\t<%d,%d,%d>\n  }\n", t->x, t->z, t->w);
	fprintf(fp, "}\n");
	return 1;
}


void CQuadObj::exportFile(FILE *fp, const char *format, const double *matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0)
		exportPovrayFile(fp, matrix);
}


void CQuadObj::flipNormal(void)
{
    //flip vertex normals and polygon normals;
    CPolyObj::flipNormal();
    //flip quad orientation
	Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
    for (int i=0; i<m_nPolygonCount; i++){
        Vector4i * p = &m_pQuad[i];
        int tmp = p->w;
        p->w = p->y;
        p->y = tmp;
    }
}


//======================================================================

static inline int _arrayindex2d(const int x, const int y, const int nx)
{
    const int i=x+y*nx;
    return i;
}

void getRectangleSurface(const int nx, const int ny, Vector4i *&pQuad, int &nPolygonCount)
{
    nPolygonCount=nx*ny;
    pQuad = new Vector4i[nPolygonCount];
    assert(pQuad!=NULL);
    int count = 0;
    const int nx1=nx+1;
    for (int j=0; j<ny; j++){
        for (int i=0; i<nx; i++){
            Vector4i *p=&pQuad[count++];
            p->x = _arrayindex2d(i, j, nx1);
            p->y = _arrayindex2d(i+1, j, nx1);
            p->z = _arrayindex2d(i+1, j+1, nx1);
            p->w = _arrayindex2d(i, j+1, nx1);
        }
    }
}

void doMesh2dRectangle(
        const Vector3d& lowleft, const Vector3d& upright, 
        const int nx, const int ny, const int nz,
		Vector3d *& m_pVertex, Vector4i *&m_pQuad, 
		int& m_nVertexCount, int& m_nPolygonCount)
{
    assert(nx==0 || ny==0 || nz==0);
	int i, j, k, count;
	m_nVertexCount = (nx+1)*(ny+1)*(nz+1);
	m_pVertex = new Vector3d [m_nVertexCount];
	assert(m_pVertex!=NULL);

	//init the vertices;
    double ddx=0, ddy=0, ddz=0;
    if (nx) ddx=(upright.x-lowleft.x)/nx;
	if (ny) ddy=(upright.y-lowleft.y)/ny;
	if (nz) ddz=(upright.z-lowleft.z)/nz;
    count = 0;
    for (k=0; k<=nz; k++){
        const double dz=k*ddz;
	    for (j=0; j<=ny; j++){
		    const double dy = j*ddy;
		    for (i=0; i<=nx; i++){
			    const double dx = i*ddx;
			    Vector3d& v = m_pVertex[count++];
			    v.x = lowleft.x+dx;
			    v.y = lowleft.y+dy;
			    v.z = lowleft.z+dz;
		    }
	    }
    }
	//get the boundary surface
    int NX, NY;
    if (nz==0)
        NX=nx, NY=ny;
    else if (ny==0)
        NX=nx, NY=nz;
    else
        NX=ny, NY=nz;
    getRectangleSurface(NX, NY, m_pQuad, m_nPolygonCount);
}


//=================================Merge Two Objects=========================================
static inline int 
_searchDupVertexInArray(const Vector3d& p, const Vector3d *a, const int alen, const double TOLSQRDIS)
{
	for (int i=0; i<alen; i++){
		if (Distance2(p, a[i])<TOLSQRDIS)
			return i;
	}
	return -1;
}


static inline double 
_computeDupVertexErrorTol(const CQuadObj& a, const CQuadObj& b, const double &zerotol)
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

//Merge quad obj a and b to obj c
static void __mergeQuadObj(CQuadObj & obja, CQuadObj & objb, CQuadObj &objc, const double errscale)
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

	//find the dup vertices in object b
    if (nva<50){
	    for (i=0; i<nvb; i++){	//find ref vertices for array B
		    const int j=i;
		    const int pos=_searchDupVertexInArray(pvb[j], pva, nva, TOLSQRDIS);
		    pVertIndexB[j] = pos;
	    }
    }
    else{//for large array A, we do spatial sorting before the search 
        CPointOrgnizer3D pointset(pva, nva);
	    for (i=0; i<nvb; i++){
		    const int j=i;
		    const int pos=pointset.searchPoint(pvb[j], TOLSQRDIS);
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

	delete [] pVertIndexA;
	delete [] pVertIndexB;
}


void MergeQuadObj(CQuadObj & obja, CQuadObj & objb, CQuadObj &objc, const double errscale)
{
    if (obja.m_nVertexCount>objb.m_nVertexCount)
        __mergeQuadObj(obja, objb, objc, errscale);
    else
        __mergeQuadObj(objb, obja, objc, errscale);
}



//===========================================================

static int _loadQuadFile(FILE *fp, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad)
{
	int i, nVerticeFieldNumber;
	double x, y, z;

	fscanf(fp, "%d", &nv);
	fscanf(fp, "%d", &nquad);
	fscanf(fp, "%d", &nVerticeFieldNumber);
	assert(nVerticeFieldNumber==1);

	pVertex = new Vector3d[nv];
	pQuad = new Vector4i[nquad];
	assert(pVertex != NULL);
	assert(pQuad != NULL);

	for (i=0; i<nv; i++){
		const int count=fscanf(fp, "%lf %lf %lf", &x,&y,&z);
		if (count != 3){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			goto CLEANUP;
		}
		pVertex[i] = Vector3d(x, y, z);
	}

	for (i=0; i<nquad; i++){
		int tx, ty, tz, tw;
		const int count= fscanf(fp, "%d %d %d %d", &tx, &ty, &tz, &tw);
		if (count!=4 || tx<0 || ty<0 || tz<0 || tw<0){
			fprintf(stderr, "Error: reading triangle %d connectivity!\n", i);
			goto CLEANUP;
		}
		pQuad[i]=Vector4i(tx, ty, tz, tw);
	}
	return 1;

CLEANUP:	//error handling
	SafeDeleteArray(pVertex);
	SafeDeleteArray(pQuad);
	pVertex = NULL;
	pQuad = NULL;
	return 0;
}

static CPolyObj* loadQuadFile(const char *fname)
{
	CQuadObj *obj = NULL;
	Vector3d* pVertex;
	Vector4i* pQuad;
	int nv, nquad;

	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return obj;
	if (_loadQuadFile(fp, pVertex, nv, pQuad, nquad))
		obj = new CQuadObj(pVertex, nv, pQuad, nquad, false);
	fclose(fp);
	return obj;
}


static void _saveQuadFile(FILE *fp, const Vector3d* pVertex, const int nv, const Vector4i* pQuad, const int nquad, const double *matrix)
{
	int i;
	const int nVerticeFieldNumber=1;
	fprintf(fp, "%d ", nv);
	fprintf(fp, "%d ", nquad);
	fprintf(fp, "%d\n", nVerticeFieldNumber);

	for (i=0; i<nv; i++){
		Vector3d v;
		if (matrix)
			TransformVertex3dToVertex3d(pVertex[i], matrix, &v.x);
		else
			v = pVertex[i];
		const double x = v.x;
		const double y = v.y;
		const double z = v.z;
		fprintf(fp, "%lg %lg %lg\n", x, y, z);
	}
	for (i=0; i<nquad; i++){
		const Vector4i *v = &pQuad[i];
		const int tx = v->x;
		const int ty = v->y;
		const int tz = v->z;
		const int tw = v->w;
		fprintf(fp, "%d %d %d %d\n", tx, ty, tz, tw);
	}
}

void saveQuadFile(CQuadObj *obj, const char *fname, const double *matrix)
{
	if (obj==NULL) return;
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return;
	Vector4i *pQuad = (Vector4i*)obj->m_pPolygon;
	_saveQuadFile(fp, obj->m_pVertex, obj->m_nVertexCount, pQuad, obj->m_nPolygonCount, matrix);
	fclose(fp);
}

static bool _int_loadquad = CPolyObjLoaderFactory::AddEntry(".quad", loadQuadFile);

