/******************************************
FILE: ogldrawall.h
Author: Nan Zhang
Date: Dec 8, 2007
 I want to put all the opengl drawing functions into this header file
 Draw lots of primitives
********************************************/

#ifndef _INC_OGL_DRAW_ALL_H_DEC082007
#define _INC_OGL_DRAW_ALL_H_DEC082007

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <vectorall.h>
#include <assert.h>

//Draw a mesh with no texture
// npoly: number of polygons
// vpp: vertex number per polygon =  3 or 4

inline void __drawSurfaceUsingArray(
	const void *vertices, GLenum vertexType, 
	const void *pnorm, GLenum normalType, 
	const int *tris, const int npoly, const int vpp)
{
	assert(vpp==3 || vpp==4);
	const int nsize = npoly*vpp;
	GLenum modes[2] = {GL_TRIANGLES, GL_QUADS};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
		glVertexPointer(3, vertexType, 0, vertices);	
		glNormalPointer(normalType, 0, pnorm);
		glDrawElements(modes[vpp-3], nsize, GL_UNSIGNED_INT, tris);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


inline void drawSurfaceUsingArray(
	const Vector3d *vertices, const Vector3f *pnorm, const int *tris, const int npoly, const int vpp=3)
{
	__drawSurfaceUsingArray(vertices, GL_DOUBLE, pnorm, GL_FLOAT, tris, npoly, vpp);
}

// npoly: number of polygons
// vpp: vertex number per polygon =  3 or 4
inline void drawSurfaceUsingArray(
	const Vector3f *vertices, const Vector3f *pnorm, const int *tris, const int npoly, const int vpp=3)
{
	__drawSurfaceUsingArray(vertices, GL_FLOAT, pnorm, GL_FLOAT, tris, npoly, vpp);
}


//Draw a mesh with texture
// npoly: number of polygons
// vpp: vertex number per polygon =  3 or 4
inline void __drawTexturedSurfaceUsingArray(
	const void *vertices, GLenum vertexType, 
	const void *pnorm, GLenum normType, 
	const void *ptexcoord, GLenum texType, int texSize,
	const int *tris, const int npoly, const int vpp=3)
{
	const int nsize = npoly*vpp;
	assert(vpp==3 || vpp==4);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, vertexType, 0, vertices);	
	glNormalPointer(normType, 0, pnorm);

	if (ptexcoord!=NULL){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		int stride = 0;
		if (texSize==2 || texSize==3){
			if (texType == GL_DOUBLE)
				stride = sizeof(double)*3;
			else
				stride = sizeof(float)*3;
		}
		glTexCoordPointer(texSize, texType, stride, ptexcoord);
	}

	GLenum modes[2] = {GL_TRIANGLES, GL_QUADS};
	glDrawElements(modes[vpp-3], nsize, GL_UNSIGNED_INT, tris);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if (ptexcoord!=NULL)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


inline void draw1DTexturedSurfaceUsingArray(
	const Vector3d *vertices, const Vector3f *pnorm, const float *ptexcoord, const int *tris, const int npoly, const int vpp=3)
{
	__drawTexturedSurfaceUsingArray(
		vertices, GL_DOUBLE,
		pnorm, GL_FLOAT, 
		ptexcoord, GL_FLOAT, 1, 
		tris, npoly, vpp);
}


//Draw a mesh with texture
// npoly: number of polygons
// vpp: vertex number per polygon =  3 or 4
inline void draw2DTexturedSurfaceUsingArray(
	const Vector3d *vertices, const Vector3f *pnorm, const Vector3f *ptexcoord, const int *tris, const int npoly, const int vpp=3)
{
	__drawTexturedSurfaceUsingArray(
		vertices, GL_DOUBLE,
		pnorm, GL_FLOAT, 
		ptexcoord, GL_FLOAT, 2, 
		tris, npoly, vpp);
}


#endif