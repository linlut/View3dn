//FILE: springcontainer.cpp

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <math.h>
#include <vectorall.h>
#include "scenenode.h"
#include "springcontainer.h"


void CSpringContainer::addSpring(const CConstaintSpring & spring)
{
	if (!_springInBuffer(spring)){
		springs.push_back(spring);
	}
	m_nElementCount = springs.size();
}

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

inline Vector3d _getVertexWorldPosition(CSceneNode *p, const int vid)
{
	CPolyObj *pobj = p->m_pObject;
	Vector3d r = pobj->m_pVertex[vid];
	if (p->m_pFrame){
		Vector3d rr;
		const double *toworld = p->m_pFrame->worldMatrix();
	    TransformVertex3dToVertex3d(r, toworld, &rr.x);
		r = rr;
	}
	return r;
}

void CSpringContainer::glDraw(const CGLDrawParms &dparms)
{
	const int nsize = springs.size();
	if (nsize==0) return;

	const Vector3f lclr=dparms.m_cHLineColor;
	const float lthick = dparms.m_fLineWidth;
		
	//draw the vertices;
	glColor3fv(&lclr.x);
	glLineWidth(lthick);
	glEnable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
	for (int i=0; i<nsize; i++){
		CConstaintSpring *s = &springs[i];
		Vector3d v1= _getVertexWorldPosition(s->m_pSceneNode1, s->m_nVertexID1);
		Vector3d v2= _getVertexWorldPosition(s->m_pSceneNode2, s->m_nVertexID2);
		glVertex3dv(&v1.x);
		glVertex3dv(&v2.x);
	}
	glEnd();
}


int CSpringContainer::exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer)
{
    const int ELMTYPEID =7;   //=======ID for rod element
	const int nsize = springs.size();
	if (nsize==0) return 0;

	for (int i=0; i<nsize; i++){
		CConstaintSpring *s = &springs[i];
		const int idx = tetbaseno + i;
		const int vbase1 = s->m_pSceneNode1->m_FemAttrib.m_nAccVertexCount;
		const int vbase2 = s->m_pSceneNode2->m_FemAttrib.m_nAccVertexCount;
		const int vx = vbase1 + s->m_nVertexID1;
		const int vy = vbase2 + s->m_nVertexID2;
		//================================================
		//Format: index#, setid, objid, nodeX, nodeY
		//   setid:  1: FEM
		//           7: TRUSS
		//================================================
		fprintf(fp, "%d %d %d %d %d\n", idx, ELMTYPEID, objsetid, vx, vy);
	}
	return nsize;
}



int CSpringContainer::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double thickness)
{
	assert(m_nVertexCount==0);
    return m_nVertexCount;
}


void CSpringContainer::DrawPickingObjectFace(int)
{
}

void CSpringContainer::DrawPickingObject(int objid)
{
	//SETUP_PICKING_GLENV();
    //glPushName(objid);
	//DrawLineObject(m_pVertex, m_nVertexCount, (Vector2i*)m_pElement, m_nElementCount);
    //glPopName();
}

void CSpringContainer::DrawPickingObjectLine(const int objid)
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


void CSpringContainer::GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
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



void CSpringContainer::GetPickedLine(const int eid, int & v0, int &v1)
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


void CSpringContainer::GetPickedPlane(const int eid, Vector3d v[4])
{
}


int CSpringContainer::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib)
{
	/*
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
			fprintf(stderr, "Error: reading triangle %d\n", i);
			exit(0);
		}
		pTriangle[i] = Vector2i(n0, n1);
	}
*/
	return 1;
}



double CSpringContainer::estimatedVertexRadius(void)
{
	/*
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
	*/
	return 1;
}

