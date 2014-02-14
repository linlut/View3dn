//FILE: scenenode.h
#ifndef __INC_SCENENODE_H_NZ03122008_
#define __INC_SCENENODE_H_NZ03122008_

#include <vector>
#include <typeinfo>
#include <qobject.h>
#include <qwidget.h>
#include <QGLViewer/manipulatedFrame.h>

#include <sysconf.h>
#include <femattrib.h>
#include <geomath.h>
#include <drawparms.h>
#include <object3d.h>
#include "polyobj.h"


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
		const char *oname = this->GetObjectName();
		if (strstr(oname, "M")!=NULL)
			return true;
		return false;
	}

	void setMirroredNode(void) 
	{
		char *oname = (char *)this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<14);
		oname[nlen]='M';
		oname[nlen+1]=0;
	}

	bool isRot90Node(void) 
	{
		char *oname = (char *)this->GetObjectName();
		if (strstr(oname, "R0")!=NULL)
			return true;
		return false;
	}

	void setRot90Node(void)
	{
		char *oname = (char *)this->GetObjectName();
		const int nlen = strlen(oname);
		assert(nlen<13);
		oname[nlen]='R';
		oname[nlen+1]='0';
		oname[nlen+2]=0;
	}
	
	bool isRot180Node(void) 
	{
		const char *oname = this->GetObjectName();
		if (strstr(oname, "R1")!=NULL)
			return true;
		return false;
	}

	inline void setRot180Node(void)
	{
		char *oname = (char *)this->GetObjectName();
		int nlen = strlen(oname);
		assert(nlen<13);
		oname[nlen]='R';
		oname[nlen+1]='1';
		oname[nlen+2]=0;
	}

	inline bool isRot270Node(void) 
	{
		const char *oname = this->GetObjectName();
		if (strstr(oname, "R2")!=NULL)
			return true;
		return false;
	}

	inline void setRot270Node(void)
	{
		char *oname = (char *)this->GetObjectName();
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

	void glDraw(const Vector3f &campos, const Vector3f &camdir, 
				const Vector3d *pDeformedVertices, Vector3d *vertexbuff, 
				const int* pDeformedQuads = NULL, const int nPolygon = 0,
				const bool outputMesh=false);

};



#endif
