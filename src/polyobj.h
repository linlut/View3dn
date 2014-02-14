//polyobj.h

#ifndef __INC_POLYOBJ_H__

#define __INC_POLYOBJ_H__

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <map>
#include <string>

#include "object3d.h"
#include "memmgr.h"
#include "drawparms.h"
#include "maths.h"
#include "MeshRenderLib.h"


#define OID_NULL      -1
#define OID_POLYGON   0
#define OID_TRIANGLE  1
#define OID_QUAD      2
#define OID_TEX3D     3

#define OID_CONTROL_POINT  100
#define OID_CONTROL_LINE   101
#define OID_CONTROL_QUAD   102

#define STRNAME_TRIANGLE "tri"
#define STRNAME_QUAD     "qad"
#define STRNAME_TEX3D    "tex3d"


//this function is used to generate an estimation of the max polygonsize, 
//in case the surface mesh is to be expanded.
inline int DETERMINE_DEFORMABLE_POLYSIZE(const int nply)
{
	return nply*2 + 2048;
}

inline void PolyCopyVertices(const int nv, Vector3d *v, const bool allocbuf, int &m_nVertexCount, Vector3d *&m_pVertex)
{
	//if new buffer space is required, we make a copy of them;
    m_nVertexCount = nv;			//total vertex count
	if (!allocbuf) m_pVertex = v;	//the vertex array
	else{
		m_pVertex = new Vector3d[nv];
		assert(m_pVertex!=NULL);
		memcpy(m_pVertex, v, nv*sizeof(Vector3d));
	}
}

inline void PolyCopySurfaces(const int ntri, int *ptri, const int nVPT,  const bool allocbuf, int &m_nPolygonCount, int *&m_pPolygon)
{
	//if new buffer space is required, we make a copy of them;
    m_nPolygonCount = ntri;				//total vertex count
	if (!allocbuf) m_pPolygon = ptri;	//the vertex array
	else{
		m_pPolygon = new int[ntri*nVPT];
		assert(m_pPolygon!=NULL);
		memcpy(m_pPolygon, ptri, ntri*nVPT*sizeof(int));
	}
}

inline void PolyCopyElements(const int nelm, int *pelm, const int nVPE,  const bool allocbuf, int &m_nElementCount, int *&m_pElement)
{
	//if new buffer space is required, we make a copy of them;
    m_nElementCount = nelm;				//total vertex count
	if (!allocbuf) m_pElement = pelm;	//the vertex array
	else{
		m_pElement = new int[nelm*nVPE];
		assert(m_pElement!=NULL);
		memcpy(m_pElement, pelm, nelm*nVPE*sizeof(int));
	}
}


inline void TransformVertex3dToVertex3d(const Vector3d& p, const double* matrix, double v[3])
{
	if (matrix==NULL){
		v[0]=p.x, v[1]=p.y, v[2]=p.z;
	}
	else{
		double u[3], m2[4][4];
		vmCopy(matrix, m2);
		u[0]=p.x, u[1]=p.y, u[2]=p.z;
		pmMult(u, m2, v);
	}
}

inline void TransformVertex3fToVertex3d(const Vector3d& p, const double* matrix, Vector3d& q)
{
    TransformVertex3dToVertex3d(p, matrix, &q.x);
}

inline void TransformVertex3fToVertex3d(const Vector3f& p, const double* matrix, double v[3])
{
    const Vector3d q(p.x, p.y, p.z); 
    TransformVertex3dToVertex3d(q, matrix, v);
}

inline double READ_FORTRAN_FLOAT(char * s)
{
	ASSERT0(s[0]!=0);
	double d;
	int i;
	const int n = strlen(s);
	if (n>4){
		bool hase =false;
		bool hassign = false;
		int pos=0;
		for (i=0; i<n; i++){
			if (s[i]=='E' || s[i]=='e'){
				hase = true;
				break;
			}
		}
		if (!hase){
			for (i=n-4; i<n; i++){
				if (s[i]=='-' || s[i]=='+'){
					hassign = true;
					pos = i;
					break;
				}
			}
			if (hassign){
				for (i=n-1; i>=pos; i--)
					s[i+1] = s[i];
				s[pos]='E';
				s[n+1]=0;
			}
		}
	}
	sscanf(s, "%lf", &d);
	if (fabs(d)<1E-37) d=0;
	return d;
}



inline void SETUP_PICKING_GLENV(void)
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_BLEND);
}



class CEdgeTableCell;

class CPolyObj: public CObject3D, public CMemoryMgr
{
public:
	enum CONST_VALUES
	{
		ATTRIBUTE_NUM = 48
	};

	int m_nObjID;				//created for deformation, for fast checking the objIDs;
	int m_nObjType;				//created for deformation, for fast checking the objIDs;

	int m_nVertexCount;			//total vertex count
	Vector3d * m_pVertex;		//vertex array pointer

	//boundary surface description
	int m_nPolygonCount;		//total polygon count
    int *m_pPolygon;			//number of vertices per polygon
    int m_nPlyVertexCount;      //number of vertices per polygon

	//Solid element count and type
	int m_nElementCount;		// # of element
    int m_nElmVertexCount;      //number of vertices per element
	int *m_pElement;			// element array pointer

    Vector3d * m_pLoad;         //vertex load (used in FEM) 

	Vector3f * m_pVertexNorm;	//vertex normals

	Vector3f * m_pPolyNorm;		//polygon normals;

	float *m_pVertexTexCoor1D;	//1D texture coordinate for vertices;

	unsigned char *m_pFemBoundCond; //Boundary condition bits in FEM simulation

	float *m_pFAttributes[ATTRIBUTE_NUM];	//attributes array, float
	int *m_pIAttributes[ATTRIBUTE_NUM];		//attributes array, int
	char m_strFVarNames[ATTRIBUTE_NUM][9];	//attributes names, float
	char m_strIVarNames[ATTRIBUTE_NUM][9];	//attributes names, int

	CEdgeTableCell **m_ppEdgeTable;

	unsigned char (*m_pVertexColor)[4];

	int m_nMaterialID;			//Material ID;

	float m_fGivenRad;			//Sphere radius if we render each vertex using a ball

public:

	CPolyObj(void);

	virtual ~CPolyObj(void);

	void CopyAttributes(CPolyObj &aobj, int *vindex);

	void copyVerticesAndConnection(Vector3d *pVertex, const int nv, const int nVPT, int *pTri, const int nTri, const bool allocbuf)
	{
		PolyCopyVertices(nv, pVertex, allocbuf, m_nVertexCount, m_pVertex);
		PolyCopySurfaces(nTri, pTri, nVPT,  allocbuf, m_nPolygonCount, m_pPolygon);
	}

	void AddVertexAttributeFloat(const char *attrname, float *pattrib);

	void AddVertexAttributeInt(const char *attrname, int *pattrib);

	// get attribute index;
	int GetFAttributeIndexByName(const char *strname)
	{
		int i, idx=-1;
		for (i=0; i<ATTRIBUTE_NUM; i++){
			float * p = m_pFAttributes[i];
			if (p==NULL) continue;
			if (strcmp(m_strFVarNames[i], strname)==0){
				idx = i;
				break;
			}
		}
		return idx;
	}

	int* GetVertexAbsIndexPointer(void)
	{
		int i = GetFAttributeIndexByName("IP");
		if (i>=ATTRIBUTE_NUM || i<0){
			printf("Error: no abs index!\n");
			return NULL;
		}
		int *p = (int*)m_pFAttributes[i];
		return p;
	}

	float* GetVertexObjectIDPointer(void)
	{
		float *p;
		int index = GetFAttributeIndexByName("OBJ");
		if (index <0 || index>ATTRIBUTE_NUM-3)
			p = NULL;	
		else
			p = m_pFAttributes[index];		
		return p;
	}

	double GetHiddenLineOffset(void)
	{
		const double scale = 4.20E-4f;
		AxisAlignedBox box;
		this->GetBoundingBox(box);
		const Vector3d dim = box.maxp - box.minp;
		const double offset = (dim.x+dim.y+dim.z)*scale;
		return offset;
	}

	bool NeedLighting(void)
	{
		if (m_pDrawParms==NULL)
			return false;
		return m_pDrawParms->m_bEnableLighting;
	}

	bool NeedTexture1D(void)
	{
		if (m_pDrawParms==NULL)
			return false;
		return m_pDrawParms->m_bEnableTexture1D;
	}

    bool isTriangleBoundary(void) const
    {
        return (m_nPlyVertexCount==3); 
    }
    
    bool isQuadBoundary(void) const
    {
        return (m_nPlyVertexCount==4); 
    }

	//Transform the vertices to another vertex array, or
	//save to the original vertex buffer, if pvertex=NULL
	void transformVertexArray(const double matrix[4][4], Vector3d *pvertex=NULL);
	
    bool isFiniteElement(void) const
    {
        return (m_nPlyVertexCount>=3 && m_nElmVertexCount>=4);
    }

    bool isFiniteMass(void) const
    {
        return false;
    }

    bool isShellElement(void) const
    {
        const bool b=(m_nPlyVertexCount>=3) && (m_nElmVertexCount==0);
        return b;
    }

    bool isParticleElement(void) const
    {
        const bool b=(m_nPlyVertexCount==0) && (m_nElmVertexCount==0);
        return b;
    }

    bool isRodElement(void) const
    {
        const bool b=(m_nElmVertexCount==2);
        return b;
    }

    //=====================================================================
	virtual int ObjectCount(int objidbuff[], const int bufflen);

	//=====================Drawing functions================================
	//prepare the normal vectors for rendering, 
	//in case of animation we need to recompute normals
	virtual void checkNormals(const bool recompute = false);

	//volume rendering function, NULL for surface objects
	virtual void drawVolumetricElements(void);

	void glDrawVertices(const Vector3f &color)
	{	
		_DrawVertices(color);
	}

	void glDrawVertexNormals(const float scalefactor, const Vector3f& color)
	{
		_DrawVertexNormals(scalefactor, color);
	}

	virtual double estimatedVertexRadius(void)
	{
		AxisAlignedBox box;
		GetBoundingBox(box);
		const Vector3d dif = box.maxp - box.minp;
		const double r = fabs(dif.x+dif.y+dif.z)*3e-3f;
		return r;
	}

	virtual void CopyAttribNames2DrawParms(CGLDrawParms &drawparms)
	{
		int i;
		for (i=0; i<ATTRIBUTE_NUM; i++) 
            drawparms.m_strVarNames[i] = m_strFVarNames[i];
	}

	virtual void ComputeBoundingBox(Vector3d& lowleft, Vector3d &upright)
	{
		int i;
		lowleft = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
		upright = Vector3d(-MAXFLOAT,-MAXFLOAT,-MAXFLOAT);
		for (i=0; i<m_nVertexCount; i++){
			const Vector3d& vv = m_pVertex[i];
            Vector3d v(vv.x, vv.y, vv.z);
			Minimize(lowleft, v);
			Maximize(upright, v);
		}
	}

	virtual void ComputeBoundingBox(Vector3f& lowleft, Vector3f &upright)
	{
        Vector3d v1, v2;
	    ComputeBoundingBox(v1, v2);
        lowleft=Vector3f(v1.x, v1.y, v1.z);
        upright=Vector3f(v2.x, v2.y, v2.z);
	}

	virtual void glDrawVertexSphere(const int eid, const bool reversenorm);

	virtual void glDrawVertexSphere(const int eid, void *p=NULL)
	{
		bool reversenorm = false;
		if (p) reversenorm=true;
		glDrawVertexSphere(eid, reversenorm);
	}

	virtual void drawVertexCube(const int eid);

	virtual void drawVertexTetra(const int eid);

	virtual void drawVertexPoint(const int eid);

	virtual void glDrawElementFaceGoround(const int eid, void *pvoid){};

	virtual void glDrawElementFacePhong(const int eid, void *pvoid){};

	virtual void glDrawElementLine(const int eid, const float offset, const bool needtex){};

	virtual void glDraw(const CGLDrawParms &dparms);

	virtual void ComputePolygonNormals(void){assert(0);};

    virtual void ComputeVertexNormals(void){assert(0);};

	virtual double* computeElementVolumes(void){ return NULL; }

    virtual double* computeVertexVolumes(void){ return NULL; }

    virtual void flipNormal(void)
    {
        int i;
        if (m_pVertexNorm){
	        for (i=0; i<m_nVertexCount; i++) 
                m_pVertexNorm[i]=-m_pVertexNorm[i];
        }
        if (m_pPolyNorm){
	        for (i=0; i<m_nPolygonCount; i++) 
                m_pPolyNorm[i]=-m_pPolyNorm[i];
        }
    }

	//======================FILE IMPORT/EXPORT===============================
	virtual const char *Description(void) const
	{
		return "plyobj";
	}

	virtual int LoadFile(const char *fname)
	{
		assert(0);
		return 1;
	}

	virtual void SetCustomizedObjectName(const int KEY)
	{
		int key;
		float* pMat = GetVertexObjectIDPointer();
		if (pMat==NULL) 
			key = KEY;
		else
			key = (int) pMat[0];
		//set the object name;
		const char *des = this->Description();
		char namebuff[20]="";
		sprintf(namebuff, "%s_%02d", des, key);
		this->SetObjectName(namebuff);
	}

	void ComputeVertexTextureCoordiates(
		const bool useNewRange, const int attribIdx, const int mappingType, float& minval, float& maxval);

	virtual int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib)=0;


	int LoadPltVertices(FILE *fp, const int nv, const int nTotalAttrib);


	static int ReadPltFileHeader(FILE *fp, char *chunktype, 
		int &nvert, int &nface, float &radius, 
		int &nfattrib, char fattrib[][9], char ifattrib[][9]);


	virtual void applyTranslation(const double& tx, const double& ty, const double& tz)
	{
		const Vector3d disp(tx, ty, tz);
		for (int i=0; i<m_nVertexCount; i++){
			Vector3d &v= m_pVertex[i];
			v += disp;
		}
	}


	virtual void mirrorXYPlane(void)
	{
		//reverse the z value
		const int nv = this->m_nVertexCount;
		for (int i=0; i<nv; i++){
			double &z= m_pVertex[i].z;
			z = -z;
		}
	}


	virtual void applyScaling(const double& sx, const double& sy, const double& sz)
	{
		AxisAlignedBox box;
		GetBoundingBox(box);

		//scale geometry;
		for (int i=0; i<m_nVertexCount; i++){
			Vector3d &v= m_pVertex[i];
			v.x *= sx;
			v.y *= sy;
			v.z *= sz;
		}		
		//scale the bounding box;
		const Vector3d center=(box.minp+box.maxp)*0.5f;
		Vector3d minp = box.minp-center;
		Vector3d maxp = box.maxp-center;
		minp.x*=sx, minp.y*=sy, minp.z*=sz; 
		maxp.x*=sx, maxp.y*=sy, maxp.z*=sz; 
		box.minp = minp+center;
		box.maxp = maxp+center;
		SetBoundingBox(box);
	}


	//=====================Boundary conditions===========================
	virtual bool *getBoundaryVertexFlags(void);


	//setup the boundary condition buffer before propergation
	virtual void setupBoundaryConditionBufferBeforePropergation(void)
	{
		// Alloc space for bit buffer
		if (m_pFemBoundCond==NULL){
			m_pFemBoundCond = new unsigned char [m_nVertexCount];
			assert(m_pFemBoundCond!=NULL);
			for (int i=0; i<m_nVertexCount; i++) m_pFemBoundCond[i]=0;
		}
		else{// Clear the visisted bit and inqueue bit
			for (int i=0; i<m_nVertexCount; i++) m_pFemBoundCond[i]&=0x7f;
		}
	}


	//apply boundary conditions to the selected vertices
	virtual void setBoundaryFixBit(const int bitpos)
	{
		CPolyObj *pobj = this;
		const int s = 1<<bitpos;
		unsigned char *pBits= pobj->m_pFemBoundCond;
		if (pBits==NULL) return;

		for (int i=0; i<pobj->m_nVertexCount; i++){
			unsigned char& b = pBits[i];
			//if the MSB is set, then it is selected or active
			if (b&0x80) 
				b |= s;
		}
	}

	//==============CIA3D Export Functions===============================
	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL)
	{
		assert(0);
	}

	virtual void exportBoundaryConditions(FILE *fp, const int baseno=0);

	virtual int exportElemConnectivity(FILE *fp, const int objID, const int objSetID, const int tetbaseno, const int vbaseno, const int *vbasebuffer=NULL) 
	{ assert(0); return 0; }

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double thickness=0);

	virtual int exportElemNodeDisplacement(FILE *fp, const int baseno, const Vector3d & displ);

    virtual int exportElemNodeDisplacementAndAngle(FILE *fp, const int baseno, const Vector3d & displ);

	virtual int exportElemNodeVelocity(FILE *fp, const int baseno, const Vector3d & velocity);

    virtual int exportElemNodeVelocityAndAngle(FILE *fp, const int baseno, const Vector3d & velocity);

	virtual int exportElemNodeLoad(FILE *fp, const int baseno, const Vector3d & velocity);

	virtual void propergateFemBoundSeed(const int tid, const float THRESHOLD)
	{
		assert(0);
	}

	virtual void getPolygonCoordinates(const int index, Vector3d v[], int &nv){ nv = 0; }

protected:
	CObjectBase *m_pRenderingObject; //a pointer to a more advanced render tech.

	CGLDrawParms * m_pDrawParms;

	//drawing funcs for OpenGL
	void _BeginDrawing(CGLDrawParms *pDrawParms);

	void _EndDrawing(CGLDrawParms *pDrawParms);

	void _DrawShadedSurfaces(void);

	void _DrawLineForSurfaces(const float offset=0);

	void _DrawHiddenLineSurfaces(void);

	void _DrawVertexAttributeText(void);

	void _DrawVertices(const Vector3f &vertexcolor);

	void _DrawVertexNormals(const float scalefactor, const Vector3f& color);

	//FEM simulation
	virtual void propergateFemBoundSeed(const int tid, const float THRESHOLD, const int *ptri, int nstep);

private:
    GLuint m_dispListID;
    GLuint m_vboID;
    GLuint m_eboID;

    MeshRender::MeshRender m_meshRender;

	void _clearAllBuffer(void);

	void _init();

};


//Object loader
class CPolyObjLoaderFactory
{
public:
	typedef CPolyObj* PolyObjLoader(const char *fname);

private:
	std::map<std::string, PolyObjLoader*> regPolyloader;

public:
	CPolyObjLoaderFactory(void){}

	~CPolyObjLoaderFactory(void)
	{
		regPolyloader.clear();
	}

	bool addEntry(const char *file_ext, PolyObjLoader *loader)
	{
		regPolyloader[file_ext]=loader;
		return true;
	}

	CPolyObj *loadPolygonMesh(const char *fname)
	{
		int i;
		CPolyObj *pobj = NULL;
		const char *fname_ext = "";

		//get '.' position
		const int slen = strlen(fname);
		for (i=slen-1; i>=0; i--){
			if (fname[i]=='.') break;
		}
		if (i>0){
			//normal file with a file extension
			fname_ext = &fname[i];
		}
		else{
			//no . found, assume d3plot file
		}

		//call the load function
		PolyObjLoader *pLoader = regPolyloader[fname_ext];
		if (pLoader!= NULL) pobj = pLoader(fname);
		return pobj;
	}

	inline static CPolyObjLoaderFactory *GetInstance(void)
	{
		static CPolyObjLoaderFactory m;
		return &m;
	}

	inline static bool AddEntry(const char *file_ext, PolyObjLoader *loader)
	{
		CPolyObjLoaderFactory *p = GetInstance();
		return p->addEntry(file_ext, loader);
	}

	inline static CPolyObj *LoadPolygonMesh(const char *fname)
	{
		CPolyObjLoaderFactory *p = GetInstance();
		CPolyObj *o = p->loadPolygonMesh(fname);
		return o;
	}

};


//Object writer
class CPolyObjWriterFactory
{

public:
	typedef bool PolyObjWriter(CPolyObj*, const char *fname, const double *matrix);

private:
	std::map<std::string, PolyObjWriter*> regPolyWriter;

public:
	CPolyObjWriterFactory(void){};

	~CPolyObjWriterFactory(void){};

	bool addEntry(const char *file_ext, PolyObjWriter *writer)
	{
		regPolyWriter[file_ext]=writer;
		return true;
	}

	bool savePolygonMesh(CPolyObj *pobj, const char *fname, const double *matrix)
	{
		int i;
		bool state = false;
		
		//get '.' position
		const int slen = strlen(fname);
		for (i=slen-1; i>=0; i--)
			if (fname[i]=='.') break;
		if (i<=0) return state;
		const char *fname_ext = &fname[i];

		PolyObjWriter *pWriter = regPolyWriter[fname_ext];
		if (pWriter!= NULL)
			state = pWriter(pobj, fname, matrix);
		return state;
	}

	static CPolyObjWriterFactory *GetInstance(void)
	{
		static CPolyObjWriterFactory m;
		return &m;
	}

	static bool AddEntry(const char *file_ext, PolyObjWriter *Writer)
	{
		return GetInstance()->addEntry(file_ext, Writer);
	}

	static bool SavePolygonMesh(CPolyObj *pobj, const char *fname, const double *matrix)
	{
		bool f = GetInstance()->savePolygonMesh(pobj, fname, matrix);
		return f;
	}
};


#endif