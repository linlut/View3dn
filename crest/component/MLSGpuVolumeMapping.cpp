//File: MLSGpuVolumeMapping.cpp

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/component/MLSGpuVolumeMapping.h>
#include <crest/system/ztime.h>
#include <crest/GPU/Cuda/cuda0.h>

namespace cia3d{
namespace component{

static int MLSGpuVolumeMappingClass = 
cia3d::core::RegisterObject("MLSGpuVolumeMapping")
.add<MLSGpuVolumeMapping>()
.addAlias("MLSGpuVolumeMapping")
;

#define SafeDeleteBuffer(x) if (*(x)>0){glDeleteBuffers(1, (x)), *(x)=0;}
MLSGpuVolumeMapping::~MLSGpuVolumeMapping()
{
	SafeDeleteBuffer(&vboRefVertexArray);
	SafeDeleteBuffer(&vboDefRefVertexArray);
	//cudaFreeDeviceArray(m_pDeviceDefRefVertex);

	SafeDeleteBuffer(&vboNeighborArray);
	SafeDeleteBuffer(&vboVertexArray);
	SafeDeleteBuffer(&vboDefVertexArray);
	SafeDeleteBuffer(&vboQuatArray);
	SafeDeleteBuffer(&vboNormArray);
	SafeDeleteBuffer(&vboDefNormArray);
}

static void createVBO(GLuint* vbo, const unsigned int sizeByte, const void* data=NULL, const GLenum usage=GL_DYNAMIC_DRAW)
{
    // create buffer object
    glGenBuffers( 1, vbo);
    glBindBuffer( GL_ARRAY_BUFFER, *vbo);

    // initialize buffer object
    glBufferData( GL_ARRAY_BUFFER, sizeByte, data, usage);
    glBindBuffer( GL_ARRAY_BUFFER, 0);

    // register buffer object with CUDA
	gpuCudaGLRegisterBufferObject(*vbo);
}


//source obj: the simulation object
//Destination obj: the visual object
void MLSGpuVolumeMapping::init(CPolyObj *src, CPolyObj *dst)
{
	//init GPU vertex buffers and CUDA
	const int shiftbit = ARRAY_LEN_SHIFTBIT;
	const int padding = ARRAY_LEN_PADDING;
	int i;
	const int NSRC = ((src->m_nVertexCount + 15)>>4)<<4;
	const int NDST = ((dst->m_nVertexCount + padding)>>shiftbit)<<shiftbit;
	unsigned int sizeByte=_MAX_(sizeof(Vector3f),sizeof(Vector4i))*_MAX_(NSRC, NDST) + 1024;
	char *buffer = new char[sizeByte];
	assert(buffer!=NULL);
	Vector3f *p3 = (Vector3f*) buffer;

	//init reference array
	for (i=0; i<src->m_nVertexCount; i++)
		p3[i]=Vector3f(src->m_pVertex[i].x, src->m_pVertex[i].y, src->m_pVertex[i].z);
	sizeByte = NSRC * sizeof(Vector3f);
	createVBO(&vboRefVertexArray, sizeByte, p3, GL_STATIC_DRAW);
	//cudaAllocateDeviceArray((void **)(&m_pDeviceDefRefVertex), sizeByte);
	createVBO(&vboDefRefVertexArray, sizeByte, NULL, GL_STATIC_DRAW);

	//init connectivity array
	Vector4i *pConn = (Vector4i*)buffer;
	for (i=0; i<dst->m_nVertexCount; i++){
		int *x = m_RefVertex[i].refVertexID;
		pConn[i]=Vector4i(x[0], x[1], x[2], x[3]);
	}
	for (i=dst->m_nVertexCount; i<NDST; i++){
		int *x = m_RefVertex[0].refVertexID;
		pConn[i]=Vector4i(x[0], x[1], x[2], x[3]);
	}
	sizeByte = NDST * sizeof(Vector4i);
	createVBO(&vboNeighborArray, sizeByte, pConn, GL_DYNAMIC_DRAW);

	//init the vertex array
	for (i=0; i<dst->m_nVertexCount; i++)
		p3[i]=Vector3f(dst->m_pVertex[i].x, dst->m_pVertex[i].y, dst->m_pVertex[i].z);
	sizeByte = NDST * sizeof(Vector3f);
	createVBO(&vboVertexArray, sizeByte, p3, GL_STATIC_DRAW);
	createVBO(&vboDefVertexArray, sizeByte, NULL, GL_DYNAMIC_DRAW);

	sizeByte = NDST * sizeof(Vector4f);
	createVBO(&vboQuatArray, sizeByte, NULL, GL_DYNAMIC_DRAW);

	//clear CPU buffer
	m_RefVertex.clear();
	m_RefVertex.resize(0);
	delete [] buffer;
}


inline void 
_updateBuffer(const CPolyObj *m_pSrcObj, const CPolyObj *m_pDstObj, const GLuint vboDefRefVertexArray, const Vector3d *input, Vector3d *output)
{
	int i;
	const int n0 = m_pSrcObj->m_nVertexCount;
	const int n1 = n0 * sizeof(Vector3d);
	const int n2 = m_pDstObj->m_nVertexCount * sizeof(Vector3f);
	Vector3f *pbuffer = NULL;
	if (n1>n2) pbuffer = new Vector3f[n0];
	else pbuffer = (Vector3f*)output;

	//Update the deformation reference array
	const Vector3d *src = input;
	Vector3f *pDefRefVert = pbuffer;
	for (i=0; i<n0; i++, src++, pDefRefVert++)
		*pDefRefVert = Vector3f((float)src->x, (float)src->y, (float)src->z);
    glBindBuffer(GL_ARRAY_BUFFER, vboDefRefVertexArray);
	glBufferSubData(GL_ARRAY_BUFFER, 0, n0*sizeof(Vector3f), pbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	/*{
	for (i=0; i<n0; i++){
		const Vector3d *p = &input[i];
		pbuffer[i]=Vector3f(p->x, p->y, p->z);
	}
	cudaCopyArrayToDevice(m_pDeviceDefRefVertex, pbuffer, 0,  n0*sizeof(Vector3f));
	}*/
	if (n1>n2) delete [] pbuffer;
}


//GPU implementation of the MLS alg., the returns are in the graphics card
void MLSGpuVolumeMapping::updateMapping(const Vector3d* input, Vector3d* output)
{
	//update VBO
	_updateBuffer(m_pSrcObj, m_pDstObj, vboDefRefVertexArray, input, output);

	//Perform GPU based deformation
	const int timerid=4;
	startFastTimer(timerid);
	const int nv = m_pDstObj->m_nVertexCount;
	runGpuMlsDeformation(
		nv,						//length of the problem (here is the vertex length)
		vboRefVertexArray,		//static vertex array of the reference model, float3
		vboDefRefVertexArray,    //deformed vertex array of the reference model, float3
		//&m_pDeviceDefRefVertex[0].x,
		vboNeighborArray,		//neighbourhood array, or connectivity, int4
		vboVertexArray,		    //static vertex array of the visual model, float3
		vboDefVertexArray,		//deformed vertex array of the visual model, float3
		vboQuatArray);		    //rotation quaternion array, float4
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "GPU MLS time is");

	//Copy buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboDefVertexArray);
	Vector3f *pDefVert = (Vector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (pDefVert==NULL) return;
	for (int i=0; i<m_pDstObj->m_nVertexCount; i++, pDefVert++){
		output[i] = Vector3d(pDefVert->x, pDefVert->y, pDefVert->z);
		//if (i<10) printf("%d: %lg %lg %lg\n", i, output[i].x, output[i].y, output[i].z);
	}
	//printf("\n\n");
	glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	return;
}

} // namespace component
} // namespace cia3d

