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
#include <crest/ogl/GPUTetRendering.h>
#include <crest/io/nametableitem.h>
#include <crest/system/ztime.h>
#include <crest/component/MLSGpuVolumeMapping.h>
#include <crest/component/BarycentricPolarMapping.h>
#include <zntoolbox.h>

const int SliceDirection=-1;

/*
  Why does my FBO app not write to the 2nd render target?

The FBO extension makes it really easy and intuitive to write applications that write to 
multiple render targets. Unfortunately, FBO makes MRT so easy that people frequently forget 
that MRT must be explicitly enabled; simply attaching multiple color attachments to the 
FBO is not sufficient by itself.

A proper example combining FBO and MRT would be something like this:

Let's assume you have textures attached to GL_COLOR_ATTACHMENT0_EXT and GL_COLOR_ATTACHMENT1_EXT:

// Set up tex0 and tex1 for render-to-texture
glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex0, 0);
glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, tex1, 0);

Then you just call glDrawBuffers(), passing in the color buffer names. Something like:

GLenum buffers[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
glDrawBuffers(2, buffers);

Then in your shader, just output to the first two color outputs (in Cg, those would be the ones 
bound to the COLOR0 and COLOR1 semantics).

Note that the COLOR0 and COLOR1 Cg semantics correspond to the first and second enumerants that 
were passed in to glDrawBuffers, respectively. In this example, it just so happens that 
COLOR0 => GL_COLOR_ATTACHMENT0_EXT and COLOR1 => GL_COLOR_ATTACHMENT1_EXT. But the correspondence is 
by way of the order in buffers rather than by any notion of COLOR0 always corresponding to 
GL_COLOR_ATTACHMENT0_EXT and so on, which is not the case. 
*/
/*
How do I write to more than one render target at once?

You can accomplish this by calling glDrawBuffers(), which is a new function in OpenGL 2.0. It used 
to be called glDrawBuffersATI(), which was introduced by the ATI_draw_buffers OpenGL extension. This 
is called MRT (Multiple Render Targets), and is supported by ATI 9x00 and X8xx (and newer) hardware 
and NV4x (and newer) NVIDIA hardware.

Similar functionality is available in Direct3D9 using SetRenderTarget to set any of the MRTs. 
*/

/*
oid main(int argc, char** argv)
{
   initGLUT(argc, argv);
   InitGL();
   initGlew();

   //create Textures
   GLuint tex1, tex2;

   //texture settings
   GLenum texTarget = GL_TEXTURE_2D;
   GLenum texInternalFormat = 3;
   GLenum texFormat = GL_RGB;
   
   //texture1
   glGenTextures(1, &tex1);   
   glBindTexture(texTarget,tex1);
    // turn off filtering and wrap modes
    glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexImage2D(texTarget,   0, texInternalFormat, 512, 512, 0, texFormat, GL_UNSIGNED_BYTE,   0);
   if (glGetError() != GL_NO_ERROR)
      printf("error creating tex1\n");

   //texture2
   glGenTextures(1, &tex2);
   glBindTexture(texTarget, tex2);
    // turn off filtering and wrap modes
    glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexImage2D(texTarget, 0, texInternalFormat, 512, 512,   0, texFormat, GL_UNSIGNED_BYTE,   0);
   if (glGetError() != GL_NO_ERROR)
      printf("error creating tex2\n");

   //framebuffer
   GLuint fb;
   glGenFramebuffersEXT(1, &fb);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
   
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texTarget, tex1, 0);
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, texTarget, tex2, 0);

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
   GLenum drawbuffers[] = {   GL_COLOR_ATTACHMENT0_EXT,
                        GL_COLOR_ATTACHMENT1_EXT};
   glDrawBuffers(2, drawbuffers);
   CheckFramebufferStatus();  // returns: framebuffer COMPLETE

   //setup CG
   cgSetErrorCallback(cgErrorCallback);
   cgContext = cgCreateContext();

   fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
   if (fragmentProfile == CG_PROFILE_UNKNOWN)
      printf("error loading fragmentprofile\n");

   cgGLSetOptimalOptions(fragmentProfile);

   //Load Shader
   CGprogram fragmentProgram;
   fragmentProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "CGShaders/ShaderMRTTest.cg", fragmentProfile, "pixel_main", 0);
   cgGLLoadProgram(fragmentProgram);

}
*/


namespace cia3d{
namespace ogl{

using namespace	cia3d::component;

static unsigned char GPUTetRenderingClass = 
	cia3d::core::RegisterObject("GPUTetRendering")
	.add<GPUTetRendering>()
	.addAlias("GPUTetRendering");


GPUTetRendering::GPUTetRendering(void): 
	GPURendering(), 
	m_slicer(), 
	m_volume(),
	m_transferfunc()
{
	edge_tex = 0;
	m_vboTextureCoord = 0;
	m_vboQuaternion = 0;
	m_texscaling = Vector3f(1,1,1); 
	m_textranslation = Vector3f(0,0,0);
	glTexImage3D = NULL;
	//=============
	m_flipTexX = false;
	m_flipTexY = false;
	m_flipTexZ = false;

#ifdef WIN32
	glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
	if (glTexImage3D == NULL){ 
		printf("Error in line %d: Couldn't load glTexImage3D function. Aborting.\n", __LINE__, __DATE__);
		exit(0);
	}
#else
	perror("glTexImage3D:Not implemented!\n");
	assert(0);
#endif
}

GPUTetRendering::~GPUTetRendering(void)
{
	SafeDeleteVBO(m_vboTextureCoord);
	SafeDeleteVBO(m_vboQuaternion);
}

void GPUTetRendering::parse(ZBaseObjectDescription* arg )
{
	//first, construct for parent class
	GPURendering::parse(arg);

	//=========================================
	QDomElement element = arg->toElement();
	QString volumetexturefile = element.attribute("volumetexturefile");
	QString transferfunctionfile = element.attribute("transferfunctionfile");
	if (!volumetexturefile.isEmpty())
		m_volume.load(volumetexturefile);
	
	if (!transferfunctionfile.isEmpty())
		m_transferfunc.load(transferfunctionfile);
	//=========================================
	//read parameters
	Vector3f texscaling(1,1,1);
	if (!m_volume.isEmpty()){
		const float K=1/1000.0f;
		float nx = m_volume.m_nx*K;
		float ny = m_volume.m_ny*K;
		float nz = m_volume.m_nz*K;
		texscaling = Vector3f(1/nx, 1/ny, 1/nz);
	}
	Vector3f textranslation(0,0,0);
	float slicedistance=0.1f;
	typedef cia3d::io::CNameTableItem  NameTableItem;
	NameTableItem table[]={
		NameTableItem("texscaling", 'v', &texscaling), 
		NameTableItem("textranslation", 'v', &textranslation),
		NameTableItem("slicedistance", 'f', &slicedistance)
		//NameTableItem("fliptexx", 'b', &m_flipTexX),
		//NameTableItem("fliptexy", 'b', &m_flipTexY),
		//NameTableItem("fliptexz", 'b', &m_flipTexZ),
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	m_texscaling = Vector3f(texscaling.x, texscaling.y, texscaling.z);
	m_textranslation = Vector3f(textranslation.x, textranslation.y, textranslation.z);
	m_slicer.setSliceDistance(slicedistance);
	m_gpuProgram.setParameter1f("StepLength", SliceDirection*slicedistance);

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
//-------------------------------------------------
	/*
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
	0, 0, 0, 0
	*/
};

void GPUTetRendering::_init(void)
{
	// create edge table texture
	const int tablength = sizeof(edge_table)/8;
	printf("Lookup Tab length is %d\n\n", tablength);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &edge_tex);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, edge_tex);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8, tablength, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, edge_table);

	int i;
	//load volume texture into GPU
	if (!m_volume.isEmpty()){
		m_volume.computeGradient();
		m_volume.installDensityVolumeOnGPU();
		m_volume.installGradientVolumeOnGPU();
		m_transferfunc.installTransferFuncOnGPU();
	}

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
		if (m_flipTexX) qf.x = 1.0f - qf.x;
		if (m_flipTexY) qf.y = 1.0f - qf.y;
		if (m_flipTexZ) qf.z = 1.0f - qf.z;
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

	/*
		//init element VBO
		Vector4i *pelm = (Vector4i *)pobj->m_pElement;
		nsize = pobj->m_nElementCount*sizeof(Vector4i);
		if (pobj->m_nElmVertexCount==8){ //Hexa. mesh, we need to do something to convert into tet mesh
			nsize*=6;
			pelm = (Vector4i *)_buffer;
			Vector8i *phex = (Vector8i*)pobj->m_pElement;
			for (i=0; i<pobj->m_nElementCount; i++)
				hexConnectivityToTetConnectivity(phex[i], &pelm[i*6]);
		}
		glGenBuffers(1, &m_vboElement);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboElement);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nsize, pelm, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	*/

	//free buffer
	delete [] _buffer;
	return;
}

inline void drawQuad(const float z) {
	float x=0.5f;
	float y=0.5f;

	glBegin(GL_QUADS);
	glVertex3f(-x, -y, z);
	glVertex3f(x, -y, z);
	glVertex3f(x, y, z);
	glVertex3f(-x, y, z);
	glEnd();
}

void GPUTetRendering::_test0(CPolyObj *pobj, const Vector3d *pDeformedVertices, CGLDrawParms *pdraw)
{
	/*
	const GLuint PRIMTYPE = GL_LINES_ADJACENCY_EXT;
	int nprims = m_pVisualObj->m_nElementCount*4;
	if (m_pVisualObj->m_nElmVertexCount==8) nprims*=6;

	//=====================================================
	CGLDrawParms::glDrawMeshStyle _tmp = pdraw->m_nDrawType;
	pdraw->m_nDrawType = CGLDrawParms::DRAW_MESH_LINE;
	//CGLDrawParms::glDrawVertexStyle _tmp2 = pdraw->m_nDrawVertexStyle;
	//pdraw->m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_POINT;
	//pdraw->m_fVertexSizeScale=10;
	//pobj->glDraw(*pdraw);
	{//================POINTS============================
	//setup vertex array from VBO
	glColor3f(1,0,0);
	glPointSize(6);
	glLineWidth(4);
	glNormal3f(1,1,1);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboVertex);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		//setup element array from VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboElement);
		//draw
		glDrawElements(GL_POINTS, nprims, GL_UNSIGNED_INT, 0);
		//clear
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
	}//===================================================

	m_gpuProgram.bind();
	{//===================================================
		//setup vertex array from VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_vboVertex);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		//setup element array from VBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboElement);
		//draw
		float posx= pdraw->m_ViewPos.x, posy=pdraw->m_ViewPos.y, posz=pdraw->m_ViewPos.z;
		m_gpuProgram.setParameter3f(0, "viewpos", posx, posy, posz);
		float dirx = pdraw->m_ViewDir.x, diry = pdraw->m_ViewDir.y, dirz=pdraw->m_ViewDir.z;
		m_gpuProgram.setParameter3f(0, "viewdir", dirx, diry, dirz);
		float iso = 0.0f;
		for (int i=0; i<200; i++, iso+=0.1f){
			m_gpuProgram.setParameter1f("IsoValue", iso);
			glDrawElements(PRIMTYPE, nprims, GL_UNSIGNED_INT, 0);
		}
		//clear
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
	}//===================================================
	m_gpuProgram.unbind();
*/
}


inline void updateVertexVBO(const Vector3f *v, const int nv)
{
	const int nsize = sizeof(Vector3f)*nv;
	glBufferSubData(GL_ARRAY_BUFFER, 0, nsize, v);
}

inline void updateQuaternionVBO(const Vector4f *v, const int nv)
{
	const int nsize = sizeof(Vector4f)*nv;
	glBufferSubData(GL_ARRAY_BUFFER, 0, nsize, v);
}

void GPUTetRendering::_predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw)
{
	{
		//draw all the elements using a bucket order from the view position
		const float posx= pdraw->m_ViewPos.x, posy=pdraw->m_ViewPos.y, posz=pdraw->m_ViewPos.z;
		m_gpuProgram.setParameter3f(0, "viewpos", posx, posy, posz);
		m_gpuProgram.setParameter3f(2, "viewpos", posx, posy, posz);
		const Vector3f lightpos = Vector3f(1,1,1)*0.005f + pdraw->m_ViewPos;
		m_gpuProgram.setParameter3f(2, "lightpos", lightpos.x, lightpos.y, lightpos.z);
		float dirx = pdraw->m_ViewDir.x, diry = pdraw->m_ViewDir.y, dirz=pdraw->m_ViewDir.z;
		m_gpuProgram.setParameter3f(0, "viewdir", dirx, diry, dirz);
		const GLuint denVolumeID = m_volume.getDensityVolumeTextureID();
		const GLuint gradVolumeID = m_volume.getGradientVolumeTextureID();
		m_gpuProgram.setTextureParameter(2, "densityVolume", denVolumeID);
		m_gpuProgram.setTextureParameter(2, "gradientVolume", gradVolumeID);
		const GLuint transferFuncID = m_transferfunc.getTransferFuncTextureID();
		m_gpuProgram.setTextureParameter(2, "transferFunc", transferFuncID);
		//rotation quaternion vbo, vary
		glBindBuffer(GL_ARRAY_BUFFER, m_vboQuaternion);
		if (pQuat) updateQuaternionVBO(pQuat, pobj->m_nVertexCount);
		glColorPointer(4, GL_FLOAT, 0, NULL);	
		glEnableClientState(GL_COLOR_ARRAY);
		//texture coord vbo, fixed
		glBindBuffer(GL_ARRAY_BUFFER, m_vboTextureCoord);
		glTexCoordPointer(3, GL_FLOAT, 0, NULL);	
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

void GPUTetRendering::_postdraw(void)
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

void GPUTetRendering::_renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw)
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
		_predraw(pobj, pDeformedVertices, pQuat, pdraw);
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

inline void GPUTetRendering::
_drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *_vertexbuff, CGLDrawParms *pdraw)
{
	//==============Deformation=======================
	/*
	static vector<Vector3f> defNormal;
	const bool bDefNormal = pMapping->canDeformNormal();
	assert(bDefNormal==true);
	if (bDefNormal){
		const unsigned int nv = pVisualObj->m_nVertexCount;
		if (nv>defNormal.size())
			defNormal.resize(nv);
		pMapping->m_pNormal = &defNormal[0];
	}
	*/
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

void GPUTetRendering::glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw)
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


} // namespace ogl
} // namespace cia3d
