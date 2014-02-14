//dynamicmesh.h

#ifndef __INC_DYNAMICMESH_H__

#define __INC_DYNAMICMESH_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <dynamic_array.h>
#include "tetraobj.h"
#include "marchingtet.h"
#include "vertexsmoothing.h"


class CCia3dObj;

class CDynamicMesh: public CObject3D
{

private:
	bool m_bExportTextureCoor;		//output the 3D texture coordinate or not

	bool m_bReconstrucSurface;		//do the reconstruction;

	bool m_bTMMP;	

	bool m_bExportBoundingSph[20];	//flag to export the bounding sphere

	bool m_bDisableObject[20];		//flag to export the bounding sphere

	AxisAlignedBox m_bbox[20];		//Bounding boxes of objects;

	CMemoryMgr *m_pSSurfaceMgr;		//Memory manager for the static boundary surface 

	CMemoryMgr *m_pDSurfaceMgr;		//Memory manager for the dynamic boundary surface

	CVertexInfo * m_pStaticVertex;	//Vertex information
	int m_nVertexCount;				//Total vertex count

	Vector3i * m_pTriangleBoundary; //The initial boundary
	int m_nTriangleCount;			//Number of triangles

	Vector4i* m_pTetra;				//PTR to tetrahedra
	int * m_pTetIndex;				//Index to tetrahedra buffer for split
	int m_nTetraCount;				//Total count of tetrahedra

	int m_nMaterialCount;			//Total count of meterial types
									//We assume it is the same as object count

	CDynamicArray<Vector3i> m_Triangles;			//Boundary surface in each timestep

	CDynamicArray<CMarchingTet> m_SplitTets;		//Store the dynamic tetrahedra

	CDynamicArray<CMarchingTetEdge> m_SplitTetEdges;//Edges for the dynamic tetrahera

public:
	CDynamicArray<CGeneratedVertex> m_FractVertex;	//Boundary surface in each timestep

	CDynamicArray<Vector3i> m_FractTri;				//Boundary surface in each timestep

	CDynamicArray<CGeneratedVertex> m_FractVertex2;	//for particle

	CDynamicArray<Vector3i> m_FractTri2;			//for particle

private:

	//===================================================

	void _updateVertexPositions(CCia3dObj & obj);

	void _updateBoundrayMeshConnectivity(CCia3dObj & obj);

	void _updateParticles(const int tm, CCia3dObj & obj);

	//find out the vertices which are the first to cut in a tet
	void _checkFlatCuts(void); 

	void _exportPovrayFile(const int tm, void *pwin, CCia3dObj & obj, const char *fname);

	void _exportPovrayMesh(const int tm, CCia3dObj & obj, FILE *fp);

	void _exportUnPatchedMesh(const int matid, FILE *fp);

	void _exportPatchedMesh(FILE *fp, const int *tindex=NULL);

	void _exportPatchedMeshAlt(FILE *fp, const int *tindex=NULL); //alternate method

	void _extractMissingTriangles(const int matid);

	//----------Simplification functions----------------
	int _getridDoubleSidedTriangle(Vector3i *ptri, const int nv, const int ntri);

	void _computeBoundingBox(const int matid);

	void _simplifyFracturedSurface(const int matid);

	//=============some util functions==================
	bool _verticesSameSurfaceComponent(const int v0, const int v1)
	{
		CVertexInfo *p1 = &m_pStaticVertex[v0];
		CVertexInfo *p2 = &m_pStaticVertex[v1];
		return p1->sameList(p2);
	}

	bool _verticesSameSurfaceComponent(const int v0, const int v1, const int v2)
	{
		int c = 0;
		if (_verticesSameSurfaceComponent(v0, v1)) c++;
		if (_verticesSameSurfaceComponent(v0, v2)) c++;
		if (_verticesSameSurfaceComponent(v1, v2)) c++;
		if (c>=2) return true;
		return false;
	}

	bool _verticesSameSurfaceComponent(const int v0, const int v1, const int v2, const int v3)
	{
		const bool b01=_verticesSameSurfaceComponent(v0, v1);
		const bool b02=_verticesSameSurfaceComponent(v0, v2);
		const bool b03=_verticesSameSurfaceComponent(v0, v3);
		const bool b12=_verticesSameSurfaceComponent(v1, v2);
		const bool b13=_verticesSameSurfaceComponent(v1, v3);
		const bool b23=_verticesSameSurfaceComponent(v2, v3);
		if (b01 && b02 && b03) return true;
		if (b01 && b12 && b13) return true;
		if (b02 && b12 && b23) return true;
		if (b03 && b13 && b23) return true;
		return false;
	}

	bool _onDynBoundarySurface(const Vector4i & tet) const
	{
		const bool rx= m_pStaticVertex[tet.x].getDynBoundaryFlag();
		if (!rx) return false;
		const bool ry= m_pStaticVertex[tet.y].getDynBoundaryFlag();
		if (!ry) return false;
		const bool rz= m_pStaticVertex[tet.z].getDynBoundaryFlag();
		if (!rz) return false;
		const bool rw= m_pStaticVertex[tet.w].getDynBoundaryFlag();
		if (!rw) return false;
		return true;
	}

	void _dumpTetAsPovTriangles(const Vector4i &tet)
	{
		const Vector3f p0 = m_pStaticVertex[tet.x].m_vCurrPosition;
		const Vector3f p1 = m_pStaticVertex[tet.y].m_vCurrPosition;
		const Vector3f p2 = m_pStaticVertex[tet.z].m_vCurrPosition;
		const Vector3f p3 = m_pStaticVertex[tet.w].m_vCurrPosition;

		printf("triangle{<%f,%f,%f>,<%f,%f,%f>,<%f,%f,%f>}\n", p0.x, p0.y, -p0.z, p1.x, p1.y, -p1.z, p2.x, p2.y, -p2.z);
		printf("triangle{<%f,%f,%f>,<%f,%f,%f>,<%f,%f,%f>}\n", p0.x, p0.y, -p0.z, p1.x, p1.y, -p1.z, p3.x, p3.y, -p3.z);
		printf("triangle{<%f,%f,%f>,<%f,%f,%f>,<%f,%f,%f>}\n", p0.x, p0.y, -p0.z, p2.x, p2.y, -p2.z, p3.x, p3.y, -p3.z);
		printf("triangle{<%f,%f,%f>,<%f,%f,%f>,<%f,%f,%f>}\n", p1.x, p1.y, -p1.z, p2.x, p2.y, -p2.z, p3.x, p3.y, -p3.z);
	}

	//======================================================

	int _addMarchingTetAndEdgesToQueue(const int tetindex, int vbuff[], const int vlen);

	void _extractFracturedMaterialInterface(const int tm, const int matid, FILE *fp, int *tindex=NULL);

	void _buildBoundaryVertexClusters(Vector3i *pTriBuff, const int tribufflen);

	//=======================================================
	void __getBoundingBoxOfObject(const int matid, Vector3f& lowleft, Vector3f& upright);

	//=======================================================
	void __checkParticleStatusChanges(const Vector4i& tet, bool* pParticleFlag, int* vbuff, int &vlen, bool& hasParticleBefore, bool& hasParticleNow);

	void __checkTetConnectivityChanges(const Vector4i& tet, bool* pParticleFlag, int* vbuff, int &vlen);

	void __simplifyOneFracturedSurface(CDynamicArray<CGeneratedVertex>& fractVertex, CDynamicArray<Vector3i> &fractTri, const float TH);

public:

	//Construction, where the fname is the file name of a c3din1.in file
	//The initial vertex postions and connectivity information are constructed 
	//here. 
	CDynamicMesh(const char *fname);

	~CDynamicMesh();

	void SetExportBoudningSphFlag(const int i, const bool f)
	{
		if (i<0 || i>19) return;
		m_bExportBoundingSph[i] = f;
	}

	void SetDisableObjectFlag(const int i, const bool f)
	{
		if (i<0 || i>19) return;
		m_bDisableObject[i] = f;	
	}

	void SetExportTextureCoor(const bool f)
	{
		m_bExportTextureCoor = f;
	}

	void SetReconstrucSurface(const bool f)
	{
		m_bReconstrucSurface = f;	
	}

	void DecideBoundaryVertices(const char *fname);

	bool LoadNextTimeStepPlt(const int timestamp, const char *infname, const char *ofname, void *pwin);	
};


#endif