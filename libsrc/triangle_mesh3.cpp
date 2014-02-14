/*
 triangle_mesh.cpp
*/
#include "sysconf.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "ply.h"
#include "vectorall.h"
#include "dynamic_array.h"
#include "drawparms.h"
#include "triangle_mesh.h"
#include "geomath.h"
#include "matrix.h"

#include <math.h>
#define Vector3D Vector3f


inline void BUILD_VERTEX_NORMAL(Vector3D v[], const int nVert, Vector3I t[], const int ntri, Vector3D norm[])
{
	int i;

	for (i=0; i<nVert; i++) 
		norm[i]=Vector3D(0,0,0);

	for (i=0; i<ntri; i++){
		int a = t[i].x;
		int b = t[i].y;
		int c = t[i].z;
		Vector3D n = compute_triangle_normal(v[a], v[b], v[c]);
		norm[a] += n;
		norm[b] += n;
		norm[c] += n;
	}
	for (i=0; i<nVert; i++) 
		norm[i]=Normalize(norm[i]);
}


inline void CTriangleMesh::DrawFlatTriangle(int i)
{
	int  p, t;
	Vector3D *v;
	unsigned char *rgb;

	for (t=0; t<3; t++){
		p = m_Triangles[i][t];
		if ((m_pVertexColor!=NULL) && (m_pDrawParms->m_bEnableTexture1D)){
			rgb= m_pVertexColor[p];
			glColor3ub(rgb[0], rgb[1], rgb[2]);
		}
		//set normal
		v = &m_pVertexNorm[p];
		glNormal3fv((GLfloat*)v);
		//set vertex
		v = &m_Vertice[p];
		glVertex3fv((GLfloat*)v);
	}
}



inline void CTriangleMesh::DrawTriangleWithTextured1D(const int i)
{
	const float K = 1.0f/255;
	Vector3I *p = &m_Triangles[i];
	const int v1 = p->x; 
	const int v2 = p->y;
	const int v3 = p->z;

	if (m_pDrawParms->m_bEnableLighting){
		if (m_pTriangleNorm){
			Vector3D *pnorm = &m_pTriangleNorm[i];
			glNormal3fv((GLfloat*)pnorm);
		}
	}

	Vector3D& a = m_Vertice[v1];
	Vector3D& b = m_Vertice[v2];
	Vector3D& c = m_Vertice[v3];

	int ix = m_pVertexColor[v1][3];
	int iy = m_pVertexColor[v2][3];
	int iz = m_pVertexColor[v3][3];
	float tx=ix*K;
	float ty=iy*K;
	float tz=iz*K;
    glTexCoord1f(tx);
    glVertex3f(a.x, a.y, a.z);
    glTexCoord1f(ty);
    glVertex3f(b.x, b.y, b.z);
    glTexCoord1f(tz);
    glVertex3f(c.x, c.y, c.z);
}


/*
inline void CTriangleMesh::DrawTriangleWithTextured1D(const int i)
{
	Vector3I *p = &m_Triangles[i];
	const int v1 = p->x; 
	const int v2 = p->y;
	const int v3 = p->z;

	Vector3D& a = m_Vertice[v1];
	Vector3D& b = m_Vertice[v2];
	Vector3D& c = m_Vertice[v3];
	
	int idx0 = m_pVertexColor[v1][3];
	int idx1 = m_pVertexColor[v2][3];
	int idx2 = m_pVertexColor[v3][3];
	unsigned char *pc0= m_DrawParms.m_texture1D[idx0];
	unsigned char *pc1= m_DrawParms.m_texture1D[idx1];
	unsigned char *pc2= m_DrawParms.m_texture1D[idx2];

	if (!m_DrawParms.m_bEnableLighting){
		glColor3ub(pc0[0], pc0[1], pc0[2]);
		glVertex3fv((float*)&a);
		glColor3ub(pc1[0], pc1[1], pc1[2]);
		glVertex3fv((float*)&b);
		glColor3ub(pc2[0], pc2[1], pc2[2]);
		glVertex3fv((float*)&c);
	}
	else{
		if (m_pTriangleNorm){
			Vector3D *pnorm = &m_pTriangleNorm[i];
			glNormal3fv((GLfloat*)pnorm);
		}
		const float K = 1.0f/255;
		float matDiffuse[3]={pc0[0], pc0[1], pc0[2]};
		matDiffuse[0]*=K, 
		matDiffuse[1]*=K, 
		matDiffuse[2]*=K;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  matDiffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  matDiffuse);
		glVertex3fv((float*)&a);
		glVertex3fv((float*)&b);
		glVertex3fv((float*)&c);
	}
}
*/


void CTriangleMesh::DrawFlatSurfaces(void)
{
	if (m_pVertexNorm == NULL){
		m_pVertexNorm = new Vector3D[m_nVerticesNumber];
		if (m_pVertexNorm==NULL)
			return;
		BUILD_VERTEX_NORMAL(m_Vertice, m_nVerticesNumber, m_Triangles, m_nTriangleNumber, m_pVertexNorm);
	}

	glBegin(GL_TRIANGLES);
	for (int f=0; f<m_nTriangleNumber; f++){
		DrawFlatTriangle(f);
	}
	glEnd();
}

void CTriangleMesh::DrawHiddenlineSurfaces(const float color[3])
{
	//float c[3]={1, 1, 1};
    //glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glLineWidth(1);
	for (int f=0; f<m_nTriangleNumber; f++){
		DrawHiddenlineTriangle(f, color);
	}
    glDisable(GL_STENCIL_TEST);
}


void CTriangleMesh::DrawHiddenlineTriangle(const int i, const float color[3])
{
	//disable using face color
	unsigned char (*tmp)[4] = m_pVertexColor;
	float *linecolor = (float*)&(m_pDrawParms->m_cHLineColor);
	float *plycolor = (float*)&(m_pDrawParms->m_cObjectColor);
	m_pVertexColor = NULL;

	glLineWidth(1.2f);
    glDisable(GL_LIGHTING);
    glColor3fv(linecolor);
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	DrawFlatTriangle(i);
	glEnd();

    glColor3fv(plycolor);
    glStencilFunc(GL_EQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);
	DrawFlatTriangle(i);
	glEnd();

    glColor3fv(linecolor);
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	DrawFlatTriangle(i);
	glEnd();

	m_pVertexColor = tmp;
}




//===================LINE & HIDDENLINE  DRAWING===========================
inline void DrawOffsetTrianleEdge(Vector3D vt[], const Vector3D &norm, const int v0, const int v1, const int v2, const Vector3D& displacement)
{
	Vector3D p0 = vt[v0] + displacement;
	Vector3D p1 = vt[v1] + displacement;
	Vector3D p2 = vt[v2] + displacement;
	glNormal3fv((GLfloat*)&norm);
	if (v0<v1){
		glVertex3fv((float*)&p0);
		glVertex3fv((float*)&p1);
	}
	if (v1<v2){
		glVertex3fv((float*)&p1);
		glVertex3fv((float*)&p2);
	}
	if (v2<v0){
		glVertex3fv((float*)&p2);
		glVertex3fv((float*)&p0);
	}
}

inline void DrawOffsetQuadEdge(Vector3D vt[], const Vector3D& norm, const int v0, const int v1, const int v2, const int v3, const Vector3D& displacement)
{
	Vector3D p0 = vt[v0] + displacement;
	Vector3D p1 = vt[v1] + displacement;
	Vector3D p2 = vt[v2] + displacement;
	Vector3D p3 = vt[v3] + displacement;

	glNormal3fv((GLfloat*)&norm);
	if (v0<v1){
		glVertex3fv((float*)&p0);
		glVertex3fv((float*)&p1);
	}
	if (v1<v2){
		glVertex3fv((float*)&p1);
		glVertex3fv((float*)&p2);
	}
	if (v2<v3){
		glVertex3fv((float*)&p2);
		glVertex3fv((float*)&p3);
	}
	if (v3<v0){
		glVertex3fv((float*)&p3);
		glVertex3fv((float*)&p0);
	}
}


void CTriangleMesh::DrawMeshLine(const CGLDrawParms *dparms, const float offset)
{
	int i;
	Vector3D disp;

	//ensure the normals exist
	ComputeMeshNormals();

	//then, draw the lines
    glDisable(GL_LIGHTING);
	glColor3fv((float*)&dparms->m_cHLineColor);
    glLineWidth(dparms->m_fLineWidth);
	glBegin(GL_LINES);
	if (m_pQuads ==NULL){
		for (i=0; i<m_nTriangleNumber; i++){
			Vector3D &norm = m_pTriangleNorm[i];
			disp = norm * offset;
			Vector3I & tri = m_Triangles[i];
			DrawOffsetTrianleEdge(m_Vertice, norm, tri.x, tri.y, tri.z, disp);
		}
	}
	else{
		for (i=0; i<m_nQuads; i++){
			Vector3D &norm = m_pTriangleNorm[i*2];
			disp = norm * offset;
			Vector4I & quad = m_pQuads[i];
			DrawOffsetQuadEdge(m_Vertice, norm, quad.x, quad.y, quad.z, quad.w, disp);
		}
	}
	glEnd();
}


void CTriangleMesh::DrawMeshHiddenLine(CGLDrawParms *dparms)
{
	m_pDrawParms = dparms;
	float* isocolor = (float*)&dparms->m_cObjectColor;
	int smooth = dparms->m_bUseVertexNormal;

	//compute triangle normal;
	ComputeMeshNormals();

	//draw the flat surface first;
	dparms->setupMaterial();
	DrawShadedSurfaces(smooth);

	//draw offset lines;
	DrawMeshOffsetLine(dparms);
}



void CTriangleMesh::DrawMeshOffsetLine(CGLDrawParms *dparms)
{
	glDisable(GL_TEXTURE_1D);
    glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	Vector3D dim = upright-lowleft;
	float maxsize = _MAX3_(dim.x, dim.y, dim.z);
	float offset = maxsize * 3e-4f;
	DrawMeshLine(dparms, offset);
}

//===============HIDDENLINE===================================



//===============PICKING===================================
inline void DrawPickinigTrianleEdge(Vector3D vt[], const Vector3D& disp, const int v0, const int v1, const int v2, const int Name)
{
	Vector3D p0 = vt[v0] + disp;
	Vector3D p1 = vt[v1] + disp;
	Vector3D p2 = vt[v2] + disp;

    glPushName(Name);
	glBegin(GL_LINES);
	glVertex3fv((float*)&p0);
	glVertex3fv((float*)&p1);
	glEnd();
    glPopName();

    glPushName(Name+1);
	glBegin(GL_LINES);
	glVertex3fv((float*)&p1);
	glVertex3fv((float*)&p2);
	glEnd();
    glPopName();

    glPushName(Name+2);
	glBegin(GL_LINES);
	glVertex3fv((float*)&p2);
	glVertex3fv((float*)&p0);
	glEnd();
    glPopName();
}

inline void DrawPickingQuadEdge(Vector3D vt[], const Vector3D& disp, const int v0, const int v1, const int v2, const int v3, const int Name)
{
	Vector3D p0 = vt[v0] + disp;
	Vector3D p1 = vt[v1] + disp;
	Vector3D p2 = vt[v2] + disp;
	Vector3D p3 = vt[v3] + disp;

    glPushName(Name);
	glBegin(GL_LINES);
		glVertex3fv((float*)&p0);
		glVertex3fv((float*)&p1);
	glEnd();
    glPopName();

    glPushName(Name+1);
	glBegin(GL_LINES);
		glVertex3fv((float*)&p1);
		glVertex3fv((float*)&p2);
	glEnd();
    glPopName();

    glPushName(Name+2);
	glBegin(GL_LINES);
		glVertex3fv((float*)&p2);
		glVertex3fv((float*)&p3);
	glEnd();
    glPopName();

    glPushName(Name+3);
	glBegin(GL_LINES);
		glVertex3fv((float*)&p3);
		glVertex3fv((float*)&p0);
	glEnd();
    glPopName();
}

void CTriangleMesh::DrawMeshLines4Picking(void)
{
	int i, name = 0;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

	//draw the triangles first;
    glPushName(-1);
	glBegin(GL_TRIANGLES);
	for (i=0; i<m_nTriangleNumber; i++){
		for (int t=0; t<3; t++){
			int p = m_Triangles[i][t];
			Vector3D* v = &m_Vertice[p];
			glVertex3fv((GLfloat*)v);
		}
	}
	glEnd();
    glPopName();

	//then, draw the lines
	Vector3D dim = upright-lowleft;
	float maxsize = _MAX3_(dim.x, dim.y, dim.z);
	float offset = maxsize * 3e-4f;

    glLineWidth(2);
	if (m_pQuads ==NULL){
		if (m_pTriangleNorm){
			for (i=0; i<m_nTriangleNumber; i++, name+=3){
				Vector3D &norm = m_pTriangleNorm[i];
				Vector3D disp = norm * offset; 
				Vector3I & tri = m_Triangles[i];
				DrawPickinigTrianleEdge(m_Vertice, disp, tri.x, tri.y, tri.z, name);
			}
		}
	}
	else{
		if (m_pTriangleNorm){
			for (i=0; i<m_nQuads; i++, name+=4){
				Vector3D &norm = m_pTriangleNorm[i*2];
				Vector3D disp = norm * offset; 
				Vector4I & quad = m_pQuads[i];
				DrawPickingQuadEdge(m_Vertice, disp, quad.x, quad.y, quad.z, quad.w, name);
			}
		}
	}
}

bool CTriangleMesh::GetPickedLine(const int edgeid, Vector3D &v0, Vector3D &v1)
{
	int edgecount, nelm;
	int i, j, *ptr;

	if (m_pQuads == NULL){
		edgecount = 3;
		nelm = m_nTriangleNumber;
	}
	else{
		edgecount = 4;
		nelm = m_nQuads;
	}

	i = edgeid / edgecount;
	j = edgeid % edgecount;
	if (i> nelm) return false;

	if (edgecount==3)
		ptr = (int*)(&m_Triangles[i]);
	else
		ptr = (int*)(&m_pQuads[i]);
	int a = ptr[j];
	int b = ptr[(j+1)%edgecount];
	v0 = m_Vertice[a];
	v1 = m_Vertice[b];
	return true;
}


//================================PICKING===================================


/*
void CTriangleMesh::DrawHiddenlineSurfaces(float isocolor[3])
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glLineWidth(1);

	//draw the edge lines
    glColor3f(0, 0, 0);
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawFlatSurfaces();

	//
    glColor3fv(isocolor);
    glStencilFunc(GL_EQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
	DrawFlatSurfaces();

    glColor3f(0, 0, 0);
    glStencilFunc(GL_ALWAYS, 0, 1);
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
	DrawFlatSurfaces();
//***********************
     // Render the mesh into the stencil buffer.
     glEnable(GL_STENCIL_TEST);
     glStencilFunc(GL_ALWAYS, 1, -1);
     glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
     glColor3fv(isocolor);
 	 DrawFlatSurfaces();

     // Render the thick wireframe version.
     glColor3f(1, 1, 0);
     glStencilFunc(GL_NOTEQUAL, 1, -1);
     glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
     glLineWidth(3);
     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
 	 DrawFlatSurfaces();
     glDisable(GL_STENCIL_TEST);
}
*/



//====================================================================

void CTriangleMesh::DrawUnsmoothedFlatSurfaces(void)
{
	int f, p, t;
	Vector3D *v[3];
	Vector3D v1, v2; 
	float rgb[3][3];
	const float k=(float)(1/255.0);
	bool cflag = m_pVertexColor && m_pDrawParms->m_bEnableTexture1D;

	//compute mesh normal;
	ComputeMeshNormals();

	if (m_pDrawParms->m_bEnableLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glBegin(GL_TRIANGLES);
	for (f=0; f<m_nTriangleNumber; f++){
		for (t=0; t<3; t++){
			p = m_Triangles[f][t];
			v[t] = &m_Vertice[p];
			if (cflag){
				rgb[t][0] = m_pVertexColor[p][0]*k;
				rgb[t][1] = m_pVertexColor[p][1]*k;
				rgb[t][2] = m_pVertexColor[p][2]*k;
			}
		}
		//set triangle normal
		glNormal3fv((GLfloat*)&m_pTriangleNorm[f]);

		//set vertices;
		if (cflag){
			glColor3fv(rgb[0]);
			glVertex3fv((GLfloat*)v[0]);
			glColor3fv(rgb[1]);
			glVertex3fv((GLfloat*)v[1]);
			glColor3fv(rgb[2]);
			glVertex3fv((GLfloat*)v[2]);
		}
		else{
			glVertex3fv((GLfloat*)v[0]);
			glVertex3fv((GLfloat*)v[1]);
			glVertex3fv((GLfloat*)v[2]);
		}
	}
	glEnd();

}


void CTriangleMesh::DrawShadedSurfaces(const int smooth)
{
	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_1D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    //glPolygonMode( GL_FRONT, GL_FILL);
    //glPolygonMode( GL_BACK, GL_LINE);

	//for blending
    glDisable(GL_BLEND);
    //glEnable(GL_BLEND);
	//glDepthMask(GL_FALSE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//RGBColor clr = m_pDrawParms->m_cObjectColor;
	//glColor3f(clr.x, clr.y, clr.z);
	m_pDrawParms->setupMaterial();


	if (smooth == 0)
		DrawUnsmoothedFlatSurfaces();
	else
		DrawFlatSurfaces();
}



void CTriangleMesh::DrawSurfaceLines(const float isocolor[3])
{
	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
	glColor3fv(isocolor);
	DrawFlatSurfaces();
}


/*
void ZGenRotationMatrix(const Vector3d& z, Matrix2d &m)
{
	Vector3d x, y;
	double len;
	
	x = Vector3d(1, 0, 0);
	y= CrossProd(z, x);
	len = Magnitude(y);
	if (len<1E-10){
		x = Vector3d(0, 1, 0);
		y= CrossProd(z, x);
	}
	y = Normalize(y);
	x = CrossProd(y, z);
	x = Normalize(x);
	GenRotationMatrix(x, y, z, m);
	m.Transpose();
}
*/

void CTriangleMesh::DrawSurfaceAndPoints(const float isocolor[3], const int smooth)
{	
	int i;
	unsigned char ptcolor[3]={200, 60, 60};

	glDisable(GL_LIGHTING);
	glPointSize(4);
	glBegin(GL_POINTS);
	glColor3ub(ptcolor[0], ptcolor[1], ptcolor[2]);
	for (i=0; i<m_nVerticesNumber; i++){
		//glNormal3fv((GLfloat*)&m_pVertexNorm[i]);
		glVertex3fv((float*)&m_Vertice[i]);
	}
	glEnd();
}


/*
void CTriangleMesh::DrawSurfaceAndPoints(const float isocolor[3], const int smooth)
{
	const float k = ((float)1.0)/255;
	//Draw the surfaces first;
	//DrawShadedSurfaces(smooth);

	//radius
	Vector3D dist, rad;
	float r;
	dist= upright - lowleft;
	rad = dist / sqrt(m_nVerticesNumber);
	//r = _MAX3_(rad.x, rad.y, rad.z) * 0.6;
	r = _MAX3_(rad.x, rad.y, rad.z) * 0.2;

	//compute offset;
	Vector3D diff = upright - lowleft;
	double offset = _MAX3_(fabs(diff.x), fabs(diff.y), fabs(diff.z));
	offset*= 3.3e-5;

	//Then, draw the points;
	int i;
	GLUquadric *quad;
	Vector3D off;
	unsigned char rgb[3]={200, 60, 60};
	Matrix m;

	glEnable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_FILL);
	for (i=0; i<m_nVerticesNumber; i++){
		Vector3D &v = m_Vertice[i];
		//rgb = &m_pVertexColor[i][0];
		glPushMatrix();

		//matrix 
		GenRotationMatrix(m_pVertexNorm[i], m);
		off =v+ m_pVertexNorm[i]*offset;
		glTranslatef(off.x, off.y, off.z);
		glMultMatrixf((float*)&m);

		//draw obj
		gluSphere(quad, r, 6, 6);
		//clr[0] = rgb[0]*k;
		//clr[1] = rgb[1]*k;
		//clr[2] = rgb[2]*k;
		//glMaterialfv(GL_FRONT, GL_DIFFUSE, clr);
		//glNormal3f(0, 0, 1);
		//glColor3ub(rgb[0], rgb[1], rgb[2]);
		//gluDisk(quad, 0, r, 32, 3);
		glPopMatrix();
	}
	gluDeleteQuadric(quad);

}
*/


void CTriangleMesh::DrawLineAndPoints(const float isocolor[3], const int smooth)
{
	unsigned char ptcolor[3]={200, 60, 60};
	unsigned char ptcolor2[3]={0, 200, 0};
	unsigned char ptcolor3[3]={200, 200, 200};
	//Draw the line connection first;
	DrawSurfaceLines(isocolor);

	//Then, draw the points;
	int i;
	Vector3D d = upright - lowleft;
	float size= fabs((d.x+d.y+d.z))/3 * 0.05f;

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glPointSize(3);
	glBegin(GL_POINTS);
	for (i=0; i<m_nVerticesNumber; i++){
		//glNormal3fv((GLfloat*)&m_pVertexNorm[i]);
		int c=CheckVertexDegree(i);
		if (c<=8)
			glColor3ub(ptcolor[0], ptcolor[1], ptcolor[2]);
		else{ 
			if (c==9)
				glColor3ub(ptcolor2[0], ptcolor2[1], ptcolor2[2]);
			else
				glColor3ub(ptcolor3[0], ptcolor3[1], ptcolor3[2]);
		}
		glVertex3fv((float*)&m_Vertice[i]);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glLineWidth(1);
	glBegin(GL_LINES);
	Vector3D l;
	for (i=0; i<m_nVerticesNumber; i++){
		glVertex3fv((GLfloat*)&m_Vertice[i]);
		l= m_pVertexNorm[i]* size + m_Vertice[i];
		glVertex3fv((GLfloat*)&l);
	}
	glEnd();

}


void CTriangleMesh::DrawVerticesAsSpheres(CGLDrawParms *dparms)
{
	const float K = 1.0f/255;
	float r;
	bool ispointset=isPointSet();

	if (dparms->m_bEnableLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	if (dparms->m_bEnableTexture1D)
		glEnable(GL_TEXTURE_1D);
	else
		glDisable(GL_TEXTURE_1D);

//	if (!ispointset){
	if (m_fParticleRadius==0){
		Vector3D dist= upright - lowleft;
		Vector3D rad = dist * (1.0/sqrt((double)m_nVerticesNumber));
		r = _MAX3_(rad.x, rad.y, rad.z) * 0.30f;
	}
	else
		r = m_fParticleRadius;

	glMatrixMode(GL_MODELVIEW);
	GLUquadric * quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_FILL);
	dparms->setupMaterial();
	glEnable(GL_DEPTH_TEST);      

	for (int i=0; i<m_nVerticesNumber; i++){
		//set texture coordinate;
		if (m_pVertexColor){
			float tx = m_pVertexColor[i][3];
			tx*=K;
			glTexCoord1f(tx);
		}
		//if (ispointset)
		//	r = m_pVertexAux[i].z * 0.65f;
		Vector3D v = (m_Vertice[i]);
		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		if (m_pRadius)
			r = m_pRadius[i];
		gluSphere(quad, r, 15, 7);
		glPopMatrix();
	}
	gluDeleteQuadric(quad);
}



void CTriangleMesh::DrawSurfaceTexture1D(void)
{
	int i;

	glEnable(GL_DEPTH_TEST);      
	glEnable(GL_TEXTURE_1D);
    glDisable(GL_BLEND);

	if (m_pDrawParms->m_bEnableLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glShadeModel(GL_SMOOTH);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

	if (!isPointSet()){
		ComputeMeshNormals();
		glBegin(GL_TRIANGLES);
			for (i=0; i<m_nTriangleNumber; i++)
				DrawTriangleWithTextured1D(i);
		glEnd();
		return;
	}
	else{
		DrawVerticesAsSpheres(m_pDrawParms);
	}
}



void CTriangleMesh::DrawSurfaces(const int mode, const int smooth, const float isocolor[3])
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	switch (mode){
	case DRAW_SURFACE_SHADING:
		DrawShadedSurfaces(smooth);
		break;
	case DRAW_SURFACE_HIDDENLINE:
		DrawHiddenlineSurfaces(isocolor);
		break;
	case DRAW_SURFACE_LINE:
		DrawEdgeTable(isocolor);
		break;
	case DRAW_POINT_EMBOSS:
		DrawSurfaceAndPoints(isocolor, smooth);
		break;
	case DRAW_LINE_POINT_EMBOSS:
		DrawLineAndPoints(isocolor, smooth);
		break;
	}
}


void CTriangleMesh::glDraw(CGLDrawParms *dparms)
{
	m_pDrawParms = dparms;
	float* isocolor = (float*)&dparms->m_cObjectColor;
	int smooth = dparms->m_bUseVertexNormal;
	glShadeModel(GL_SMOOTH);

	_BeginDrawing(dparms);
	switch (dparms->m_nDrawType){
	case CGLDrawParms::DRAW_MESH_HIDDENLINE:
		if (dparms->m_bEnableTexture1D){
			DrawSurfaceTexture1D();
			DrawMeshOffsetLine(dparms);
		}
		else{
			//DrawHiddenlineSurfaces(isocolor);
			DrawMeshHiddenLine(dparms);
		}

		break;

	case CGLDrawParms::DRAW_MESH_LINE:
		//DrawEdgeTable(isocolor);
		DrawMeshLine(dparms);
		break;

	case CGLDrawParms::DRAW_MESH_POINT_EMBOSS:
		//DrawSurfaceAndPoints(isocolor, smooth);
		DrawVerticesAsSpheres(dparms);
		break;

	case CGLDrawParms::DRAW_MESH_LINE_POINT_EMBOSS:
		DrawLineAndPoints(isocolor, smooth);
		break;

	case CGLDrawParms::DRAW_MESH_SHADING:
	//case CGLDrawParms::DRAW_MESH_TEXTURE2D:
		if (dparms->m_bEnableTexture1D){
			DrawSurfaceTexture1D();
		}
		else
			DrawShadedSurfaces(smooth);
		break;

	case CGLDrawParms::DRAW_MESH_NONE:
		break;
	}

	_EndDrawing(dparms);

}



static int counter = 1;
void CTriangleMesh::DrawEdgeTable(const float isocolor[3])
{
	int i, j;
	CEdgeTableCell* p;

	if (m_ppEdgeTable==NULL){
		BuildEdgeTable();
	}

	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
	glColor3fv(isocolor);

	glBegin(GL_LINES);
	for (i=0; i<m_nVerticesNumber; i++){
		p = m_ppEdgeTable[i];
		while (p!=NULL){
			j= p->vertex;
			if (p->tag != counter){
				p->tag = counter;
				glNormal3fv((GLfloat*)&m_pVertexNorm[i]);
				glVertex3fv((GLfloat*)&m_Vertice[i]);
				glNormal3fv((GLfloat*)&m_pVertexNorm[j]);
				glVertex3fv((GLfloat*)&m_Vertice[j]);
				MarkEdgeWithTag(m_ppEdgeTable[j], i, counter);
			}
			p = p->pNext;
		}
	}
	glEnd();
	counter++;
}




void computeTextureCoordinates(float *pArray, 
							  const int nArray, 
							  unsigned int * pColor, 
							  const ScalarToScalarMappingType mappingfunc,
							  const float minval, 
							  const float maxval)
{
	const float ln2 = 1.0f/log(2.00);
	const float KK = 1.0f/255;
	int i;
	float dx = maxval - minval; 
	if (dx < 1e-30f)
		dx = 1e-30f;
	dx = 255 / dx;

	switch(mappingfunc){
		case ScalarMappingLinear:
			for (i=0; i<nArray; i++){
				float x = pArray[i];
				unsigned char * clr = (unsigned char* )(&pColor[i]);
				float t= (x - minval) * dx;
				if (t<1) 
					t=1;
				else if (t>=254)
					t = 254;
				clr[3] = (unsigned char) t;
			}
			break;
		case ScalarMappingLog2:
			for (i=0; i<nArray; i++){
				float x = pArray[i];
				unsigned char * clr = (unsigned char* )(&pColor[i]);
				float t= (x - minval) * dx;
				t*=KK;
				t = sqrt(t);
				t*=255;
				if (t<1) 
					t=1;
				else if (t>=254)
					t = 254;
				clr[3] = (unsigned char) t;
			}

			break;
		case ScalarMappingLog10:
			for (i=0; i<nArray; i++){
				float x = pArray[i];
				unsigned char * clr = (unsigned char* )(&pColor[i]);
				float t= (x - minval) * dx;
				t *= KK;
				t *= t;
				t = t*255+1;
				if (t<1) 
					t=1;
				else if (t>=254)
					t = 254;
				clr[3] = (unsigned char) t;
			}
			break;
		default:
			assert(0);
	}

}



void computeAttribRange(float *pArray, const int nArray, float &minval, float &maxval)
{
	int i;

	//find max value
	maxval = -1e30f;
	minval = +1e30f;

	for (i=0; i<nArray; i++){
		float x = pArray[i];
		if (x>maxval)
			maxval = x;
		else if (x<minval)
			minval = x;
	}

	float diff = maxval - minval;
	if (fabs(diff)<1e-28f){ //zero value, no diff
		maxval += 1e-28f;
	}
	//NANZHANG sigg
	//minval = 0;
}



void CTriangleMesh::ComputeVertexTextureCoordiates(CGLDrawParms *pDrawParms, bool useNewRange)
{
	unsigned int *pclr = (unsigned int *)m_pVertexColor;
	int nv =m_nVerticesNumber;

	int idx = pDrawParms->m_nTextureCoorType;
	float *pArray = m_pAttributes[idx];
	char *strName = m_strVarNames[idx];
	assert(pArray!=NULL);
	assert(strName[0]!=0);
	
	if (pArray!=NULL && pclr!=NULL){	
		float minval = pDrawParms->m_fMinRange;
		float maxval = pDrawParms->m_fMaxRange;
		if (useNewRange){
			computeAttribRange(pArray, nv, minval, maxval);
			pDrawParms->m_fMinRange = minval;
			pDrawParms->m_fMaxRange = maxval;
		}
		//NANZHANG SIGGRAPH 2006
		//minval = 0;
		//pDrawParms->m_fMinRange = 0;
		computeTextureCoordinates(pArray, nv, pclr, pDrawParms->m_nMappingType, minval, maxval);
	}
}


void CTriangleMesh::_BeginDrawing(CGLDrawParms *pDrawParms)
{
	//env. seting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	//do mapping
	if (pDrawParms->m_bEnableTexture1D){
		if (pDrawParms->m_bRecomputeTexture1DParameter){
			ComputeVertexTextureCoordiates(pDrawParms, true);
			pDrawParms->m_bRecomputeTexture1DParameter = false;
		}
	}
}


void CTriangleMesh::_EndDrawing(CGLDrawParms *pDrawParms)
{
	if (pDrawParms->m_bRecomputeTexture1DParameter){
		pDrawParms->m_bRecomputeTexture1DParameter = false;
	}
}
