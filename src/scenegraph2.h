// scenegraph.h
#ifndef __INC_SCENE_GRAPH__
#define __INC_SCENE_GRAPH__

#include <vector>
#include <typeinfo>
#include <qobject.h>
#include <qwidget.h>
#include <QGLViewer/manipulatedFrame.h>

#include <sysconf.h>
#include <femattrib.h>
#include <drawparms.h>
#include <object3d.h>
#include "cia3dobj.h"
#include "cia3dfile.h"
#include "tetraobj.h"
#include "hexobj.h"

using namespace std;

inline void DrawBoundingBox(const Vector3f& v0, const Vector3f& v7)
{
	Vector3f v1(v7.x, v0.y, v0.z);
	Vector3f v2(v0.x, v7.y, v0.z);
	Vector3f v3(v7.x, v7.y, v0.z);
	Vector3f v4(v0.x, v0.y, v7.z);
	Vector3f v5(v7.x, v0.y, v7.z);
	Vector3f v6(v0.x, v7.y, v7.z);

	glBegin(GL_LINE_STRIP);
	glVertex3fv((float*)(&v0)); glVertex3fv((float*)(&v1));
	glVertex3fv((float*)(&v3)); glVertex3fv((float*)(&v2));
	glVertex3fv((float*)(&v0));
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3fv((float*)(&v4)); glVertex3fv((float*)(&v5));
	glVertex3fv((float*)(&v7)); glVertex3fv((float*)(&v6));
	glVertex3fv((float*)(&v4));
	glEnd();

	glBegin(GL_LINES);
	glVertex3fv((float*)(&v0)); glVertex3fv((float*)(&v4));
	glVertex3fv((float*)(&v1)); glVertex3fv((float*)(&v5));
	glVertex3fv((float*)(&v2)); glVertex3fv((float*)(&v6));
	glVertex3fv((float*)(&v3)); glVertex3fv((float*)(&v7));
	glEnd();
}

inline void DrawBoundingBox(const Vector3d& v0, const Vector3d& v7)
{
	Vector3d v1(v7.x, v0.y, v0.z);
	Vector3d v2(v0.x, v7.y, v0.z);
	Vector3d v3(v7.x, v7.y, v0.z);
	Vector3d v4(v0.x, v0.y, v7.z);
	Vector3d v5(v7.x, v0.y, v7.z);
	Vector3d v6(v0.x, v7.y, v7.z);

	glBegin(GL_LINE_STRIP);
	glVertex3dv((&v0.x)); glVertex3dv((&v1.x));
	glVertex3dv((&v3.x)); glVertex3dv((&v2.x));
	glVertex3dv((&v0.x));
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex3dv((&v4.x)); glVertex3dv((&v5.x));
	glVertex3dv((&v7.x)); glVertex3dv((&v6.x));
	glVertex3dv((&v4.x));
	glEnd();

	glBegin(GL_LINES);
	glVertex3dv(&v0.x); glVertex3dv(&v4.x);
	glVertex3dv(&v1.x); glVertex3dv(&v5.x);
	glVertex3dv(&v2.x); glVertex3dv(&v6.x);
	glVertex3dv(&v3.x); glVertex3dv(&v7.x);
	glEnd();
}


class CDeformedControl
{
public:
	int m_nType;
	Vector3f m_Vertex[4];

	CDeformedControl()
	{
		m_nType = -1;
	}

	CDeformedControl(const int etype, const Vector3f *v)
	{
		m_nType = etype;
		m_Vertex[0]=v[0];
		m_Vertex[1]=v[1];
		m_Vertex[2]=v[2];
		m_Vertex[3]=v[3];
	}

	CDeformedControl(const int etype, const Vector3d *v)
	{
		m_nType = etype;
		for (int i=0; i<4; i++)
			m_Vertex[i]=Vector3f(v[i].x, v[i].y, v[i].z);
	}
};


class CSeparationQuad
{
public:
	Vector3f m_Vertex[4];
	Vector4f m_Norm;
	int m_MajorAxis;

private:
	void _computeNormalAndAxis(void)
	{
		Vector3f n = compute_triangle_normal(m_Vertex[0], m_Vertex[1], m_Vertex[2]);
		m_Norm.x = n.x;
		m_Norm.y = n.y;
		m_Norm.z = n.z;
		m_Norm.w = -DotProd(n, m_Vertex[0]);

		if (n.x<0) n.x=-n.x;
		if (n.y<0) n.y=-n.y;
		if (n.z<0) n.z=-n.z;
		const float maxn = _MAX3_(n.x, n.y, n.z);
		if (n.x==maxn) 
			m_MajorAxis=0;
		else if (n.y==maxn) 
			m_MajorAxis=1;
		else
			m_MajorAxis=2;
	}

public:

	CSeparationQuad(void){};

	CSeparationQuad(const Vector3f &v0, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3)
	{
		m_Vertex[0] = v0;
		m_Vertex[1] = v1;
		m_Vertex[2] = v2;
		m_Vertex[3] = v3;
		_computeNormalAndAxis();
	}

	CSeparationQuad(const Vector3f v[4])
	{
		m_Vertex[0] = v[0];
		m_Vertex[1] = v[1];
		m_Vertex[2] = v[2];
		m_Vertex[3] = v[3];
		_computeNormalAndAxis();
	}

	void project2D(float x[4], float y[4])
	{/*
		const int m = quad.m_MajorAxis;
		Vector3f *p = m_Vertex;
		if (m==0){
			x[0]=p->y, y[0]=p->z; p++;
			x[1]=p->y, y[1]=p->z; p++;
			x[2]=p->y, y[2]=p->z; p++;
			x[3]=p->y, y[3]=p->z;
		}
		else if (m==1){
			x[0]=p->z, y[0]=p->x; p++;
			x[1]=p->z, y[1]=p->x; p++;
			x[2]=p->z, y[2]=p->x; p++;
			x[3]=p->z, y[3]=p->x; p++;
		}
		*/
		assert(0);
	}

};


class CBaseMapping: public CObjectBase
{

public:
	CPolyObj *m_pSrcObj; //source object

	CPolyObj *m_pDstObj; //destination object

	Vector3f *m_pNormal; //for deforming the normal array, used by MLS deformation

	Vector4f *m_pQuat;   //rotation quaternion for deformation, used by MLS deformation

public:

	CBaseMapping(void): CObjectBase()
	{
		m_pSrcObj = m_pDstObj = NULL;
		m_pNormal = NULL;
		m_pQuat = NULL;
	}

	virtual ~CBaseMapping(){}
	
	
	virtual void topologyChange(void) = 0;
	
	//Update the mapping whenever the source object changes
	virtual void updateMapping(const Vector3d* vin, Vector3d* vout) = 0;

	virtual void updateMapping(const Vector3d* vin, Vector3f* vout) = 0;

	virtual bool canDeformNormal(void)
	{
		return false;
	}
};


class CBaseRendering: public CObjectBase
{
protected:
	CPolyObj *m_pVisualObj;
	CBaseMapping *m_pMapping;

public:
	CBaseRendering(void): CObjectBase()
	{
		m_pVisualObj = NULL;
		m_pMapping = NULL;
	}

	virtual ~CBaseRendering(){}
		
	virtual void glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdrawparm){}
};


class CSceneNode : public CObject3DBase
{
private:

public: //vars;

	CPolyObj		*m_pObject;				//Visual polygonal object

	CPolyObj		*m_pCollisionObject;	//Collision object, since CIA3D doesnot support, we use it for laptools

	CPolyObj		*m_pSimulationObject;	//Simulation solid, will drive the visual one

	CBaseMapping	*m_pMapping;			//Mapping relation between the two

	CBaseRendering  *m_pRendering;			//GPU and other faster renderer, enhanced for the functions defined in CPolyObj

	CGLDrawParms	m_DrawParms;			//Controls the visual display options

	CObjectFemAttrib m_FemAttrib;			//FE material attributes

	qglviewer::ManipulatedFrame *m_pFrame;	//transform for OpenGL rendering 

    bool m_bFramePtrOwned;					//boolean sharing flag for m_pFrame 

private:

	void _initNode(CPolyObj *p=NULL);

public: //funcs

    CSceneNode(void): CObject3DBase(), m_DrawParms(), m_FemAttrib()
	{
		_initNode(NULL);
	}

    CSceneNode(CPolyObj *p): m_DrawParms(), m_FemAttrib()
	{
		_initNode(p);
	}

	virtual ~CSceneNode()
	{
		// memory leak, don't care at this time, for debug
		//if (m_bFramePtrOwned) 
        //    SafeDelete(m_pFrame);
	}

	bool isEmptyNode(void)
	{
		return  (m_pObject==NULL) && 
				(m_pSimulationObject==NULL) && 
				(m_pCollisionObject==NULL) && 
				(m_pMapping==NULL);
	}

	bool isMirroredNode(void) 
	{
		char *oname = this->GetObjectName();
		if (strstr(oname, "M")!=NULL)
			return true;
		return false;
	}

	void setMirroredNode(void) 
	{
		char *oname = this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<14);
		oname[nlen]='M';
		oname[nlen+1]=0;
	}

	bool isRot90Node(void) 
	{
		char *oname = this->GetObjectName();
		if (strstr(oname, "R0")!=NULL)
			return true;
		return false;
	}

	void setRot90Node(void)
	{
		char *oname = this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<13);
		oname[nlen]='R';
		oname[nlen+1]='0';
		oname[nlen+2]=0;
	}
	
	bool isRot180Node(void) 
	{
		char *oname = this->GetObjectName();
		if (strstr(oname, "R1")!=NULL)
			return true;
		return false;
	}

	void setRot180Node(void)
	{
		char *oname = this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<13);
		oname[nlen]='R';
		oname[nlen+1]='1';
		oname[nlen+2]=0;
	}

	bool isRot270Node(void) 
	{
		char *oname = this->GetObjectName();
		if (strstr(oname, "R2")!=NULL)
			return true;
		return false;
	}

	void setRot270Node(void)
	{
		char *oname = this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<13);
		oname[nlen]='R';
		oname[nlen+1]='2';
		oname[nlen+2]=0;
	}

	bool isOriginalNode() 
	{
		if (isMirroredNode() || 
			isRot270Node() ||
			isRot180Node() ||
			isRot90Node())
			return false;
		return true;
	}

	void setFemAttrib(const char *attribName, const char *attribValue)
	{
		double v0, v1, v2;
		sscanf(attribValue, "%lf, %lf, %lf", &v0, &v1, &v2);
		if (strcmp(attribName, "Displacement")==0){
			m_FemAttrib.m_vDisplacement = Vector3d(v0,v1,v2);
		}
		else if (strcmp(attribName, "Velocity")==0){
			m_FemAttrib.m_vVelocity = Vector3d(v0,v1,v2);
		}
		else if (strcmp(attribName, "Shell Thickness")==0){
			m_FemAttrib.m_lfShellThickness = v0;
		}
	}

	void setMaterialAttrib(const char *matname)
	{
		const int slen = strlen(matname);
		if (slen>=40){
			fprintf(stderr, "The material name is too long: %s\n", matname);
			return;
		}
		strcpy(m_FemAttrib.m_strMatName, matname);
	}	
	
	void applyManipulation(qglviewer::ManipulatedFrame * p=NULL);
   
	Vector3f localToWorldTransform(const Vector3f& v) const;

	Vector3d localToWorldTransform(const Vector3d& v) const;

	void localToWorldTransform(const Vector3f *v,  const int len, Vector3f *tv) const;

	void localToWorldTransform(const Vector3d *v,  const int len, Vector3d *tv) const;

	Vector3f worldToLocalTransform(const Vector3f & v) const;

	Vector3d worldToLocalTransform(const Vector3d & v) const;

	void worldToLocalTransform(const Vector3f* v, const int len, Vector3f *tv) const;

	void worldToLocalTransform(const Vector3d* v, const int len, Vector3d *tv) const;

	AxisAlignedBox worldBoundingBox(void) const;

	void glDraw(const Vector3f &campos, const Vector3f &camdir, const Vector3d *pDeformedVertices, Vector3d *vertexbuff, const bool outputMesh=false);

};


class CSceneGraph: public QObject
{
	Q_OBJECT

signals:
	void activeSceneNodeChanged(CSceneNode* p);
	void pickingCursorChanged(QCursor *qcur);
	void updateGLWindowRendering(void);
	void updateGLWindowsBoundingBox(const Vector3d&, const Vector3d&);

public:
	enum PICKING_TYPE_ID{
		PICK_OBJECT=0,	//start
		PICK_FACE,		//==1
		PICK_LINE,		//==2
        PICK_NODE,       
		PICK_ELEMENT, 
		PICK_NULL_TYPE,	
	};

	CFemObjAttrib m_FemObjAttrib;

	vector<CDeformedControl> m_DeformedControlList;
	vector<CSeparationQuad> m_SeparationQuadList;

private:

	vector<CSceneNode> m_SceneList;		//list of scene nodes;

	//active object, face, and line
	enum PICKING_TYPE_ID m_nPickingType;	//picking which obj, face, edge, vertex, etc...

	CSceneNode * m_pActiveSceneNode;		//active scene node;
	CSceneNode * m_pActiveSceneNode4Face;	//active scene node;
	CSceneNode * m_pActiveSceneNode4Edge;	//active scene node;
	CSceneNode * m_pActiveSceneNodeMaster;	//active scene node;
	CSceneNode * m_pActiveSceneNodeSlave;	//active scene node;
	int m_nActiveFaceID;
	int m_nActiveVertexID0;
	int m_nActiveVertexID1;

	//saved mirror information
	Vector3d m_savedMirrorTriangle[3];
	Vector3d m_savedLineVertices[2];

private:

	void _initSceneGraph(void)
	{
		m_pActiveSceneNode = NULL;
		m_pActiveSceneNode4Face = NULL;
		m_pActiveSceneNode4Edge = NULL;
		m_pActiveSceneNodeMaster = NULL;	
		m_pActiveSceneNodeSlave = NULL;

		m_nPickingType = CSceneGraph::PICK_OBJECT;	

		m_nActiveFaceID = -1;
		m_nActiveVertexID0 = -1;
		m_nActiveVertexID1 = -1;

		m_savedMirrorTriangle[0] = Vector3d(0);
		m_savedMirrorTriangle[1] = Vector3d(0);
		m_savedMirrorTriangle[2] = Vector3d(0);
		m_savedLineVertices[0] = Vector3d(0);
		m_savedLineVertices[1] = Vector3d(0);
	}

	void _invalidNodePointers(const CSceneNode * p)
	{
		if (p == m_pActiveSceneNode){
			m_pActiveSceneNode = NULL;
			setActiveSceneNode(NULL);
		}
		if (p == m_pActiveSceneNode4Face)
			m_pActiveSceneNode4Face = NULL;
		if (p == m_pActiveSceneNode4Edge)
			m_pActiveSceneNode4Edge = NULL;
		if (p == m_pActiveSceneNodeMaster)
			m_pActiveSceneNodeMaster = NULL;
		if (p == m_pActiveSceneNodeSlave)
			m_pActiveSceneNodeSlave = NULL;
	}

	int findAllNamedObjects(const char *objname, int idbuffer[], const int bufflen)
	{
		int n=SceneNodeCount();
		int c = 0;
		for (int i=0; i<n; i++){
			CSceneNode *p = GetSceneNode(i);
			const char *pstr = p->m_pObject->Description();
			if (strcmp(pstr, objname)==0){
				idbuffer[c] = i;
				c++;
			}
		}
		assert(c<bufflen);
		return c;
	}

	void _exportGroup1(FILE *fp, CC3d1inIOControl &cio, int setID[]);
	void _exportGroup2(FILE *fp, const CC3d1inIOControl &cio);
	void _exportGroup3(FILE *fp, const int setID[]);
	void _exportGroup4(FILE *fp);
	void _exportGroup5(FILE *fp);
	void _exportGroup6(FILE *fp);
	void _exportGroup7(FILE *fp);

	void _exportPovrayMesh(FILE *fp);
 
public:

	void hookObject(QObject *pobj);

	void hookGLWindow(QGLViewer *pwin);

	CSceneGraph(void)
	{
		m_SceneList.reserve(32);
		m_SceneList.clear();
		m_DeformedControlList.reserve(16);
		m_DeformedControlList.clear();
		m_SeparationQuadList.reserve(32);
		m_SeparationQuadList.clear();

		_initSceneGraph();
	}

	CSceneNode* AddSceneNode(const CSceneNode &node)
	{	
		m_SceneList.push_back(node);
		const int nsize = m_SceneList.size()-1;
		return &m_SceneList[nsize];
	}

	void DeleteSceneNode(const int i)
	{
		CSceneNode * p = &(m_SceneList[i]);
		_invalidNodePointers(p);

		vector<CSceneNode>::iterator k = m_SceneList.begin();
		m_SceneList.erase(k+i);
	}

	void RemoveAll(void)
	{
		setActiveSceneNode(NULL);
		_initSceneGraph();
		m_SceneList.clear();

		m_DeformedControlList.clear();
		m_SeparationQuadList.clear();

	}

	void RemoveAllMirroredObjects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isMirroredNode()){
				vector<CSceneNode>::iterator k = m_SceneList.begin();
				m_SceneList.erase(k+i);
				if (m_pActiveSceneNode==pnode)
					m_pActiveSceneNode = NULL;
			}
		}
	}

	void RemoveAllRot90Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot90Node()){
				vector<CSceneNode>::iterator k = m_SceneList.begin();
				m_SceneList.erase(k+i);
				if (m_pActiveSceneNode==pnode)
					m_pActiveSceneNode = NULL;
			}
		}
	}

	void RemoveAllRot180Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot180Node()){
				vector<CSceneNode>::iterator k = m_SceneList.begin();
				m_SceneList.erase(k+i);
				if (m_pActiveSceneNode==pnode)
					m_pActiveSceneNode = NULL;
			}
		}
	}

	void RemoveAllRot270Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot270Node()){
				vector<CSceneNode>::iterator k = m_SceneList.begin();
				m_SceneList.erase(k+i);
				if (m_pActiveSceneNode==pnode)
					m_pActiveSceneNode = NULL;
			}
		}
	}

	int SceneNodeCount(void) const
	{
		return m_SceneList.size();
	}

	int SceneNodeIndex(const CSceneNode *p) 
	{
		const int nsize = m_SceneList.size();
		for (int i=0; i<nsize; i++){
			const CSceneNode &pp = m_SceneList[i];
			if ((&pp)==p) return i;
		}
		return -1;
	}

	CSceneNode *GetSceneNode(const int i)
	{
		CSceneNode *p=NULL;
		int n = m_SceneList.size();
		if (i>=0 && i<n)
			p = &(m_SceneList[i]);
		return p;
	}
	
	//==================PICKING functions===================
	CSceneNode* GetActiveSceneNode(void)
	{
		return m_pActiveSceneNode;
	}

	void GetActiveSceneNodeForAlignment(CSceneNode* &pmaster, CSceneNode * &pslave)
	{
		pmaster = m_pActiveSceneNodeMaster;	
		pslave = m_pActiveSceneNodeSlave;	
	}

    void setActiveSceneNodeForAlignment(CSceneNode* pmaster, CSceneNode * pslave)
	{
		m_pActiveSceneNodeMaster = pmaster;	
		m_pActiveSceneNodeSlave = pslave;	
	}

	void GetActiveSceneNodeFace(CSceneNode* &pnode, int &faceid)
	{
		pnode = m_pActiveSceneNode4Face;
		faceid = m_nActiveFaceID;
	}

	void SetActiveSceneFaceForAlignment(CSceneNode* pnode, const int faceid)
	{
		m_pActiveSceneNode4Face = pnode;
		m_nActiveFaceID = faceid;
	}

	//In case there is no active facet, the saved position is returned
	void GetActiveFaceLocal(CSceneNode *&p, Vector3d vert[], int &nvert)
	{
		p = m_pActiveSceneNode4Face;
		if (p==NULL){
			nvert = 0; 
			return;
		}
		CPolyObj *ply = (CPolyObj*)p->m_pObject;
		ply->getPolygonCoordinates(m_nActiveFaceID, vert, nvert);
	}

	void GetActiveFaceLocal(CSceneNode *&p, Vector3f vert[], int &nvert)
	{
        Vector3d vv[10];
        assert(nvert<=10);
        GetActiveFaceLocal(p, vv, nvert);
        for (int i=0; i<nvert; i++){
            Vector3d *pp=&vv[i];
            vert[i].x=pp->x;
            vert[i].y=pp->y;
            vert[i].z=pp->z;
        }
	}

	void GetActiveFace(CSceneNode *&p, Vector3f v[4], int &nvert)
	{
		Vector3f v2[4];
		GetActiveFaceLocal(p, v2, nvert);
		if (p)
			p->localToWorldTransform(v2, nvert, v);
		else{ 
			nvert = 0;
		}
	}

	void GetActiveFace(CSceneNode *&p, int &fid)
	{
		p = m_pActiveSceneNode4Face;	
		fid = m_nActiveFaceID;

	}

	void GetActiveLine(CSceneNode *&p, int &vid0, int &vid1)
	{
		p = m_pActiveSceneNode4Edge;	
		vid0 = m_nActiveVertexID0;
		vid1 = m_nActiveVertexID1;
	}


	void SetActiveLine(CSceneNode * p, const int & v0, const int &v1)
	{
		m_pActiveSceneNode4Edge = p;
		m_nActiveVertexID0 = v0;
		m_nActiveVertexID1 = v1;
	}

	//In case there is no active line, the saved position is returned!
	void GetActiveLineLocal(CSceneNode *& p, Vector3d & v0, Vector3d &v1)
	{
		p = m_pActiveSceneNode4Edge;
		if (p==NULL){
			return;
		}

		CPolyObj *ply = (CPolyObj*) p->m_pObject;
		const Vector3d *pVertex = ply->m_pVertex;
		v0 = pVertex[m_nActiveVertexID0];
		v1 = pVertex[m_nActiveVertexID1];
	}

	void GetActiveLine(CSceneNode *& p, Vector3d & v0, Vector3d &v1)
	{
		GetActiveLineLocal(p, v0, v1);
		if (p){
			Vector3d va[2]={v0, v1};
			Vector3d vb[2];
			p->localToWorldTransform(va, 2, vb);
			v0 = vb[0]; v1 = vb[1];
		}
	}

	void GetActiveLine(CSceneNode *& p, Vector3f & v0, Vector3f &v1)
	{
		Vector3d a, b;
		GetActiveLine(p, a, b);
		v0=Vector3f(a.x, a.y, a.z);
		v1=Vector3f(b.x, b.y, b.z);
	}

	bool HasActiveFace(void)
	{
		return (m_pActiveSceneNode4Face!=NULL);
	}

	bool HasActiveLine(void)
	{
		return (m_pActiveSceneNode4Edge!=NULL);
	}

	//This method is called at the end of the select() procedure. 
	//It should finalise the selection process and update the data 
	//structure/interface/computation/display... according to the newly selected entity.	
	//The default implementation is empty. Overload this method if needed, and use 
	void postSelection(const int objid, const int x, const int y);

	//==================PICKING functions===================
	bool HasMirroredObjects(void) 
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isMirroredNode())
				return true;
		}
		return false;
	}

	bool HasRot90Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot90Node())
				return true;
		}
		return false;
	}

	bool HasRot180Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot180Node())
				return true;
		}
		return false;
	}

	bool HasRot270Objects(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=nsize-1; i>=0; i--){
			CSceneNode * pnode = &(m_SceneList[i]);
			if (pnode->isRot270Node())
				return true;
		}
		return false;
	}

	void DumpNodeNames(void)
	{
		int i, nsize;
		nsize = m_SceneList.size();
		for (i=0; i<nsize; i++){
			CSceneNode * pnode = &(m_SceneList[i]);
			char *s = pnode->GetObjectName();
			printf("Node %2d: %s\n", i, s);
		}
	}

	void GetBoundingBox(AxisAlignedBox & box);


	//=============Object type and count query====================

	int totalVertexCount(void);

	int totalElementCount(void);

	int elementTypeCount(void);

	/*
	int findAllTetraObjects(int idbuffer[], const int bufflen)
	{
		return findAllNamedObjects("tetra", idbuffer, bufflen);
	}

	int findAllHexObjects(int idbuffer[], const int bufflen)
	{
		return findAllNamedObjects("hexobj", idbuffer, bufflen);
	}
	*/

	//=============Export different format files====================
	void exportPovrayFile(void *pQGLWindow, const char *fname);

	void exportSelectedObjNeutralMeshFile(FILE *fp);

	void exportFEMDataFile(const char *fname, CC3d1inIOControl &cio);

	//=============Rendering styles ====================

	void glDraw(CGLDrawParms & global_drawparm, vector<Vector3d>* deformedVertices=NULL,  const bool outputMesh=false);

	void _glDrawPickedObjectBoundary(void);
	void _glDrawPickedObjectAlignment(void);
	void _glDrawPickedObjectNormal(void);
	void glDrawPickedObject(void);

	void updateGLWindows(void);  
    
    void setGLWindowsBoundingBox(const Vector3d &lowleft, const Vector3d& upright);

	void glDraw4Picking(void);

	void setPickingType(const PICKING_TYPE_ID itype);

	void pickingType(PICKING_TYPE_ID& itype)
	{
		//ifunc = m_nPickingFunc;	
		//istage = m_nPickingStage;
		itype = m_nPickingType;	
	}

	void setActiveSceneNode(CSceneNode* p);

	void SetDefaultActiveSceneNode(void)
	{
		CSceneNode *p=NULL;
		int n = m_SceneList.size();
		if (n>0){
			p=&m_SceneList[n-1]; 
		}
		setActiveSceneNode(p);
	}

	void nextActiveObject(void);

	void restoreMirrorRotNodes(CSceneNode* pMirrorNode, const int fid, CSceneNode *pRotNode, const int lineid0, const int lineid1)
	{
		m_pActiveSceneNode4Face = pMirrorNode;	
		m_pActiveSceneNode4Edge = pRotNode;
		m_nActiveFaceID = fid;
		m_nActiveVertexID0 = lineid0;
		m_nActiveVertexID1 = lineid1;
	}

	//Mirror and rotation axis support
	bool mirrorTriangleBeenSet(void) const
	{
		const Vector3d z(0);
		if (m_savedMirrorTriangle[0]==z && m_savedMirrorTriangle[1]==z && m_savedMirrorTriangle[2]==z)
			return false;
		return true;
	}

	void setMirrorTriangle(const Vector3d v[3])
	{
		m_savedMirrorTriangle[0]=v[0];
		m_savedMirrorTriangle[1]=v[1];
		m_savedMirrorTriangle[2]=v[2];
	}

	void getMirrorTriangle(Vector3d v[3]) const
	{
		v[0] = m_savedMirrorTriangle[0];
		v[1] = m_savedMirrorTriangle[1];
		v[2] = m_savedMirrorTriangle[2];
	}

	bool rotationAxisBeenSet(void) const
	{
		const Vector3d z(0);
		if (m_savedLineVertices[0]==z && m_savedLineVertices[1]==z)
			return false;
		return true;
	}

	void setRotationAxis(Vector3d v[2])
	{
		m_savedLineVertices[0]=v[0];
		m_savedLineVertices[1]=v[1];
	}

	void getRotationAxis(Vector3d v[2])
	{
		v[0] = m_savedLineVertices[0];
		v[1] = m_savedLineVertices[1];
	}

};


extern CSceneNode* AddOneObjectToSceneGraph(CPolyObj* q1, CSceneGraph &m_SceneGraph, const Vector3f *rgb=NULL);

extern void AddObjectsToSceneGraph(CCia3dObj *pObjs[], const int OBUFFLENGTH, CSceneGraph &m_SceneGraph);

//extern void ExportPovrayCamera(QGLViewer* m_pGLUIWin, FILE *fp);
//extern void ExportPovrayMaterial(FILE *fp);
extern void BestMatchDrawParms(CSceneGraph & sg, CSceneNode *poldnodes, const int nlen, const bool bRecomputeTexture1DRange);

#endif
