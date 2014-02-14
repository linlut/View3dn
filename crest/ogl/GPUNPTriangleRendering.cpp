//File: GPUNPTriangleRending.cpp

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <geomath.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>
#include <crest/ogl/GPUNPTriangleRendering.h>
#include <crest/io/nametableitem.h>
#include <crest/system/ztime.h>
#include <crest/component/MLSGpuVolumeMapping.h>
#include <crest/component/BarycentricPolarMapping.h>
#include <zntoolbox.h>
#include <ogldrawall.h>
#include <pntriangle.h>


namespace cia3d{
namespace ogl{

using namespace	cia3d::component;

static unsigned char GPUNPTriangleRenderingClass = 
	cia3d::core::RegisterObject("GPUNPTriangleRendering")
	.add<GPUNPTriangleRendering>()
	.addAlias("GPUNPTriangleRendering");


GPUNPTriangleRendering::GPUNPTriangleRendering(void): 
	GPURendering()
{

}

GPUNPTriangleRendering::~GPUNPTriangleRendering(void)
{
}

void GPUNPTriangleRendering::parse(ZBaseObjectDescription* arg )
{
	//first, construct for parent class
	GPURendering::parse(arg);

	//=========================================
	QDomElement element = arg->toElement();
	QString volumetexturefile = element.attribute("volumetexturefile");
	QString transferfunctionfile = element.attribute("transferfunctionfile");
	
	//=========================================
	//read parameters
	Vector3f texscaling(1,1,1);
	Vector3f textranslation(0,0,0);
	float slicedistance=0.1f;
	typedef cia3d::io::CNameTableItem  NameTableItem;
	NameTableItem table[]={
		NameTableItem("texscaling", 'v', &texscaling), 
		NameTableItem("textranslation", 'v', &textranslation),
		NameTableItem("slicedistance", 'f', &slicedistance)
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	m_texscaling = Vector3f(texscaling.x, texscaling.y, texscaling.z);
	m_textranslation = Vector3f(textranslation.x, textranslation.y, textranslation.z);

	//init
	_init();
}


void GPUNPTriangleRendering::_init(void)
{
	// create edge table texture
	const int tablength = 0; //sizeof(edge_table)/8;
	printf("Lookup Tab length is %d\n\n", tablength);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &edge_tex);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, edge_tex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int i;

	//Init the tetrahedral mesh storage in GPU
	char *_buffer=NULL;
	CPolyObj *pobj = m_pVisualObj;
	//init the vertex buffer
	{
	const unsigned int nsize1 = pobj->m_nVertexCount * sizeof(Vector4f);
	const unsigned int nsize2 = pobj->m_nElementCount * 6 * sizeof(Vector4i);
	unsigned int nsize = _MAX_(nsize1, nsize2);
	nsize = _MAX_(nsize, 1024);
	_buffer = new char[nsize];
	Vector3f *pv = (Vector3f*)_buffer;
	assert(pv!=NULL);
	for (i=0; i<pobj->m_nVertexCount; i++){
		Vector3d q = pobj->m_pVertex[i];
		pv[i] = Vector3f(q.x, q.y, q.z);
	}
	nsize = pobj->m_nVertexCount*sizeof(Vector3f);
	glGenBuffers(1, &m_vboVertex);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboVertex);
	glBufferData(GL_ARRAY_BUFFER, nsize, pv, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//init the texture coordinate VBO
	{
	Vector3f *pv = (Vector3f*)_buffer;
	for (i=0; i<pobj->m_nVertexCount; i++){
		Vector3d q = pobj->m_pVertex[i];
		Vector3f qf(q.x, q.y, q.z);
		qf.x *= m_texscaling.x;		//First scaling,
		qf.y *= m_texscaling.y;		
		qf.z *= m_texscaling.z;
		qf+=m_textranslation;		//Then translation
		pv[i] = qf;
	}
	const int nsize = pobj->m_nVertexCount*sizeof(Vector3f);
	glGenBuffers(1, &m_vboTextureCoord);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboTextureCoord);
	glBufferData(GL_ARRAY_BUFFER, nsize, pv, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//init the rotation quaternion VBO
	{
	Vector4f zero(0.5f,0.5f,0.5f,1);
	Vector4f *pv = (Vector4f*)_buffer;
	for (i=0; i<pobj->m_nVertexCount; i++) pv[i]=zero;
	const int nsize = pobj->m_nVertexCount*sizeof(Vector4f);
	glGenBuffers(1, &m_vboQuaternion);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboQuaternion);
	glBufferData(GL_ARRAY_BUFFER, nsize, pv, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	//free buffer
	delete [] _buffer;
	return;
}


void GPUNPTriangleRendering::_predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw)
{
	//==========FBO=====================================
	pdraw->m_pFboMRT=NULL;
	if (pdraw->m_pFboMRT!=NULL){
		GLuint fbo = pdraw->m_pFboMRT[0];
		GLuint *texbuffers = &pdraw->m_pFboMRT[2];
		ASSERT0(fbo!=0);
	    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	    glDrawBuffersARB(4, texbuffers);
	}
}

void GPUNPTriangleRendering::_postdraw(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	m_gpuProgram.clearTextureParameter(2, "densityVolume");
	m_gpuProgram.clearTextureParameter(2, "gradientVolume");
	m_gpuProgram.clearTextureParameter(2, "transferFunc");	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
}

void GPUNPTriangleRendering::_renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw)
{
	const GLuint PRIMTYPE = GL_LINES_ADJACENCY_EXT;
	//=====================================================
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glDisable(GL_BLEND);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	{//===================================================
	m_gpuProgram.bind();
		_predraw(pobj, pDeformedVertices, pQuat, pdraw);
		const int timerid=7;
		startFastTimer(timerid);
		{//===========BUCKET SORTING===========================
			float dh = 0; //m_slicer.getSliceDistance()*SliceDirection;
			float iso;
			{
				const int nv = pobj->m_nVertexCount;
				const int *pelm = pobj->m_pElement;
				const int nelm = pobj->m_nElementCount;
				const int elmtype = pobj->m_nElmVertexCount;
				int floatsize=4;
				const void *v = pDeformedVertices;
				if (v==NULL){ v=pobj->m_pVertex; floatsize=8; }
			}
			//------------------------------------------------------
			bool r=false;
			m_gpuProgram.setParameter3f(0, "IsoValue", iso, 0, dh);
			int counter = 0;
			const unsigned int OBUFFER_SIZE=20000*4;
			int outbuff[OBUFFER_SIZE];
			//------------------------------------------------------
			do{
				int nout=0;
				counter++;
				//To avoid GPU latency, it's better to use less setParameter calls
				iso += dh;
				if ((nout>0) && (iso>0)){
					if (nout<OBUFFER_SIZE){
						nout *= 4;
						m_gpuProgram.setParameter3f(0, iso, 0, dh);
						glDrawElements(PRIMTYPE, nout, GL_UNSIGNED_INT, outbuff);
					}
					else{
						printf("Tet Buffer too small! Plz resize const OBUFFER_SIZE to %dX4.\n", nout);
						exit(0);
					}
				}
			}while(r);
		}//=================================================================
		//block until all GL execution is complete
		glFinish();	  
		stopFastTimer(timerid);
		reportTimeDifference(timerid, "Tet Render Time:\t");
		_postdraw();
	m_gpuProgram.unbind();	
	}//===================================================
	glDisable(GL_BLEND);
}

inline void GPUNPTriangleRendering::
_drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *_vertexbuff, CGLDrawParms *pdraw)
{
	pMapping->m_pNormal = NULL;  //ROT normal not needed!
	Vector3f *vertexbuff = (Vector3f*)_vertexbuff;
	pMapping->updateMapping(pDeformedVertices, vertexbuff);

	//==============Rendering=========================
	BaseMapping *vmapping = dynamic_cast<MLSVolumeMapping*>(pMapping);
	if (vmapping==NULL) 
		vmapping = dynamic_cast<BarycentricPolarMapping*>(m_pMapping);
	if (vmapping)
		_renderForMLSVolumeMapping(pVisualObj, vertexbuff, vmapping->m_pQuat, pdraw);
	else{
		MLSGpuVolumeMapping *vmapping2 = dynamic_cast<MLSGpuVolumeMapping*>(pMapping);
		if (vmapping2){
		}
	}
}

void GPUNPTriangleRendering::glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw)
{
	if (m_pVisualObj==NULL) return;

	//pdraw->BeginDrawing();
	if (pDeformedVertices){ //If deformation process is running
		if (m_pMapping && m_pVisualObj){//Render visual object
			//deform vertices and normals
			_drawWithMappingVisual(m_pVisualObj, m_pMapping, pDeformedVertices, vertexbuff, pdraw);
		}
		else{
			//ASSERT0(pSimulationObj!=NULL);
			//Vector3d* bakPTR = pSimulationObj->m_pVertex;
			//pSimulationObj->m_pVertex = (Vector3d*)pDeformedVertices;
			//pSimulationObj->checkNormals(true);
			//pSimulationObj->glDraw(*pdraw);			
			//pSimulationObj->m_pVertex = bakPTR;
		}
	}
	else{	//no simulation is running


// npoly: number of polygons
// vpp: vertex number per polygon =  3 or 4


	}
}


void GPUNPTriangleRendering::_glDraw(const Vector3d *pVertex, CGLDrawParms *pdrawparm)
{
	const int BUFFSIZE = 100;
	Vector3f vertices[BUFFSIZE], normals[BUFFSIZE];
	Vector3i tris[BUFFSIZE];

	CPolyObj *pobj = m_pVisualObj;
	const int ntri = pobj->m_nPolygonCount;
	const Vector3i *ptri = (const Vector3i*)pobj->m_pPolygon;
	const Vector3f *pNormal = (const Vector3f*)pobj->m_pVertexNorm;
	for (int i=0; i<ntri; i++){
		Vector3i tri = ptri[i];
		const Vector3d& p1 = pVertex[tri.x];
		const Vector3d& p2 = pVertex[tri.y];
		const Vector3d& p3 = pVertex[tri.z];
		const Vector3f& n1 = pNormal[tri.x];
		const Vector3f& n2 = pNormal[tri.y];
		const Vector3f& n3 = pNormal[tri.z];
		const int nvbase =0;
		int nv, npoly;
		bezierPatchCoefficients(p1, p2, p3, n1, n2, n3, nvbase, vertices, normals, nv, tris, npoly);
		drawSurfaceUsingArray(vertices, normals, &tris[0].x, npoly, 3);
	}

}




} // namespace ogl
} // namespace cia3d
