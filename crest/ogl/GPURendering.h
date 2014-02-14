//File: GPURendering.h

#ifndef _INC_GPU_RENDERING_H_
#define _INC_GPU_RENDERING_H_

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <crest/ogl/BaseRendering.h>
#include <crest/ogl/Shader.h>


namespace cia3d{
namespace ogl{


class GPURendering: public BaseRendering
{
private: 
	void _init(void);

protected:
	GPUProgram m_gpuProgram;
	GLuint m_vboVertex;
	GLuint m_vboElement;

	void _safeDeleteVBO(GLuint& x)
	{
		if (x>0) 
			glDeleteBuffers(1, &x);
		x=0;
	}

public:
	void SafeDeleteVBO(GLuint& x)
	{
		_safeDeleteVBO(x);
	}

	GPURendering(void): BaseRendering(), 
		m_gpuProgram()
	{
		m_vboVertex = 0;
		m_vboElement = 0;
	}

	virtual ~GPURendering()
	{
		SafeDeleteVBO(m_vboVertex);
		SafeDeleteVBO(m_vboElement);
	}

	virtual void parse(ZBaseObjectDescription* arg);

	virtual void setGPUPrograms(const char *vp, const char *vpmain, 
								const char *gp, const char *gpmain, 
								const char *fp, const char *fpmain);
	
};

}//ogl
}//cia3d

#endif
