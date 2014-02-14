//main.cpp
#include "MeshRenderLib.h"

namespace MeshRender{
    extern void initAllMeshRenderingContexts();
}

extern "C"{
	int meshrender_init()
	{
		return 0;
	}

	//OpenGL related initialization, should be called when a GL context is setup
	int meshrender_initGL()
	{
        MeshRender::initAllMeshRenderingContexts();

		return 0;
	}
}

