
#include "view3d.h"
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qfont.h>
#include "cia3dobj.h"
//#include <GL/glext.h>


/*
glClipPlane

The glClipPlane function specifies a plane against which all geometry is clipped.

void glClipPlane(
  GLenum plane,
  const GLdouble *equation
);

Parameters

plane
    The clipping plane that is being positioned. Symbolic names of the form GL_CLIP_PLANEi, where i is an integer between 0 and GL_MAX_CLIP_PLANES 1, are accepted. 
equation
    The address of an array of four double-precision floating-point values. These values are interpreted as a plane equation. 

Remarks

Geometry is always clipped against the boundaries of a six-plane frustum in x, y, and z. The glClipPlane function allows the specification of additional planes, not necessarily perpendicular to the x- , y- , or z-axis, against which all geometry is clipped. Up to GL_MAX_CLIP_PLANES planes can be specified, where GL_MAX_CLIP_PLANES is at least six in all implementations. Because the resulting clipping region is the intersection of the defined half-spaces, it is always convex.

The glClipPlane function specifies a half-space using a four-component plane equation. When you call glClipPlane, equation is transformed by the inverse of the modelview matrix and stored in the resulting eye coordinates. Subsequent changes to the modelview matrix have no effect on the stored plane-equation components. If the dot product of the eye coordinates of a vertex with the stored plane equation components is positive or zero, the vertex is in with respect to that clipping plane. Otherwise, it is out.

Use the glEnable and glDisable functions to enable and disable clipping planes. Call clipping planes with the argument GL_CLIP_PLANEi, where i is the plane number.

By default, all clipping planes are defined as (0,0,0,0) in eye coordinates and are disabled.

It is always the case that GL_CLIP_PLANEi = GL_CLIP_PLANE0 + i.

The following functions retrieve information related to glClipPlane:

glGetClipPlane

glIsEnabled with argument GL_CLIP_PLANEi
Error Codes

The following are the error codes generated and their conditions.
Error code 	Condition
GL_INVALID_ENUM 	plane was not an accepted value.
GL_INVALID_OPERATION 	glClipPlane was called between a call to glBegin and the corresponding call to glEnd.
*/



/*

Assuming that your application thread is made current to an OpenGL rendering context, the following routine can be used to determine at run-time if the OpenGL implementation really supports a particular extension:

    #include <GL/gl.h>
    #include <string.h>

    int
    isExtensionSupported(const char *extension)
    {
      const GLubyte *extensions = NULL;
      const GLubyte *start;
      GLubyte *where, *terminator;

      // Extension names should not have spaces. 
      where = (GLubyte *) strchr(extension, ' ');
      if (where || *extension == '\0')
        return 0;
      extensions = glGetString(GL_EXTENSIONS);
      // It takes a bit of care to be fool-proof about parsing the
         OpenGL extensions string. Don't be fooled by sub-strings,
         etc. 
      start = extensions;
      for (;;) {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
          break;
        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
          if (*terminator == ' ' || *terminator == '\0')
            return 1;
        start = terminator;
      }
      return 0;
    }

With the isExtensionSupported routine, you can check if the current OpenGL rendering context supports a given OpenGL extension. To make sure that the EXT_bgra extension is supported before using it, you can do the following:

    // At context initialization. 
    int hasBGRA = isExtensionSupported("GL_EXT_bgra");

     // When trying to use EXT_bgra extension. 
    #ifdef GL_EXT_bgra
      if (hasBGRA) {
        glDrawPixels(width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
      } else
    #endif
      {
        // No EXT_bgra so bail (or implement software workaround). 
        fprintf(stderr, "Needs EXT_bgra extension!\n");
        exit(1);
      }

Notice that if the EXT_bgra extension is lacking at either run-time or compile-time, the code above will detect the lack of EXT_bgra support. Sure the code is a bit messy, but the code above works. You can skip the compile-time check if you know what development environment you are using and you do not expect to ever compile with a <GL/gl.h> that does not support the extensions that your application uses. But the run-time check really should be performed since who knows on what system your program ends up getting run on.
# Win32’s Scheme for Getting Extension Function Pointers

The example above for safely detecting and using the EXT_bgra extension at run-time and compile-time is straightforward because the EXT_bgra simply adds two new enumerants (GL_BGRA_EXT and GL_BGR_EXT) and does not require any new function pointers.

Using an extension that includes new function call entry-points is harder in Win32 because you must first request the function pointer from the OpenGL ICD driver before you can call the OpenGL function.

The EXT_point_parameters extension provides eye-distance attenuation of OpenGL’s point primitive. This extension is used by Id Software in Quake 2 when the extension is present for rendering particle systems. With the extension, firing weapon and explosions are rendered as huge clusters of OpenGL point primitives with OpenGL automatically adjusting the point size based on the distance of the particles from the viewer. Closer particles appear bigger; particles in the distance appear smaller. A particle whose size would be smaller than a pixel is automatically faded based on its sub-pixel size. Anyone that wants to see the improvement this extension brings to a 3D game should play Quake 2 on a PC with NVIDIA’s RIVA 128 graphics processor. Start a gun battle and check out the particles!

The EXT_point_parameters extension adds two new OpenGL entry points called glPointParameterfEXT and glPointParameterfvEXT. These routines allow the application to specify the attenuation equation parameters and fade threshold. The problem is that because of the way Microsoft chose to support OpenGL extension functions, an OpenGL application cannot simply link with these functions. The application must first use the wglGetProcAddress routine to query the function address and then call through the returned address to call the extension function.

First, declare function prototype typedefs that match the extension’s entry points. For example:
 

    #ifdef _WIN32
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFEXTPROC)(GLenum pname, GLfloat param);
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFVEXTPROC)(GLenum pname, const GLfloat *params);
    #endif

Your <GL/gl.h> header file may already have these typedefs declared if your <GL/gl.h> defines the GL_EXT_point_parameters macro. Now declare global variables of the type of these function prototype typedefs like this:

    #ifdef _WIN32
    PFNGLPOINTPARAMETERFEXTPROC glPointParameterfEXT;
    PFNGLPOINTPARAMETERFVEXTPROC glPointParameterfvEXT;
    #endif

The names above exactly match the extension’s function names. Once we use wglGetProcAddress to assign these function variables the address of the OpenGL driver’s extension functions, we can call glPointParameterfEXT and glPointParameterfvEXT as if they were normal functions. You pass wglGetProcAddress the name of the routine as an ASCII string. Verify that the extension is supported and, if so, initialize the function variables like this:

      int hasPointParams = isExtensionSupported("GL_EXT_point_parameters");

    #ifdef _WIN32
      if (hasPointParams) {
        glPointParameterfEXT = (PFNGLPOINTPARAMETERFEXTPROC)
        wglGetProcAddress("glPointParameterfEXT");
        glPointParameterfvEXT = (PFNGLPOINTPARAMETERFVEXTPROC)
        wglGetProcAddress("glPointParameterfvEXT");
      }
    #endif

Note that before the code above is called, you should have a current OpenGL rendering context.

With the function variables properly initialized to the extension entry-points, you can use the extension like this:

      if (hasPointParams) {
        static GLfloat quadratic[3] = { 0.25, 0.0, 1/60.0 };
        glPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, quadratic);
        glPointParameterfEXT(GL_POINT_FADE_THRESHOLD_SIZE_EXT, 1.0);
      }
*/


/*
void initPointSprites( void )
{
	//
	// Load up the point sprite's texture...
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\particle.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0,
				     GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
	}

	if( pTextureImage )
	{
		if( pTextureImage->data )
			free( pTextureImage->data );

		free( pTextureImage );
	}

	//
    // If you want to know the max size that a point sprite can be set 
    // to, do this.
	//

    // Query for the max point size supported by the hardware
    float maxSize = 0.0f;
    glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );
    glPointSize( maxSize );

	//
	// Initialize our particles so they'll start at the origin with some 
	// random direction and color.
	//

	for( int i = 0; i < MAX_PARTICLES; ++i )
    {
		g_particles[i].m_vCurPos = vector3f(0.0f,0.0f,0.0f);
		g_particles[i].m_vCurVel = getRandomVector() * getRandomMinMax( 0.5f, 5.0f );
        
        g_particles[i].m_r = getRandomMinMax( 0.0f, 1.0f );
        g_particles[i].m_g = getRandomMinMax( 0.0f, 1.0f );
        g_particles[i].m_b = getRandomMinMax( 0.0f, 1.0f );
	}
}
*/

/*
void renderPointSprites( void )
{
    //
    // Set up for blending...
    //

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    //
	// Set up the OpenGL state machine for using point sprites...
	//

    // This is how will our point sprite's size will be modified by 
    // distance from the viewer
    float quadratic[] =  { 1.0f, 0.0f, 0.01f };
    glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

    // Query for the max point size supported by the hardware
    float maxSize = 0.0f;
    glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

    // Clamp size to 100.0f or the sprites could get a little too big on some  
    // of the newer graphic cards. My ATI card at home supports a max point 
    // size of 1024.0f!
    if( maxSize > 100.0f )
        maxSize = 100.0f;

    glPointSize( maxSize );

    // The alpha of a point is calculated to allow the fading of points 
    // instead of shrinking them past a defined threshold size. The threshold 
    // is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
    // the minimum and maximum point sizes.
    glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f );

    glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1.0f );
    glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, maxSize );

    // Specify point sprite texture coordinate replacement mode for each 
    // texture unit
    glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

    //
	// Render point sprites...
	//

    glEnable( GL_POINT_SPRITE_ARB );

	glBegin( GL_POINTS );
    {
        for( int i = 0; i < MAX_PARTICLES; ++i )
        {
            glColor4f( g_particles[i].m_r, 
                       g_particles[i].m_g, 
                       g_particles[i].m_b, 
                       1.0f );

	        glVertex3f( g_particles[i].m_vCurPos.x,
                        g_particles[i].m_vCurPos.y,
                        g_particles[i].m_vCurPos.z );
        }
    }
	glEnd();

	glDisable( GL_POINT_SPRITE_ARB );
}
*/
