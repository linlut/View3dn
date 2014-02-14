#include "ShaderPath.h"
#include "ShadersManagement.h"
#include "GLSLProgramObject.h"

#ifndef nullptr
#define nullptr 0
#endif

namespace MeshRender{

GLSLProgramObject::GLSLProgramObject() :
	_vertexShaders()
	,_fragmentShaders()
	,_progId(0)
	,mMacroList()
{
}

GLSLProgramObject::~GLSLProgramObject()
{
	destroy();
}

void GLSLProgramObject::destroy()
{
	for (unsigned i = 0; i < _vertexShaders.size(); i++)
		glDeleteShader(_vertexShaders[i]);
	for (unsigned i = 0; i < _fragmentShaders.size(); i++)
		glDeleteShader(_fragmentShaders[i]);
	if (_progId != 0)
		glDeleteProgram(_progId);
	_progId = 0;
	_vertexShaders.clear();
	_fragmentShaders.clear();
}

void GLSLProgramObject::attachVertexShaderString(const char* shaderprogram)
{
    GLuint shaderId = createShaderFromString(shaderprogram, nullptr, GL_VERTEX_SHADER);
	if (shaderId == 0) {
		const char *errmsg = "Error: Vertex shader failed to compile";
	    std::cerr << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}
	_vertexShaders.push_back(shaderId);
}

void GLSLProgramObject::attachVertexShader(const char* filename)
{
    std::string resolved_path;
    if (sdkPath.getFilePath(filename, resolved_path)){
	    const GLuint shaderId = createShader(resolved_path.c_str(), GL_VERTEX_SHADER);
	    if (0 == shaderId){
			const char *errmsg = "Error: Vertex shader failed to compile";
		    std::cerr << errmsg << std::endl;
			throw std::runtime_error(errmsg);
	    }
	    _vertexShaders.push_back(shaderId);
    }
    else {
        std::cerr << "Error: Failed to find vertex shader!\n";
    }
}

void GLSLProgramObject::attachFragmentShaderString(const char* shaderprogram)
{
	const GLuint shaderId = createShaderFromString(shaderprogram, nullptr, GL_FRAGMENT_SHADER);
	if (shaderId == 0){
	    std::cerr << "Error: Fragment shader failed to compile!\n";
    }
    else{
	    _fragmentShaders.push_back(shaderId);
    }
}

void GLSLProgramObject::attachFragmentShader(const char* filename)
{
    std::string resolved_path;
    if (sdkPath.getFilePath( filename, resolved_path)) {
		GLuint shaderId = createShader(resolved_path.c_str(), GL_FRAGMENT_SHADER);
	    if (shaderId == 0) {
			const char *errmsg = "Error: Fragment shader failed to compile!\n";
		    std::cerr << errmsg;
			return; 
	    }
	    _fragmentShaders.push_back(shaderId);
    }
	else{
		std::cerr << "Error: Failed to find fragment shader\n";
	}
}

void GLSLProgramObject::attachGeometryShaderString(const char* shaderprogram)
{
	GLuint shaderId = createShaderFromString(shaderprogram, NULL, GL_GEOMETRY_SHADER);
	if (shaderId == 0){
	    const char *errmsg = "Error: Geometry shader failed to compile\n";
	    std::cerr << errmsg;
		throw std::runtime_error(errmsg);
    }
	_geometryShaders.push_back(shaderId);
}

void GLSLProgramObject::attachGeometryShader(const char* filename)
{
    std::string resolved_path;
    if (sdkPath.getFilePath( filename, resolved_path)) {
		//resetShadersGlobalMacros();
		//for (size_t i=0; i<mMacroList.size(); i++)
		//	setShadersGlobalMacro(mMacroList[i].macro.c_str(), mMacroList[i].value.c_str());
		GLuint shaderId = createShader(resolved_path.c_str(), GL_GEOMETRY_SHADER);
	    if (shaderId == 0) {
			const std::string errmsg = "Error: Geometry shader failed to compile\n";
			std::cerr << errmsg;
		    throw std::runtime_error(errmsg);
	    }
	    _geometryShaders.push_back(shaderId);
    }
	else{
		std::cerr << "Error: Failed to find geometry shader\n";
	}
}


void GLSLProgramObject::link()
{
	if (_progId == 0) _progId = glCreateProgram();

    for (unsigned i = 0; i < _vertexShaders.size(); i++) 
        glAttachShader(_progId, _vertexShaders[i]);
    for (unsigned i = 0; i < _fragmentShaders.size(); i++) 
        glAttachShader(_progId, _fragmentShaders[i]);
    for (unsigned i = 0; i < _geometryShaders.size(); i++) 
        glAttachShader(_progId, _geometryShaders[i]);

    glLinkProgram(_progId);
    GLint success = GL_FALSE;
    glGetProgramiv(_progId, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        char tempbuf[1024] = "";
        glGetProgramInfoLog(_progId, sizeof(tempbuf), NULL, tempbuf);
        fprintf(stderr, "Failed to link program:\n%s\n", tempbuf);
    }
}

void GLSLProgramObject::bind()
{
	glUseProgram(_progId);
}

void GLSLProgramObject::unbind()
{
	glUseProgram(0);
}

void GLSLProgramObject::setUniform(const char* name, const GLfloat* val, int count)
{
    if (name == NULL || val == NULL || count <= 0){
        std::cerr << "Invalid parameter(s) in file " << __FILE__ << ", line " << __LINE__ <<"\n";
        return;
    }
	const GLint id = glGetUniformLocation(_progId, name);
	if (id == -1){
        std::cerr << "Name: " << name << " not found in shader program!\n";
		return;
	}

    if (count % 4 == 0){
		glUniform4fv(id, count >> 2, val);
    }
    else if (count % 3 == 0){
		glUniform3fv(id, count / 3, val);
    }
    else if (count % 2 == 0){
		glUniform2fv(id, count >> 1, val);
    }
    else{
        glUniform1fv(id, count, val);
	}
}

void GLSLProgramObject::setUniform(const char* name, const GLint* val, int count)
{
    if (name == NULL || val == NULL || count <= 0){
        std::cerr << "Invalid parameter(s) in file " << __FILE__ << ", line " << __LINE__ <<"\n";
        return;
    }
	const GLint id = glGetUniformLocation(_progId, name);
	if (id == -1){
        std::cerr << "Name: " << name << " not found in shader program!\n";
		return;
	}

    if (count % 4 == 0){
		glUniform4iv(id, count >> 2, val);
    }
    else if (count % 3 == 0){
		glUniform3iv(id, count / 3, val);
    }
    else if (count % 2 == 0){
		glUniform2iv(id, count >> 1, val);
    }
    else{
        glUniform1iv(id, count, val);
	}
}

void GLSLProgramObject::setTextureUnit(const char* texname, int texunit)
{
	GLint linked;
	glGetProgramiv(_progId, GL_LINK_STATUS, &linked);
	if (linked != GL_TRUE) {
		std::cerr << "Error: setTextureUnit needs program to be linked.\n";
		return;
	}
	const GLint id = glGetUniformLocation(_progId, texname);
	if (id == -1) {
		std::cerr << "Warning: Invalid texture " << texname << std::endl;
		return;
	}
	glUniform1i(id, texunit);
}

void GLSLProgramObject::bindTexture(GLenum target, const char* texname, GLuint texid, int texunit)
{
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(target, texid);
	setTextureUnit(texname, texunit);
	glActiveTexture(GL_TEXTURE0);
}

void GLSLProgramObject::bindFragDataLocation(GLuint colorNumber,  const char* name)
{
	if (_progId == 0){
		_progId = glCreateProgram();
	}

    glBindFragDataLocationEXT(_progId,  colorNumber,  name);
}

GLint GLSLProgramObject::getFragDataLocation(const char* name)
{
	if (!_progId) return -1;
    return glGetFragDataLocationEXT(_progId,  name);
}

void GLSLProgramObject::setMacro(const char *macro, const char *value)
{
	bool hit = false;
	for (size_t i=0; i<mMacroList.size(); i++){
		ShaderMacroStruct& m = mMacroList[i];
		if (m.macro == macro){
			m.value = value;
			hit = true;
			break;
		}
	}

	if (!hit){
		ShaderMacroStruct m;
		m.macro = macro;
		m.value = value;
		mMacroList.push_back(m);
	}
}

}