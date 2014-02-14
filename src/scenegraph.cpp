// scenegraph.cpp

#include <qcursor.h>
#include <QGLViewer/qglviewer.h>
#include "scenegraph.h"
#include "scenegraph-deformpatch.h"
#include "sphere16.h"


using namespace qglviewer;


void BestMatchDrawParms(CSceneGraph & sg, CSceneNode *poldnodes, const int nlen, const bool bRecomputeTexture1DRange=true)
{
	CSceneNode *p1, *p2;
	const int nsize = sg.size();
	if (poldnodes==NULL || nlen<=0) return;

	for (int i=0; i<nsize; i++){
		p1 = sg.GetSceneNode(i);
		const char *name1 = p1->GetObjectName();
		
		//first, check name matches;
		for (int j=0; j<nlen; j++){
			p2 = &poldnodes[j];
			const char *name2 = poldnodes[j].GetObjectName();
			if (strcmp(name1, name2)==0){
				goto MATCHFOUND;
			}
		}

		//second, we may use the same object, same poly type, use the first 3 chars
		for (int j=0; j<nlen; j++){
			p2 = &poldnodes[j];
			const char *name2 = poldnodes[j].GetObjectName();
			if ((name1[0]==name2[0]) && (name1[1]==name2[1]) && 
				(name1[2]==name2[2]) && (name1[3]==name2[3]))
				goto MATCHFOUND;
		}

		//second, we may use the same object, use the first 3 chars
		for (int j=0; j<nlen; j++){
			p2 = &poldnodes[j];
			const char *name2 = poldnodes[j].GetObjectName();
			if (name1[0]==name2[0] && name1[1]==name2[1] && name1[2]==name2[2]){
				goto MATCHFOUND;
			}
		}
		
		//finally, we just set to use the fisrt node of the old buffer;
		p2 = &poldnodes[0];

MATCHFOUND:
		
		//we have to save the old transformation matrix
		//double mat[16];
		//for (i=0; i<16; i++) mat[i] = p1->m_DrawParms.m_Identity[i];
		p1->m_DrawParms = p2->m_DrawParms;
		//for (i=0; i<16; i++) p1->m_DrawParms.m_Identity[i] = mat[i];

		//texture
		p1->m_DrawParms.m_bRecomputeTexture1DParameter = true;
		p1->m_DrawParms.m_bRecomputeTexture1DRange = bRecomputeTexture1DRange;
	}
}


CSceneNode* AddOneObjectToSceneGraph(CPolyObj* q1, CSceneGraph &m_SceneGraph, const Vector3f *rgb)
{
	//set node name
	CPolyObj *q = (CPolyObj*)q1;
	CSceneNode *px = m_SceneGraph.AddSceneNode(CSceneNode(q));
	const char *objname = q->GetObjectName();
	if (strlen(objname)==0){
		const int nid = m_SceneGraph.SceneNodeIndex(px);
		q->SetCustomizedObjectName(nid);		
		objname = q->GetObjectName();
	}
	px->SetObjectName(objname);

	//change default color if required
	if (rgb){
		px->m_DrawParms.m_cObjectColor = Vector3f(rgb->x, rgb->y, rgb->z);
	}
	else{
		px->m_DrawParms.m_cObjectColor = CGLDrawParms::allocateNewColor();
	}

	//set the local frame;
	px->m_pFrame = new qglviewer::ManipulatedFrame;
	assert(px->m_pFrame!=NULL);

	//set the bounding box
	AxisAlignedBox abox;
	px->m_pObject->GetBoundingBox(abox);
	const Vector3d c=(abox.minp+abox.maxp)*0.50;
	px->m_pFrame->setPosition(Vec(c.x, c.y, c.z));
	px->m_pObject->applyTranslation(-c.x, -c.y, -c.z);
	abox.minp -= c;
	abox.maxp -= c;
	px->m_pObject->SetBoundingBox(abox);

	//return the scene node pointer
	return px;
}


void AddObjectsToSceneGraph(CCia3dObj *pObjs[], const int OBUFFLENGTH, CSceneGraph &m_SceneGraph)
{
	for (int i=0; i<OBUFFLENGTH; i++){
		CCia3dObj *p = pObjs[i];
		if (p==NULL) continue;
		for (int j=0; j<CIA3D_SUBOBJ_LIMIT; j++){
			CObject3D * q1 = p->m_pObjList[j];
			if (q1==NULL) break;
			AddOneObjectToSceneGraph((CPolyObj*)q1, m_SceneGraph);
		}
	}
}


void CSceneGraph::_invalidNodePointers(const CSceneNode * p)
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


int CSceneGraph::totalVertexCount(void)
{
	int nPoint=0;
	const int n=size();
	for (int i=0; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		CPolyObj *ply = p->m_pObject;
		nPoint += ply->m_nVertexCount;
	}
	return nPoint;
}


//centerstyle: 0: in the center
//			   1: in the lower center
static CSceneNode* _addNodeToSceneGraphWithManipulation(
	CSceneGraph &m_SceneGraph, const CSceneNode* node, qglviewer::ManipulatedFrame *pFrame, const int centerstyle=0)
{
	CPolyObj *pobj= node->m_pCollisionObject;
	CSceneNode *px = m_SceneGraph.AddSceneNode(*node);
	px->m_pObject = pobj;
	px->SetObjectName(pobj->GetObjectName());

	//set the local frame;
	ASSERT0(pFrame!=NULL);
	px->m_pFrame = pFrame;

	//set the bounding box
	AxisAlignedBox abox;
	px->m_pObject->GetBoundingBox(abox);
	
	Vector3d c;
	switch(centerstyle){
	default:
	case 0: 
		c=(abox.minp+abox.maxp)*0.50; 
		break;
	case 1: 
		c=(abox.minp+abox.maxp)*0.50; 
		c.z = abox.minp.z;
		break;
	}
	px->m_pFrame->setPosition(qglviewer::Vec(c.x, c.y, c.z));
	px->m_pObject->applyTranslation(-c.x, -c.y, -c.z);
	abox.minp -= c;
	abox.maxp -= c;
	px->m_pObject->SetBoundingBox(abox);

	//return the scene node pointer
	return px;
}

CSceneNode* CSceneGraph::AddSceneNode(
	const CSceneNode &node, qglviewer::ManipulatedFrame *pFrame, const int centerstyle)
{	
	if (pFrame == NULL){
		m_SceneList.push_back(node);
		const int nsize = m_SceneList.size()-1;
		return &m_SceneList[nsize];
	}
	//pFrame != NULL;
	return _addNodeToSceneGraphWithManipulation(*this, &node, pFrame, centerstyle);
}


int CSceneGraph::totalElementCount(void)
{
	int nElm=0;
	for (int i=0; i<size(); i++){
		CSceneNode *p = GetSceneNode(i);
		CPolyObj *ply = p->m_pObject;
        if (ply->isFiniteElement() || ply->isRodElement())
            nElm += ply->m_nElementCount;
        else if (ply->isShellElement())
            nElm += ply->m_nPolygonCount;
	}
	return nElm;
}


int CSceneGraph::elementTypeCount(void)
{
	int nElemType=0;
	int hasTet=0, hasHex=0;
	const int n=size();
	for (int i=0; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		const char *pstr = p->m_pObject->Description();
		if (strcmp(pstr, "tetra")==0)
			hasTet = 1;
		else if (strcmp(pstr, "hexobj")==0)
			hasHex = 1;
	}

	nElemType = hasTet+hasHex;
	return nElemType;
}


static void _checkScene4Deformation(CSceneGraph &s, vector<Vector3d>& deformedVertices, Vector3d *pointers[], int &nMaxVBufferLength)
{
	int i;
	const int VIDBASE = 1;			//the fortran array starts from 1
	const int nobj = s.size();
	for (i=0; i<nobj; i++) pointers[i]=NULL;
	nMaxVBufferLength = 0;
	if (deformedVertices.size()<2){ //If there is no deformation input, just return
		//do something here
	}
	else{
		Vector3d *baseptr = &deformedVertices[VIDBASE]; 
		for (i=0; i<nobj; i++){
			const CSceneNode *pnode = s.GetSceneNode(i);
			CPolyObj *pSimulationObj = pnode->m_pSimulationObject;
			CPolyObj *pVisualObj = pnode->m_pObject;
			assert(pSimulationObj!=NULL);
			pointers[i] = baseptr;
			baseptr += pSimulationObj->m_nVertexCount;
			if (pVisualObj){
				if (nMaxVBufferLength<pVisualObj->m_nVertexCount)
					nMaxVBufferLength = pVisualObj->m_nVertexCount;
			}
		}
	}
	nMaxVBufferLength += 4;
}


void CSceneGraph::glDraw(
	CGLDrawParms &global_drawparm, 
	vector<Vector3d>* pDeformedVertices, 
	const vector<Vector4i>* pDeformedQuads,
	const vector<int> *pQuadCount,
	const bool outputMesh)
{		
	const int BUFFLENGTH=100;
	static vector<Vector3d> deformVertexBuffer;
	Vector3f campos = global_drawparm.m_ViewPos;
	Vector3f camdir = global_drawparm.m_ViewDir;
	Vector3d *pointers[BUFFLENGTH], *vertexbuff=NULL;
	int nMaxVBufferLength;
	const int nsize = m_SceneList.size();

	//process deformed vertex info.
	if (pDeformedVertices){
		_checkScene4Deformation(*this, *pDeformedVertices, pointers, nMaxVBufferLength);
		if (deformVertexBuffer.size()< (unsigned int)nMaxVBufferLength)
			deformVertexBuffer.resize(nMaxVBufferLength);
	    vertexbuff = &deformVertexBuffer[0];
	}
	else{
		for (int i=0; i<this->size(); i++) pointers[i]=NULL;
	}

	//process deformed poly. info.
	int nply[BUFFLENGTH];
	const Vector4i *plybase[BUFFLENGTH];
	const int nobj = size();
	if (pDeformedQuads && pQuadCount && (*pDeformedQuads).size()>0){
		int plyindex = 0;
		for (int i=0; i<nobj; i++){
			plybase[i] = &((*pDeformedQuads)[plyindex]);
			nply[i] = (*pQuadCount)[i];
			plyindex += nply[i];
		}
	}
	else{
		for (int i=0; i<nobj; i++){
			plybase[i] = NULL; nply[i] = 0;
		}
	}

	//draw all the objects;
	for (int i=0; i<nsize; i++){
		glPushMatrix();
		CSceneNode &node = this->m_SceneList[i];
		node.m_DrawParms.m_pFboMRT = global_drawparm.m_pFboMRT;
		for (int j=0; j<4; j++)
			node.m_DrawParms.m_Viewport[j] = global_drawparm.m_Viewport[j];
		node.m_DrawParms.m_fFocalLength = global_drawparm.m_fFocalLength;
		node.glDraw(campos, camdir, 
					pointers[i], vertexbuff, 
					(const int *)plybase[i], nply[i],
					outputMesh);
		glPopMatrix();
	}

	//render for deformation controls in the volume deformation paper
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
		//deformation
		renderAdditionalParts(*this);
		//not in deformation, show control springs
		if (pDeformedVertices==NULL && m_pConstraintSpringObj!= NULL)
			m_pConstraintSpringObj->glDraw(global_drawparm);
	glDisable(GL_COLOR_MATERIAL);
}


void CSceneGraph::glDrawPickedObject(void)
{
	_glDrawPickedObjectNormal();
	_glDrawPickedObjectAlignment();
	_glDrawPickedObjectBoundary();
}


static void _glDrawVerticesWithBoundaryCond(CPolyObj *pobj)
{
	if (pobj==NULL) return;
	unsigned char *pBits = pobj->m_pFemBoundCond;
	if (pBits==NULL) return;

	//setup different colors
	Vector3d *v = pobj->m_pVertex;
    Vector3f clrsel(0.5f, 0.5f, 0.5f);
	Vector3f colors[8]={
        Vector3f(0,0,0), //Black for blank vertices
		Vector3f(1,0,0), //x, 0, 0
		Vector3f(0,1,0), //0, y, 0
		Vector3f(1,1,0), 
		Vector3f(0,0,1), 
		Vector3f(1,0,1),
		Vector3f(0,1,1),
		Vector3f(1,1,1)
    };

	//draw the vertices as spheres
	const double r = pobj->estimatedVertexRadius();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	unsigned char b0 = -1;

	for (int i=0; i<pobj->m_nVertexCount; i++){
		unsigned char b1 = pBits[i];
		if ((b1)){
			if (b0!=b1){
				if (b1==0x80)
					glColor3fv((float*)&clrsel.x);
				else{
					int index = b1&0x7;
					glColor3fv(&colors[index].x);
				}
			}
            Vector3f vf(v[i].x, v[i].y, v[i].z);
			CSphere16::getInstance().glDraw(vf, r, false);
		}
		b1 = b0;
	}
	glDisable(GL_COLOR_MATERIAL);
}


static void 
_glDrawNodeSurface(CSceneNode *pnode, Vector3d vert[4], const int nvert, const Vector3f &linecolor)
{
	if (pnode==NULL) return;

	glPushMatrix();
		if (pnode->m_pFrame) glMultMatrixd(pnode->m_pFrame->matrix());
		glLineWidth(2);
		glDisable(GL_LIGHTING);
		glColor3f(linecolor.x, linecolor.y, linecolor.z);
		glLineWidth(4);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0, 1.0);
			if (nvert==4){
				glBegin(GL_QUADS);
					glVertex3dv((&vert[0].x));
					glVertex3dv((&vert[1].x));
					glVertex3dv((&vert[2].x));
					glVertex3dv((&vert[3].x));
				glEnd();
			}
			else{
				glBegin(GL_TRIANGLES);
					glVertex3dv((&vert[0].x));
					glVertex3dv((&vert[1].x));
					glVertex3dv((&vert[2].x));
				glEnd();
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopMatrix();
}


void CSceneGraph::_glDrawPickedObjectBoundary(void)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	CSceneNode *pnode = m_pActiveSceneNode4Face;
	if (pnode){
		//draw selected node or face;
		int nvert=0;
		Vector3d vert[4];
		GetActiveFaceLocal(pnode, vert, nvert);
		_glDrawNodeSurface(m_pActiveSceneNode4Face, vert, nvert, Vector3f(0.51f,0.51f,0));
	}

	//draw the boundary vertices;
	const int nsize= size();
	for (int i=0; i<nsize; i++){
		CSceneNode *pnode = GetSceneNode(i);
		CPolyObj *pobj = pnode->m_pObject;
		if (pobj){
			glPushMatrix();
			if (pnode->m_pFrame)
				glMultMatrixd(pnode->m_pFrame->matrix());
			_glDrawVerticesWithBoundaryCond(pobj);
			glPopMatrix();
		}
	}
}

static void _glDrawNodeBoundingBox(CSceneNode *pnode, const Vector3f &linecolor)
{
	if (pnode==NULL) return;
	glPushMatrix();
	if (pnode->m_pFrame) glMultMatrixd(pnode->m_pFrame->matrix());
	CGLDrawParms *pdraw = &pnode->m_DrawParms;
	pdraw->BeginDrawing();
		CObject3D *p = pnode->m_pObject;
		if (p == NULL) 
			p= pnode->m_pSimulationObject;
		AxisAlignedBox box;
		p->GetBoundingBox(box);
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glLineWidth(2);
		glColor3f(linecolor.x, linecolor.y, linecolor.z);
		glDisable(GL_LIGHTING);
        const Vector3d& p0=box.minp; 
        const Vector3d& p1=box.maxp; 
		DrawBoundingBox(p0, p1);
		const Vector3d dist = (p1 - p0)*0.33;
		glLineWidth(4);
		glColor3f(1.0,0,0);
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p0.x+dist.x, p0.y, p0.z);
		glEnd();
		glColor3f(0,1,0);
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p0.x, dist.y+p0.y, p0.z);
		glEnd();
		glColor3f(0,0,1);
		glBegin(GL_LINES);
			glVertex3f(p0.x, p0.y, p0.z);
			glVertex3f(p0.x, p0.y, p0.z+dist.z);
		glEnd();
	pdraw->PostDrawing();
	glPopMatrix();
}

void CSceneGraph::_glDrawPickedObjectAlignment(void)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	//draw the two selected objects
	_glDrawNodeBoundingBox(m_pActiveSceneNodeMaster, Vector3f(0.5f,0.5f,0));
	_glDrawNodeBoundingBox(m_pActiveSceneNodeSlave, Vector3f(0,0.5f,0.5f));
}


void CSceneGraph::_glDrawPickedObjectNormal(void)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	CSceneNode *pnode = NULL;
	Vector3d vert[4];

	//glDisable(GL_BLENDING);
	glColor3f(1,0,0);
	//draw selected node or face;
	switch (m_nPickingType){
	case CSceneGraph::PICK_OBJECT:	//draw obj
		_glDrawNodeBoundingBox(m_pActiveSceneNode, Vector3f(0.9f,0,0));
		break;
	case CSceneGraph::PICK_FACE:	//draw face
		if (m_pActiveSceneNode4Face){
			int nvert=0;
			GetActiveFaceLocal(pnode, vert, nvert);
			_glDrawNodeSurface(m_pActiveSceneNode4Face, vert, nvert, Vector3f(1,0,0));
		}
		break;
	case CSceneGraph::PICK_LINE:	//draw active line
		if (m_pActiveSceneNode4Edge){
			glPushMatrix();
			GetActiveLineLocal(pnode, vert[0], vert[1]);
			assert(pnode!=NULL);
			if (pnode->m_pFrame) 
                glMultMatrixd(pnode->m_pFrame->matrix());
		    glLineWidth(3);
			glBegin(GL_LINES);
				glVertex3dv(&vert[0].x);
				glVertex3dv(&vert[1].x);
			glEnd();
			glPopMatrix();
		}
	default:
		break;
	};
}



static void DUMP_POLY_INFO(CPolyObj *ply, const int id)
{
	const Vector3d* vert = ply->m_pVertex;

	//the input polyobj is a quad object
	{
	CQuadObj *pquad = dynamic_cast<CQuadObj*>(ply);
	if (pquad!=NULL){
		Vector4i *m_pQuad = (Vector4i *)pquad->m_pPolygon;
		int* q = &m_pQuad[id].x;
		printf("Quad info is dumped:\n");
		for (int i=0; i<4; i++){
			const Vector3d *v = &vert[q[i]];
			printf("Node %d (%lf %lf %lf)\n", q[i], v->x, v->y, v->z); 
		}
		const Vector3f norm = pquad->m_pPolyNorm[id];
		printf("Normal (%f %f %f)\n", norm.x, norm.y, norm.z); 
		//dump texture coord
		if (ply->m_pVertexTexCoor1D!=NULL){
			float *ptex = ply->m_pVertexTexCoor1D;
			printf("Tex coord 1D (%f %f %f)\n", ptex[q[0]], ptex[q[1]], ptex[q[2]], ptex[q[3]]);
		}
		printf("\n");
		return;
	}
	}

	//the input polyobj is a triangle object
	{
	CTriangleObj *ptri = dynamic_cast<CTriangleObj*>(ply);
	if (ptri!=NULL){
		Vector3i *m_pQuad = (Vector3i *)ptri->m_pPolygon;
		int* q = &m_pQuad[id].x;
		printf("Triangle info is dumped:\n");
		for (int i=0; i<3; i++){
			const Vector3d *v = &vert[q[i]];
			printf("Node %d (%lf %lf %lf)\n", q[i], v->x, v->y, v->z); 
		}
		const Vector3f norm = ptri->m_pPolyNorm[id];
		printf("Normal (%f %f %f)\n", norm.x, norm.y, norm.z); 
		//dump texture coord
		if (ply->m_pVertexTexCoor1D!=NULL){
			float *ptex = ply->m_pVertexTexCoor1D;
			printf("Tex coord 1D (%f %f %f)\n", ptex[q[0]], ptex[q[1]], ptex[q[2]]);
		}
		printf("\n");
		return;
	}
	}
}


static void DUMP_LINE_INFO(CPolyObj* pobj, const int nx, const int ny)
{
	printf("Line nodes are: (%d %d)\n", nx, ny);
	const Vector3d* vert = pobj->m_pVertex;
	int q[2]={nx, ny};

	if (pobj!=NULL){
		for (int i=0; i<2; i++){
			const Vector3d *v = &vert[q[i]];
			printf("Node %d (%lg %lg %lg)\n", q[i], v->x, v->y, v->z); 
		}
	}
}


static void 
_getObjectAndFaceID(const CSceneGraph::PICKING_TYPE_ID picktype, 
                    const int pickedid, CSceneGraph &scene, int& objid, int& faceid)
{	
	int c1=0, c2=0;
	objid=-1, faceid=-1;
	for (int i=0; i<scene.size(); i++){
		CSceneNode *p = scene.GetSceneNode(i);
		CObject3D *pobj = p->m_pObject;
		if (!pobj) continue;
		c2 = c1+ pobj->GetPickableElementCount(picktype);
		if (pickedid>=c1 && pickedid<c2){		// find obj;
			objid = i;
			faceid = pickedid - c1;
			if (picktype==CSceneGraph::PICK_FACE)
				DUMP_POLY_INFO((CPolyObj*)pobj, faceid);
			break;
		}
		c1=c2;
	}
}

static void 
_getObjectAndLineID(const int picktype, const int pickedid, CSceneGraph &scene, int& objid, int& vid1, int &vid2)
{
	int i, c1, c2;
	for (i=0, c1=0; i<scene.size(); i++){
		CSceneNode *p = scene.GetSceneNode(i);
		CObject3D *pobj = p->m_pObject;
		if (!pobj) continue;
		c2 = c1+ pobj->GetPickableElementCount(picktype);
		if (pickedid>=c1 && objid<c2){		//obj found;
			pobj->GetPickedLine(pickedid-c1, vid1, vid2);
			objid = i;
			if (picktype==CSceneGraph::PICK_LINE)
				DUMP_LINE_INFO((CPolyObj*)pobj, vid1, vid2);
			break;
		}
		c1=c2;
	}
}

void CSceneGraph::postSelection(const int objid, const int x, const int y)
{
	CSceneNode * p = NULL;
	int vx, vy, nodeid, faceid;
	vx=vy=nodeid=faceid=-1;

	//get picking state
	enum PICKING_TYPE_ID itype;
	this->pickingType(itype);

	switch (itype){
		case CSceneGraph::PICK_OBJECT: //pick poly obj;
			p = GetSceneNode(objid);
			setActiveSceneNode(p);
			break;
        case CSceneGraph::PICK_FACE:   //pick face
			_getObjectAndFaceID(itype, objid, *this, nodeid, faceid);
			m_pActiveSceneNode4Face = GetSceneNode(nodeid);
			m_nActiveFaceID = faceid;
			break;
		case CSceneGraph::PICK_LINE:   //pick edge
			_getObjectAndLineID(itype, objid, *this, nodeid, vx, vy);
			p = GetSceneNode(nodeid);
			SetActiveLine(p, vx, vy);
			break;
		case CSceneGraph::PICK_NODE:   //pick a vertex
            break;
		default:
			break;
	};
}


void CSceneGraph::glDraw4Picking(void)
{
	const int pickingstype = (int)m_nPickingType;
	int i, k;
	int nsize = m_SceneList.size();

	//Apr. 25, 2007
	//in volman, the startobjid is modified as 1
	const int startobjid = 0;
	for (i=k=startobjid; i<nsize; i++){
		CSceneNode &node = m_SceneList[i];
		glPushMatrix();
		if (node.m_pFrame) glMultMatrixd(node.m_pFrame->matrix());

		CObject3D *pobj = node.m_pObject;
		CGLDrawParms *pdraw = &node.m_DrawParms;
		assert(pobj!=NULL);
		pdraw->BeginDrawing();
		switch(pickingstype){
		case 0:
			pobj->DrawPickingObject(k);
			break;
		case 1:
			pobj->DrawPickingObjectFace(k);
			break;
		case 2:
			pobj->DrawPickingObjectLine(k);
			break;
		};
		k+= pobj->GetPickableElementCount(pickingstype);
		pdraw->PostDrawing();
		glPopMatrix();
	}
}


void CSceneGraph::setActiveSceneNode(CSceneNode* p)
{
	m_pActiveSceneNode = p;
	emit activeSceneNodeChanged(p);
}


void CSceneGraph::GetBoundingBox(AxisAlignedBox & box)
{
	box.minp = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	box.maxp = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	int n = size();
	for (int i=0; i<n; i++){
		const CSceneNode *p = GetSceneNode(i);
		AxisAlignedBox obox = p->worldBoundingBox();
		box = Union(box, obox);
	}
}


static int _getNPointsWithBoundaryConstraint(CSceneGraph &m)
{
	int i, j, c=0;
	const int nobj = m.size();
	for (i=0; i<nobj; i++){
		 CSceneNode *pnode = m.GetSceneNode(i);
		 CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
		 unsigned char* pBound = pobj->m_pFemBoundCond;
		 if (pBound==NULL) continue;
		 for (j=0; j<pobj->m_nVertexCount; j++){
			unsigned char b = pBound[j] & 0x7f;
			if (b) c++;
		 }
	}
	return c;
}


//Type and Set are two terms used in CIA3D
//Type is more general classification, such as FEM, beam, truss, etc
//Set is below Type, it means different objects or several objects of the same type, with the 
//same material property
//Now the types are 
//	1: FEM
//	7: TRUSS
//Output: typeObjCount: returns the number of objects in each type
//        objSetID: returns the sequnce # for each object in each type
static int _elementTypeCount(CSceneGraph * s, const int NTYPE, int typeObjCount[], int objSetID[])
{
    int i, *b = typeObjCount;
	for (i=0; i<=NTYPE; i++) b[i]=0;
    for (i=0; i<s->size(); i++){
		int setid;
        CSceneNode *pnode = s->GetSceneNode(i);
        CPolyObj *p=pnode->m_pObject;
        if (p->isFiniteElement()){
			b[1]++;
			setid = b[1];
		}
        else if (p->isFiniteMass()){
			b[2]++;
			setid = b[2];
		}
        else if (p->isShellElement()){
			b[3]++;
			setid = b[3];
		}
        else if (p->isParticleElement()){
			b[4]++;
			setid = b[4];
		}
		else if (p->isRodElement()){
			b[7]++;
			setid = b[7];
		}
		else{
			printf("Unknow element type: object %d.\n", i);
			continue;
		}
		objSetID[i] = setid;
    }

    int c=0;
	for (i=1; i<=NTYPE; i++){
		if (b[i]>0) c++;
	}
    return c;
}


void _fprintfFiniteElementClass(FILE *fp, CSceneGraph * ps)
{
	int i;
	const int nsize = ps->size();
    const int TYPEID = 1;	//TYPEID=1 for FEM
	const int COMP_TYPE=1;  //1: FEM elemnt
							//3: TRUSS element
	//if no FEM entities, directly return
	bool hasfem =false;
	for (i=0; i<nsize; i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (pobj->isFiniteElement()){
			hasfem = true;
			break;
		}
	}
	if (!hasfem) return;

	//print the leading SIGN
	fprintf(fp, "%d\n", TYPEID);

	//print all the entities
	int c = 1;
	for (i=0; i<nsize; i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (!pobj->isFiniteElement()) continue;
		const int objid = i+1;
        if (pobj->m_nElmVertexCount==4){
			//sequenceID, nObjID, nVertexperElm, nGaussPoint, nMaterialID, computationType
			/*
				READ (15,*)
					KSETS,
     .				LPART(J_TS(KTYPE,KSETS)),
		.           LNODT(J_TS(KTYPE,KSETS)),
     .              LGAUS(J_TS(KTYPE,KSETS)),
     .              LMATS(J_TS3(KTYPE,KSETS,1)),
     .              LGEOM(J_TS(KTYPE,KSETS))
			*/
			fprintf(fp, "%d %d 4 1 %d %d\n", c, objid, objid, COMP_TYPE);
        }
		else if (pobj->m_nElmVertexCount==8){
			fprintf(fp, "%d %d 8 2 %d %d\n", c, objid, objid, COMP_TYPE);			
		}
		else{
			printf("Error: vertex per element!\n");
			assert(0);
		}
		c++;
	}
}


//ID=7
void _fprintfRodElementClass(FILE *fp, CSceneGraph * ps)
{
    int i, c=1;
    const int ELEMENT_ID = 7;
	const int rodpropertyID = 1;
	const int nsize = ps->size();

	//if no FEM entities, directly return
	bool hasrod =false;
	for (i=0; i<nsize; i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (pobj->isRodElement()){
			hasrod = true;
			break;
		}
	}
	if (!hasrod) return;

	//print the leading SIGN
	fprintf(fp, "%d\n", ELEMENT_ID);

	//print all elements
	for (i=0; i<nsize; i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (!pobj->isRodElement()) continue;
		const int objid = i+1;
        if (pobj->m_nElmVertexCount==2){
			//sequenceID, nObjID, nVertexperElm, nGaussPoint, nMaterialID, computationType(rodpropertyID)
		    fprintf(fp, "%d %d 2 1 %d %d\n", c++, objid, objid, rodpropertyID);
        }
		else{
			assert(0);
		}
	}
}



void _fprintfFiniteMassClass(FILE *fp, CSceneGraph * ps)
{
	printf("Not implemented yet!\n");
	return;
}

void _fprintfShellElementClass(FILE *fp, CSceneGraph * ps)
{
	printf("Not implemented yet!\n");
	return;

    int i, j, c=1;
    const int FE_ID = 3;
	fprintf(fp, "%d\n", FE_ID);
	for (i=0; i<ps->size(); i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (!pobj->isShellElement()) continue;
		const int objid = i+1;
        const int layer = pnode->m_FemAttrib.m_nLayer;
        if (pobj->m_nPlyVertexCount==3){
		    //id, objid, vertexcount, gausspoint, materialid, layercount
		    fprintf(fp, "%d %d 3 1 %d %d\n", c++, objid, objid, layer);
            if (layer>1){
                for (j=0; j<layer; j++)
                    fprintf(fp, "%d ", 1);
                fprintf(fp, "\n");
            }
        }
	}
	for (i=0; i<ps->size(); i++){
		CSceneNode *pnode = ps->GetSceneNode(i);
		CPolyObj * pobj = (CPolyObj*)pnode->m_pObject;
        if (!pobj->isShellElement()) continue;
		const int objid = i+1;
        const int layer = pnode->m_FemAttrib.m_nLayer;
        if (pobj->m_nPlyVertexCount==4){
		    fprintf(fp, "%d %d 4 2 %d %d\n", c++, objid, objid, layer);
            if (layer>1){
                for (j=0; j<layer; j++)
                    fprintf(fp, "%d ", 1);
                fprintf(fp, "\n");
            }
        }
	}
}


void _fprintfParticleElementClass(FILE *fp, CSceneGraph * ps)
{
	assert(0);
}


void CSceneGraph::_exportGroup1(FILE *fp, CC3d1inIOControl &cio, int objSetID[])
{
/* sample data from a file
 4
 P3/PATRAN Neutral File from: /scratch1/nanzhang/test2.db
 GROUP 1
 5291 25687 3 50000 1 50 1.00E-09 1
 20 20 0 0
 0 0 3
 1 3
 1
 1 1 4 1 3
 2 2 4 1 2
 3 3 4 1 1
*/
	const int NTYPE = 7; //currently only 7 classes are defined!
    int nTypeObjCount[NTYPE+1];	
	fprintf(fp, "GROUP 1\n");
	//write line 1
	const int nPoint = totalVertexCount();
	const int nElem = totalElementCount();
	const int nMat = this->size();
	const int nStep=cio.m_nStep; 
	const int nAlgo=cio.m_nAlgo;
	const int nMiter=cio.m_nMiter;
	const double fTol=cio.m_fTol;
	const int nType = _elementTypeCount(this, NTYPE, nTypeObjCount, objSetID); 
	fprintf(fp, "%d %d %d %d %d %d %.14lg %d\n", 
				nPoint, nElem, nMat, nStep, nAlgo, nMiter, fTol, nType);
	//write line 2
	int nOutd=120, nOutp=120, nReqd=0, nReqs=0;
	fprintf(fp, "%d %d %d %d\n", nOutd, nOutp, nReqd, nReqs);
	//write line 3
	cio.m_nVfix = _getNPointsWithBoundaryConstraint(*this);
	const int nVfix=cio.m_nVfix;
	const int nPrev=cio.m_nPrev;
	const int nLaps=cio.m_nLaps;
	fprintf(fp, "%d %d %d\n", nVfix, nPrev, nLaps);
	//Line 4, doc section 1.2.6, parameters for element type and set;
    for (int i=1; i<=NTYPE; i++){
        if (nTypeObjCount[i]==0) continue;
	    fprintf(fp, "%d %d\n", i, nTypeObjCount[i]);
    }
    if (nTypeObjCount[1]>0)  //FiniteElm class 
        _fprintfFiniteElementClass(fp, this);
    if (nTypeObjCount[2]>0)  //FMass class 
        _fprintfFiniteMassClass(fp, this);
    if (nTypeObjCount[3]>0)  //SHELL class 
        _fprintfShellElementClass(fp, this);
    if (nTypeObjCount[4]>0)  //PARTICLE class 
        _fprintfParticleElementClass(fp, this);
    if (nTypeObjCount[7]>0)//ROD class 
        _fprintfRodElementClass(fp, this);

}


void CSceneGraph::_exportGroup2(FILE *fp, const CC3d1inIOControl &cio)
{
/*
 GROUP 2
 frict  method  iseah  nreqc  penan, penat
 0.00E+00 0 0 0 0.00E+00 0.00E+00
*/
	const double fFrict=cio.m_fFrict;
	const double fPenan=cio.m_fPenan;
	const double fPenat=cio.m_fPenat;
	const int nMethod=cio.m_nMethod;
	const int nIseah=cio.m_nIseah;
	const int nReqc=cio.m_nReqc;
	fprintf(fp, "\n\nGROUP 2\n");
	fprintf(fp, "frict method iseah nreqc penan penat\n");
	fprintf(fp, "%.14lG %d %d %d %.14lG %.14lG\n", fFrict, nMethod, nIseah, nReqc, fPenan, fPenat);
}


void CSceneGraph::_exportGroup3(FILE *fp, const int objSetID[])
{
/*
 GROUP 3
 1 1 3 13 52 6 44
 2 1 3 61 55 57 76
*/
	int i, vcount, ecount;
	fprintf(fp, "\n\nGROUP 3\n");
	const int n = size();

	//Export element connection;	
	ecount=1;	//starting element count = 1, for FORTRAN
	vcount=1;	//starting vertex count = 1, for FORTRAN
	for (i=0; i<n; i++){
		CSceneNode *pnode = GetSceneNode(i);
		pnode->m_FemAttrib.m_nAccVertexCount = vcount;
		CPolyObj *ply = pnode->m_pObject;
		const int objID = i+1;
		const int nOutElm = ply->exportElemConnectivity(fp, objID, objSetID[i], ecount, vcount);
		assert(nOutElm == ply->m_nElementCount);
		ecount += ply->m_nElementCount;
		vcount += ply->m_nVertexCount;
	}

	//Export nodes;
    int c;
	Matrix2d mat; SetIdentityMatrix(mat);
	for (i=0, c=1; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		const double *pmatrix = &mat.x[0][0];
		if (p->m_pFrame) 
			pmatrix = p->m_pFrame->matrix();
        const double th = p->m_FemAttrib.m_lfShellThickness;
        CPolyObj *ply = (CPolyObj*)p->m_pObject;
		c+= ply->exportElemNodes(fp, c, pmatrix, th);
	}

	//fprintf(fp, "0 0 0 0 0\n");
	//Export material, this part is not implemented by CIA3D yet
	for (i=0, c=1; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		//fprintf(fp, "%d\n", i+1);
		//fprintf(fp, "1 1 1 1 1 1\n");
	}
	
	//Export boundary conditions
	for (i=0, c=1; i<n; i++){
		CPolyObj *ply = (CPolyObj*)(GetSceneNode(i)->m_pObject);
		ply->exportBoundaryConditions(fp, c);
		c+=ply->m_nVertexCount;
	}

	//Export for selected stress
	fprintf(fp, "5 5\n");

}


void CSceneGraph::_exportGroup4(FILE *fp)
{
	fprintf(fp, "\n\nGROUP 4\n");
	fprintf(fp, "total numbet of segment\n");
	fprintf(fp, "0\n");
	fprintf(fp, "segment number global nodal number\n");
	fprintf(fp, "total number of hitt nodes\n");
	fprintf(fp, "0\n");
	fprintf(fp, "hitt nodes number\n");
}


void CSceneGraph::_exportGroup5(FILE *fp)
{
	fprintf(fp, "\n\nGROUP 5\n");
	const int nConm=0;				//number of conncentrate mass
	const int nAcce=0;				//number of acceleration record;
	const int ifunc=1;				//indicator of dynamic load in the time domaion
	const double totalTime = 10;	//10 sec. good enough for contact problem
	const double dtime=1e-9;
	const double dtend=100;
	const double dtrec=100;
	double aalfa=0, beeta=0, azero=0, bzero=0, omega=0;

	//Section 1.6.2
	fprintf(fp, "%d %d %d\n", nConm, nAcce, ifunc);
	fprintf(fp, "%lG %lG %lG %lG %lG %lG %lG %lG %lG\n", 
				totalTime, dtime, dtend, dtrec, 
				aalfa, beeta, azero, bzero, omega);				

	//Section 1.6.3, export nodal displacement;	
	int i, c, n=size();
	for (i=0, c=1; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		const Vector3d displ = p->m_FemAttrib.m_vDisplacement;
		CPolyObj *ply = (CPolyObj*)p->m_pObject;
        if (ply->isShellElement())
		    c+=ply->exportElemNodeDisplacementAndAngle(fp, c, displ);
        else
            c+=ply->exportElemNodeDisplacement(fp, c, displ);
	}

	//Export nodal v;	
	for (i=0, c=1; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		const Vector3d velocity = p->m_FemAttrib.m_vVelocity;
		CPolyObj *ply = (CPolyObj*)p->m_pObject;
        if (ply->isShellElement())
            c+= ply->exportElemNodeVelocityAndAngle(fp, c, velocity);
        else
		    c+= ply->exportElemNodeVelocity(fp, c, velocity);
	}
}


void CSceneGraph::_exportGroup6(FILE *fp)
{
	fprintf(fp, "\n\nGROUP 6\n");
	fprintf(fp, "0.250 0.50 3 2\n");
}


void CSceneGraph::_exportGroup7(FILE *fp)
{
	int nVertex=totalVertexCount();

	fprintf(fp, "\n\nGROUP 7\n");
	fprintf(fp, "parameter for loads\n");
	fprintf(fp, "global\n");
	fprintf(fp, "%d 0 0 0 0 0\n", nVertex);

	//Export nodal load;	
	const int n = size();
	for (int i=0, c=1; i<n; i++){
		CSceneNode *p = GetSceneNode(i);
		const Vector3d load = p->m_FemAttrib.m_vLoad;
		CPolyObj *ply = (CPolyObj*)p->m_pObject;
		c+= ply->exportElemNodeLoad(fp, c, load);
	}
	
    //The format requires one extra data entry;
	CSceneNode *p = GetSceneNode(n-1);
	const Vector3d load = p->m_FemAttrib.m_vLoad;
	const CPolyObj *ply = (CPolyObj*)p->m_pObject;
    if (ply->isShellElement())
	    fprintf(fp, "%d 0 0 0 0 0\n", nVertex);
    else
	    fprintf(fp, "%d 0 0 0\n", nVertex);
}


void CSceneGraph::exportFEMDataFile(const char *fname, CC3d1inIOControl &cio)
{
	//pre-alloc a buffer for setIDs, make sure the buffer is big enough
	const int BUFSIZE = 256;
	const int nsize = m_SceneList.size();
	if (nsize<=0) return;
	int setID[BUFSIZE];
	assert(nsize<BUFSIZE);

	//Insert the constraint springs into the scene graph
	CSceneNode snode, *ppos=NULL;
	if (m_pConstraintSpringObj!=NULL){
		if (m_pConstraintSpringObj->m_nElementCount>0){
			snode.m_pObject = m_pConstraintSpringObj;
			ppos = AddSceneNode(snode);
		}
	}

	//======begin export=================
	FILE *fp=fopen(fname, "w");
	if (fp==NULL) return;

    QDate date=QDate::currentDate(); 
    QTime tm=QTime::currentTime(); 
	const int femalg = cio.m_nIStat;
	fprintf(fp, "%d\n", femalg);
	fprintf(fp, "Generated by M3d, Date: %s, Time: %s.\n", 
                date.toString().ascii(), tm.toString().ascii());

	_exportGroup1(fp, cio, setID);
	_exportGroup2(fp, cio);
	_exportGroup3(fp, setID);
	_exportGroup5(fp);
	_exportGroup4(fp);
	_exportGroup6(fp);
	_exportGroup7(fp);

	fclose(fp);
	//======end export=================

	if (ppos){
		const int i = SceneNodeIndex(ppos);
		DeleteSceneNode(i);
	}
}


void CSceneGraph::_exportPovrayMesh(FILE *fp)
{
	int i;
	const int nsize = m_SceneList.size();
	//draw all the objects;
	for (i=0; i<nsize; i++){
		CSceneNode &node = m_SceneList[i];
		CObject3D *pobj = node.m_pObject;
		assert(pobj!=NULL);
		const double *matrix = node.m_pFrame->matrix();

		fprintf(fp, "#declare %s =\n", pobj->GetObjectName());
		pobj->exportFile(fp, ".pov", matrix);
		fprintf(fp, "\n\n\n");
	}
}


void ExportPovrayMaterial(FILE *fp)
{
	const char* incl="\n#include \"usrdef.inc\"\n";
	fprintf(fp,"%s\n\n\n", incl);
}


static void ExportPovrayObjects(CSceneGraph & sg, FILE *fp)
{
	int i;
	const int nsize = sg.size();
	if (nsize==0) return;

	//draw all the objects;
	fprintf(fp, "union{\n");
	for (i=0; i<nsize; i++){
		CSceneNode *node = sg.GetSceneNode(i);
		CObject3D *pobj = node->m_pObject;
		assert(pobj!=NULL);
		fprintf(fp, "object {\n");
		fprintf(fp, "\t%s\n", pobj->GetObjectName());
		fprintf(fp, "\ttexture { CIA3DTex%d }\n", i);
		//fprintf(fp, "\ttexture { White_Chocolate }\n");
		fprintf(fp, "}\n");
	}
	fprintf(fp, "}\n\n\n");
}


static inline Vector3f VEC2POVVEC(const Vector3f & v)
{
	Vector3f r = v;
	r.z = -r.z;
	return r;
}

void ExportPovrayCamera(Camera* pcamera, FILE *fp)
{
	Vector3f pos, upvec, viewdir, lookat;
	float fov, fovpi;
	assert(pcamera!=NULL);
	assert(fp!=NULL);

	//output camera info.
	Vec posvec = pcamera->position();
	pos = Vector3f(posvec.x, posvec.y, posvec.z);
	Vec up = pcamera->upVector(); 
	upvec = Vector3f(up.x, up.y, up.z);
	Vec view = pcamera->viewDirection(); 
	viewdir = Vector3f(view.x, view.y, view.z);
	fovpi = pcamera->horizontalFieldOfView();
	fov = fovpi * 180 / PI;

	upvec.normalize();
	viewdir.normalize();
	Vector3f xAxis = CrossProd(viewdir,upvec);
	if (Magnitude(xAxis) < 1E-10){
      // target is aligned with upVector, this means a rotation around X axis
      // X axis is then unchanged, let's keep it !
		//xAxis = pcamera->frame()->inverseTransformOf(Vec(1.0, 0.0, 0.0));
		assert(0);
    }
	xAxis.normalize();
	
	//the size matter;
	float znear = 1.0f; //pcamera->zNear();
	viewdir = viewdir*znear;
	double dw = 2*znear*tan(fovpi * 0.5);
	dw *= (800.0/pcamera->screenWidth());
	xAxis = xAxis * dw;
	//upvec = upvec * (dw / pcamera->aspectRatio());
	upvec = upvec * dw; 
	lookat = pos + viewdir;

	pos = VEC2POVVEC(pos);
	upvec = VEC2POVVEC(upvec);
	viewdir = VEC2POVVEC(viewdir);
	xAxis = VEC2POVVEC(xAxis);
	lookat = VEC2POVVEC(lookat);

	fprintf(fp, "//Window width=%d, height=%d.\n", pcamera->screenWidth(), pcamera->screenHeight());
	fprintf(fp, "camera {\n");
	fprintf(fp, "\tperspective\n");
	fprintf(fp, "\tlocation <%f, %f, %f>\n", pos.x, pos.y, pos.z);
	fprintf(fp, "\tdirection <%f, %f, %f>\n", viewdir.x, viewdir.y, viewdir.z);
	fprintf(fp, "\tright <%f, %f, %f>\n", xAxis.x, xAxis.y, xAxis.z);
	fprintf(fp, "\tup <%f, %f, %f> *(image_height/image_width)\n", upvec.x, upvec.y, upvec.z);
	//fprintf(fp, "\tlook_at <%f, %f, %f>\n", lookat.x, lookat.y, lookat.z);
	fprintf(fp, "\tangle %f\n", fov);
	fprintf(fp, "}\n\n\n");
	
	//export a view associated camera;
	fprintf(fp, "light_source{\n");
	fprintf(fp, "\t<%f, %f, %f>\n", pos.x, pos.y, pos.z); 
    fprintf(fp, "\trgb <1.0, 1.0, 1.0>*EYELIGHT_INTENSITY\n");  
    fprintf(fp, "\tshadowless");
	fprintf(fp, "}\n\n\n");
}


void CreateIncFileName(const char *fname, char * povincfname, char* povincfullfname)
{
	int i, slen = strlen(fname);
	for (i=slen-1; i>=0; i--)
		if (fname[i]=='/') break;
	if (i<0)
		sprintf(povincfname, "%s", fname);
	else
		sprintf(povincfname, "%s", &fname[i+1]);
	slen = strlen(povincfname);
	assert(slen>=4);
	povincfname[slen-3]='i', 
	povincfname[slen-2]='n', 
	povincfname[slen-1]='c';

	strcpy(povincfullfname, fname);
	slen = strlen(povincfullfname);
	assert(slen>=4);
	povincfullfname[slen-3]='i', 
	povincfullfname[slen-2]='n', 
	povincfullfname[slen-1]='c';	
}


void CSceneGraph::exportPovrayFile(void *pwin, const char *fname)
{
	FILE *fp = fopen(fname, _WA_);
	assert(fp!=NULL);
	assert(pwin!=NULL);
	char povincfname[512], povincfullfname[512];

	//Assign the inc file name;
	CreateIncFileName(fname, povincfname, povincfullfname);
	
	//---------Main FILE---------------
	//
	ExportPovrayMaterial(fp);
	QGLViewer *pviewer = (QGLViewer*)pwin;
	qglviewer::Camera *pcamera = pviewer->camera();
	ExportPovrayCamera(pcamera, fp);

	fprintf(fp, "#include \"%s\"\n", povincfname);
	ExportPovrayObjects(*this, fp);
	fclose(fp);

	//---------MESH FILE---------------
	fp=fopen(povincfullfname, _WA_);
	assert(fp!=NULL);
	_exportPovrayMesh(fp);
	fclose(fp);
}



void CSceneGraph::exportSelectedObjNeutralMeshFile(FILE *fp)
{
//	if (m_pActiveSceneNode==NULL)
//		return;
//	m_pActiveSceneNode->m_pObject->exportFile(fp, ".mesh");
}


void CSceneGraph::setPickingType(PICKING_TYPE_ID itype)
{
	m_nPickingType = itype;		//0: polyobj, 1: face, 2: line
	QCursor cursor;
	switch(m_nPickingType){
	case CSceneGraph::PICK_OBJECT:
		cursor.setShape(Qt::PointingHandCursor);
		break;
	case CSceneGraph::PICK_FACE:
		cursor.setShape(Qt::SizeAllCursor);
		break;
	case CSceneGraph::PICK_LINE:
		cursor.setShape(Qt::CrossCursor);
		break;
	default:
		cursor.setShape(Qt::ArrowCursor);
		break;
	}
	emit pickingCursorChanged(&cursor);
}


void CSceneGraph::updateGLWindows(void)
{
	emit updateGLWindowRendering();
}


void CSceneGraph::setGLWindowsBoundingBox(const Vector3d &lowleft, const Vector3d& upright)
{
	emit updateGLWindowsBoundingBox(lowleft, upright);
}


void CSceneGraph::hookObject(QObject *pobj)
{
	connect(this, SIGNAL(activeSceneNodeChanged(CSceneNode*)), pobj, SLOT(activeSceneNodeChanged(CSceneNode*)));
	connect(this, SIGNAL(pickingCursorChanged(QCursor*)), pobj, SLOT(pickingCursorChanged(QCursor*)));
}


void CSceneGraph::hookGLWindow(QGLViewer *pwin)
{
	connect(this, SIGNAL(updateGLWindowRendering()), pwin, SLOT(updateGL()));
	connect(this, SIGNAL(updateGLWindowsBoundingBox(const Vector3d&,const Vector3d&)), 
		    pwin, SLOT(updateGLWindowsBoundingBox(const Vector3d&,const Vector3d&)));

}


void CSceneGraph::nextActiveObject(void)
{
	const int nsize = size();
	if (nsize==0) return;

	CSceneNode *pActiveNode = GetActiveSceneNode();
	if (pActiveNode==NULL)
		pActiveNode = GetSceneNode(nsize-1);
	for (int i=0; i<nsize; i++){
		CSceneNode * p = GetSceneNode(i);
		if (pActiveNode == p){
			CSceneNode * p2 = GetSceneNode((i+1)%nsize);
			setActiveSceneNode(p2);
			break;
		}
	}
}