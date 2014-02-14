//FILE: cuda0.h


#ifndef _INC_CRESTSOFA_GPU_CUDA_CUDA0_H_
#define _INC_CRESTSOFA_GPU_CUDA_CUDA0_H_

#define ARRAY_LEN_SHIFTBIT 8
#define ARRAY_LEN_PADDING ((1L<<ARRAY_LEN_SHIFTBIT) -1)

extern "C"
{
 void gpuCudaGLRegisterBufferObject(GLuint vbo);
 
 void gpuCudaGLUnregisterBufferObject(GLuint vbo);
 
 void cudaCheckErrorGL(void);

 void cudaDeviceClose(const int argc, char **argv);

 void cudaDeviceInit(void);

 void glewDeviceInit(void);

 void cudaAllocateDeviceArray(void **devPtr, size_t size);

 void cudaFreeDeviceArray(void *devPtr);

 void cudaCopyArrayFromDevice(void* host, const void* device, unsigned int vbo, int size);

 void cudaCopyArrayToDevice(void* device, const void* host, int offset, int size);

 void runGpuMlsDeformation(
	const int nv,					//length of the problem (here is the vertex length)
	GLuint vboRefVertexArray,		//static vertex array of the reference model, float3
	GLuint vboDefRefVertexArray,    //deformed vertex array of the reference model, float3
	//float * pDeviceDefRefVertex,
	GLuint vboNeighborArray,		//neighbourhood array, or connectivity, int4
	GLuint vboVertexArray,		    //static vertex array of the visual model, float3
	GLuint vboDefVertexArray,		//deformed vertex array of the visual model, float3
	GLuint vboQuatArray);		    //rotation quaternion array, float4

}

#endif