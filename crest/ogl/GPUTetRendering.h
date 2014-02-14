//File: GPUTetRendering.h

#ifndef _INC_GPUTET_RENDERING_H_
#define _INC_GPUTET_RENDERING_H_

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

class GPUTetRendering: public GPURendering
{
private:
	//Texture coordinate of the vertices, it is fixed, therefore, one vbo is used
	//to store into GPU
	GLuint m_vboTextureCoord;
	GLuint m_vboQuaternion;
	GLuint edge_tex;
	//a class to generate the view dependent slices 
	//it actually just sort the elements into slabs using a bucket sorting tech.
	//then, the related elements are stored in a buffer for rendering
	ViewSlicer m_slicer;
	//volume class for storing, loading a volumetric data
	CVolume3D m_volume;
	//1D transfer function
	CTransferFunc m_transferfunc;
	// VERY IMPORTANT:
	// this line loads the address of the glTexImage3D function into the function pointer of the same name.
	// glTexImage3D is not implemented in the standard GL libraries and must be loaded dynamically at run time,
	// the environment the program is being run in MAY OR MAY NOT support it, if not we'll get back a NULL pointer.
	// this is necessary to use any OpenGL function declared in the glext.h header file
	// the Pointer to FunctioN ... PROC types are declared in the same header file with a 
	// type appropriate to the function name
	PFNGLTEXIMAGE3DPROC glTexImage3D;
	//scaling and translation parameters for the tet mesh
	Vector3f m_texscaling;
	Vector3f m_textranslation;

	//----for flipping texture coord-----------
	bool m_flipTexX;
	bool m_flipTexY;
	bool m_flipTexZ;
	bool ___padding;

private:
	//init the input
	void _init(void);
	//drawing functions
	void _test0(CPolyObj *pobj, const Vector3d *pDeformedVertices, CGLDrawParms *pdraw);

	//the GPU volume renderer
	void _predraw(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* pQuat, CGLDrawParms *pdraw);
	void _postdraw(void);
	void _renderForMLSVolumeMapping(CPolyObj *pobj, const Vector3f *pDeformedVertices, const Vector4f* quat, CGLDrawParms *pdraw);

	void _drawWithMappingVisual(CPolyObj *pVisualObj, CBaseMapping* pMapping, const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdraw);

public:
	GPUTetRendering(void);

	virtual ~GPUTetRendering();

	virtual void parse(ZBaseObjectDescription* arg);

	virtual void glDraw(const Vector3d *pDeformedVertices, Vector3d *vertexbuff, CGLDrawParms *pdrawparm);
};

}//ogl
}//cia3d

#endif
