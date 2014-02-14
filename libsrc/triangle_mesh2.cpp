/*
 triangle_mesh.cpp
*/
#include "sysconf.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "matrix.h"
#include "ply.h"
#include "vectorall.h"
#include "dynamic_array.h"
#include "drawparms.h"
#include "triangle_mesh.h"
#include "geomath.h"

#define Vector3D Vector3f

bool Merge(CTriangleMesh & t1, CTriangleMesh & t2, CTriangleMesh &t3)
{
	int vertex_num, triangle_num, i;
	t3.m_nVerticesNumber = t1.m_nVerticesNumber+t2.m_nVerticesNumber;
	vertex_num = t3.m_nVerticesNumber;
	t3.m_Vertice = new Vector3D[vertex_num];
	assert(t3.m_Vertice!=NULL);
	for (i=0; i<t1.m_nVerticesNumber; i++) 
		t3.m_Vertice[i]=t1.m_Vertice[i];
	for (i=0; i<t2.m_nVerticesNumber; i++) 
		t3.m_Vertice[t1.m_nVerticesNumber+i]=t2.m_Vertice[i];

	//
	t3.m_nTriangleNumber = t1.m_nTriangleNumber+t2.m_nTriangleNumber;
	triangle_num = t3.m_nTriangleNumber;
	t3.m_Triangles = new Vector3I[triangle_num];
	for (i=0; i<t1.m_nTriangleNumber; i++) 
		t3.m_Triangles[i]=t1.m_Triangles[i];
	for (i=0; i<t2.m_nTriangleNumber; i++){
		const int x = t1.m_nVerticesNumber;
		t3.m_Triangles[i+t1.m_nTriangleNumber]=t2.m_Triangles[i]+Vector3I(x,x,x);
	}

	//normal
	t3.m_pVertexNorm = new Vector3D[vertex_num];
	assert(t3.m_pVertexNorm!=NULL);
	for (i=0; i<t1.m_nVerticesNumber; i++) 
		t3.m_pVertexNorm[i]=t1.m_pVertexNorm[i];
	for (i=0; i<t2.m_nVerticesNumber; i++) 
		t3.m_pVertexNorm[i+t1.m_nVerticesNumber]=t2.m_pVertexNorm[i];

	//
	t3.m_nVerticeFieldNumber=2;
	t3.m_pPrintFunc = NULL;
	t3.m_ppEdgeTable = NULL;
	return true;
}



int CTriangleMesh::copyTriangleMesh(int vertex_num, Vector3D vertex[], int triangle_num, Vector3I *triangle)
{
	int i;
	Vector3D n;

	//
	m_nVerticesNumber = vertex_num;
	m_Vertice = new Vector3D[vertex_num];
	assert(m_Vertice!=NULL);
	for (i=0; i<vertex_num; i++) m_Vertice[i]=vertex[i];

	//
	m_nTriangleNumber = triangle_num;
	m_Triangles = new Vector3I[triangle_num];
	for (i=0; i<triangle_num; i++) m_Triangles[i]=triangle[i];

	//normal
	m_pVertexNorm = new Vector3D[vertex_num];
	assert(m_pVertexNorm!=NULL);
	for (i=0; i<vertex_num; i++) m_pVertexNorm[i]=Vector3D(0,0,0);
	for (i=0; i<m_nTriangleNumber; i++){
		int a, b, c;
		a = m_Triangles[i][0], 
		b = m_Triangles[i][1], 
		c = m_Triangles[i][2];
		n= compute_triangle_normal(m_Vertice[a], m_Vertice[b], m_Vertice[c]);
		m_pVertexNorm[a]+=n;
		m_pVertexNorm[b]+=n;
		m_pVertexNorm[c]+=n;
	}
	for (i=0; i<m_nVerticesNumber; i++){
		m_pVertexNorm[i].normalize();
	}

	//
	m_nVerticeFieldNumber=2;
	m_pPrintFunc = NULL;
	m_ppEdgeTable = NULL;

	return 1;
}



//=====================================================================
// Find connected surface components;
//
//=====================================================================

static inline int find_starting_vertex(unsigned char pVertexColor[][4], const int nVertNum)
{
	const unsigned char emptyval = (unsigned char)0xff;
	for (int i=0; i<nVertNum; i++){
		if (pVertexColor[i][3]==emptyval)
			return i;
	}
	return -1;
}

static inline void init_vertex_clusterid(unsigned char pVertexColor[][4], const int nVertNum)
{
	for (int i=0; i<nVertNum; i++)
		pVertexColor[i][3] = 255;
}


static inline int get_surface_components(unsigned char pVertexColor[][4], const int nVertNum)
{
	int n = -1;

	for (int i=0; i<nVertNum; i++){
		int t = pVertexColor[i][3];
		if (t>n)
			n = t;
	}
	printf("Total %d surface components!\n", n+1);
	return (n+1);
}


static inline int get_component_surface_count(Vector3I tris[], unsigned char pVertexColor[][4], const int nTriangleNumber, const unsigned char compID)
{
	int count = 0;
	Vector3I *t;

	for (int i=0; i<nTriangleNumber; i++){
		t = tris + i;
		if (pVertexColor[t->x][3]==compID){
			count++;
			continue;
		}
		if (pVertexColor[t->y][3]==compID){
			count++;
			continue;
		}
		if (pVertexColor[t->z][3]==compID){
			count++;
			continue;
		}
	}
	return count;
}


static void colorize_vertex_and_neighbors(const int vid, const int clusterid, unsigned char pVertexColor[][4], 
										  	CEdgeTableCell **ppEdgeTable, const int depth
										  )
{
	if (depth>20) return;
	int vbuffer[20], vid2, count=0;
	unsigned char t;
	pVertexColor[vid][3] = (unsigned char)clusterid;

	CEdgeTableCell *p = ppEdgeTable[vid];
	while (p!=NULL){
		vid2 = p->vertex;
		t = pVertexColor[vid2][3];
		if (t==0xff){
			vbuffer[count] = vid2;
			count ++;
		}
		else if (t!=(unsigned char)clusterid){
			printf("Error: two surface components meet together!\n");
			return;
		}
		p = p->pNext;
	}

	assert(count<20);
	for (int i=0; i<count; i++){
		colorize_vertex_and_neighbors(vbuffer[i], clusterid, pVertexColor, ppEdgeTable, depth+1);
	}

}


bool CTriangleMesh::ComputeSurfaceComponents(int & nSurfComponents)
{
	//build edge table;
	if (m_ppEdgeTable==NULL){
		printf("Building edge table...\n");
		BuildEdgeTable();
	}

	if (m_pVertexColor==NULL){
		m_pVertexColor = new unsigned char [m_nVerticesNumber][4];
		assert(m_pVertexColor!=NULL);
	}

	//for each vertex;
	int clusterid=0;
	init_vertex_clusterid(m_pVertexColor, m_nVerticesNumber);

	do{
		int vid = find_starting_vertex(m_pVertexColor, m_nVerticesNumber);
		if (vid==-1) break;
		colorize_vertex_and_neighbors(vid, clusterid, m_pVertexColor, m_ppEdgeTable, 0);
		clusterid++;
	}while(1);

	printf("Total %d vertices, %d triangles, %d cluster generated!\n", m_nVerticesNumber, m_nTriangleNumber, clusterid);
	nSurfComponents = clusterid;
	return true;

}

//=====================================================================
//Some save file format support

//--------------save a UDO file -------------------------
bool CTriangleMesh::SaveUDOFile(const char *filename)
{
	int nSurfComponents = get_surface_components(m_pVertexColor, m_nVerticesNumber);
	char objname[200];
	char objtxtname[] = "Object";
	int objcount=0;
	char fname[200];
	int i;

	sprintf(fname, "%s%d", filename, objcount);
	FILE *fp = fopen(fname, _WA_);
	if (fp==NULL) return false;
	sprintf(objname, "%s%d", objtxtname, objcount);
	fprintf(fp, "Name '%s'\n", objname);
	fprintf(fp, "IncludeFile ''\n");

	//vertices;
	fprintf(fp, "[%s:Vertices]", objname);
	fprintf(fp, "%d\n", m_nVerticesNumber);
	for (i=0; i<m_nVerticesNumber; i++){
		fprintf(fp, "%f %f %f\n", m_Vertice[i].x, m_Vertice[i].y, m_Vertice[i].z);
	}

	//edges;
	fprintf(fp, "[%s:Edges]", objname);
	fprintf(fp, "%d\n", m_nTriangleNumber*3/2);
	for (i=0; i<m_nVerticesNumber; i++){
		fprintf(fp, "%f %f %f\n", m_Vertice[i].x, m_Vertice[i].y, m_Vertice[i].z);
	}

	//faces;
	fprintf(fp, "[%s:Edges]", objname);
	fprintf(fp, "%d\n", m_nTriangleNumber*3/2);
	for (i=0; i<m_nVerticesNumber; i++){
		fprintf(fp, "%f %f %f\n", m_Vertice[i].x, m_Vertice[i].y, m_Vertice[i].z);
	}
	return true;
}



//Save the mesh into POVRAY's mesh2 format;
bool CTriangleMesh::SavePOVFile(const char *filename)
{
	FILE *fp;
	Vector3D *p;
	Vector3I *t;
	int i;

	fp = fopen(filename, "w");
	if (fp==NULL)
		return false;

	//output vertices;
	fprintf(fp, "mesh2 {\n");
	fprintf(fp, "  vertex_vectors {\n");
	fprintf(fp, "\t%d\n", m_nVerticesNumber); 
	for (i=0; i<m_nVerticesNumber-1; i++){
		p = &m_Vertice[i];
		fprintf(fp, "\t<%f,%f,%f>,\n", p->x, p->y, p->z);
	}
	p = &m_Vertice[m_nVerticesNumber-1];
	fprintf(fp, "\t<%f,%f,%f>\n  }\n", p->x, p->y, p->z);

	//output triangles
	fprintf(fp, "  face_indices {\n");
	fprintf(fp, "\t%d\n", m_nTriangleNumber); 

	for (i=0; i<m_nTriangleNumber-1; i++){
		t = &m_Triangles[i];
		fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->y, t->z);
	}
	t = &m_Triangles[m_nTriangleNumber-1];
	fprintf(fp, "\t<%d,%d,%d>\n  }\n", t->x, t->y, t->z);

	fprintf(fp, "}\n");

	//close and return;
	fclose(fp);
	return true;

/* An example mesh
mesh2 {
   vertex_vectors {
      24, 
      <0,0,0>, <0.5,0,0>, <0.5,0.5,0>, //1
      <0.5,0,0>, <1,0,0>, <0.5,0.5,0>, //2
      <1,0,0>, <1,0.5,0>, <0.5,0.5,0>, //3
      <1,0.5,0>, <1,1,0>, <0.5,0.5,0>, //4
      <1,1,0>, <0.5,1,0>, <0.5,0.5,0>, //5
      <0.5,1,0>, <0,1,0>, <0.5,0.5,0>, //6
      <0,1,0>, <0,0.5,0>, <0.5,0.5,0>, //7
      <0,0.5,0>, <0,0,0>, <0.5,0.5,0>  //8
   }
   face_indices {
      8, 
      <0,1,2>,    <3,4,5>,       //1 2
      <6,7,8>,    <9,10,11>,     //3 4
      <12,13,14>, <15,16,17>,    //5 6
      <18,19,20>, <21,22,23>     //7 8
   }
   pigment {rgb 1}
}
*/
}



bool CTriangleMesh::SaveMultiObjPOVFile(const char *filename)
{
	int nSurfComponents = get_surface_components(m_pVertexColor, m_nVerticesNumber);
	int i, objcount, nTotalSurf=0;
	Vector3D *p;
	Vector3I *t;

	FILE *fp = fopen(filename, _WA_);
	if (fp==NULL) return false;

	int *tribuffer = new int [m_nTriangleNumber];
	assert(tribuffer!=NULL);
	int tricount = 0;

	for (objcount=0; objcount<nSurfComponents; objcount++){
		//get surface number for a component
		int nSurfCount = get_component_surface_count(m_Triangles, m_pVertexColor, m_nTriangleNumber, objcount);
		if (nSurfCount==0) continue;
		printf("Find surface component %d with %d triangles.\n", objcount, nSurfCount);

		//write a mesh block
		fprintf(fp, "#declare MESHOBJ%d=\n", objcount);
		fprintf(fp, "mesh2 {\n");
		fprintf(fp, "  vertex_vectors {\n");
		fprintf(fp, "\t%d\n", m_nVerticesNumber); 
		for (i=0; i<m_nVerticesNumber-1; i++){
			p = &m_Vertice[i];
			fprintf(fp, "\t<%f,%f,%f>,\n", p->x, p->z, p->y);
		}
		p = &m_Vertice[m_nVerticesNumber-1];
		fprintf(fp, "\t<%f,%f,%f>\n  }\n", p->x, p->z, p->y);

		//output triangles
		fprintf(fp, "  face_indices {\n");
		fprintf(fp, "\t%d\n", nSurfCount); 

		tricount = 0;
		for (i=0; i<m_nTriangleNumber; i++){
			t = &m_Triangles[i];
			if (m_pVertexColor[t->x][3]==objcount ||
				m_pVertexColor[t->y][3]==objcount ||
				m_pVertexColor[t->z][3]==objcount){
				tribuffer[tricount] = i;
				tricount++;
			}
		}

		for (i=0; i<tricount-1; i++){
			t = &m_Triangles[tribuffer[i]];
			fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->y, t->z);
		}
		t = &m_Triangles[tribuffer[tricount-1]];
		fprintf(fp, "\t<%d,%d,%d>\n  }\n", t->x, t->y, t->z);
		//fprintf(fp, "\n  pigment {rgb 1}\n\n");  

		fprintf(fp, "}\n\n\n\n");
		nTotalSurf+=nSurfCount;
	}

	fclose(fp);
	delete [] tribuffer;
	printf("Total %d vertices, %d triangles written to %s file.\n", m_nVerticesNumber, nTotalSurf, filename);
	return true;
}



bool CTriangleMesh::SaveMultiObjVRMLFile(const char *filename)
{
	int nSurfComponents = get_surface_components(m_pVertexColor, m_nVerticesNumber);
	int i, objcount, nTotalSurf=0;
	Vector3D *p;
	Vector3I *t;

	FILE *fp = fopen(filename, _WA_);
	if (fp==NULL) return false;

	int *tribuffer = new int [m_nTriangleNumber];
	assert(tribuffer!=NULL);
	int tricount = 0;

	printf("#VRML V1.0 ascii\n\n");

	for (objcount=0; objcount<nSurfComponents; objcount++){
		//get surface number for a component
		int nSurfCount = get_component_surface_count(m_Triangles, m_pVertexColor, m_nTriangleNumber, objcount);
		if (nSurfCount==0) continue;
		printf("Find surface component %d with %d triangles.\n", objcount, nSurfCount);

		//write a mesh block
		fprintf(fp, "#Model\n");
		fprintf(fp, "Separator {\n");
		fprintf(fp, "  Coordinate3 { point [\n");
		for (i=0; i<m_nVerticesNumber-1; i++){
			p = &m_Vertice[i];
			fprintf(fp, "\t%f,%f,%f,\n", p->x, p->z, p->y);
		}
		p = &m_Vertice[m_nVerticesNumber-1];
		fprintf(fp, "\t%f,%f,%f]\n  }\n", p->x, p->z, p->y);

		//output triangles
		fprintf(fp, "  IndexedFaceSet {\n");
		fprintf(fp, "  coordIndex  [\n"); 

		tricount = 0;
		for (i=0; i<m_nTriangleNumber; i++){
			t = &m_Triangles[i];
			if (m_pVertexColor[t->x][3]==objcount ||
				m_pVertexColor[t->y][3]==objcount ||
				m_pVertexColor[t->z][3]==objcount){
				tribuffer[tricount] = i;
				tricount++;
			}
		}
		for (i=0; i<tricount-1; i++){
			t = &m_Triangles[tribuffer[i]];
			fprintf(fp, "\t%d,%d,%d,%d,\n", t->x, t->y, t->z,-1);
		}
		t = &m_Triangles[tribuffer[tricount-1]];
		fprintf(fp, "\t%d,%d,%d,%d]\n", t->x, t->y, t->z, -1);
		fprintf(fp, "  }\n}\n");  
		nTotalSurf+=nSurfCount;
	}

	fclose(fp);
	delete [] tribuffer;
	printf("Total %d vertices, %d triangles written to %s file.\n", m_nVerticesNumber, nTotalSurf, filename);
	return true;
}


//============================Some math op on vertices===============================

void CTriangleMesh::ScaleVertices(const float factor)
{
	int n = m_nVerticesNumber;
	Vector3D * v = m_Vertice;
	if (v==NULL) return;

	for (int i=0; i<n; i++){
		v[i] *= factor;
	}
}

void CTriangleMesh::TranslateVertices(const Vector3D & offset)
{
	int n = m_nVerticesNumber;
	Vector3D * v = m_Vertice;
	if (v==NULL) return;

	for (int i=0; i<n; i++){
		v[i] += offset;
	}
}

