//File: grasper.cpp

#include <objrw.h>
#include <ogldrawall.h>
#include "grasper.h"


inline void _setJaw(
	const int maxnv, const vector<Vector3d>& pos, const vector<int>& index, 
	Vector3d*& pLeftJawVertex, int*& pLeftJawVertexIndex, int &nLeftJawVertexCount)
{
	int i;
	unsigned int nsize = pos.size();
	if (nsize==0) return;
	unsigned char *counters = new unsigned char [maxnv];
	for (i=0; i<maxnv; i++) counters[i]=0;
	for (i=0; i<nsize; i++){
		const int j = index[i];
		counters[j]++;
	}
	//get the vertex count, and then allocate space
	int c=0;
	for (i=0; i<maxnv; i++){
		if (counters[i]>0) c++;
	}
	assert(c>0);
	pLeftJawVertex = new Vector3d[c]; 
	pLeftJawVertexIndex = new int [c];
	nLeftJawVertexCount = c;
	//copy, while removing duplication
	c = 0;
	for (i=0; i<nsize; i++){
		const int v = index[i];
		const int vcount = counters[v];
		if (vcount>0){
			pLeftJawVertex[c] = pos[i];
			pLeftJawVertexIndex[c] = v;
			counters[v]=0;
			c++;
		}
	}
	assert(c==nLeftJawVertexCount);

CLEANUP:
	SafeDeleteArray(counters);
}

void CGrasperHead::setLeftJaw(
	const int maxnv, const vector<Vector3d>& pos, const vector<int>& index)
{
	_setJaw(maxnv, pos, index, m_pLeftJawVertex, m_pLeftJawVertexIndex, m_nLeftJawVertexCount);
}

void CGrasperHead::setRightJaw(
	const int maxnv, const vector<Vector3d>& pos, const vector<int>& index)
{
	_setJaw(maxnv, pos, index, m_pRightJawVertex, m_pRightJawVertexIndex, m_nRightJawVertexCount);
}

void CGrasperHead::decideTranslationRotation(void)
{
	int i;
	AxisAlignedBox box;
	box.minp = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	box.maxp = - box.minp;
	for (i=0; i<m_nLeftJawVertexCount; i++){
		Minimize(box.minp, m_pLeftJawVertex[i]);
		Maximize(box.maxp, m_pLeftJawVertex[i]);
	}
	for (i=0; i<m_nRightJawVertexCount; i++){
		Minimize(box.minp, m_pRightJawVertex[i]);
		Maximize(box.maxp, m_pRightJawVertex[i]);
	}
	m_translation.x = (box.minp.x+box.maxp.x)*0.5;
	m_translation.y = (box.minp.y+box.maxp.y)*0.5;
	m_translation.z = box.maxp.z;
	m_axis = Vector3d(0, 1, 0);

	for (i=0; i<m_nLeftJawVertexCount; i++)
		m_pLeftJawVertex[i] -= m_translation;
	for (i=0; i<m_nRightJawVertexCount; i++)
		m_pRightJawVertex[i] -= m_translation;
}

void CGrasperHead::applyRotation(Vector3d *pVertex)
{
	int i;
	Matrix rot, trans, mat;
	double ang = -m_jawAngleRadian;
    SetTranslationMatrix(m_translation, trans);
	
    RotationAxisMatrix(m_axis, ang, rot);
	mat = rot * trans;
	for (i=0; i<m_nLeftJawVertexCount; i++){
		const Vector3d v = m_pLeftJawVertex[i];
		const int vindex = m_pLeftJawVertexIndex[i];
		const Vector3d npos = v* mat;
		pVertex[vindex] = npos;
	}

	ang = -ang;
    RotationAxisMatrix(m_axis, ang, rot);
	mat = rot * trans;
	for (i=0; i<m_nRightJawVertexCount; i++){
		const Vector3d v = m_pRightJawVertex[i];
		const int vindex = m_pRightJawVertexIndex[i];
		const Vector3d npos = v* mat;
		pVertex[vindex] = npos;
	}
}

//=========================================================================
/*

void CGrasperTriObj::_initJaws(
	const Vector3d *pVertex, const int nVertex, const int *pPoly, const int nPolygon, const int plytype, 
	const OglMaterial material[], const unsigned int faceMatIndex[], const int nFaceMatIndex)
{
	int i;
	const Vector3i *ptri = (const Vector3i *)pPoly;
	vector<Vector3d> leftjawPos, rightjawPos;
	vector<int> leftjawIndex, rightjawIndex;

	//
	m_pMedialBuffer = new bool [nVertex];
	for (i=0; i<m_nVertexCount; i++) m_pMedialBuffer[i]=false;

	for (int i=0; i<nFaceMatIndex; i++){
		int x0, x1, matid;
		decodeMaterialIndex(i, faceMatIndex, nFaceMatIndex, nPolygon, x0, x1, matid);
		const OglMaterial& m0 = material[matid];
		const Vector3f *pColor = (const Vector3f *)&m0.ambient[0];
		const Vector3f color = *pColor;
		if (color == LEFTJAW || color == LEFTJAW_MEDIAL){
			const bool isMedial = (color==LEFTJAW_MEDIAL);
			for (int j=x0; j<x1; j++){
				const Vector3i tri = ptri[j];
				leftjawPos.push_back(pVertex[tri.x]), leftjawIndex.push_back(tri.x);
				leftjawPos.push_back(pVertex[tri.y]), leftjawIndex.push_back(tri.y);
				leftjawPos.push_back(pVertex[tri.z]), leftjawIndex.push_back(tri.z);
				if (isMedial){
					m_pMedialBuffer[tri.x]=true;
					m_pMedialBuffer[tri.y]=true;
					m_pMedialBuffer[tri.z]=true;
				}
			}
		}
		else if (color == RIGHTJAW || color == RIGHTJAW_MEDIAL){
			const bool isMedial = (color==LEFTJAW_MEDIAL);
			for (int j=x0; j<x1; j++){
				const Vector3i tri = ptri[j];
				rightjawPos.push_back(pVertex[tri.x]), rightjawIndex.push_back(tri.x);
				rightjawPos.push_back(pVertex[tri.y]), rightjawIndex.push_back(tri.y);
				rightjawPos.push_back(pVertex[tri.z]), rightjawIndex.push_back(tri.z);
				if (isMedial){
					m_pMedialBuffer[tri.x]=true;
					m_pMedialBuffer[tri.y]=true;
					m_pMedialBuffer[tri.z]=true;
				}
			}
		}
	}

	m_pGrasperHead = new CGrasperHead;
	assert(m_pGrasperHead!=NULL);
	m_pGrasperHead->setLeftJaw(nVertex, leftjawPos, leftjawIndex);
	m_pGrasperHead->setRightJaw(nVertex, rightjawPos, rightjawIndex);
	m_pGrasperHead->decideTranslationRotation();
}


void CGrasperTriObj::glDraw(const CGLDrawParms &dparms)
{
	if (m_pGrasperHead)
		m_pGrasperHead->applyRotation(m_pVertex);
	CTexturedTriangleObj::glDraw(dparms);
}

*/