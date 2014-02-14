// scenegraph.h
#ifndef __INC_SCENE_GRAPH_NZ20080312__
#define __INC_SCENE_GRAPH_NZ20080312__

#include "scenenode.h"
#include "cia3dobj.h"
#include "cia3dfile.h"
#include "tetraobj.h"
#include "hexobj.h"


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

	//An object to store all the constraint springs;
	//Since the springs will connect vertices from other objects, we will not 
	//allocate vertex buffer for these springs
	CPolyObj *m_pConstraintSpringObj;

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
		m_pConstraintSpringObj = NULL;

		m_pActiveSceneNode = NULL;
		m_pActiveSceneNode4Face = NULL;
		m_pActiveSceneNode4Edge = NULL;
		m_pActiveSceneNodeMaster = NULL;	
		m_pActiveSceneNodeSlave = NULL;

		m_nPickingType = CSceneGraph::PICK_OBJECT;	

		m_nActiveFaceID = -1;
		m_nActiveVertexID0 = -1;
		m_nActiveVertexID1 = -1;

		const Vector3d z0(0,0,0);
		m_savedMirrorTriangle[0] = z0;
		m_savedMirrorTriangle[1] = z0;
		m_savedMirrorTriangle[2] = z0;
		m_savedLineVertices[0] = z0;
		m_savedLineVertices[1] = z0;
	}

	void _invalidNodePointers(const CSceneNode * p);

	/*
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
	*/

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

	CSceneGraph(void):
		m_SceneList(), 
		m_DeformedControlList(),
		m_SeparationQuadList()
	{
		m_SceneList.reserve(32);
		m_SceneList.clear();
		m_DeformedControlList.reserve(16);
		m_DeformedControlList.clear();
		m_SeparationQuadList.reserve(32);
		m_SeparationQuadList.clear();

		_initSceneGraph();
	}

	virtual ~CSceneGraph(void)
	{
		SafeDelete(m_pConstraintSpringObj);
	}

	//add a scene node into the scene graph
	CSceneNode* AddSceneNode(const CSceneNode &node, qglviewer::ManipulatedFrame *pFrame=NULL, const int centerstyle=0);

	void DeleteSceneNode(const int i)
	{
		CSceneNode * p = &(m_SceneList[i]);
		_invalidNodePointers(p);

		vector<CSceneNode>::iterator k = m_SceneList.begin();
		m_SceneList.erase(k+i);
	}

	void RemoveAll(void)
	{
		//delete the spring obj
		SafeDelete(m_pConstraintSpringObj);

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

	int size(void) const
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
			const char *s = pnode->GetObjectName();
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
	void glDraw(CGLDrawParms & global_drawparm, 
		vector<Vector3d>* deformedVertices=NULL,  
		const vector<Vector4i>* pDeformedQuads = NULL,
		const vector<int>* pQuadCount=NULL,
		const bool outputMesh=false);

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
		const Vector3d z(0, 0, 0);
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
		const Vector3d z(0,0,0);
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

extern void BestMatchDrawParms(CSceneGraph & sg, CSceneNode *poldnodes, const int nlen, const bool bRecomputeTexture1DRange);

#endif
