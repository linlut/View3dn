#pragma once
#ifndef INC_RENDERING_CONTEXT
#define INC_RENDERING_CONTEXT

#if _MSC_VER
#include <windows.h>
#endif
#include <GL/glew.h>
#include "GLSLProgramObject.h"
#include "base_object.h"

namespace MeshRender{

class RenderingContext: public CObjectBase
{

public:
	RenderingContext(const int MRTBufferNum);
	virtual ~RenderingContext();

    virtual void bind(const int pass, const GLfloat *floatShaderParms, const GLuint* intShaderParms)=0;
    virtual void unbind(void)=0;
    virtual void initShaderProgram(void)=0;
    virtual void destroyShaderProgram(void);
    virtual void initRenderTarget(const int w, const int h)=0;
    virtual void deleteRenderTarget(void);

	static GLenum mDrawBuffers[16];
    static void check_GL_errors(const int lineno);
	
    inline GLuint getTextureID(int i=0)
    {
        return mTextureIDs[i];
    }

    inline const GLuint* getTextureIDs(void)
    {
        return mTextureIDs;
    }

    inline GLuint getFboID(void)
    {
        return mFboID;
    }

    inline int getMRTBufferNum(void) const
    {
        return mMrtBufferNum; 
    }

    GLSLProgramObject& getProgramObject(void)
    {
        return mProgramObject;
    }

protected:
    GLSLProgramObject mProgramObject;
    int mMrtBufferNum;          //number of MRT color buffers used
	GLuint mFboID;              //framebuffer object name
	GLuint mTextureIDs[16];     //2D or 3D texture names
		
};

}

#endif //INC_RENDERING_CONTEXT