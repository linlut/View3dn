//CUDA interface file

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

// includes, system
#include <stdlib.h>
#include <stdio.h>

// includes, GL
#include <GL/glew.h>
#include <GL/glut.h>

// includes
#include <cuda.h>
#include <cutil.h>
#include <cudaGL.h>
#include <cutil_math.h>
#include <cutil_gl_error.h>
#include <cuda_gl_interop.h>
#include <crest/system/ztime.h>
#include "cuda0.h"


void cudaCheckErrorGL(void)
{
    CUT_CHECK_ERROR_GL();
}

void gpuCudaGLRegisterBufferObject(GLuint vbo)
{
    // register buffer object with CUDA
    CUDA_SAFE_CALL(cudaGLRegisterBufferObject(vbo));    
}

void gpuCudaGLUnregisterBufferObject(GLuint vbo)
{
    CUDA_SAFE_CALL(cudaGLUnregisterBufferObject(vbo));
}

void cudaDeviceInit(void)
{
	CUT_DEVICE_INIT();	
	
	//Interoperability with OpenGL initialized
	cuGLInit();
}

void glewDeviceInit(void)
{
    glewInit();    
    if (!glewIsSupported( "GL_VERSION_2_0 " 
        "GL_ARB_pixel_buffer_object")) {
        fprintf( stderr, "ERROR: Support for necessary OpenGL extensions missing.\n");
        fflush( stderr);
    }	
    
    if (!glewIsSupported(
        "GL_VERSION_2_0 "
		"GL_ARB_vertex_program "
		"GL_ARB_fragment_program "
        "GL_ARB_texture_float "
		"GL_NV_gpu_program4 " // include GL_NV_geometry_program4
        "GL_ARB_texture_rectangle "
		))
    {
        printf("Unable to load extension()s:\n  GL_ARB_vertex_program\n  GL_ARB_fragment_program\n"
               "  GL_ARB_texture_float\n  GL_NV_gpu_program4\n  GL_ARB_texture_rectangle\n  OpenGL Version 2.0\nExiting...\n");
        exit(-1);
    }
}

void cudaDeviceClose(const int argc, char **argv)
{
    CUT_EXIT(argc, argv);
}

void cudaAllocateDeviceArray(void **devPtr, size_t size)
{
    CUDA_SAFE_CALL(cudaMalloc(devPtr, size));
}

void cudaFreeDeviceArray(void *devPtr)
{
    CUDA_SAFE_CALL(cudaFree(devPtr));
}

void cudaCopyArrayFromDevice(void* host, const void* device, unsigned int vbo, int size)
{   
    if (vbo)
        CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&device, vbo));
    CUDA_SAFE_CALL(cudaMemcpy(host, device, size, cudaMemcpyDeviceToHost));
    if (vbo)
        CUDA_SAFE_CALL(cudaGLUnmapBufferObject(vbo));
}

void cudaCopyArrayToDevice(void* device, const void* host, int offset, int size)
{
    CUDA_SAFE_CALL(cudaMemcpy((char *) device + offset, host, size, cudaMemcpyHostToDevice));
}


//======================GPU code=============================================

#define QFLOAT float
typedef struct
{
	float x[9];
} matrix3x3;
	
__device__ inline 
float3 mat_float3_mult(const matrix3x3 rot, const float3 t)
{
	float3 r;
	r.x = rot.x[0]*t.x + rot.x[1]*t.y + rot.x[2]*t.z;
	r.y = rot.x[3]*t.x + rot.x[4]*t.y + rot.x[5]*t.z;
	r.z = rot.x[6]*t.x + rot.x[7]*t.y + rot.x[8]*t.z;
	return r;
}

#include <crest/GPU/Cuda/etkQuaternion.cu>
#include <crest/GPU/Cuda/etkQuartic.cu>
#include <crest/GPU/Cuda/etkRegistration.cu>


__device__ inline float Distance2(float3 p, float3 q)
{
	float3 r = p - q;
	float o = r.x*r.x+r.y*r.y+r.z*r.z;
	return o;
}

__device__ inline 
void computeSquareDistanceWeights(const float3& center, const float3 refVertex[4], QFLOAT weights[4])
{
	QFLOAT dist0 = Distance2(center, refVertex[0]);
	QFLOAT dist1 = Distance2(center, refVertex[1]);
	QFLOAT dist2 = Distance2(center, refVertex[2]);
	QFLOAT dist3 = Distance2(center, refVertex[3]);
	QFLOAT bias = (dist0+dist1+dist2+dist3)*0.25*0.03;
	weights[0] = 1/(dist0+bias);
	weights[1] = 1/(dist1+bias);
	weights[2] = 1/(dist2+bias);
	weights[3] = 1/(dist3+bias);
	QFLOAT sum_w= weights[0]+weights[1]+weights[2]+weights[3];
	QFLOAT inv_sum_w = 1.0/sum_w;
	weights[0] *= inv_sum_w;
	weights[1] *= inv_sum_w;
	weights[2] *= inv_sum_w;
	weights[3] *= inv_sum_w;
}

///////////////////////////////////////////////////////////////////////////////
//! @param data  data in global memory
///////////////////////////////////////////////////////////////////////////////
__global__ 
void gpuMlsDeformation(
	const unsigned int width,//Vertex ARRAY WIDTH
	const float3 *refVert,	 //reference vertex array in their initial positions
	const float3 *defRefVert,//reference vertex array-deformed 
	const float3 *vert,		 //the vertex array to be deformed	
	const int4 *conn,		 //index to the references
	float3 *defv,			 //deformed vertices
	float4 *quat)			 //rotation matrix in deformation
{
	float3 u, p[4], q[4];
	float weights[4];
    const unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
    const unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;
	const unsigned int index = y*width+x;	

	int4 refVertID = conn[index];
	u = vert[index];	
		
	//setup static vertices	
	p[0] = refVert[refVertID.x];
	p[1] = refVert[refVertID.y];
	p[2] = refVert[refVertID.z];
	p[3] = refVert[refVertID.w];
		
	//get wieghts
	computeSquareDistanceWeights(u, p, weights);
	
	//get reference points
	q[0] = defRefVert[refVertID.x];
	q[1] = defRefVert[refVertID.y];
	q[2] = defRefVert[refVertID.z];
	q[3] = defRefVert[refVertID.w];

	//computation
	const int nPoints=4;
	float4 result_quat;
	matrix3x3 result_rot;
	float3 result_trans;
	_etkRegister(nPoints, p, q, weights, &result_quat, &result_rot, &result_trans);

    //write output vertex 
    quat[index] = result_quat;    
    defv[index] = result_trans + mat_float3_mult(result_rot, u);
}


//===============================CPU again==============================
void runGpuMlsDeformation(
	const int nv,					//length of the problem (here is the vertex length)
	GLuint vboRefVertexArray,		//static vertex array of the reference model, float3
	GLuint vboDefRefVertexArray,    //deformed vertex array of the reference model, float3
	//float *pDeviceDefRefVertex,
	GLuint vboNeighborArray,		//neighbourhood array, or connectivity, int4
	GLuint vboVertexArray,		    //static vertex array of the visual model, float3
	GLuint vboDefVertexArray,		//deformed vertex array of the visual model, float3
	GLuint vboQuatArray)		    //rotation quaternion array, float4
{
	const unsigned int shiftbit = ARRAY_LEN_SHIFTBIT;
	const unsigned int padding = ARRAY_LEN_PADDING;
	const unsigned int mesh_width=16;
	const unsigned int mesh_height=(((nv+padding)>>shiftbit)<<shiftbit)/mesh_width;
	
    // map OpenGL buffer objects for writing from CUDA
	float3 *refVert=NULL;	//reference vertex array in their initial positions
	float3 *defRefVert=NULL;//reference vertex array-deformed 
	int4 *conn=NULL;		//index to the references
	float3 *vert=NULL;		//the vertex array to be deformed	
	float3 *defv=NULL;		//deformed vertices
	float4 *quat=NULL;		//rotation matrix in deformation	
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&refVert, vboRefVertexArray));
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&defRefVert, vboDefRefVertexArray));
    //defRefVert = (float3*)pDeviceDefRefVertex;
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&conn, vboNeighborArray));
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&vert, vboVertexArray));    
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&defv, vboDefVertexArray));
    CUDA_SAFE_CALL(cudaGLMapBufferObject((void**)&quat, vboQuatArray));

    // execute the kernel
    dim3 block(16, 8, 1);
    dim3 grid(mesh_width/block.x, mesh_height/block.y, 1);
    gpuMlsDeformation<<<grid, block>>>(mesh_width, refVert, defRefVert, vert, conn, defv, quat);

    // unmap buffer objects
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboQuatArray));
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboDefVertexArray));
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboVertexArray));
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboNeighborArray));
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboDefRefVertexArray)); 
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( vboRefVertexArray));
}

/*
CUT_SAFE_CALL(cutResetTimer(hTimer));
CUT_SAFE_CALL(cutStartTimer(hTimer));

VectorAddition<<< grid, threads >>>( vector1_gpu, vector2_gpu, result_gpu);
CUDA_SAFE_CALL(cudaThreadSynchronize());

performanceMetrics.exe_GPU_time += cutGetTimerValue(hTimer);
*/
