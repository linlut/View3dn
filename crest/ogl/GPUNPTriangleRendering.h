//File: GPUNPTriangleRendering.h

#ifndef _INC_GPUNPTRIANGLE_RENDERING_H_
#define _INC_GPUNPTRIANGLE_RENDERING_H_

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

class GPUNPTriangleRendering: public GPURendering
{
private:
	//Texture coordinate of the vertices, it is fixed, therefore, one vbo is used
	//to store into GPU
	GLuint m_vboTextureCoord;
	GLuint m_vboQuaternion;
	GLuint edge_tex;
	//scaling and translation parameters for the tet mesh
	Vector3f m_texscaling;
	Vector3f m_textranslation;

private:
	//init the input
	void _init(void);

	//the GPU volume renderer
	void _predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw);
	void _postdraw(void);
	void _renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* quat, CGLDrawParms *pdraw);

	void _drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw);

	void _glDraw(const Vector3d *pVertex, CGLDrawParms *pdrawparm);

public:
	GPUNPTriangleRendering(void);

	virtual ~GPUNPTriangleRendering();

	virtual void parse(ZBaseObjectDescription* arg);

	virtual void glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdrawparm);
};

}//ogl
}//cia3d

#endif
