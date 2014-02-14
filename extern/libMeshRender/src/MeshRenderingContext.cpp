#include <assert.h>
#include "MeshRenderingContext.h"

#ifdef GL_TEXTURE_2D_ARRAY
    const unsigned int texarray_target = GL_TEXTURE_2D_ARRAY;
#else
#ifdef GL_TEXTURE_2D_ARRAY_EXT
    const unsigned int texarray_target = GL_TEXTURE_2D_ARRAY_EXT;
#endif
#endif

//PROJ_PATH is a macro defined in the CMAKE file
#define SHADER_PATH PROJ_PATH"/src/shaders/"

#ifndef CHECK_GL_ERRORS
#define CHECK_GL_ERRORS CheckGLError
#endif

//Global context variables, they should be fixed and constant
MeshRender::FlatShadingRC flatShadingRc;
MeshRender::GouraudShadingRC gouraudShadingRc;
MeshRender::PhongShadingRC phongShadingRc;
MeshRender::FlatHiddenLineRC fhiddenLineRC;
MeshRender::GouraudHiddenLineRC ghiddenLineRC;
MeshRender::PhongHiddenLineRC phiddenLineRC;

namespace MeshRender{
	//Initialization of static rendering contexts, called when OpenGL context is setup
	void initAllMeshRenderingContexts(void)
	{
		gouraudShadingRc.initRenderTarget(0, 0);
		gouraudShadingRc.initShaderProgram();

		flatShadingRc.initRenderTarget(0, 0);
		flatShadingRc.initShaderProgram();

		phongShadingRc.initRenderTarget(0, 0);
		phongShadingRc.initShaderProgram();

		ghiddenLineRC.initRenderTarget(0, 0);
		ghiddenLineRC.initShaderProgram();

		fhiddenLineRC.initRenderTarget(0, 0);
		fhiddenLineRC.initShaderProgram();

        phiddenLineRC.initRenderTarget(0, 0);
		phiddenLineRC.initShaderProgram();
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    MRBaseRC::MRBaseRC(int mrt) : RenderingContext(mrt)
    {
        int nsize = sizeof(uniformsf)/sizeof(uniformsf[0]);
        for (int i=0; i<nsize; i++)
            uniformsf[i] = vec4(0,0,0,0);

        nsize = sizeof(uniformsint)/sizeof(uniformsint[0]);
        for (int i=0; i<nsize; i++)
            uniformsint[i] = ivec4(0,0,0,0);        
    }

    MRBaseRC::~MRBaseRC()
    {
    }

    void MRBaseRC::uploadAllUniformsToGPU()
    {
        if (sizeof(GLfloat) == 4){
            const int len = sizeof(uniformsf)/sizeof(float);
		    mProgramObject.setUniform("uniformsf", &uniformsf[0].x, len);
		    CHECK_GL_ERRORS;
        }
        else{
            std::cerr << "Error: Floating point number size is " << sizeof(float) << "!\n";
        }
        if (sizeof(int) == 4){
            const int len = sizeof(uniformsint)/sizeof(int);
		    mProgramObject.setUniform("uniformsint", &uniformsint[0].x, len);
		    CHECK_GL_ERRORS;
        }
        else{
            std::cerr << "Error: Integer number size is " << sizeof(int) << "!\n";
        }
    }

	void MRBaseRC::bind(const int pass, const GLfloat *floatShaderParms, const GLuint* intShaderParms)
	{
		mProgramObject.bind();
		CHECK_GL_ERRORS;
	}

	void MRBaseRC::unbind(void)
	{
		mProgramObject.unbind();
	}


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	FlatShadingRC::FlatShadingRC() :
		MRBaseRC(0)
	{
	}

	FlatShadingRC::~FlatShadingRC()
	{
	}

	void FlatShadingRC::setElementNormalTexture(GLuint texID)
    {
		const GLuint elmNormalTexID = texID;
		mProgramObject.bindTextureRECT("texElmNormal", elmNormalTexID, 0);
		CHECK_GL_ERRORS;
    }

	void FlatShadingRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"flatshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"flatshading_fp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	GouraudShadingRC::GouraudShadingRC() :
		MRBaseRC(0)
	{
	}

	GouraudShadingRC::~GouraudShadingRC()
	{
	}

	void GouraudShadingRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"gouraudshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"gouraudshading_fp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	FlatHiddenLineRC::FlatHiddenLineRC() :
		MRBaseRC(0)
	{
	}

	FlatHiddenLineRC::~FlatHiddenLineRC()
	{
	}

	void FlatHiddenLineRC::setElementNormalTexture(GLuint texID)
    {
		const GLuint elmNormalTexID = texID;
		mProgramObject.bindTextureRECT("texElmNormal", elmNormalTexID, 0);
		CHECK_GL_ERRORS;
    }

	void FlatHiddenLineRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"flatshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"flatshading_fp.glsl");
		mProgramObject.attachGeometryShader(SHADER_PATH"hiddenline_gp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	GouraudHiddenLineRC::GouraudHiddenLineRC() :
		MRBaseRC(0)
	{
	}

	GouraudHiddenLineRC::~GouraudHiddenLineRC()
	{
	}

	void GouraudHiddenLineRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"gouraudshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"gouraudshading_fp.glsl");
		mProgramObject.attachGeometryShader(SHADER_PATH"hiddenline_gp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	PhongShadingRC::PhongShadingRC() :
		MRBaseRC(0)
	{
	}

	PhongShadingRC::~PhongShadingRC()
	{
	}

	void PhongShadingRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"phongshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"phongshading_fp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	PhongHiddenLineRC::PhongHiddenLineRC() :
		MRBaseRC(0)
	{
	}

	PhongHiddenLineRC::~PhongHiddenLineRC()
	{
	}

	void PhongHiddenLineRC::initShaderProgram(void)
	{
		mProgramObject.attachVertexShader(SHADER_PATH"phongshading_vp.glsl");
		mProgramObject.attachFragmentShader(SHADER_PATH"phongshading_fp.glsl");
		mProgramObject.attachGeometryShader(SHADER_PATH"hiddenline_gp.glsl");
		mProgramObject.link();
		CHECK_GL_ERRORS;
	}

}//namespace