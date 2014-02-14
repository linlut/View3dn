
//file for deformation
//scenegraph-deformpatch.h

#ifndef _INC_SCENEGRAPH_DEFORM_PATCH_H_
#define _INC_SCENEGRAPH_DEFORM_PATCH_H_


inline void myDrawLine(const Vector3f& v0, const Vector3f& v1, const Vector3f& color)
{
	glLineWidth(2);
	glColor3fv(&color.x);
	glBegin(GL_LINES);
		glVertex3fv(&v0.x);
		glVertex3fv(&v1.x);
	glEnd();
	glLineWidth(1);
}

inline void drawDragLine(CPolyObj *ply1, CPolyObj *ply2, Vector3d v[2])
{
	ASSERT0(ply1->m_nObjID == ply2->m_nObjID);
	const Vector3d *p1 = &v[0];
	const Vector3d *p2 = &v[1];
	glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		glVertex3dv(&p1->x);
		glColor3f(1, 0, 0);
		glVertex3dv(&p2->x);
	glEnd();
}

inline void drawConnectionLine(CPolyObj *ply1, CPolyObj *ply2, Vector3d v[2])
{
	ASSERT0(ply1->m_nObjID == ply2->m_nObjID);
	const Vector3d *p1 = &v[0];
	const Vector3d *p2 = &v[1];
	glBegin(GL_LINES);
		glColor3f(0.8f, 0.8f, 0.8f);
		glVertex3dv(&p1->x);
		glVertex3dv(&p2->x);
	glEnd();
}

inline void renderAdditionalParts(CSceneGraph &m_SceneGraph)
{
	const int nsize = m_SceneGraph.size();
	if (nsize<=1) return;

	int i=1;
	Vector3d v[4];
	CPolyObj *ply1, *ply2, *ply3, *ply4;
	CSceneNode *pnode1, *pnode2, *pnode3, *pnode4;

	//at each rendering, first clear the deformed points buffer
	m_SceneGraph.m_DeformedControlList.clear();
	m_SceneGraph.m_SeparationQuadList.clear();

	//scan the scene graph and draw, also remember to record the vertices in deformed points buffer
	glEnable(GL_DEPTH_TEST);
	while (i<nsize){
		pnode1 = m_SceneGraph.GetSceneNode(i);
		ply1 = (CPolyObj *)pnode1->m_pObject;
		if (ply1==NULL){
			i++;
			continue;
		}
		switch (ply1->m_nObjType){
		case OID_CONTROL_POINT:
			pnode2 = m_SceneGraph.GetSceneNode(i+1);
			ply2 = (CPolyObj *)pnode2->m_pObject;
			TransformVertex3dToVertex3d(ply1->m_pVertex[0], pnode1->m_pFrame->matrix(), &v[0].x);
			TransformVertex3dToVertex3d(ply2->m_pVertex[0], pnode2->m_pFrame->matrix(), &v[1].x);
			if (pnode1->m_DrawParms.m_nDrawType!=CGLDrawParms::DRAW_MESH_NONE)
				drawDragLine(ply1, ply2, v);
			i+=2;
			//add into the buffer
			{  CDeformedControl ctrl(OID_CONTROL_POINT, v);
			   m_SceneGraph.m_DeformedControlList.push_back(ctrl);
			}
			break;
		case OID_CONTROL_LINE:
			pnode2 = m_SceneGraph.GetSceneNode(i+1);
			ply2 = (CPolyObj *)pnode2->m_pObject;
			pnode3 = m_SceneGraph.GetSceneNode(i+2);
			ply3 = (CPolyObj *)pnode3->m_pObject;
			pnode4 = m_SceneGraph.GetSceneNode(i+3);
			ply4 = (CPolyObj *)pnode4->m_pObject;
			TransformVertex3dToVertex3d(ply1->m_pVertex[0], pnode1->m_pFrame->matrix(), &v[0].x);
			TransformVertex3dToVertex3d(ply2->m_pVertex[0], pnode2->m_pFrame->matrix(), &v[1].x);
			TransformVertex3dToVertex3d(ply3->m_pVertex[0], pnode3->m_pFrame->matrix(), &v[2].x);
			TransformVertex3dToVertex3d(ply4->m_pVertex[0], pnode4->m_pFrame->matrix(), &v[3].x);
			if (pnode1->m_DrawParms.m_nDrawType!=CGLDrawParms::DRAW_MESH_NONE){
				Vector3d va[2]={v[0], v[2]};
				Vector3d vb[2]={v[1], v[3]};
				drawDragLine(ply1, ply3, va);
				drawDragLine(ply2, ply4, vb);
				drawConnectionLine(ply1, ply2, &v[0]);
				drawConnectionLine(ply3, ply4, &v[2]);
			}
			i+=4;
			//add into the buffer
			{  CDeformedControl ctrl(OID_CONTROL_LINE, v);
			   m_SceneGraph.m_DeformedControlList.push_back(ctrl);
			}
			break;
		case OID_CONTROL_QUAD:
			{
				const double *pmatrix = pnode1->m_pFrame->matrix();
				TransformVertex3dToVertex3d(ply1->m_pVertex[0], pmatrix, &v[0].x);
				TransformVertex3dToVertex3d(ply1->m_pVertex[1], pmatrix, &v[1].x);
				TransformVertex3dToVertex3d(ply1->m_pVertex[2], pmatrix, &v[2].x);
				TransformVertex3dToVertex3d(ply1->m_pVertex[3], pmatrix, &v[3].x);
				Vector3f n0(v[0].x, v[0].y, v[0].z);				
				Vector3f n1(v[1].x, v[1].y, v[1].z);				
				Vector3f n2(v[2].x, v[2].y, v[2].z);				
				Vector3f n3(v[3].x, v[3].y, v[3].z);	
				CSeparationQuad quad(n0, n1, n2, n3);
				m_SceneGraph.m_SeparationQuadList.push_back(quad);
				//draw this quad on the crack tip
				if (pnode1->m_DrawParms.m_nDrawType!=CGLDrawParms::DRAW_MESH_NONE){
					Vector3f color(0,0,1);
					myDrawLine(n0, n1, color);
				}
			}
			i++;
			break;
		default:
			i++;
			break;
		};
	}
}





#endif

