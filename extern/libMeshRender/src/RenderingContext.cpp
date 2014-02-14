#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "RenderingContext.h"

namespace MeshRender{

GLenum RenderingContext::mDrawBuffers[16]={
    GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT,
	GL_COLOR_ATTACHMENT3_EXT,
	GL_COLOR_ATTACHMENT4_EXT,
	GL_COLOR_ATTACHMENT5_EXT,
	GL_COLOR_ATTACHMENT6_EXT,
	GL_COLOR_ATTACHMENT7_EXT
#ifdef GL_COLOR_ATTACHMENT8_EXT
    ,
    GL_COLOR_ATTACHMENT8_EXT,
	GL_COLOR_ATTACHMENT9_EXT,
	GL_COLOR_ATTACHMENT10_EXT,
	GL_COLOR_ATTACHMENT11_EXT,
	GL_COLOR_ATTACHMENT12_EXT,
	GL_COLOR_ATTACHMENT13_EXT,
	GL_COLOR_ATTACHMENT14_EXT,
	GL_COLOR_ATTACHMENT15_EXT
#endif
};

RenderingContext::RenderingContext(const int MRTBufferNum):
    mMrtBufferNum(MRTBufferNum),
    mFboID(0),
    mProgramObject()
{
    const int n = sizeof(mTextureIDs)/sizeof(mTextureIDs[0]);
    for (int i=0; i<n; i++) mTextureIDs[i]=0;
	/*
	if (1){
		const char *macro = "MRT";
		char value[256]="2";
		sprintf(value, "%d", mMrtBufferNum);
		mProgramObject.setMacro(macro, value);
	}
	*/
}

RenderingContext::~RenderingContext()
{
    try{
        deleteRenderTarget();
    }
    catch(...){
        std::cerr<<"RenderingContext destructor error--"<<__FILE__<<": "<<__LINE__<<"\n";
    }
    try{
        destroyShaderProgram();
    }
    catch(...){
        std::cerr<<"RenderingContext destructor error--"<<__FILE__<<": "<<__LINE__<<"\n";
    }
}

void RenderingContext::check_GL_errors(const int lineno) 
{ 
    const GLenum err = glGetError(); 
    if (err){
        fprintf(stderr, "GL Error %x at line %d\n", err, lineno); 
    }
}

void RenderingContext::destroyShaderProgram(void)
{
    mProgramObject.destroy();
}

void RenderingContext::deleteRenderTarget(void)
{
    if (mFboID != 0){
	    glDeleteFramebuffers(1, &mFboID); 
        mFboID = 0;
    }
    const int n = sizeof(mTextureIDs)/sizeof(mTextureIDs[0]);
	GLuint buffer[512];
	int k = 0;
	for (int i=0; i<n; i++){
		if (mTextureIDs[i] != 0){
			buffer[k++] = mTextureIDs[i];
			mTextureIDs[i] = 0;
		}
	}
	if (k > 0) glDeleteTextures(k, buffer);

}

}//namespace