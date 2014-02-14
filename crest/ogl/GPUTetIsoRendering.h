//File: GPUTetIsoRendering.h

#ifndef _INC_GPUTETISO_RENDERING_H_
#define _INC_GPUTETISO_RENDERING_H_

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <zvolume.h>
#include <transferfunc.h>
#include <crest/ogl/GPURendering.h>
#include <crest/ogl/ViewSlicer.h>


namespace cia3d{
namespace ogl{

class GPUTetIsoRendering: public GPURendering
{
private:
	//Texture coordinate of the vertices, it is fixed, therefore, one vbo is used
	GLuint m_vboTextureCoord;
	GLuint edge_tex;
	//a class to generate the view dependent slices 
	//it actually just sort the elements into slabs using a bucket sorting tech.
	//then, the related elements are stored in a buffer for rendering
	ViewSlicer m_slicer;
	//1D transfer function
	CTransferFunc m_transferfunc;

private:
	//init the input
	void _init(void);

	//the GPU volume renderer
	void _predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const float* pTexcoord, CGLDrawParms *pdraw);
	void _postdraw(void);
	void _renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const float* pTexcoord, CGLDrawParms *pdraw);

	void _drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw);

public:
	
	static GPUTetIsoRendering *getInstance(CPolyObj *pobj);

	GPUTetIsoRendering(void);

	virtual ~GPUTetIsoRendering();

	virtual void setSlicingDistance(const float dist);

	virtual void setTransferFunction(const QString& transferfunctionfile, const int itype);

	virtual void setTransferFunction(const float minval, const float maxval, const int itype=0);

	virtual void parse(ZBaseObjectDescription* arg);

	virtual void defaultInit(CPolyObj *pVisualObj);

	//for rendering objects in crest_sofa
	virtual void glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdrawparm);
	//for rendering objects in View3d (visualize FEM simulation results)
	virtual void glSimpleDraw(const Vector3f *pVertices, const float *pTexcoord, CGLDrawParms *pdrawparm);
};

}//ogl
}//cia3d

#endif
