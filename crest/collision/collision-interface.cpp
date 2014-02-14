//FILE: collision-interface.cpp
#include <view3dn/scenegraph.h>
#include <crest/extlibs/CollisionDR/OMap.h>
#include <crest/extlibs/CollisionDR/CrestObjects.h>
#include <crest/extlibs/CollisionDR/OMap.h>

#include <view3dn/scenegraph.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/collision/mousegrabber.h>
#include <crest/simulation/simu_engine.h>
#include <crest/system/ztime.h>
#include <view3dn/grasper.h>
#include <view3dn/texturedtmeshobj.h>
#include "collision-interface.h"


int USE_INTERNAL_SIMULATOR=1;

static CrestObjects objs;

void clearCollisonBuffer(void)
{
	objs.ClearObjectInfo();
}

void clearResponseData(void)
{
	const BOOL bC = true;
	const BOOL bR = true;
	objs.ClearCRNodes(bC, bR);
}

//==================================================================
//for the mouse grabber class implmentation
using namespace cia3d::collision;

static inline void 
checkGrabberTools(CSceneGraph &laptool, Vector3d *deformedVertices, const int vlen)
{
	if (deformedVertices==NULL) return;
	const int nnode = laptool.SceneNodeCount();
	for (int i=0; i<nnode; i++){
		CSceneNode *pnode = laptool.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pObject;
		ASSERT0(pobj!=NULL);
		if (pobj->m_nVertexCount==1){
			CMouseGrabber *ptool = dynamic_cast<CMouseGrabber *>(pobj);
			if (ptool==NULL) continue;
			//if the grabbing vertex ID is not initialized, we need to find which one to grab
			const double *pmatrix = NULL;
			if (pnode->m_pFrame)
				pmatrix = pnode->m_pFrame->matrix();
			ptool->findGrabbingVertex(pmatrix, deformedVertices, vlen);
		}
	}
	//if (nnode>0) CCollisionResponseBuffer::increaseDisplacementCounter();
}

void checkGrabberTools(CSceneGraph &laptool, vector<Vector3d> *deformedBuf)
{
	Vector3d *deformedVertices= NULL;
	int vlen = 0;
	if (deformedBuf){
		vector<Vector3d> & x = (*deformedBuf);
		//This is the raw fortran input, the array index starts from 1
		vlen = x.size();
		if (vlen>1)
			deformedVertices = &x[0];
		else
			vlen = 0;
	}

	checkGrabberTools(laptool, deformedVertices, vlen);
}

//For mouse grabber, the return value should be the abs. vertex position
void getMouseGrabberPositionConstraints(
	CSceneGraph &laptool, const vector<Vector3d> *deformedBuf, CCollisionResponseBuffer &colbuf)
{
	//check validity
	if (deformedBuf==NULL) return;
	const vector<Vector3d> & x = (*deformedBuf);
	if (x.size()<1) return;

	//prepare buffers
	ASSERT0(colbuf.nBufferLength==0);
	int& n = colbuf.nBufferLength;
	Vector3d *pDisp = colbuf.dispbuffer;
	int *pIndex= colbuf.indexbuffer;

	const int nnode = laptool.SceneNodeCount();
	for (int i=0; i<nnode; i++){
		CSceneNode *pnode = laptool.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pObject;
		ASSERT0(pobj!=NULL);
		if (pobj->m_nVertexCount==1){
			CMouseGrabber *ptool = dynamic_cast<CMouseGrabber *>(pobj);
			if (ptool==NULL) continue;
			//return the absolute vertex coor.
			pDisp[n] = ptool->m_pVertex[1]; // - ptool->m_pVertex[2];
			pIndex[n] = ptool->getRealtimePosID();
			ASSERT0(pIndex[n]!=-1);
			printf("Location %d is (%lg %lg %lg), FORTRAN VID is %d\n", n, pDisp[n].x, pDisp[n].y, pDisp[n].z, pIndex[n]);
			n++;
			if (n>colbuf.DISPACEMENT_BUFFER_LENGTH) break;
		}
	}
}


//===========================================================

namespace cia3d{
namespace collision{

int checkIntLong(void)
{
	const int nObjectType = TISSUE_TYPE;
	CrestObjectInfo o1(nObjectType);
	o1.pElement = NULL; o1.pVertex = NULL;
	CrestObjects ss; ss.AddObjectInfo(o1);
	if (sizeof(int) != sizeof(long)){
		printf("INT/LONG size mismatch. Problem in collision detection!\n"); exit(0);
	}
	return 1;
}
//static int r = checkIntLong();


static void 
_convertQuadMeshToTriMesh(
	const int *pPly, const int nPly, const int nPlyVertexCount, const bool allocbuffer, //input
	int *&pTriPly, int &nTriPly)								//output
{
	if (nPlyVertexCount==3){
		pTriPly = (int*)pPly; nTriPly = nPly;
		return;
	}
	//for quad mesh, here we pre-allocate a large enough mesh 
	nTriPly = nPly + nPly;
	Vector3i* ptri=NULL;
	if (allocbuffer){
		const int nSize = DETERMINE_DEFORMABLE_POLYSIZE(nTriPly);
		ptri = new Vector3i [nSize];
	}
	else
		ptri = (Vector3i*)pTriPly;
	assert(ptri!=NULL);
	//copy quad mesh into tri.
	const Vector4i* pquad = (const Vector4i *)pPly;
	for (int i=0; i<nPly; i++){
		const Vector4i& quad = pquad[i];
		const int j = i+i;
		ptri[j] = Vector3i(quad.x, quad.y, quad.z);
		ptri[j+1] = Vector3i(quad.x, quad.z, quad.w);
	}
	pTriPly = (int *)ptri;
}

inline int 
addTissueObjects(
	const Vector3d *fortDeformVert, 
	CSceneGraph & scene, 
	CrestObjects & objs, 
	const bool hastopologychange, 
	const bool updateonly, 
	int &nTotalFace)
{
	const int FORTVBASE = 1;
	int nVertexBase = FORTVBASE;
	const int tissueBaseID = 4;
	const int nn = scene.SceneNodeCount();
	const int nObjectType = TISSUE_TYPE;
	CrestObjectInfo o1(nObjectType);
	nTotalFace = 0;

	for (int i=0; i<nn; i++){
		CSceneNode * pnode = scene.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pSimulationObject;
		if (pobj==NULL) pobj = pnode->m_pObject;	

		o1.nID = i + tissueBaseID;     //OBJect ID, tissue starts at FOUR
		o1.nObjectType = nObjectType;

		o1.pVertex = (DFloat*)&fortDeformVert[nVertexBase];
		pobj->ComputeVertexNormals();
		o1.pNormal = &pobj->m_pVertexNorm[0].x;
		o1.nVertex = pobj->m_nVertexCount;

		o1.pElement = (long*)pobj->m_pElement;
		o1.nElement = pobj->m_nElementCount;
		o1.nElementType = pobj->m_nElmVertexCount;

		{//boundary surface handling 
			int *pPly = pobj->m_pPolygon;
			const int nPly = pobj->m_nPolygonCount;
			const int nPlyVertexCount = pobj->m_nPlyVertexCount;
			int *pTriPly = NULL, nTriPly=0;
			if (!updateonly){
				const bool allocbuffer = true;
				_convertQuadMeshToTriMesh(pPly, nPly, nPlyVertexCount, allocbuffer, pTriPly, nTriPly);
			}
			else{
				CrestObjectInfo *pcrestobj = objs.GetObjectInfo(o1.nID);
				pTriPly = (int*)pcrestobj->pPolygon;
				nTriPly = pcrestobj->nPolygon;
				if (hastopologychange){
					const bool allocbuffer = false;
					_convertQuadMeshToTriMesh(pPly, nPly, nPlyVertexCount, allocbuffer, pTriPly, nTriPly);
				}
			}
			o1.nPolygonType = 3;
			o1.pPolygon = (long*)pTriPly;
			o1.nPolygon = nTriPly;
		}

		//if there is a need to only update information, we don't need to add it once 
		//again since it is already there
		if (updateonly){
			CrestObjectInfo *pcrestobj = objs.GetObjectInfo(o1.nID);
			pcrestobj->pVertex = o1.pVertex;
			pcrestobj->nVertex = o1.nVertex;
			pcrestobj->pNormal = o1.pNormal;
			pcrestobj->nPolygon = o1.nPolygon;
			pcrestobj->pPolygon = o1.pPolygon;
		}
		else
			objs.AddObjectInfo(o1);		
		nVertexBase+= pobj->m_nVertexCount;
		nTotalFace += o1.nPolygon;
	}
	return nn;
}

static void initPoleTool(CrestInstrumentInfo& tool, const int ID)
{
	tool.fJawRadian = 0;
	tool.fJawRadianOld = 0;
	tool.id = ID;
	tool.nGraspStatus = 0;
}


static void initGrasperTool(CrestInstrumentInfo& tool, const int ID, CGrasperHead & ghead, bool *pMedial)
{
	double ang, angold;
	ghead.getJawAngleRadian(ang, angold);
	tool.fJawRadian = ang;
	tool.fJawRadianOld = angold;
	tool.id = ID;
	tool.nGraspStatus = 0;
	//int nGraspStatus;///< three status 1/-1/0 for holding/closing/releasing
	tool.pMID = pMedial;
}

inline int addInstrumentObjects(
	CSceneGraph & scene, CrestObjects & objs, const bool updateonly=false)
{
	const int nn = scene.SceneNodeCount();
	const int nObjectType = INSTRUMENT_TYPE;
	CrestObjectInfo o1(nObjectType);
	const int toolBaseID = 0;
	int i;

	//check the buffer size for tool vertex buffer
	if (nn<1) return 0;
	int nTotalVert = 0;
	for (i=0; i<nn; i++){
		const CSceneNode * pnode = (const CSceneNode *)scene.GetSceneNode(i);
		const CPolyObj *pobj = pnode->m_pObject;
		nTotalVert+= pobj->m_nVertexCount;
	}
	static vector<Vector3d> _vbuffer;
	if (_vbuffer.size()<nTotalVert){
		_vbuffer.resize(nTotalVert);
	}

	//assign the buffer
	int instrumentCount = 0;
	int nVertexBase = 0;
	Vector3d *pVertexBase = &_vbuffer[0];
	for (i=0; i<nn; i++){
		CSceneNode * pnode = scene.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pObject;
		if (pobj->m_nVertexCount<=2){//Make sure every obj is instrucment
			instrumentCount = 0;
			break;
		}
		instrumentCount++;
		pnode->localToWorldTransform(pobj->m_pVertex, pobj->m_nVertexCount, &pVertexBase[nVertexBase]);
		o1.pVertex = (DFloat*)&pVertexBase[nVertexBase];
		nVertexBase+= pobj->m_nVertexCount;
		pobj->ComputeVertexNormals();
		o1.pNormal = &pobj->m_pVertexNorm[0].x;
		o1.nVertex = pobj->m_nVertexCount;

		o1.pElement = (long*)pobj->m_pElement;
		o1.nElement = pobj->m_nElementCount;
		o1.nElementType = pobj->m_nElmVertexCount;

		o1.nPolygonType = pobj->m_nPlyVertexCount;
		o1.pPolygon = (long*)pobj->m_pPolygon;
		o1.nPolygon = pobj->m_nPolygonCount;
		o1.nID = i + toolBaseID;     //tool object id, tool starts at ZERO
		o1.nObjectType = nObjectType;

		//set intrument information
		CTexturedTriangleObj *pgrasper = dynamic_cast<CTexturedTriangleObj*>(pobj);
		if (pgrasper==NULL || pgrasper->m_pGrasperHead==NULL)
			initPoleTool(o1.IInfo, o1.nID);
		else{
			bool *pMedial =  pgrasper->m_pMedialBuffer;
			initGrasperTool(o1.IInfo, o1.nID, *(pgrasper->m_pGrasperHead), pMedial);
		}
		//append into the list
		if (!updateonly)
			objs.AddObjectInfo(o1);	
		else{
			CrestObjectInfo *pcrestobj = objs.GetObjectInfo(o1.nID);
			assert(pcrestobj!=NULL);
			*pcrestobj = o1;
		}
	}

	return instrumentCount;
}

inline void runCollisionDetection(CrestObjects & objs)
{
	const int id =3;
	startFastTimer(id);
		static COMap objmap;
		objmap.LookUpMap(&objs);
	stopFastTimer(id);
	reportTimeDifference(id, "Collision Detection Time");
}

void parseCollisionResults(
	CrestObjects & objs, const Vector3d fortranVert[], const int BUFFERLIMIT, 
	int &nBufferLength, int indexbuffer[], Vector3d dispbuffer[])
{
	//=====decide base vertex index==========================
	int nvertex=1;					//Fortran array index starts at 1
	if (USE_INTERNAL_SIMULATOR)		//My own simulator, starts at 0
		nvertex = 0;
	
	//=====loop through all objects==========================
	nBufferLength = 0;
	const int nObj = objs.m_ObjectInfo.size();
	int c1=0, c2=0;
	for(int i=0; i<nObj; i++){
		CrestObjectInfo* pObjInfo = &objs.m_ObjectInfo[i];
		//Assume the tissue objects are placed in the array continiously
		if(pObjInfo->nObjectType!=TISSUE_TYPE) 
			continue;	
		for(int j=0; j<INSTRUMENT_MAX; j++){
			{//pusher data
				int n = pObjInfo->Response.nPPos[j]; 
				//n = 0;	//DISABLE the pushed nodes
				c1+=n;
				struct tCRNode *psrc = pObjInfo->Response.PPos[j];
				for (int k=0; k<n; k++, nBufferLength++){
					const int fortidx = psrc[k].id + nvertex;
					indexbuffer[nBufferLength] = fortidx;
					const vertex& _tt = psrc[k].vector;					
					dispbuffer[nBufferLength] = Vector3d(_tt.x, _tt.y, _tt.z);
				}
			}
			{//grasper data
				const int n = pObjInfo->Response.nCPos[j]; 
				c2+=n;
				struct tCRNode *psrc = pObjInfo->Response.CPos[j];
				for (int k=0; k<n; k++, nBufferLength++){
					const int iInstrument = psrc[k].id_instrument;
					const int iInstNode = psrc[k].id_iNode*3;
					const double* pData=&(objs.m_ObjectInfo[iInstrument].pVertex[iInstNode]);
					psrc[k].vector.x=*pData;
					psrc[k].vector.y=pData[1];
					psrc[k].vector.z=pData[2];
					const int fortidx = psrc[k].id + nvertex;
					indexbuffer[nBufferLength] = fortidx;
					const vertex& _tt = psrc[k].vector;
					dispbuffer[nBufferLength] = Vector3d(_tt.x, _tt.y, _tt.z);
				}
			}
			if (nBufferLength>BUFFERLIMIT){
				nBufferLength = BUFFERLIMIT;
				printf("ALERT: Buffer overflow in parseCollisionResults!\n");
				return;						
			}
		}
		nvertex+= pObjInfo->nVertex;
	}
	//printf("Pusher node # %d, grasper node # %d.\n", c1, c2);
}


inline void _collisonInstrumentTissue(	//Input parameters:
    const bool hastopologychange,		//Topology change?
    const double &cur_time,				//current time, in mili-sec
	CSceneGraph &tissue,				//The scene graph with tissue objects in
	CSceneGraph &instrument,			//instruments
	const Vector3d*fortDeformedVert,//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	const int BUFFERLIMIT,			//Output buffer size limit
	int &nBufferLength,				//
	int indexbuffer[],				//
	Vector3d dispbuffer[])			//
{
	int nTotalFace;
	if (objs.m_nObjects==0){
		const bool updateonly = false;
		const int ntool = addInstrumentObjects(instrument, objs, updateonly);
		if (ntool<=0) return;
		const int ntissue = addTissueObjects(fortDeformedVert, tissue, objs, hastopologychange, updateonly, nTotalFace);
		if (ntissue<=0) return;
	}
	else{
		const bool updateonly = true;
		addInstrumentObjects(instrument, objs, updateonly);
		addTissueObjects(fortDeformedVert, tissue, objs, hastopologychange, updateonly, nTotalFace);
	}

	//do collsion detection
	if (nTotalFace==0){//If there is no object boundary surface in the scene, no need 
		printf("SCENE EMPTY! HALT!\n");
		exit(0);
		nBufferLength = 0;
	}
	else{
		printf("Total %d triangles for CD!\n", nTotalFace);
		runCollisionDetection(objs);
		parseCollisionResults(objs, fortDeformedVert, BUFFERLIMIT, nBufferLength, indexbuffer, dispbuffer);
	}

	//----report buffer info.
	//printf("Collision buffer size %d.\n", nBufferLength);
	for (int i=0; i<nBufferLength; i++){
		const int vv = indexbuffer[i];
		const double x = dispbuffer[i].x;
		const double y = dispbuffer[i].y;
		const double z = dispbuffer[i].z;
		printf("VID %d, %lg %lg %lg\n", vv, x, y, z);
	}
	//nBufferLength = 0;
	if (USE_INTERNAL_SIMULATOR){ 
		const int nv=nBufferLength;
		const int *pVertexID = indexbuffer;
		const Vector3d *pVertexPos = dispbuffer;
		CSimuEngine *peng = CSimuEngine::getInstance();
		const double inputTimeInterval=200.00;   //33 milisec (30hz) impluse contraint
		peng->updateCollisionConstraint(pVertexPos, pVertexID, nv, cur_time, inputTimeInterval);
	}
}


}//end namespace collision
}//end namespace cia3d



inline void backupTissuePolygonBuffers(CSceneGraph &tissue, int *pPoly[], int nPoly[])
{
	const int nsize = tissue.SceneNodeCount();
	for (int i=0; i<nsize; i++){
		CSceneNode *pnode = tissue.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pSimulationObject;
		pPoly[i] = pobj->m_pPolygon;
		nPoly[i] = pobj->m_nPolygonCount;
	}
}

inline void restoreTissuePolygonBuffers(CSceneGraph &tissue, int *pPoly[], int nPoly[])
{
	const int nsize = tissue.SceneNodeCount();
	for (int i=0; i<nsize; i++){
		CSceneNode *pnode = tissue.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pSimulationObject;
		pobj->m_pPolygon = pPoly[i];
		pobj->m_nPolygonCount = nPoly[i];
	}
}

inline void changeTissueSurface(
	CSceneGraph &tissue, 
	const vector<Vector4i> *pDeformedSurfacesBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	const vector<int> *pDeformedSurfacesSizeBuf)	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
{
	if (pDeformedSurfacesBuf == NULL || pDeformedSurfacesSizeBuf==NULL) return;
	if (pDeformedSurfacesBuf->size()==0 || pDeformedSurfacesSizeBuf->size()==0) return;
	const int nsize = tissue.SceneNodeCount();
	int nTotalPly = 0;
	for (int i=0; i<nsize; i++){
		CSceneNode *pnode = tissue.GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pSimulationObject;
		pobj->m_pPolygon = (int*)&((*pDeformedSurfacesBuf)[nTotalPly]);
		const int nply = (*pDeformedSurfacesSizeBuf)[i];
		pobj->m_nPolygonCount = nply;
		nTotalPly+=nply;
	}
}

//===================================================================
//The following extern vars are defined in file: fortran_interface.cpp
//===================================================================
void collisonInstrumentTissue(				//Input parameters:
    const double &curtime,					//current time, in mili-sec
	CSceneGraph &tissue,					//The scene graph with tissue objects in
	CSceneGraph &instrument,				//instruments
	const vector<Vector3d> *pDeformedVerticesBuf,	//deformed vertices, in FORT array (starting at 1, 0 is for omit the adding 1)
	const vector<Vector4i> *pDeformedSurfacesBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	const vector<int> *pDeformedSurfacesSizeBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	CCollisionResponseBuffer &colbuf				//returned data
	)
{
	//if the buffer is empty, return;
	if (pDeformedVerticesBuf==NULL) return;
	const vector<Vector3d> & deformedBuf = *pDeformedVerticesBuf;
	if (deformedBuf.size()<2) return;

	extern int topologyChange;
	const bool hastopologychange = (topologyChange>0);
	if (topologyChange){
		topologyChange = 0;
		clearResponseData();
	}

	//get the vertex buffer and run
	const int BUFFERSIZE = 100;
	int *pPoly[BUFFERSIZE], nPoly[BUFFERSIZE];
	backupTissuePolygonBuffers(tissue, pPoly, nPoly);
		changeTissueSurface(tissue, pDeformedSurfacesBuf, pDeformedSurfacesSizeBuf);
		const Vector3d *fortDeformedVert = &deformedBuf[0];
		_collisonInstrumentTissue(
			hastopologychange,
			curtime,		//current time, in mili-sec
			tissue,			//The scene graph with tissue objects in
			instrument,		//instruments
			fortDeformedVert,//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
			colbuf.DISPACEMENT_BUFFER_LENGTH,		
			colbuf.nBufferLength, 
			colbuf.indexbuffer, 
			colbuf.dispbuffer);
	restoreTissuePolygonBuffers(tissue, pPoly, nPoly);
}