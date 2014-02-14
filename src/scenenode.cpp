//FILE: scenenode.cpp

#include "polyobj.h"
#include "scenenode.h"

using namespace qglviewer;

void CSceneNode::_initNode(CPolyObj *p)
{
	m_pObject = p;
	m_pSimulationObject = NULL;
	m_pCollisionObject = NULL;
	m_pMapping = NULL;
	m_pRendering = NULL;
	m_pFrame = NULL;
    m_bFramePtrOwned = true;

	m_pSimulationObject = NULL;	//Simulation solid, will drive the visual one
	m_pMapping = NULL;			//Mapping relation between the two
}


static void output_mesh(CPolyObj *pVisualObj)
{
	static int outputcount=0;
	//Save the file into a mesh
	char fname[200];
	sprintf(fname, "obj-%04d.obj", outputcount++);
	bool r = CPolyObjWriterFactory::GetInstance()->savePolygonMesh(pVisualObj, fname, NULL);
	if (r)
		printf("Write %s OK!\n", fname);
	else
		printf("Write %s FAILED!!\n", fname);
}


///===============================
// A normal buffer shared for public usage
vector<Vector3f> defNormal;
///===============================
static inline void 
drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw, const bool outputMesh)
{
	const bool bDefNormal = pMapping->canDeformNormal() && (pdraw->m_bShowVertexNormal || pdraw->m_bEnableSmoothing);
	if (bDefNormal){
		const int nv = pVisualObj->m_nVertexCount;
		if (nv>defNormal.size())
			defNormal.resize(nv);
		pMapping->m_pNormal = &defNormal[0];
	}
	pMapping->updateMapping(pDeformedVertices, vertexbuff);
	//backup old vertex&normal arrays, use the deformed ones
	Vector3d* bakVert = pVisualObj->m_pVertex;
	Vector3f* bakNorm = pVisualObj->m_pVertexNorm;
	pVisualObj->m_pVertex = vertexbuff;
	if (!bDefNormal)
		pVisualObj->checkNormals(true);	//force recompute normal
	else
		pVisualObj->m_pVertexNorm = &defNormal[0];
	pVisualObj->glDraw(*pdraw);
	//Save the file into a mesh
	if (outputMesh) output_mesh(pVisualObj);
	pVisualObj->m_pVertex = bakVert;
	pVisualObj->m_pVertexNorm = bakNorm;
}


void CSceneNode::glDraw(
	const Vector3f &campos, const Vector3f &camdir, 
	const Vector3d *pDeformedVertices, Vector3d *vertexbuff, 
	const int* pDeformedQuads, const int nPolygon,
	const bool outputMesh)
{
	CSceneNode &node = *this;
	CPolyObj *pVisualObj = node.m_pObject;					//visual object
	CPolyObj *pSimulationObj = node.m_pSimulationObject;	//simulation object
	CBaseMapping *pMapping = node.m_pMapping;				//mapping object	
	CGLDrawParms *pdraw = &node.m_DrawParms;
	pdraw->m_bMirrorFlag = node.isMirroredNode();

	if (node.m_pFrame){
		const double *pmatrix = node.m_pFrame->matrix();
		glMultMatrixd(pmatrix);
		//compute local camera paramters;
		Vec viewpos(campos.x, campos.y, campos.z);
		Vec localviewpos = node.m_pFrame->transformOf(viewpos);
		Vec lookat(campos.x+camdir.x, campos.y+camdir.y, campos.z+camdir.z);
		Vec locallookat = node.m_pFrame->transformOf(lookat);
		Vec localviewdir = locallookat - localviewpos;
		pdraw->m_ViewPos = Vector3f(localviewpos.x, localviewpos.y, localviewpos.z);
		pdraw->m_ViewDir = Vector3f(localviewdir.x, localviewdir.y, localviewdir.z);
	}
	else{
		//otherwise, directly copy camera info
		pdraw->m_ViewPos = campos;
		pdraw->m_ViewDir = camdir;
	}

	//=========if the alt. rendering tool is defined, use it===
	if (m_pRendering){
		m_pRendering->glDraw(pDeformedVertices, vertexbuff, pdraw);
		return;
	}

	//==============The native draw function====================
	pdraw->BeginDrawing();
	if (pDeformedVertices){ //If deformation process is running
		if (pMapping && pVisualObj){//Render visual object
			//deform vertices and normals
			drawWithMappingVisual(pVisualObj, pMapping, pDeformedVertices, vertexbuff, pdraw, outputMesh);
		}
		else{
			ASSERT0(pSimulationObj!=NULL);
			Vector3d* bakPTR = pSimulationObj->m_pVertex;
			int* bakPly = pSimulationObj->m_pPolygon;
			int bakNPly = pSimulationObj->m_nPolygonCount;
			pSimulationObj->m_pVertex = (Vector3d*)pDeformedVertices;
			if (pDeformedQuads){
				pSimulationObj->m_pPolygon = (int *)pDeformedQuads;
				pSimulationObj->m_nPolygonCount = nPolygon;
			}
			pSimulationObj->checkNormals(true);
			pSimulationObj->glDraw(*pdraw);			
			pSimulationObj->m_pVertex = bakPTR;
			pSimulationObj->m_pPolygon = bakPly;
			pSimulationObj->m_nPolygonCount = bakNPly;
		}
	}
	else{	//no simulation is running
		if (pVisualObj)
			pVisualObj->glDraw(*pdraw);
		else if (pSimulationObj)
			pSimulationObj->glDraw(*pdraw);
	}
	pdraw->PostDrawing();
	//==========================================================
}


void CSceneNode::applyManipulation(qglviewer::ManipulatedFrame * pMan)
{
	qglviewer::ManipulatedFrame *p;
	if (pMan) 
		p = pMan;
	else 
		p = m_pFrame;

	//transform the vertices;
	double matrix[4][4];
	vmCopy(p->matrix(), matrix);
	//mTranspose(matrix, matrix);
	CPolyObj * pobj = (CPolyObj*)this->m_pObject;
	pobj->transformVertexArray(matrix, pobj->m_pVertex);

	//clear the matrix to unit matrix;
	unitMatrix(matrix);
	double * pmat = (double*)p->matrix();
	mvCopy(matrix, pmat);

	//set bounding box;
	Vector3d lowleft, upright, center;
	pobj->ComputeBoundingBox(lowleft, upright);
	pobj->SetBoundingBox(AxisAlignedBox(lowleft, upright));
	center = (lowleft+upright)*0.5f;
	p->setPosition(center.x,center.y,center.z);
	pobj->applyTranslation(-center.x, -center.y,-center.z);
}

Vector3f CSceneNode::localToWorldTransform(const Vector3f& v) const
{
	if (m_pFrame==NULL) return v;

	Vector3f r;
	double d[3];
	const double *pmatrix = m_pFrame->matrix();
    TransformVertex3fToVertex3d(v, pmatrix, d);
    r.x=d[0]; r.y=d[1]; r.z=d[2];
	return r;
}

Vector3d CSceneNode::localToWorldTransform(const Vector3d& v) const
{
	if (m_pFrame==NULL) return v;

	Vector3d r;
	double d[3];
	const double *pmatrix = m_pFrame->matrix();
    TransformVertex3dToVertex3d(v, pmatrix, d);
    r.x=d[0]; r.y=d[1]; r.z=d[2];
	return r;
}

Vector3f CSceneNode::worldToLocalTransform(const Vector3f & v) const
{
	if (m_pFrame==NULL){
		printf("NULL frame pointer, possible error!\n");
		return v;
	}
	qglviewer::Frame invf = m_pFrame->inverse();
	Vector3f r;
	double d[3];
    const double *pmatrix = invf.matrix();
    TransformVertex3fToVertex3d(v, pmatrix, d);
    r.x=d[0]; r.y=d[1]; r.z=d[2];
    return r;
}

Vector3d CSceneNode::worldToLocalTransform(const Vector3d & v) const
{
	if (m_pFrame==NULL) return v;
	qglviewer::Frame invf = m_pFrame->inverse();
	Vector3d r;
	double d[3];
    const double *pmatrix = invf.matrix();
    TransformVertex3dToVertex3d(v, pmatrix, d);
    r.x=d[0]; r.y=d[1]; r.z=d[2];
    return r;
}

void CSceneNode::localToWorldTransform(const Vector3f *v,  const int len, Vector3f *tv) const
{
	int i;
	if (m_pFrame==NULL){
		for (i=0; i<len; i++) tv[i]=v[i];
		return;
	}
	double d[3];
	const double *pmatrix = m_pFrame->matrix();
	for (i=0; i<len; i++){
		Vector3f& r=tv[i];
		TransformVertex3fToVertex3d(v[i], pmatrix, d);
		r.x=d[0]; r.y=d[1]; r.z=d[2];
	}
}

void CSceneNode::localToWorldTransform(const Vector3d *v,  const int len, Vector3d *tv) const
{
	int i;
	if (m_pFrame==NULL){
		for (i=0; i<len; i++) tv[i]=v[i];
		return;
	}
	double d[3];
	const double *pmatrix = m_pFrame->matrix();
	for (i=0; i<len; i++){
		Vector3d& r=tv[i];
		TransformVertex3dToVertex3d(v[i], pmatrix, d);
		r.x=d[0]; r.y=d[1]; r.z=d[2];
	}
}

void CSceneNode::worldToLocalTransform(const Vector3f* v, const int len, Vector3f *tv) const
{
	int i;
	if (m_pFrame==NULL){
		for (i=0; i<len; i++) tv[i]=v[i];
		return;
	}
	qglviewer::Frame invf = m_pFrame->inverse();
	double d[3];
    const double *pmatrix = invf.matrix();
	for (i=0; i<len; i++){
		Vector3f& r=tv[i];
		TransformVertex3fToVertex3d(v[i], pmatrix, d);
		r.x=d[0]; r.y=d[1]; r.z=d[2];
	}
}

void CSceneNode::worldToLocalTransform(const Vector3d* v, const int len, Vector3d *tv) const
{
	int i;
	if (m_pFrame==NULL){
		for (i=0; i<len; i++) tv[i]=v[i];
		return;
	}
	qglviewer::Frame invf = m_pFrame->inverse();
	double d[3];
    const double *pmatrix = invf.matrix();
	for (i=0; i<len; i++){
		Vector3d& r=tv[i];
		TransformVertex3dToVertex3d(v[i], pmatrix, d);
		r.x=d[0]; r.y=d[1]; r.z=d[2];
	}
}


AxisAlignedBox CSceneNode::worldBoundingBox(void) const
{
	CPolyObj *pobj = dynamic_cast<CPolyObj*>(m_pObject);
	if (pobj==NULL) 
		pobj = dynamic_cast<CPolyObj*>(m_pSimulationObject);
	assert(pobj!=NULL);

	AxisAlignedBox box;
	pobj->GetBoundingBox(box);

	const Vector3d& bmin = box.minp;
	const Vector3d& bmax = box.maxp;
	const double x0 = bmin.x;
	const double y0 = bmin.y;
	const double z0 = bmin.z;
	const double x1 = bmax.x;
	const double y1 = bmax.y;
	const double z1 = bmax.z;

	Vector3d v[8]={ Vector3d(x0, y0, z0), 
					Vector3d(x1, y0, z0), 
					Vector3d(x0, y1, z0), 
					Vector3d(x1, y1, z0), 
					Vector3d(x0, y0, z1), 
					Vector3d(x1, y0, z1), 
					Vector3d(x0, y1, z1), 
					Vector3d(x1, y1, z1)
	};
	Vector3d vv[8];
	localToWorldTransform(v, 8, vv);

	AxisAlignedBox wbox;
	wbox.minp = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	wbox.maxp = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	int i;
	for (i=0; i<8; i++){
		Minimize(wbox.minp, vv[i]);
		Maximize(wbox.maxp, vv[i]);
	}
	return wbox;
}
