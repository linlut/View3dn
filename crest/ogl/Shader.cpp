//shader.CPP
#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <crest/ogl/Shader.h>
#include <Cg/cg.h>
#include <CG/cgGL.h>
//#include <Cg/cg_profiles.h>

namespace cia3d{
namespace ogl{

CGShader::~CGShader(void)
{
	cgDestroyProgram(m_program); 
}

void CGShader::init(const char *filename, const char *funcname, const char **options)
{
	char* gpopt[3]={0,"-profileopts Vertices=16",0};
	assert(cgIsContext(*m_pCgContext));
	//if (*m_pCGProfile == cgGLGetLatestProfile(CG_GL_GEOMETRY)){
	//	options = &gpopt[0];
	//}
    m_program = cgCreateProgramFromFile(*m_pCgContext, CG_SOURCE, filename, *m_pCGProfile, funcname, (const char**)gpopt);
    if (!cgIsProgramCompiled(m_program))
        cgCompileProgram(m_program);
    cgGLLoadProgram(m_program);
}

void CGShader::setMaxGeoOutputVertices(const int n)
{
	/*
	GLint k; 
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &k); 
	int r=n;
	if (r>k) r=k;
	GLuint prog = m_program;
	glProgramParameteriEXT(prog, GL_GEOMETRY_VERTICES_OUT_EXT, k); 
	*/
}

//==============================================================
void GPUProgram::_chooseProfiles(void)
{
    // Do one-time setup only once; setup Cg programs and textures
    // and set up OpenGL state.
	vertexProfile   = cgGLGetLatestProfile(CG_GL_VERTEX);
    fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    geometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
    cgGLSetOptimalOptions(vertexProfile);
    cgGLSetOptimalOptions(fragmentProfile);
    cgGLSetOptimalOptions(geometryProfile);
}

static void handleCgError(void) 
{
    CGerror err = cgGetError();
    fprintf(stderr, "Cg error: %s\n", cgGetErrorString(err));
    fflush(stderr);
    exit(0);
}

GPUProgram::GPUProgram(void)
{
    cgSetErrorCallback(handleCgError);
    context = cgCreateContext();
    if (!cgIsContext(context)){
		fprintf(stderr, "Cg Error: cannot create context!\n");
		exit(1);
	}
	_chooseProfiles();
}

GPUProgram::~GPUProgram(void)
{
	for (unsigned int i=0; i<m_shaders.size(); i++){
		delete m_shaders[i];
		m_shaders[i] = NULL;
	}
	m_shaders.clear();

	cgDestroyContext(context); 
}

int GPUProgram::addShader(const bool cgshader, const unsigned int shadertype, const char *filename, const char* functionname, const char **options)
{
	Shader *p=NULL;
	void *profile=NULL;

	if (cgshader){
		switch(shadertype){
		case CG_GL_VERTEX:
			profile = &vertexProfile;
			break;
		case CG_GL_FRAGMENT:
			profile = &fragmentProfile;
			break;
		case CG_GL_GEOMETRY:
			profile = &geometryProfile;
			break;
		default:
			return -1;
			break;
		};
		p = new CGShader(shadertype, &context, profile);
	}else{
		printf("Not impl!\n");
		assert(0);
	}

	assert(p!=NULL);
	p->init(filename, functionname, options);
	const int id = m_shaders.size();
	m_shaders.push_back(p);
	return id;
}

}//namespace ogl
}//cia3d