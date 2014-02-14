//File: GPURendering.cpp

#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>
#include <crest/ogl/GPURendering.h>
#include <crest/io/nametableitem.h>


namespace cia3d{
namespace ogl{

CIA3D_DECL_CLASS(GPURendering);

static unsigned char GPURenderingClass = 
	cia3d::core::RegisterObject("GPURendering")
	.add<GPURendering>()
	.addAlias("GPURendering");


void GPURendering::_init(void)
{
}

void GPURendering::parse(ZBaseObjectDescription* arg )
{
	char* opt[]={0,0};
	//call the parent parse
	BaseRendering::parse(arg);
	//return;
	//local setup
	QDomElement element = arg->toElement();
	const bool cgshader=true;

	QString vertexshaderfile = element.attribute("vertexshaderfile");
	QString vertexshadermain = element.attribute("vertexshadermain");
	QString vertexshaderopt = element.attribute("vertexshaderopt");
	opt[0] = (char*) vertexshaderopt.ascii();

	QString geometryshaderfile = element.attribute("geometryshaderfile");
	QString geometryshadermain = element.attribute("geometryshadermain");
	QString geometryshaderopt = element.attribute("geometryshaderopt");
	opt[0] = (char*) geometryshaderopt.ascii();

	QString fragmentshaderfile = element.attribute("fragmentshaderfile");
	QString fragmentshadermain = element.attribute("fragmentshadermain");
	QString fragmentshaderopt = element.attribute("fragmentshaderopt");
	opt[0] = (char*) fragmentshaderopt.ascii();

	setGPUPrograms(vertexshaderfile, vertexshadermain, 
				geometryshaderfile, geometryshadermain, 
				fragmentshaderfile, fragmentshadermain);
}


void GPURendering::setGPUPrograms(  const char *vp, const char *vpmain, 
									const char *gp, const char *gpmain, 
									const char *fp, const char *fpmain)
{
	const char** opt = NULL;
	const bool cgshader=true;
	m_gpuProgram.addShader(cgshader, CG_GL_VERTEX, vp, vpmain, opt);
	m_gpuProgram.addShader(cgshader, CG_GL_GEOMETRY, gp, gpmain, opt);
	m_gpuProgram.addShader(cgshader, CG_GL_FRAGMENT, fp, fpmain, opt);

	//init
	_init();
}

} // namespace ogl
} // namespace cia3d
