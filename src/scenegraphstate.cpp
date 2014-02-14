//FILE: scenegraphstate.cpp

#include "scenegraphstate.h"

//===============================================================
static inline void COPY_MATRIX_TO_ARRAY(const Matrix& m, double *p)
{
	p[0] = m.x[0][0];
	p[1] = m.x[0][1];
	p[2] = m.x[0][2];
	p[3] = m.x[0][3];
	p[4] = m.x[1][0];
	p[5] = m.x[1][1];
	p[6] = m.x[1][2];
	p[7] = m.x[1][3];
	p[8] = m.x[2][0];
	p[9] = m.x[2][1];
	p[10] = m.x[2][2];
	p[11] = m.x[2][3];
	p[12] = m.x[3][0];
	p[13] = m.x[3][1];
	p[14] = m.x[3][2];
	p[15] = m.x[3][3];
}


static void prepareMirrorMatrix2(const Vector3f & z, const Vector3f &p, Matrix &mat)
{
	Vector3f V = Normalize(z);
	const float rdot2 = DotProd(p, V)*2.0f;

	SetIdentityMatrix(mat);
	mat.x[0][0] = 1-2*V.x*V.x;
	mat.x[0][1] = -2*V.x*V.y;
	mat.x[0][2] = -2*V.x*V.z;
	mat.x[0][3] = rdot2*V.x;
	mat.x[1][0] = -2*V.x*V.y;
	mat.x[1][1] = 1-2*V.y*V.y;
	mat.x[1][2] = -2*V.y*V.z;
	mat.x[1][3] = rdot2*V.y;
	mat.x[2][0] = -2*V.x*V.z;
	mat.x[2][1] = -2*V.y*V.z;
	mat.x[2][2] = 1-2*V.z*V.z;
	mat.x[2][3] = rdot2*V.z;
	mat.transpose();
}


void addMirroredSceneNodes(CSceneGraph &m_SceneGraph, const Vector3f v[3])
{
	const CSceneNode tmpnode;
	const int nn = m_SceneGraph.size();
	
	cout<<"Mirror plane parameters:\n";
	printf("v0: (%f %f %f)\n", v[0].x, v[0].y, v[0].z);
	printf("v1: (%f %f %f)\n", v[1].x, v[1].y, v[1].z);
	printf("v2: (%f %f %f)\n", v[2].x, v[2].y, v[2].z);
	
	//create mirror nodes;
	for (int i=0; i<nn; i++){
		CSceneNode * pnode = m_SceneGraph.GetSceneNode(i);
		assert(pnode!=NULL);
		if (!pnode->isOriginalNode())
			continue;

		//prepare matrix, it is different for each object
		//therefore, we cant move out this part out of the loop
		Vector3f vert[3]={v[0],v[1],v[2]}, rv[3];
		pnode->worldToLocalTransform(vert, 3, rv);
		const Vector3f norm = compute_triangle_normal(rv[0], rv[1], rv[2]);
		Matrix m;
        prepareMirrorMatrix2(norm, rv[0], m);
		//m.Dump();

		//append a new node into the scene;
        m_SceneGraph.AddSceneNode(tmpnode);
		const int nsize = m_SceneGraph.size();
		CSceneNode *__pnode=m_SceneGraph.GetSceneNode(nsize-1);
		assert(__pnode!=NULL);
		CSceneNode &anode = *__pnode;
		anode.m_pObject = pnode->m_pObject;
		anode.m_DrawParms = pnode->m_DrawParms;
		//anode.m_pFrame=new qglviewer::ManipulatedFrame(*(pnode->m_pFrame));
		anode.m_pFrame=new qglviewer::ManipulatedFrame();
		*anode.m_pFrame=*(pnode->m_pFrame);

		//Matrix m2;
		//anode.m_pFrame->getMatrix(m2.x);
		//m2.Dump();
		COPY_MATRIX_TO_ARRAY(m, anode.m_DrawParms.m_Identity);
		anode.SetObjectName(pnode->GetObjectName());
		anode.setMirroredNode();
	}
}


void addMirroredSceneNodes(CSceneGraph &sceneGraph)
{
	//get the mirror plane;
	if (!sceneGraph.mirrorTriangleBeenSet())
		return;

	Vector3d vv[3];
	sceneGraph.getMirrorTriangle(vv);
	Vector3f v[3];
	v[0]=Vector3f(vv[0].x, vv[0].y, vv[0].z);
	v[1]=Vector3f(vv[1].x, vv[1].y, vv[1].z);
	v[2]=Vector3f(vv[2].x, vv[2].y, vv[2].z);

	//append nodes
	addMirroredSceneNodes(sceneGraph, v);
}


static inline void PrepareRotationMatrix(const Vector3d & v0, const Vector3d & v1, const float arc, Matrix& mt)
{	
	const Vector3d base(v0.x, v0.y, v0.z);
	const Vector3d ref(v1.x, v1.y, v1.z);
	Vector3d axis = ref - base;
	axis = Normalize(axis);
	Matrix m, t1, t2;
	RotationAxisMatrix(axis, arc, m);
    SetTranslationMatrix(-base, t1);
    SetTranslationMatrix(base, t2);
    mt = (t1 * m) * t2;
}


void addRotAngleSceneNodes(CSceneGraph &m_SceneGraph, const float ang, const Vector3f endp[2])
{
	const int nn = m_SceneGraph.size();
	const CSceneNode tmpnode;
	Matrix mt;
	Vector3d v[2];

	v[0].x = endp[0].x, v[0].y=endp[0].y, v[0].z=endp[0].z;
	v[1].x = endp[1].x, v[1].y=endp[1].y, v[1].z=endp[1].z;

	//create rotation nodes;
	for (int i=0; i<nn; i++){
		CSceneNode * pnode = m_SceneGraph.GetSceneNode(i);
		assert(pnode!=NULL);
		if (!pnode->isOriginalNode())			
			continue;
		//prepare matrix
		Vector3d rv[2];
		pnode->worldToLocalTransform(&v[0], 2, rv);
		PrepareRotationMatrix(rv[0], rv[1], (float)(ang*PI/180.0), mt);

		m_SceneGraph.AddSceneNode(tmpnode);
		const int nsize = m_SceneGraph.size();
		CSceneNode *__pnode=m_SceneGraph.GetSceneNode(nsize-1);
		assert(__pnode!=NULL);
		CSceneNode &anode = *__pnode;

		anode.m_pObject = pnode->m_pObject;
		anode.m_DrawParms = pnode->m_DrawParms;
//		anode.m_pFrame=new qglviewer::ManipulatedFrame(*(pnode->m_pFrame));
		anode.m_pFrame=new qglviewer::ManipulatedFrame();
		*anode.m_pFrame=*(pnode->m_pFrame);
		COPY_MATRIX_TO_ARRAY(mt, anode.m_DrawParms.m_Identity);	
		anode.SetObjectName(pnode->GetObjectName());
		switch((int)ang){
		case 90:
			anode.setRot90Node(); break;
		case 180:
			anode.setRot180Node(); break;
		case 270:
			anode.setRot270Node(); break;
		default:
			assert(0);
		}
		m_SceneGraph.AddSceneNode(anode);
	}
}


void addRotAngleSceneNodes(CSceneGraph &m_SceneGraph, const float ang)
{
	Vector3d v[2];
	if (!m_SceneGraph.rotationAxisBeenSet())
		return;

	m_SceneGraph.getRotationAxis(v);
	Vector3f endp[2]={  Vector3f(v[0].x, v[0].y, v[0].z), 
						Vector3f(v[1].x, v[1].y, v[1].z)};
	addRotAngleSceneNodes(m_SceneGraph, ang, endp);
}


//===========================================================
CSceneGraphState::CSceneGraphState(CSceneGraph &s)
{
	pScenegraph = &s;

	//save state
	bm = s.HasMirroredObjects();
	b90 = s.HasRot90Objects();
	b180 = s.HasRot180Objects();
	b270 = s.HasRot270Objects();
	nNodeCount = s.size(); 
	pbaknode=NULL;
	pactivenode = s.GetActiveSceneNode();

	//save mirror/rot node
	pScenegraph->GetActiveFace(pActiveSceneNode4Face, fid);
	pScenegraph->GetActiveLine(pActiveSceneNode4Edge, lineid0, lineid1);
	s.setActiveSceneNode(NULL);

	//get mirror and rotation geometry info
	Vector3d tri[3];
	CSceneNode *pnode = NULL;
	if (s.mirrorTriangleBeenSet()){
		s.getMirrorTriangle(tri);
		mirrorplane[0] = Vector3f(tri[0].x, tri[0].y, tri[0].z);
		mirrorplane[1] = Vector3f(tri[1].x, tri[1].y, tri[1].z);
		mirrorplane[2] = Vector3f(tri[2].x, tri[2].y, tri[2].z);
	}
	if (s.rotationAxisBeenSet()){
		s.getRotationAxis(tri);
		rotline[0] = Vector3f(tri[0].x, tri[0].y, tri[0].z);
		rotline[1] = Vector3f(tri[1].x, tri[1].y, tri[1].z);
	}		

	//save all the nodes
	if (nNodeCount>0) pbaknode = new CSceneNode[nNodeCount];
	for (int i=0; i<nNodeCount; i++){
		CSceneNode *p = s.GetSceneNode(i);
		pbaknode[i] = *p;
	}
}

void CSceneGraphState::restoreOldSceneGraph(void)
{
	if (pbaknode==NULL)
		return;
	CSceneGraph & sceneGraph = *pScenegraph;
	sceneGraph.RemoveAll();
	for (int i=0; i<nNodeCount; i++)
		sceneGraph.AddSceneNode(pbaknode[i]);

	sceneGraph.setActiveSceneNode(pactivenode);
	sceneGraph.restoreMirrorRotNodes(pActiveSceneNode4Face, fid, pActiveSceneNode4Edge, lineid0, lineid1);
}

void CSceneGraphState::clearSceneGraphState(void)
{
	if (!pbaknode) return;
	CSceneGraph & scenegraph = *pScenegraph;
	for (int i=0; i<nNodeCount; i++) 
		pbaknode[i].m_pFrame=NULL;
	delete [] pbaknode;
	pbaknode = NULL;
}

void CSceneGraphState::useSceneGraphState(const bool recomputeTexture1DRange)
{
	CSceneGraph & scenegraph = *pScenegraph;
	Vector3d v[3];
	
	//SET COLOR PROPERTIES
	BestMatchDrawParms(*pScenegraph, pbaknode, nNodeCount, recomputeTexture1DRange);

	//set mirror objects
	if (bm){
		v[0] = Vector3d(mirrorplane[0].x, mirrorplane[0].y, mirrorplane[0].z);
		v[1] = Vector3d(mirrorplane[1].x, mirrorplane[1].y, mirrorplane[1].z);
		v[2] = Vector3d(mirrorplane[2].x, mirrorplane[2].y, mirrorplane[2].z);
		scenegraph.setMirrorTriangle(v);
		scenegraph.RemoveAllMirroredObjects();
		addMirroredSceneNodes(scenegraph, mirrorplane);
	}

	//set rotation-symetric objects
	if (b90){ 
		scenegraph.RemoveAllRot90Objects();
		addRotAngleSceneNodes(scenegraph, 90, rotline);
	}
	if (b180){
		scenegraph.RemoveAllRot180Objects();
		addRotAngleSceneNodes(scenegraph, 180, rotline);
	}
	if (b270){
		scenegraph.RemoveAllRot270Objects();
		addRotAngleSceneNodes(scenegraph, 270, rotline);
	}
	if (b90||b180||b270){
		v[0] = Vector3d(rotline[0].x, rotline[0].y, rotline[0].z);
		v[1] = Vector3d(rotline[1].x, rotline[1].y, rotline[1].z);
		scenegraph.setRotationAxis(v);
	}

}

