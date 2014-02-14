//FILE: springcontainer.h
//This class is used to store all the spring constraints

#ifndef __INC_SPRINGCONTAINER_NZ20080312H__
#define __INC_SPRINGCONTAINER_NZ20080312H__

#include "polyobj.h"
#include "geomath.h"

class CSceneNode;

class CConstaintSpring
{
public:
	CSceneNode * m_pSceneNode1;
	CSceneNode * m_pSceneNode2;
	CPolyObj *m_pPolyObj1;
	CPolyObj *m_pPolyObj2;
	int m_nVertexID1;
	int m_nVertexID2;

public:

	CConstaintSpring(void)
	{
		m_pSceneNode1 = m_pSceneNode2 = NULL;
		m_pPolyObj1 = m_pPolyObj2 = NULL;
		m_nVertexID1 = m_nVertexID2 = -1;
	}

	CConstaintSpring(CSceneNode *pnode1, CPolyObj * pobj1, const int vertid1, CSceneNode *pnode2, CPolyObj *pobj2, const int vertid2)
	{
		m_pSceneNode1 = pnode1;
		m_pSceneNode2 = pnode2;
		m_pPolyObj1 = pobj1;
		m_pPolyObj2 = pobj2;
		m_nVertexID1 = vertid1;
		m_nVertexID2 = vertid2;
	}

	bool operator ==(const CConstaintSpring & a)
	{
		bool r=false;
		if (a.m_pSceneNode1 == m_pSceneNode1 &&
			a.m_pSceneNode2 == m_pSceneNode2 &&
			a.m_pPolyObj1 == m_pPolyObj1 &&
			a.m_pPolyObj2 == m_pPolyObj2 &&
			a.m_nVertexID1 == m_nVertexID1 &&
			a.m_nVertexID2 == m_nVertexID2)
			r= true;
		else if (a.m_pSceneNode1 == m_pSceneNode2 &&
			a.m_pSceneNode2 == m_pSceneNode1 &&
			a.m_pPolyObj1 == m_pPolyObj2 &&
			a.m_pPolyObj2 == m_pPolyObj1 &&
			a.m_nVertexID1 == m_nVertexID2 &&
			a.m_nVertexID2 == m_nVertexID1)
			r= true;
		return r;
	}

	bool operator !=(const CConstaintSpring & a)
	{
		return (!(*this == a));
	}

	~CConstaintSpring(void)
	{
	}
};


class CSpringContainer: public CPolyObj
{

private:
	
	vector<CConstaintSpring> springs;

private:

	bool _springInBuffer(const CConstaintSpring & s)
	{
		bool r=false;
		const int nsize = springs.size();
		for (int i=0; i<nsize; i++){
			if (springs[i]==s){
				r=true;
				break;
			}
		}
		return r;
	}

public:

	CSpringContainer(void):CPolyObj()
	{
        m_nPlyVertexCount=0;
        m_nElmVertexCount=2;
		springs.clear();
	}

	void addSpring(const CConstaintSpring & spring);

	virtual ~CSpringContainer()
	{
		springs.clear();
	}

	//=====================Drawing functions================================
	
	virtual void glDraw(const CGLDrawParms &dparms);

	//======================Picking functions===============================
	virtual void DrawPickingObject(const int objid);

	virtual void DrawPickingObjectFace(const int objid);

	virtual void DrawPickingObjectLine(const int objid);

	virtual void GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1);

	virtual void GetPickedLine(const int eid, int & v0, int &v1);

	virtual void GetPickedPlane(const int eid, Vector3d v[4]);

	virtual int GetPickableElementCount(const int ptype)
	{
		int n=0;
		if (ptype==0) n = 1;						//obj
		else if (ptype==1) n = m_nElementCount;		//face
		else if (ptype==2) n = m_nElementCount*2;	//edge
		return n;
	}

	virtual const char *Description(void)
	{
		return "rod";
	}

	virtual void getPolygonCoordinates(const int index, Vector3d v[], int &nv)
	{ 
		nv = 0;
		return;
	}

	virtual double estimatedVertexRadius(void);


	//====================Export to various formats;====================
	virtual int exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double th=0);

	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

};




#endif