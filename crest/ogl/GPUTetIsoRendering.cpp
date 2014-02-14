//File: GPUTetRending.cpp

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <geomath.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>
#include <crest/ogl/GPUTetIsoRendering.h>
#include <crest/io/nametableitem.h>
#include <crest/system/ztime.h>
#include <crest/component/MLSGpuVolumeMapping.h>
#include <crest/component/BarycentricPolarMapping.h>
#include <zntoolbox.h>


const int SliceDirection=-1;

namespace cia3d{
namespace ogl{

using namespace	cia3d::component;

static unsigned char GPUTetIsoRenderingClass = 
	cia3d::core::RegisterObject("GPUTetIsoRendering")
	.add<GPUTetIsoRendering>()
	.addAlias("GPUTetIsoRendering");


GPUTetIsoRendering::GPUTetIsoRendering(void): 
	GPURendering(), 
	m_slicer(), 
	m_transferfunc()
{
	edge_tex = 0;
	m_vboTextureCoord = 0;
}

GPUTetIsoRendering::~GPUTetIsoRendering(void)
{
	SafeDeleteVBO(m_vboTextureCoord);
}

void GPUTetIsoRendering::setSlicingDistance(const float slicedistance)
{
	m_slicer.setSliceDistance(slicedistance);
	m_gpuProgram.setParameter1f("StepLength", SliceDirection*slicedistance);
}

void GPUTetIsoRendering::setTransferFunction(const QString& transferfunctionfile, const int itype)
{
	if (!transferfunctionfile.isEmpty())
		m_transferfunc.load(transferfunctionfile);
}

void GPUTetIsoRendering::setTransferFunction(const float minval, const float maxval, const int itype)
{
	const Vector3i clr(1,0,0);
	switch(itype){
	case 0:
		m_transferfunc.setSpecialTransferFuncA();
		break;
	case 1:
		m_transferfunc.setSpecialTransferFuncB(minval, maxval);
		break;
	case 100:
		m_transferfunc.setIsosurfaceTransferFunc(0.499f, clr);
		break;
	default:
		break;
	};
}

void GPUTetIsoRendering::parse(ZBaseObjectDescription* arg )
{
	//first, construct for parent class
	GPURendering::parse(arg);

	//=========================================
	QDomElement element = arg->toElement();
	QString transferfunctionfile = element.attribute("transferfunctionfile");	
	setTransferFunction(transferfunctionfile, 0);

	//=========================================
	//read parameters
	float slicedistance=0.1f;
	typedef cia3d::io::CNameTableItem  NameTableItem;
	NameTableItem table[]={
		NameTableItem("slicedistance", 'f', &slicedistance)
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	//apply the slice distance
	setSlicingDistance(slicedistance);

	//init
	_init();
}


void GPUTetIsoRendering::defaultInit(CPolyObj *pVisualObj)
{
	m_pVisualObj = pVisualObj;

	//first, init gpu programs
	const char *vp="C:/users/nanzhang/project/crest/ogl/Shaders/vert5.cg";
	const char *vpmain="main";
	const char *gp="C:/users/nanzhang/project/crest/ogl/Shaders/isosurf5.cg";
	const char *gpmain="main";
	const char *fp="C:/users/nanzhang/project/crest/ogl/Shaders/fragment5.cg";
	const char *fpmain="main";
	setGPUPrograms(vp, vpmain, gp, gpmain, fp, fpmain);

	AxisAlignedBox box;
	pVisualObj->GetBoundingBox(box);
	Vector3d dist = box.maxp - box.minp;
	const float slicedistance = (dist.x+dist.y+dist.z)/3/300;
	setSlicingDistance(slicedistance);
	
	//QString transferfunctionfile="C:/users/nanzhang/research/paper/voldef/voldef-tf-view/neghip1.tf";
	//this->setTransferFunction(transferfunctionfile, 0);
	const bool useNewRange = true;
	const int attribIdx = 0;
	const int mappingType = 0;
	float minval, maxval;
	pVisualObj->ComputeVertexTextureCoordiates(useNewRange, attribIdx, mappingType, minval, maxval);
	this->setTransferFunction(minval, maxval, 1);

	//init
	_init();
}


const unsigned char edge_table[] = {
	0, 0, 0, 0,
	3, 0, 3, 1,
	2, 1, 2, 0,
	2, 0, 3, 0,
	1, 2, 1, 3,
	1, 0, 1, 2,
	1, 0, 2, 0,
	3, 0, 1, 0,
	0, 2, 0, 1,
	0, 1, 3, 1,
	0, 1, 0, 3,
	3, 1, 2, 1,
	0, 2, 1, 2,
	1, 2, 3, 2,
	0, 3, 2, 3,
	0, 0, 0, 0,

	0, 0, 0, 1,
	3, 2, 0, 0,
	2, 3, 0, 0,
	2, 1, 3, 1,
	1, 0, 0, 0,
	3, 0, 3, 2,
	1, 3, 2, 3,
	2, 0, 0, 0,
	0, 3, 0, 0,
	0, 2, 3, 2,
	2, 1, 2, 3,
	0, 1, 0, 0,
	0, 3, 1, 3,
	0, 2, 0, 0,
	1, 3, 0, 0,
	0, 0, 0, 0, 
};

void GPUTetIsoRendering::_init(void)
{
	int i;

	// create edge table texture
	const int tablength = sizeof(edge_table)/8;
	printf("Lookup Tab length is %d\n\n", tablength);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &edge_tex);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, edge_tex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, tablength, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, edge_table);

	//load transferfunc texture into GPU
	m_transferfunc.installTransferFuncOnGPU();

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
		if (pobj->m_pVertexTexCoor1D==NULL){
			const bool useNewRange = true;
			const int attribIdx = 0;
			const int mappingType = 0;
			float minval=0, maxval=1;
			pobj->ComputeVertexTextureCoordiates(useNewRange, attribIdx, mappingType, minval, maxval);
		}
		float *ptexcoord = pobj->m_pVertexTexCoor1D;
		if (ptexcoord==NULL)
			printf("Error: object has no texture coord!\n");
		const int nsize = pobj->m_nVertexCount*sizeof(float);
		glGenBuffers(1, &m_vboTextureCoord);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboTextureCoord);
		glBufferData(GL_ARRAY_BUFFER, nsize, ptexcoord, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//free buffer
	delete [] _buffer;
	return;
}

inline void updateVertexVBO(const Vector3f *v, const int nv)
{
	const int nsize = sizeof(Vector3f)*nv;
	glBufferSubData(GL_ARRAY_BUFFER, 0, nsize, v);
}

void GPUTetIsoRendering::
_predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const float* pTexcoord, CGLDrawParms *pdraw)
{
	{
		//draw all the elements using a bucket order from the view position
		const float posx= pdraw->m_ViewPos.x, posy=pdraw->m_ViewPos.y, posz=pdraw->m_ViewPos.z;
		m_gpuProgram.setParameter3f(0, "viewpos", posx, posy, posz);
		m_gpuProgram.setParameter3f(2, "viewpos", posx, posy, posz);
		float dirx = pdraw->m_ViewDir.x, diry = pdraw->m_ViewDir.y, dirz=pdraw->m_ViewDir.z;
		m_gpuProgram.setParameter3f(0, "viewdir", dirx, diry, dirz);
		const GLuint transferFuncID = m_transferfunc.getTransferFuncTextureID();
		m_gpuProgram.setTextureParameter(2, "transferFunc", transferFuncID);
		//texture coord vbo, fixed
		glBindBuffer(GL_ARRAY_BUFFER, m_vboTextureCoord);
		glTexCoordPointer(1, GL_FLOAT, 0, NULL);	
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//deformed vertex positions, vary
		glBindBuffer(GL_ARRAY_BUFFER, m_vboVertex);
		if (pDeformedVertices) updateVertexVBO(pDeformedVertices, pobj->m_nVertexCount);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		glEnableClientState(GL_VERTEX_ARRAY);
		//Element buffer is NULL, because we need to input the sorted sequence
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, edge_tex);
	}

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

void GPUTetIsoRendering::_postdraw(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	m_gpuProgram.clearTextureParameter(2, "transferFunc");	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
}

void GPUTetIsoRendering::
_renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const float* pTexcoord, CGLDrawParms *pdraw)
{
	const GLuint PRIMTYPE = GL_LINES_ADJACENCY_EXT;
	//=====================================================
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glDisable(GL_BLEND);
	if (SliceDirection>0){ //front-to-back slicing 
		//see http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch39.html
		//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		//glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_DST_ALPHA, GL_ONE);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA); ---
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	{//===================================================
	m_gpuProgram.bind();
		_predraw(pobj, pDeformedVertices, pTexcoord, pdraw);
		const int timerid=7;
		startFastTimer(timerid);
		{//===========BUCKET SORTING===========================
			m_slicer.setCamera(pdraw->m_ViewPos, pdraw->m_ViewDir);
			float dh = m_slicer.getSliceDistance()*SliceDirection;
			float iso;
			{
				const int nv = pobj->m_nVertexCount;
				const int *pelm = pobj->m_pElement;
				const int nelm = pobj->m_nElementCount;
				const int elmtype = pobj->m_nElmVertexCount;
				int floatsize=4;
				const void *v = pDeformedVertices;
				if (v==NULL){ v=pobj->m_pVertex; floatsize=8; }
				iso= m_slicer.bucketSort(v, floatsize, nv, pelm, nelm, elmtype, SliceDirection);
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
				r=m_slicer.bucketOneStep(pobj->m_pElement, pobj->m_nElmVertexCount, outbuff, nout); 
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

inline void GPUTetIsoRendering::
_drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *_vertexbuff, CGLDrawParms *pdraw)
{
	//==============Deformation=======================
	pMapping->m_pNormal = NULL;  //ROT normal not needed!
	Vector3f *vertexbuff = (Vector3f*)_vertexbuff;
	pMapping->updateMapping(pDeformedVertices, vertexbuff);

	//==============Rendering=========================
	BaseMapping *vmapping = dynamic_cast<MLSVolumeMapping*>(pMapping);
	if (vmapping==NULL) 
		vmapping = dynamic_cast<BarycentricPolarMapping*>(m_pMapping);
	if (vmapping)
		_renderForMLSVolumeMapping(pVisualObj, vertexbuff, NULL, pdraw);
	else{
		MLSGpuVolumeMapping *vmapping2 = dynamic_cast<MLSGpuVolumeMapping*>(pMapping);
		if (vmapping2){
		}
	}
}

void GPUTetIsoRendering::
glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw)
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
		//m_pVisualObj->glDraw(*pdraw);
		BaseMapping *vmapping = dynamic_cast<MLSVolumeMapping*>(m_pMapping);
		if (vmapping==NULL)
			vmapping = dynamic_cast<BarycentricPolarMapping*>(m_pMapping);
		if (vmapping){
			_renderForMLSVolumeMapping(m_pVisualObj, NULL, NULL, pdraw);
		}
		else{
			MLSGpuVolumeMapping *vmapping2 = dynamic_cast<MLSGpuVolumeMapping*>(m_pMapping);
			if (vmapping2){
			}
		}
	}
	//pdraw->EndDrawing();
}


//for rendering objects in View3d (visualize FEM simulation results)
inline void GPUTetIsoRendering::
glSimpleDraw(const Vector3f *pVertices, const float *pTexcoord, CGLDrawParms *pdrawparm)
{
	if (m_pVisualObj==NULL) return;
	_renderForMLSVolumeMapping(m_pVisualObj, pVertices, pTexcoord, pdrawparm);

}


inline GPUTetIsoRendering* GPUTetIsoRendering::getInstance(CPolyObj *pobj)
{
	GPUTetIsoRendering *pIsoRender = NULL;
	if (pobj->m_pElement==NULL) 
		return NULL;
	if (pobj->m_nElmVertexCount<4)
		return NULL;

	//If the render is empty, allocate a new render
	if (pIsoRender==NULL){
		pIsoRender = new GPUTetIsoRendering();
		if (pIsoRender){
			//init the renderer
			pIsoRender->defaultInit(pobj);
		}
	}
	return pIsoRender;
}


} // namespace ogl
} // namespace cia3d

using namespace cia3d::ogl;

void runGPUTetIsoRenderxx(CPolyObj *pobj, CGLDrawParms *pdrawparm)
{
	GPUTetIsoRendering* pIsoRender = GPUTetIsoRendering::getInstance(pobj);
	if (pIsoRender == NULL)
		return;

	//If the render is already there, check the object information, is it consistant with the obj?
	//NOT_IMPLEMENTED();
	//return NULL;

	const Vector3f *pVertices = NULL;
	const float *pTexcoord = pobj->m_pVertexTexCoor1D;
	pIsoRender->glSimpleDraw(pVertices, pTexcoord, pdrawparm);
}


typedef cia3d::ogl::GPUTetIsoRendering RENDER_t;

CObjectBase* runGPUTetIsoRender(CObjectBase *pRenderingObj, CPolyObj *pobj, CGLDrawParms *pdrawparm)
{
	GPUTetIsoRendering* pIsoRender=NULL;

	if (pRenderingObj!=NULL){
		pIsoRender = dynamic_cast<RENDER_t*>(pRenderingObj);
		if (pIsoRender==NULL) 
			return NULL;
	}
	else{
		pIsoRender = GPUTetIsoRendering::getInstance(pobj);
		if (pIsoRender == NULL)
			return NULL;
	}

	const Vector3f *pVertices = NULL;
	const float *pTexcoord = pobj->m_pVertexTexCoor1D;
	pIsoRender->glSimpleDraw(pVertices, pTexcoord, pdrawparm);
	return pIsoRender;
}

