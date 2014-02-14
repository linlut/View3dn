//File: MLSGpuVolumeMapping.h--Moving Least Square based mapping

#ifndef CIA3D_COMPONENT_MAPPING_MLSGpu_VOLUME_MAPPING_H
#define CIA3D_COMPONENT_MAPPING_MLSGpu_VOLUME_MAPPING_H

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <vectorall.h>
#include <crest/component/MLSVolumeMapping.h>


namespace cia3d{
namespace component{


class MLSGpuVolumeMapping : public MLSVolumeMapping
{	
private:
	GLuint vboRefVertexArray;		//static vertex array of the reference model, float3
	GLuint vboDefRefVertexArray;    //deformed vertex array of the reference model, float3
	//Vector3f *m_pDeviceDefRefVertex;//Remember it is a device pointer!!
	GLuint vboWeightArray;          //weight array, float4, this may be no use since we can use online computation
	GLuint vboNeighborArray;		//neighbourhood array, or connectivity, int4
	GLuint vboVertexArray;		    //static vertex array of the visual model, float3
	GLuint vboDefVertexArray;		//deformed vertex array of the visual model, float3
	GLuint vboQuatArray;		    //rotation quaternion array, float4
	GLuint vboNormArray;		    //normal array, float3
	GLuint vboDefNormArray;		    //deformed normal array, float3

	void init(CPolyObj *src, CPolyObj *dst);

public:
	
	MLSGpuVolumeMapping(void): MLSVolumeMapping()
	{
		vboRefVertexArray=0;		//static vertex array of the reference model, float3
		vboDefRefVertexArray=0;     //deformed vertex array of the reference model, float3
		//m_pDeviceDefRefVertex = NULL;
		vboWeightArray=0;           //weight array, float4, this may be no use since we can use online computation
		vboNeighborArray=0;		    //neighbourhood array, or connectivity, int4
		vboVertexArray=0;		    //static vertex array of the visual model, float3
		vboDefVertexArray=0;		//deformed vertex array of the visual model, float3
		vboQuatArray=0;	   	        //rotation quaternion array, float4
		vboNormArray=0;			    //normal array, float3
		vboDefNormArray=0;		    //deformed normal array, float3
	}
	
	virtual ~MLSGpuVolumeMapping();
	
	virtual bool canDeformNormal(void)
	{
		return false;
	}

	virtual void parse(ZBaseObjectDescription* arg)
	{
		MLSVolumeMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}

	virtual void updateMapping(const Vector3d* input, Vector3d*);

};


} // namespace component
} // namespace cia3d

#endif
