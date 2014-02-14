#pragma once
#ifndef MESH_RENDERING_CONTEXT_H
#define MESH_RENDERING_CONTEXT_H
#include "RenderingContext.h"
#include "vector4i.h"
#include "vector4f.h"
#include "vector3f.h"
#include "vector2f.h"

#define vec2 Vector2f
#define vec3 Vector3f
#define vec4 Vector4f
#define ivec4 Vector4i
#define uniform 

namespace MeshRender{

	class MRBaseRC : public RenderingContext
    {
    public:
		//MRT is not used in this RC
		virtual void initRenderTarget(const int w, const int h){}
    
        void uploadAllUniformsToGPU();

	    void bind(const int pass, const GLfloat *floatShaderParms, const GLuint* intShaderParms);

    	void unbind(void);

        //insert commond code shared btw. C++ and GLSL
        #include "shaders/common_uniforms.glsl"
        //===========end of insertion================

        inline void setFaceFrontColor(vec3 color)
        {
            const int i = FACE_FRONT_COLOR_ROW;
            uniformsf[i].x = color.x;
            uniformsf[i].y = color.y;
            uniformsf[i].z = color.z;
        }

        inline void setFaceBackColor(vec3 color)
        {
            const int i = FACE_BACK_COLOR_ROW;
            uniformsf[i].x = color.x;
            uniformsf[i].y = color.y;
            uniformsf[i].z = color.z;
        }

        inline void setLineColor(vec3 color)
        {
            const int i = LINE_COLOR_ROW;
            uniformsf[i].x = color.x;
            uniformsf[i].y = color.y;
            uniformsf[i].z = color.z;
        }

        inline void setLineWidth(float width)
        {
            const int i = LINE_COLOR_ROW;
            uniformsf[i].w = width;
        }

        inline void setScreenSize(int w, int h)
        {
            const int i = SCREEN_SIZE_ROW;
            float wf = float(w);
            float hf = float(h);
            uniformsf[i].x = wf;
            uniformsf[i].y = hf;
            uniformsf[i].z = 1.0f/wf;
            uniformsf[i].w = 1.0f/hf;
        }

        inline void setAttributeMinMax(float minval, float maxval)
        {
            const int i = ATTRIBUTE_MINMAX_ROW;
            uniformsf[i].x = minval;
            uniformsf[i].y = maxval;
        }

        void setHiddenLineFlag(bool f)
        {
            int & r = uniformsint[0].x;
            r &= 0xfffffffe;
            r |= int(f);
        }

        void setShadingFlag(bool f)
        {
            int & r = uniformsint[0].x;
            r &= 0xfffffffd;
            r |= int(f) << 1;
        }

        void setTextureFlag(bool f)
        {
            int & r = uniformsint[0].x;
            r &= 0xfffffffb;
            r |= int(f) << 2;
        }

        void setFlipNormalFlag(bool f)
        {
            int & r = uniformsint[0].x;
            r &= 0xfffffff7;
            r |= int(f) << 3;
        }

    protected:
        MRBaseRC(int mrt);
        virtual ~MRBaseRC();
    };

	//===================================================================
	class FlatShadingRC : public MRBaseRC
	{
	public:
		FlatShadingRC();
		virtual ~FlatShadingRC();
		void setElementNormalTexture(GLuint texID);
		void initShaderProgram(void);
	};

	class GouraudShadingRC : public MRBaseRC
	{
	public:
		GouraudShadingRC();
		virtual ~GouraudShadingRC();
		void initShaderProgram(void);
	};

	class FlatHiddenLineRC : public MRBaseRC
	{
	public:
		FlatHiddenLineRC();
		virtual ~FlatHiddenLineRC();
		void setElementNormalTexture(GLuint texID);
		void initShaderProgram(void);
	};

	class GouraudHiddenLineRC : public MRBaseRC
	{
	public:
		GouraudHiddenLineRC();
		virtual ~GouraudHiddenLineRC();
		void initShaderProgram(void);
	};

	class PhongShadingRC : public MRBaseRC
	{
	public:
		PhongShadingRC();
		virtual ~PhongShadingRC();
		void initShaderProgram(void);
	};

	class PhongHiddenLineRC : public MRBaseRC
	{
	public:
		PhongHiddenLineRC();
		virtual ~PhongHiddenLineRC();
		void setElementNormalTexture(GLuint texID);
		void initShaderProgram(void);
	};

}

#endif //INC_TETRENDERING_CONTEXT