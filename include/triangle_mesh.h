/*
 triangle_mesh.h
*/
#ifndef __INC_TRIANGLE_MESH__

#define __INC_TRIANGLE_MESH__

#include "sysconf.h"
#include "memmgr.h"

#ifdef USE_DOUBLE_VECTOR
#include <f3dVector3d.h>
#else
#include <vectorall.h>
#endif

#include "drawparms.h"
#include "geomath.h"


#define DRAW_SURFACE_SHADING  0
#define DRAW_SURFACE_HIDDENLINE 1
#define DRAW_SURFACE_LINE 2
#define DRAW_POINT_EMBOSS 3
#define DRAW_LINE_POINT_EMBOSS 4


class CEdgeTableCell
{
public:
	int vertex;					//the other vertex id

	int tag;					//visit tag

	int  intpID;				//index value for the intersection point
	Vector3f intp;				//store the intersection point position

	CEdgeTableCell *pNext;		//link pointer	

	CEdgeTableCell(void)
	{
		vertex = 0;
		intpID = 0;
		pNext = NULL;
		tag = 0;
	}

	CEdgeTableCell(int v, CEdgeTableCell *p)
	{
		intpID = 0;
		vertex = v;
		pNext = p;
		tag = 0;
	}
};


inline CEdgeTableCell* InsertBeforeEdgeTableQueue(CEdgeTableCell* pHead, CEdgeTableCell* pNewCell)
{
	pNewCell->pNext = pHead;
	return pNewCell;
}


inline CEdgeTableCell* VertexInQueue(const CEdgeTableCell* pHead, const int vid)
{
	if (pHead==NULL) return NULL;
	CEdgeTableCell* p = (CEdgeTableCell*)pHead;
	while (p!=NULL){
		if (p->vertex == vid) 
			return p;
		p = p->pNext;
	}
	return NULL;
}


inline void MarkEdgeWithTag(CEdgeTableCell* pHead, int vid, int newtag)
{
	CEdgeTableCell* p;

	p = pHead;
	while (p!=NULL){
		if (p->vertex == vid){
			p->tag = newtag;
			return;
		}
		p=p->pNext;
	}
}


class CTriangleMesh: public CMemoryMgr
{

public:

	int m_nVerticesNumber;

	int m_nVerticeFieldNumber;

	int m_nTriangleNumber;

	Vector3f * m_Vertice;

	Vector3f * m_pVertexNorm;

	//NZ. March, 4. Newlly defined for FEB-based physics computation
/*
	Vector3f * m_pVertexDisplace;

	Vector3f * m_pVertexVelocity;

	Vector3f * m_pVertexDeform;

	Vector3f * m_pVertexShear;

	Vector3f * m_pVertexAux;
*/
	float m_fParticleRadius;
	float *m_pRadius;
	float *m_pAttributes[32];
	char m_strVarNames[32][9];

	//NZ. Newlly defined for FEB-based physics computation

	Vector3f * m_pTriangleNorm;

	CEdgeTableCell **m_ppEdgeTable;

	Vector3I *m_Triangles;

	unsigned char (*m_pVertexColor)[4];

	typedef void (*PRINT_FUNC)(const char *msg);


private:

	Vector4I * m_pQuads;
	int m_nQuads;

	Vector3f lowleft, upright;      //bounding box of the mesh	

	CGLDrawParms * m_pDrawParms;

	void ClearAllBuffer(void);

	PRINT_FUNC m_pPrintFunc;
	
	void PrintMessage(const char *msg)
	{
		if (m_pPrintFunc!=NULL) m_pPrintFunc(msg);
	}

	void _init();

	//Draw polygons in flat mode, without specifying any GL mode
	void _BeginDrawing(CGLDrawParms *pDrawParms);
	void _EndDrawing(CGLDrawParms *pDrawParms);

	void DrawFlatSurfaces(void);
	void DrawUnsmoothedFlatSurfaces(void);
	void DrawShadedSurfaces(const int smooth);
	void DrawHiddenlineSurfaces(const float isocolor[3]);
	void DrawSurfaceLines(const float isocolor[3]);
	void DrawEdgeTable(const float isocolor[3]);
	void DrawLineAndPoints(const float isocolor[3], const int smooth);
	void DrawSurfaceAndPoints(const float isocolor[3], const int smooth);
	void DrawSurfaceTexture1D(void);
	void DrawMeshHiddenLine(CGLDrawParms *dparms);
	void DrawMeshLine(const CGLDrawParms *dparms, const float offset=0);
	void DrawMeshOffsetLine(CGLDrawParms *dparms);
	void DrawVerticesAsSpheres(CGLDrawParms *dparms);

	//
	void ValidateTriangles(void);

	void DrawFlatTriangle(const int i);

	void DrawTriangleWithTextured1D(const int i);

	void DrawHiddenlineTriangle(const int i, const float c[3]);

	void checkNormals(void);

	void ComputeVertexArrayNormal(void)
	{
		int i;

		if (m_pVertexNorm==NULL)
			m_pVertexNorm = new Vector3f[m_nVerticesNumber];
		assert(m_pVertexNorm!=NULL);

		for (i=0; i<m_nVerticesNumber; i++){
			m_pVertexNorm[i]=Vector3f(0,0,0);
		}

		for (i=0; i<m_nTriangleNumber; i++){
			int a, b, c;
			a= m_Triangles[i].x;
			b= m_Triangles[i].y;
			c= m_Triangles[i].z;

			Vector3f n= compute_triangle_normal(m_Vertice[a], m_Vertice[b], m_Vertice[c]);
			m_pVertexNorm[a]+=n;
			m_pVertexNorm[b]+=n;
			m_pVertexNorm[c]+=n;
		}

		for (i=0; i<m_nVerticesNumber; i++){
			m_pVertexNorm[i]=Normalize(m_pVertexNorm[i]);
		}
	}


public:

	CTriangleMesh(void);

	~CTriangleMesh(void);

	//constructor and deconstructor
	CTriangleMesh(const int vertex_num, Vector3f vertex[], Vector3f normal[], const int trianle_num, Vector3I* triangle);

	int copyTriangleMesh(const int vertex_num, Vector3f vertex[], const int triangle_num, Vector3I triangle[]);

	bool isPointSet(void)
	{
		if (m_nTriangleNumber!=m_nVerticesNumber)
			return false;
		if (m_Triangles==NULL)
			return false;
		Vector3I *p = &m_Triangles[0];
		if ((p->x == p->y)  && (p->x==p->z))
			return true;
		return false;
	}

	//*************data member access***************************
	int GetVerticesNumber(void)
	{
		return m_nVerticesNumber;
	}

	Vector3f* GetVerticesArray(void)
	{
		return m_Vertice;
	}

	Vector3f* GetVertexNormalArray(void)
	{
		return m_pVertexNorm;
	}

	int GetTriangleNumber(void)
	{
		return m_nTriangleNumber;
	}

	void GetTriangle(const int i, Vector3f& a, Vector3f &b, Vector3f &c)
	{
		ASSERT0(i>=0 && i<m_nTriangleNumber);
		Vector3I *p = &m_Triangles[i]; 
		a = m_Vertice[p->x];
		b = m_Vertice[p->y];
		c = m_Vertice[p->z];
	}

	Vector3I *GetTriangleTable(void)
	{	
		return m_Triangles;
	}

	//Compute the bounding box, translate all vertices 
	//to be between [-0.5box, +0.5box]
	void CenteringTriangles(void);

	void NormalizeTriangles(void);

	//******************FILE i/o**********************************
	int LoadTRIFile(const char *filename);

	int LoadNFFFile(const char *filename);

	//Load techplot .plt format file
	int LoadPltFile(const char *filename);
	int LoadPltFile(FILE *fp);

	//Load txt format file
	int LoadTXTFile(const char *filename);

	//Load smf format file
	int LoadSMFFile(const char *filename);

	//load ply format file
	int LoadPLYFile(const char *filename);

	//Load off format file
	int LoadOFFFile(const char *filename);

	//load GTS file (GTS lib)
	int LoadGTSFile(const char *filename);

	//load .m file, used by Hoppe for the mechanical part
	int LoadMFile(const char *filename);

	//Load the two formats

	int LoadFile(const char *filename);

	bool SaveSMFFile(const char *filename);

	bool SaveTXTFile(const char *filename);

	bool SavePOVFile(const char *filename);

	bool SaveMultiObjPOVFile(const char *filename);

	bool SaveMultiObjVRMLFile(const char *filename);

	bool SaveUDOFile(const char *filename);

	bool SaveFile(const char *filename);


	//******************Bounding box******************************
	void GetBoundingBox(int minp[3], int maxp[3]);
	void GetBoundingBox(float minp[3], float maxp[3]);
	void GetBoundingBox(Vector3f &minp, Vector3f &maxp)
	{
		float a[3], b[3];
		GetBoundingBox(a, b);
		minp= Vector3f(a[0], a[1], a[2]);
		maxp= Vector3f(b[0], b[1], b[2]);
	}

	void SetBoundingBox(const Vector3f& minp, const Vector3f& maxp)
	{
		lowleft = minp, upright=maxp;
	}

	//*****************optimize, edge table establishment, etc.**************
	int OptimizeMesh(const double TH=1e-6);

	//Check whether a triangle is valid!
	int isValidTriangle(const int whichtriangle);

	int BuildEdgeTable(void);

	//check if there are errorous cases
	int CheckEdgeTable(void);

	//return vertex degree;
	int CheckVertexDegree(int i);
	void CheckEdgeLength(int i, float &len, int &v);


	//*************************Draw mesh*************************************
	void DrawSurfaces(const int mode, const int smooth, const float isocolor[3]);

	void glDraw(CGLDrawParms *dparms);

	void SetPrintFunc(PRINT_FUNC func)
	{
		m_pPrintFunc= func;
	}

	//=========================PICKING==================================
	void DrawMeshLines4Picking(void);	//PICKING LINE
	bool GetPickedLine(const int edgeid, Vector3f &v0, Vector3f &v1);



	//
	void ComputeVertexShortestRadius(float *pRadius);

	//Area
	double GetTrianglesTotalArea(void);

	//Get average position;
	void VertexNeighborAveragePositionNorm(const int vid, Vector3f &pos, Vector3f &norm);

	void VertexNeighborAveragePosition(const int vid, Vector3f &pos);

	//************compute triangle, vertex normal, etc.***********************
	Vector3f ComputeTriangleNormal(const int index)
	{
		ASSERT0(index>=0 && index<m_nTriangleNumber);
		Vector3I &tri=m_Triangles[index];
		Vector3f & a = m_Vertice[tri[0]];
		Vector3f & b = m_Vertice[tri[1]];
		Vector3f & c = m_Vertice[tri[2]];
		return compute_triangle_normal(a, b, c);
	}

	float ComputeTriangleArea(const int index)
	{
		ASSERT0(index>=0 && index<m_nTriangleNumber);
		Vector3I &tri=m_Triangles[index];
		Vector3f & a = m_Vertice[tri[0]];
		Vector3f & b = m_Vertice[tri[1]];
		Vector3f & c = m_Vertice[tri[2]];
		return (float)triangle_area(a, b, c);
	}

	void ComputeTrianglePlane(const int index, Vector3f &n, float &d)
	{
		n = ComputeTriangleNormal(index);
		Vector3I &tri=m_Triangles[index];
		Vector3f & a = m_Vertice[tri[0]];
		d = -DotProd(n, a);
	}

	friend bool Merge(CTriangleMesh & t1, CTriangleMesh & t2, CTriangleMesh &t3);

	bool ComputeSurfaceComponents(int & nSurfComponents);

	void ComputeMeshNormals(void)
	{
		int f;
		if (m_pTriangleNorm==NULL){
			m_pTriangleNorm = new Vector3f[m_nTriangleNumber];
			assert(m_pTriangleNorm!=NULL);
			for (f=0; f<m_nTriangleNumber; f++){
				 m_pTriangleNorm[f]=ComputeTriangleNormal(f);
			}
		}
	}

	void ComputeVertexTextureCoordiates(CGLDrawParms *pDrawParms, bool useNewRange);

	//========================Math op on vertices=====================
	void ScaleVertices(const float factor);

	void TranslateVertices(const Vector3f & offset);

	// get attribute index;
	int GetAttributeIndexByName(const char *strname)
	{
		int i, idx=-1;
		for (i=0; i<32; i++){
			float * p = m_pAttributes[i];
			if (p==NULL) continue;
			if (strcmp(m_strVarNames[i], strname)==0){
				idx = i;
				break;
			}
		}
		return idx;
	}


};


#endif