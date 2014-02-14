//FILE: QtViewer.cpp

#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glaux.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include "ZQGLViewer.h"
#include <crest/GPU/Cuda/cuda0.h>


//using namespace qglviewer;

ZQGLViewer::ZQGLViewer(QWidget *parent, const QGLWidget *shareWidget, Qt::WFlags flags):
	QGLViewer(parent, shareWidget, flags)
{
	//MRT buffer length is 6, which stores the FBO, number of buffer(4), 
	//and the texture ID of the 4 buffers, respectively
	m_fboMRT[0] = m_fboMRT[1] = m_fboMRT[2] = m_fboMRT[3] = 0;
	m_fboMRT[4] = m_fboMRT[5] = 0;
}

ZQGLViewer::~ZQGLViewer(void)
{
	
}


//ALLOCATE RGBA 4 component, each component is a floating number (32bit)
inline void AllocTextureBuffer2D(const int w, const int h)
{
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_FLOAT, 0);
}

//Initializes the viewer OpenGL context.
void ZQGLViewer::init(void)
{
	//call the parent class's init function;
	QGLViewer::init();

	//this function's init function starts here
	glewDeviceInit();

	//create Textures
	m_fboMRT[1] = 4;
	GLuint *tex = &m_fboMRT[2];
	GLenum texTarget = GL_TEXTURE_2D;

	//generate 4 textures
	const int Width = width();
	const int Height = height();
	for (int i=0; i<4; i++){
		glGenTextures(1, &tex[i]);   
		glBindTexture(texTarget, tex[i]);
		// turn off filtering and wrap modes
		glTexParameteri(texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
		AllocTextureBuffer2D(Width, Height);
		if (glGetError() != GL_NO_ERROR) printf("Init: Error in creating tex\n");
	}

	//generate the FBO
	GLuint& fb = m_fboMRT[0];
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texTarget, tex[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, texTarget, tex[1], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, texTarget, tex[2], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, texTarget, tex[3], 0);
	if (glGetError() != GL_NO_ERROR) printf("Init: Error in attaching FBO");

	// Validate FBO after attaching textures
	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT){
		std::cerr << "Error: Framebuffer incomplete!\n";
		exit(1);
	}

	//now detach the FBO
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void ZQGLViewer::_reshape(const int w, const int h)
{
	GLuint &fbo = m_fboMRT[0];
	if (fbo==0) return;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, 0, 0);

	// Reallocate textures
	GLuint *tex = &m_fboMRT[2];
	for (int i = 0; i < 4; i++){
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		AllocTextureBuffer2D(w, h);
	}
  
	// Attach textures
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, tex[1], 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, tex[2], 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, tex[3], 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void ZQGLViewer::resizeGL(int width, int height)
{
	//parent class method
	QGLViewer::resizeGL(width, height);
	//this class's method
	_reshape(width, height);
}


