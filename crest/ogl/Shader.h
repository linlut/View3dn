//FILE: Shader.h

#ifndef _INC_CREST_SHADER_H
#define _INC_CREST_SHADER_H

#include <vector>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <assert.h>
#include <sysconf.h>
#include <object3d.h>

namespace cia3d{
namespace ogl{


class Shader
{
public:

private:

protected:
	unsigned int m_nShaderType;

public:
	Shader(unsigned int shadertype, void *context=NULL, void *profile=NULL)
	{
		m_nShaderType = shadertype;
	}
	virtual void init(const char *filename, const char *funcname, const char **options)=0;
	virtual void bind()=0;
	virtual void release()=0;

	//virtual void setParameter(const char *pname, float* val, int count=1)=0;
	virtual void setParameter1i(const char *name, const int x)=0;
	virtual void setParameter3i(const char *name, const int x, const int y, const int z)=0;
	virtual void setParameter1f(const float x)=0;
	virtual void setParameter1f(const char *name, const float x)=0;
	virtual void setParameter3f(const char *name, const float x, const float y, const float z)=0;
	virtual void setParameter3f(const float x, const float y, const float z)=0;
	virtual void setTextureParameter(const char *name, const unsigned int texid)=0;
    virtual void clearTextureParameter(const char *name)=0;

	virtual void setMaxGeoOutputVertices(const int n)=0;
};


class CGShader: public Shader
{
private:
	CGparameter m_savedParameter;

protected:
	CGcontext* m_pCgContext; 
	CGprofile* m_pCGProfile;
	CGprogram m_program;

private:
	void _bindCgProgram(void)
	{
		cgGLBindProgram(m_program);
		cgGLEnableProfile(*m_pCGProfile);
	}

	void _unbindCgProgram(void)
	{
		cgGLUnbindProgram(*m_pCGProfile);
		cgGLDisableProfile(*m_pCGProfile);
	}

public:
	CGShader(unsigned int stype, void *context, void *profile): 
		Shader(stype, context, profile)
	{
		m_pCgContext = (CGcontext*)context;
		m_pCGProfile = (CGprofile*)profile;
		m_program = 0;
		m_savedParameter = NULL;
	}

	~CGShader(void);

	virtual void init(const char *filename, const char *funcname=NULL, const char **options=NULL);

	virtual void bind()
	{
		_bindCgProgram();
	}

	virtual void release()
	{
		_unbindCgProgram();
	}

	virtual void setParameter1i(const char *name, const int x)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param) {
			cgSetParameter1i(param, x);
		}
	}

	virtual void setParameter3i(const char *name, const int x, const int y, const int z)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param) {
			cgSetParameter3i(param, x, y, z);
		}
	}

	virtual void setParameter1f(const char *name, const float x)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param) {
			cgSetParameter1f(param, x);
			m_savedParameter = param;
		}
	}

	virtual void setParameter1f(const float x)
	{
		ASSERT0(m_savedParameter!=NULL);
		cgSetParameter1f(m_savedParameter, x);
	}

	virtual void setParameter3f(const char *name, const float x, const float y, const float z)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param) {
			cgSetParameter3f(param, x, y, z);
			m_savedParameter = param;
		}
	}

	virtual void setParameter3f(const float x, const float y, const float z)
	{
		ASSERT0(m_savedParameter!=NULL);
		cgSetParameter3f(m_savedParameter, x, y, z);
	}

	virtual void setTextureParameter(const char *name, const unsigned int texid)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param){
			cgGLSetTextureParameter(param, texid);
			cgGLEnableTextureParameter(param);
		}
	}

    virtual void clearTextureParameter(const char *name)
	{
		CGparameter param = cgGetNamedParameter(m_program, name);
		if (param)
		    cgGLDisableTextureParameter(param);
	}

	virtual void setMaxGeoOutputVertices(const int n);

};


class GLShader: public Shader
{
public:
	GLShader(unsigned int stype, void *context): Shader(stype, context)
	{
	}

private:
	unsigned int id;

public:
	virtual void setTextureParameter(const char *name, const unsigned int texid)
	{
	}
    virtual void clearTextureParameter(const char *name)
	{
	}

};


class GPUProgram
{
private:
	std::vector<Shader*> m_shaders;
    CGcontext context;
	CGprofile vertexProfile;
	CGprofile fragmentProfile;
	CGprofile geometryProfile;

	void _chooseProfiles(void);

public:
	GPUProgram(void);

	~GPUProgram(void);

	int addShader(const bool cgshader, const unsigned int shadertype, const char *filename, const char* function, const char **cc_options);

	void bind(void)
	{
		const unsigned int nsize = m_shaders.size();
		for (unsigned int i=0; i<nsize; i++){
			Shader *p = m_shaders[i];
			ASSERT0(p!=NULL);
			p->bind();
		}
	}

	void unbind(void)
	{
		const unsigned int nsize = m_shaders.size();
		for (unsigned int i=0; i<nsize; i++){
			Shader *p = m_shaders[i];
			ASSERT0(p!=NULL);
			p->release();
		}
	}

	Shader *getShader(const int id)
	{
		const int nsize = m_shaders.size();
		if (id>=0 && id< nsize)
			return m_shaders[id];
		return NULL;
	}

	void setTextureParameter(const int shaderid, const char *name, const unsigned int texid)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setTextureParameter(name, texid);
	}

    void clearTextureParameter(const int shaderid, const char *name)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->clearTextureParameter(name);
	}

	void setParameter1i(const int shaderid, const char *name, const int x)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setParameter1i(name, x);
	}

	void setParameter1f(const int shaderid, const char *name, const float x)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setParameter1f(name, x);
	}

	void setParameter1f(const int shaderid, const float x)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setParameter1f(x);
	}

	void setParameter3f(const int shaderid, const char *name, const float x, const float y, const float z)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setParameter3f(name, x, y, z);
	}

	void setParameter3f(const int shaderid, const float x, const float y, const float z)
	{
		Shader *p = getShader(shaderid);
		if (p==NULL) return;
		p->setParameter3f(x, y, z);
	}

	void setParameter1i(const char *name, const int x)
	{
		setParameter1i(0, name, x);
		setParameter1i(1, name, x);
		setParameter1i(2, name, x);
	}

	void setParameter1f(const char *name, const float x)
	{
		setParameter1f(0, name, x);
		setParameter1f(1, name, x);
		setParameter1f(2, name, x);
	}

	void setParameter3f(const char *name, const float x, const float y, const float z)
	{
		setParameter3f(0, name, x, y, z);
		setParameter3f(1, name, x, y, z);
		setParameter3f(2, name, x, y, z);
	}

};

}//namespace ogl
}//cia3d

#endif