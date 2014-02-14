#pragma once
#ifndef MESH_RENDER_LIB_H
#define MESH_RENDER_LIB_H

#ifdef __cplusplus
	#include "./MeshRender.h"
#endif

extern "C"{
	//init function for application startup
	int meshrender_init(void);

	//OpenGL related initialization, should be called when a GL context is setup
	int meshrender_initGL();
}

#endif
