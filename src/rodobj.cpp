//rodobj.cpp
#include <math.h>
#include <vectorall.h>
#include "rodobj.h"

inline void 
DrawLineObject(Vector3d *v, const int nv, Vector2i *l, const int nl, const Vector3f &lclr=Vector3f(0), const float lthick=1)
{
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glColor3fv(&lclr.x);
	glLineWidth(lthick);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &v[0].x);
	glDrawElements(GL_LINES, 2*nl, GL_UNSIGNED_INT, &l[0].x);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void CRodObj::glDraw(const CGLDrawParms &dparms)
{
	m_pDrawParms = (CGLDrawParms*)(&dparms);
	const Vector3f lclr=dparms.m_cHLineColor;
	const float lthick = dparms.m_fLineWidth;

	//draw the lines;
	switch (dparms.m_nDrawType){
		case CGLDrawParms::DRAW_MESH_HIDDENLINE:
		case CGLDrawParms::DRAW_MESH_LINE:
		case CGLDrawParms::DRAW_MESH_SHADING:
			DrawLineObject(m_pVertex, m_nVertexCount, (Vector2i*)m_pElement, m_nElementCount, lclr, lthick);
			break;
		case CGLDrawParms::DRAW_MESH_NONE:
			break;
	}
		
	//draw the vertices;
	if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
		if (m_fGivenRad<1e-12f) 
			m_fGivenRad=estimatedVertexRadius();
		glDrawVertices(dparms.m_cObjectColor);
	}

	if (dparms.m_bShowVertexAttribValue){
		//_DrawVertexAttributeText();
	}	
}



int CRodObj::exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer)
{
    const int ELMTYPEID =7;   //=======ID for rod element
	Vector2i* pTriangle = (Vector2i*)m_pElement;

	assert(pTriangle!=NULL);
	assert(m_nElementCount!=0);
	for (int i=0; i<m_nElementCount; i++){
		const int idx = tetbaseno+i;
		const Vector2i *pt = &pTriangle[i];
		const int vx = vbaseno + pt->x;
		const int vy = vbaseno + pt->y;
		//================================================
		//Format: index#, setid, objid, nodeX, nodeY
		//   setid:  1: FEM
		//           7: TRUSS
		//================================================
		fprintf(fp, "%d %d %d %d %d\n", idx, ELMTYPEID, objsetid, vx, vy);
	}
	return m_nElementCount;
}



int CRodObj::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double thickness)
{
	extern void exportPlyVertices(FILE *fp, Vector3d *pVertex, const int nv, double _th, double *pThick, const int baseno, const double *pmatrix);
    exportPlyVertices(fp, m_pVertex, m_nVertexCount, thickness, NULL, baseno, pmatrix);
    return m_nVertexCount;
}



int CRodObj::SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
{
	int i;
	Vector3d lowleft, upright;
	for (i=0; i<bufflen; i++){
		CRodObj *p = new CRodObj;
		assert(p!=NULL);
		_copyToObject(*p, objidbuff[i]);
		p->ComputeBoundingBox(lowleft, upright);
		p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
		obj[i] = p;
	}
	return bufflen;
}


void CRodObj::_copyToObject(CRodObj & aobj, const int objid)
{
	/*
	Vector2i*& m_pTriangle = (Vector2i*&)m_pPolygon;
	int i, k, c, nply, *vindex;
	float* pid = GetVertexObjectIDPointer();
	assert(pid!=NULL);
	assert(objid>=0);

	//count number of lines;
	for (i=nply=0; i<m_nPolygonCount; i++){
		Vector2i& quad = m_pTriangle[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid) nply++;
	}
	assert(nply>0);
	aobj.m_nPolygonCount = nply;
	aobj.m_pPolygon = (int*)(new Vector2i[nply]);
	assert(aobj.m_pPolygon!=NULL);
	Vector2i*& m_pTriangle2 = (Vector2i*&)aobj.m_pPolygon;

	//copy the connectivity;
	for (i=c=0; i<m_nPolygonCount; i++){
		Vector2i& quad = m_pTriangle[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid)
			m_pTriangle2[c++] = quad;
	}
	assert(c==nply);

	//decide the new vertex buffer;
	vindex = new int [m_nVertexCount];
	assert(vindex!=NULL);
	for (i=0; i<m_nVertexCount; i++) vindex[i]=-1;
	for (i=c=0; i<nply; i++){
		Vector2i& quad = m_pTriangle2[i];
		k = quad.x;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.y;
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
		Vector2i& quad = m_pTriangle2[i];
		quad.x = vindex[quad.x];
		quad.y = vindex[quad.y];
		if (quad.x==-1||quad.y==-1)
			assert(0);
	}

	//copy other polyobj class attributes;
	CPolyObj::CopyAttributes(aobj, vindex);
	delete [] vindex;
	*/
}


void CRodObj::DrawPickingObjectFace(int)
{
}

void CRodObj::DrawPickingObject(int objid)
{
	SETUP_PICKING_GLENV();
    glPushName(objid);
	DrawLineObject(m_pVertex, m_nVertexCount, (Vector2i*)m_pElement, m_nElementCount);
    glPopName();
}

void CRodObj::DrawPickingObjectLine(const int objid)
{
	/*
	Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;
	int i, name = objid;
	SETUP_PICKING_GLENV();

	//draw the triangles first;
    glPushName(-1);
	glBegin(GL_TRIANGLES);
	for (i=0; i<m_nPolygonCount; i++){
		for (int t=0; t<3; t++){
			const int p = m_pTriangle[i][t];
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
	for (i=0; i<m_nPolygonCount; i++, name+=3){
		Vector3f &norm = m_pPolyNorm[i];
		Vector3f disp = norm * offset; 
        Vector3d disp1(disp.x, disp.y, disp.z);
		Vector3I & tri = m_pTriangle[i];
		DrawPickinigTrianleEdge(m_pVertex, disp1, tri.x, tri.y, tri.z, name);
	}
	*/
}


void CRodObj::GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
{
	/*
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	const int EDGECOUNT = 3;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	int t = eid / EDGECOUNT;
	int e = eid % EDGECOUNT;
	Vector3i tri = m_pTriangle[t];
	
	int e1 = (e+1)%EDGECOUNT;
	int x = tri[e];
	int y = tri[e1];
	v0 = m_pVertex[x];
	v1 = m_pVertex[y];
	*/
}



void CRodObj::GetPickedLine(const int eid, int & v0, int &v1)
{
	/*
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	const int EDGECOUNT = 3;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	const int t = eid / EDGECOUNT;
	const int e = eid % EDGECOUNT;
	const Vector3i tri = m_pTriangle[t];
	
	const int e1 = (e+1)%EDGECOUNT;
	v0 = tri[e];
	v1 = tri[e1];
	*/
}


void CRodObj::GetPickedPlane(const int eid, Vector3d v[4])
{
}


int CRodObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib)
{
	int i, count;

	//read vertices;
	m_nVertexCount = nv;
	LoadPltVertices(fp, nv, nattrib);

	//read the triangles;
	m_nElementCount = nf;
	Vector2i*& pTriangle = (Vector2i*&)m_pElement;
	pTriangle = new Vector2i[nf];
	assert(pTriangle!=NULL);

	for (i=0; i<nf; i++){
		int n0, n1;
		count= fscanf(fp, "%d %d", &n0, &n1);
		n0--, n1--;
		if (count!=2 || n0<0 || n1<0){
			fprintf(stderr, "Error: reading line %d\n", i);
			exit(0);
		}
		pTriangle[i] = Vector2i(n0, n1);
	}

	return 1;
}



double CRodObj::estimatedVertexRadius(void)
{
	Vector2i* pTriangle = (Vector2i*)m_pElement;
	assert(pTriangle!=NULL);
	int c=0;

	double len=0;
	for (int i=0; i<m_nElementCount; i+=2){
		const Vector2i *pt = &pTriangle[i];
		const int vx = pt->x;
		const int vy = pt->y;
		c++;
		const double r= Distance2(m_pVertex[vx], m_pVertex[vy]);
		len +=sqrt(r);
	}

	if (c) len = len/c *0.02;
	return len;
}

