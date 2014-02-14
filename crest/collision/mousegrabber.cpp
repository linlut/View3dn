//mousegrabber.cpp

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <geomath.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>
#include <crest/io/nametableitem.h>
#include <crest/ogl/GPURendering.h>
#include "mousegrabber.h"

namespace cia3d{
namespace collision{

CIA3D_DECL_CLASS(CMouseGrabber);

static unsigned char CMouseGrabberClass = 
	cia3d::core::RegisterObject("MouseGrabber")
	.add<CMouseGrabber>()
	.addAlias("MouseGrabber");


using namespace cia3d::core;

void CMouseGrabber::glDraw(const CGLDrawParms &dparms)
{
	glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	//=============================================
	m_pDrawParms = (CGLDrawParms*)(&dparms);
	_BeginDrawing(m_pDrawParms);
		//draw the vertices if required
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glMatrixMode(GL_MODELVIEW_MATRIX);
		glPopMatrix();
		glPopMatrix();
			//draw the grabbed vertex in gloabl coord.
			glDrawVertexSphere(1, NULL);
			//draw the realtime vertex position in the deformation
			m_pVertex[2] = m_realtimePos;
			glDrawVertexSphere(2, NULL);
			//line connecting them
			glBegin(GL_LINES);
			glVertex3dv(&m_pVertex[1].x);
			glVertex3dv(&m_pVertex[2].x);
			glEnd();
		glPushMatrix();
		glPushMatrix();
	_EndDrawing(m_pDrawParms);
	//=============================================
}


void CMouseGrabber::_init(const CPolyObj *pobj, const int vertID)
{
	//save pointers
	m_pTargetObj = (CPolyObj *)pobj;	
	m_nTargetVertexID = vertID;	
	m_nVertexCount = 1;

	//init vertex pos
	if (m_pVertex==NULL){
		m_pVertex = new Vector3d[8];
		for (int i=0; i<8; i++) m_pVertex[i]=Vector3d(0,0,0);
	}
	assert(m_pVertex!=NULL);
	m_initialPos = pobj->m_pVertex[vertID];
	m_realtimePos = m_initialPos;
	m_pVertex[0]= m_initialPos;				//This pos. is in the gloabl coor.
	m_pVertex[1]= m_initialPos;				//This pos. is in the gloabl coor.

	//set sphere radius
    m_fGivenRad=m_pTargetObj->estimatedVertexRadius();
}


void CMouseGrabber::parse(ZBaseObjectDescription* arg)
{
	//printf("Parse is called!\n");
	//first, construct for parent class
	CPointObj::parse(arg);
	QDomElement element = arg->toElement();
	ObjectLib *objlib = ObjectLib::getInstance();

	//=========================================
	QString targetObjectName = element.attribute("targetobject", "");
	CObjectBase *pTargetObj = objlib->find_object(targetObjectName);
	if (pTargetObj==NULL){ //object not found
		printf("Object %s not found!\n", targetObjectName);
		return;
	}
	CPolyObj *pPoly = dynamic_cast<CPolyObj*>(pTargetObj);
	assert(pPoly!=NULL);
	
	//=========================================
	//read parameters: targetvertexid;
	int vertexid = 0;
	typedef cia3d::io::CNameTableItem NameTableItem;
	NameTableItem table[]={
		NameTableItem("targetvertexid", 'i', &vertexid)
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	//use the target object pointer and vertexid to get the absolute vertexid
	
	//init
	_init(pPoly, vertexid);

	//set bounding box
	AxisAlignedBox box;
	Vector3d lowleft, upright;
	ComputeBoundingBox(lowleft, upright);
	box.minp = lowleft, box.maxp = upright;
	SetBoundingBox(box);
}


int CMouseGrabber::findGrabbingVertex(const double *pmatrix, const Vector3d *pVertexArray, const int n)
{
	//convert into global cooridnate
	Vector3d v = m_initialPos;
	if (pmatrix!=NULL){
		Vector3d v1 = m_pVertex[0];
		Vector3d v2;
		TransformVertex3fToVertex3d(v1, pmatrix, v2);
		v = v2;
	}
	m_pVertex[1]= v;

	//if the vertex has already been found, we just need to get it
	if (m_nRealtimePos != -1){
		m_realtimePos = pVertexArray[m_nRealtimePos];
		return m_nRealtimePos;
	}

	//otherwise, find it now
	m_nRealtimePos = -1;
	double mindis = 1e100;
	for (int i=1; i<n; i++){
		double dis = Distance2(v, pVertexArray[i]);
		if (dis<mindis){
			mindis = dis;
			m_nRealtimePos  = i;
		}
	}
	m_realtimePos = pVertexArray[m_nRealtimePos];	
	return m_nRealtimePos;
}


}//end namespace collision
}//end namespace cia3d