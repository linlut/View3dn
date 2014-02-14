//File: grasper.h
#ifndef __INC_CRESTGRASPER_20081110_H
#define __INC_CRESTGRASPER_20081110_H

#include <texturedtmeshobj.h>

const Vector3f LEFTJAW = Vector3f(1,1,0);			//YELLOW
const Vector3f RIGHTJAW = Vector3f(0,1,1);			//CYAN
const Vector3f LEFTJAW_MEDIAL = Vector3f(1,0,0);	//RED
const Vector3f RIGHTJAW_MEDIAL = Vector3f(0,0,1);	//BLUE

class CGrasperHead
{
private:
	Vector3d* m_pLeftJawVertex;
	int* m_pLeftJawVertexIndex;
	int m_nLeftJawVertexCount;

	Vector3d* m_pRightJawVertex;
	int* m_pRightJawVertexIndex;
	int m_nRightJawVertexCount;

	double m_jawAngleRadian;
	double m_jawAngleRadianOld;
	double m_jawAngleSpeed;
	Vector3d m_translation;
	Vector3d m_axis;

public:
	CGrasperHead(void)
	{
		m_pLeftJawVertex = NULL;
		m_pLeftJawVertexIndex = NULL;
		m_nLeftJawVertexCount = 0;

		m_pRightJawVertex = NULL;
		m_pRightJawVertexIndex = NULL;
		m_nRightJawVertexCount = 0;

		m_jawAngleRadian = 0;
		m_jawAngleRadianOld = 0;
		m_jawAngleSpeed = 0.0200;
	}

	~CGrasperHead(void)
	{
		SafeDeleteArray(m_pLeftJawVertex);
		SafeDeleteArray(m_pRightJawVertex);
		SafeDeleteArray(m_pLeftJawVertexIndex);
		SafeDeleteArray(m_pRightJawVertexIndex);
	}

	void setLeftJaw(const int maxnv, const vector<Vector3d>& pos, const vector<int>& index);
	void setRightJaw(const int maxnv, const vector<Vector3d>& pos, const vector<int>& index);

	//Jaw state: open, close, no change
	int getJawState(void) const
	{
		const double dif = m_jawAngleRadian - m_jawAngleRadianOld;
		if (fabs(dif)<0.005) return 0;  // NO CHANGE?
		if (dif>0) 
			return 1;	//OPEN
		else
			return -1;	//close
	}

	double getJawAngleRadian(void) const 
	{
		return m_jawAngleRadian;
	}

	double openJaw(void)
	{
		const double ANGLE_LIMIT = PI*0.5;
		m_jawAngleRadianOld = m_jawAngleRadian;
		m_jawAngleRadian += m_jawAngleSpeed;
		if (m_jawAngleRadian>ANGLE_LIMIT) 
			m_jawAngleRadian = ANGLE_LIMIT;
		return m_jawAngleRadian;
	}

	double closeJaw(void)
	{
		const double ANGLE_LIMIT = 0;
		m_jawAngleRadianOld = m_jawAngleRadian;
		m_jawAngleRadian -= m_jawAngleSpeed;
		if (m_jawAngleRadian<ANGLE_LIMIT) 
			m_jawAngleRadian = ANGLE_LIMIT;
		return m_jawAngleRadian;
	}

	void decideTranslationRotation(void);

	void applyRotation(Vector3d *pVertex);

	void addTranslation(const Vector3d & t)
	{
		m_translation+=t;
	}

	void getJawAngleRadian(double &ang, double &angold)
	{
		ang = m_jawAngleRadian;
		angold= m_jawAngleRadianOld;
	}

};



/*
class CGrasperTriObj: public CTexturedTriangleObj
{
private:
	CGrasperHead *m_pGrasperHead;

	bool * m_pMedialBuffer; 

	void _initJaws(
		const Vector3d *pVertex, const int nVertex, const int *pPoly, const int nPolygon, const int plytype, 
		const OglMaterial material[], const unsigned int faceMatIndex[], const int nFaceMatIndex);


public:	

	CGrasperTriObj(Vector3d *pVertex, const int nVertex, Vector3i *pBoundaryTri, const int nTri, const bool allocbuffer=false):
		CTexturedTriangleObj(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer)
	{
		m_pGrasperHead = NULL;
		m_pMedialBuffer = NULL;
	}

	virtual ~CGrasperTriObj(void)
	{
		SafeDelete(m_pGrasperHead);
		SafeDeleteArray(m_pMedialBuffer);
	}

	virtual void glDraw(const CGLDrawParms &dparms);
	
};
*/

#endif