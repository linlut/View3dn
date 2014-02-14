/*
 quad_mesh.cpp
*/
#include "sysconf.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "matrix.h"
#include "ply.h"
#include "quad_mesh.h"


CQuadMesh::CQuadMesh() : CMemoryMgr()
{
	init();
}


void CQuadMesh::init(void)
{
	m_nVertices = 0;
	m_nVertexField = 0;
	m_nQuadStrip = 0;				//how many strip ?
	m_pQuadStripLen = 0;			//how long is each strip?

	m_pVertices = NULL;	
	m_pVertexNorm = NULL;
	m_pQuadStripArray = NULL;		//
	m_pVertexColor = NULL;
	m_pQuadColor = NULL;
}



CQuadMesh::~CQuadMesh(void)
{
	//ClearAllBuffer();
}




/*
Simple data file format:

  (#of vertices)
  (#of Triangles)
  vertices' x y z nx ny nz
  ...
  ...
  triangle vertice order
  v0 v1 v2
  ...
  ...


*/
int CQuadMesh::LoadTXTFile(const char *filename)
{
	FILE *fp;
	int i, count, *rgb, color=0x00FFFF00;
	float x, y, z;
	float nx, ny, nz;

	return true;
}


int CQuadMesh::getQuadTotalNum()
{
	assert(0);
	return 0;
}


int CQuadMesh::LoadFile(const char *filename)
{
	int len;
	int flag=0;

	len = strlen(filename);
	if (strcmp(&filename[len-4], ".txt")==0){
		flag= LoadTXTFile(filename);
	}
	else
	if (strcmp(&filename[len-4], ".ply")==0){
		flag= LoadPLYFile(filename);
	}

	if (flag){
		//compute bounding box;
		getBoundingBox(minp, maxp);
		int nTotalQuad= getQuadTotalNum();
		fprintf(stderr, "Totally %d vertices, %d quad/tri read!\n", m_nVertices, nTotalQuad);
		fprintf(stderr, "Bounding box lowleft: (%f,%f,%f)\n", minp.x, minp.y, minp.z);
		fprintf(stderr, "Bounding box upright: (%f,%f,%f)\n", maxp.x, maxp.y, maxp.z);
	}
	return flag;
}




int CQuadMesh::SaveFile(const char *filename)
{
	return true;
}




int CQuadMesh::LoadPLYFile(const char *filename)
{
	return true;
}

void CQuadMesh::getBoundingBox(Vector3D &minp, Vector3D &maxp)
{
	assert(0);
}
