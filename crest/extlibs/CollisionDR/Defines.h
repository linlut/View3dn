/////////////////////////////////////////////////////////////////////////////
/// Collision Detection and Response
/// CREST 2008
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// This is the General Difination File.
/// Include this file if the following context is to be refered.
//////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __DEFINES_H__
#define __DEFINES_H__

//#include "StdAfx.h"
//#include <afxwin.h> 
/// OpenGL head files...
//#include "gl/gl.h"
//#include "gl/glu.h"
//#include "gl/glaux.h"

/// Default file/path
///#define BMP_DEF "C:/users/syh/res/chole2.bmp"
///#define RES_PATH "C:/Users/syh/Res"
#define RES_PATH "C:/MISScene"
#define BMP_DEF "C:/MISScene/Texture/chole2.bmp"
///#define BMP_DEF "C:/MISScene/Texture/Mesh texture.bmp"

/// Define the PI constant
#ifndef DEFINES_PI
#define DEFINES_PI
#define PI 3.1415926535897932384626
#endif DEFINES_PI

/// PHANToM definitions
#define PHANTOM_NAME 	"Default PHANToM"  ///< PHANToM configuration string.
#define PHANTOM_NAME_1  "Default PHANToM"
//#define PHANTOM_NAME_1  "PHANToM 1"
#define PHANTOM_NAME_2  "PHANToM 2"
#define PHANTOM_NAME_3  "PHANToM 3"
#define PHANTOM_NAME_4  "PHANToM 4"
#define PHANTOM_STYLUS_SWITCHING 1 ///< Use this switch to enable/disable force rendering
#define WORKSPACE_NAME	"WorkspaceBounds" ///< Apply it if the entire workspace is bounded by a cube

/// Maximum PHANToM devices
#define HAPTICS_MAX 4

/// Hardware types
#define DEVICE_TYPE_MK 0 ///< Mouse & Keyboard
#define DEVICE_TYPE_PH 1 ///< PHANToMs

/// Maximum objects in the scene...
#define TISSUE_MAX 64	
#define INSTRUMENT_MAX HAPTICS_MAX ///< no less than 4 for hardware config
#define RIGID_MAX 64
#define VISUAL_MAX 36
#define OBJECT_MAX INSTRUMENT_MAX + RIGID_MAX + TISSUE_MAX
#define CNODES_MAX 1	///< Maximum control nodes.
#define RNODES_MAX 1000	///< Maximum response nodes.
//#define GNODES_MAX 36	///< Maximum grasping nodes.


/// TYPE OF OBJECTS IN THE SCENE
#define INSTRUMENT_TYPE 0
#define RIGID_TYPE 1
#define TISSUE_TYPE 2 
#define PARTI_TYPE 3 ///< particle system simulation
#define VISUAL_TYPE 4 ///< Only for graphic rendering

/// TYPE OF MESH STRUCTURES
#define MESH_DL 1
#define MESH_SEG 2
#define MESH_SHELL 0
#define MESH_TET 3

/// Collision detection channels
#define OM_CD 1 ///< Spatial tessellation method (Occupancy map)
#define ONENODE_CD 0 ///< one tip node, brute force
#define NONE_CD -1 ///< No collision detection
/// Collision types
#define I2I 1
#define I2R 2
#define I2T 3
#define R2R 4
#define R2T 5
#define T2T 6

#define FORCE_DEFAULT_ONOFF 0 ///< 0/1: Default setting, force feedback on/off 
#define MODEL_DEF 1 ///< Enable force feedback
#define FORCEDB_DEF 3E-5	///< Force feedback gain, 3E-4 for phantom pos/10
#define NOFORCE -3 ///< Zero force feedback
#define CALIBRATE_HAPTIC 1 ///< Always perform haptic device calibration per initialization

/// Data types
#define CDFloat double
#define DFloat double
#define DWORD unsigned int
#define BOOL unsigned char
#define TRUE 1
#define FALSE 0
#define TRACE printf
//#define long int


/// Structure definition
/// Vertex structure
#ifndef vertexStructure
#define vertexStructure
typedef struct{
	union {
		CDFloat x;
		long p1;
	};
	union {
		CDFloat y;
		long p2;
	};
	union {
		CDFloat z;
		long p3;
	};
}vertex;
#endif

#ifndef TetralStructure
#define TetralStructure
typedef struct{
	long iNodes[4];
	float fVolume0;
	float fVolume;
	float fAreas0[4];
	float fAreas[4];
}Tetral;
#endif

/// OpenGL material structure
#ifndef materialStructure
#define materialStructure
typedef struct{
	float ambientIntensity[4]; ///< ambient light
	float diffuseColor[4]; ///< diffuse light
	float emissiveColor[4]; ///< self-lighting
	float shininess; ///< the extent of reflection
	float specularColor[4]; ///< point light
	float transparency;
}tMaterial;
#endif

/// Triangle physics structure - data carrier for collision response
#ifndef trianglePhysicsStructure
#define trianglePhysicsStructure
typedef struct{
		vertex pos[3];
		vertex v[3];
		vertex f[3];
		vertex n; ///< outbound normal
		float  Mass[3];
		long id[3]; ///< indices of three triangle vertices
}tTrianglePhy;
#endif

	/// Workspace
	#define MODEL_XTEST 1
	#define MODEL_YTEST 1
	#define MODEL_ZTEST 1

	#define WORKSPACE_XMIN -210.0
	#define WORKSPACE_XMAX 210.0
	#define WORKSPACE_YMIN -210.0
	#define WORKSPACE_YMAX 280.0
	#define WORKSPACE_ZMIN -210.0
	#define WORKSPACE_ZMAX 210.0

	//#define WORKSPACE_XMIN -110.0
	//#define WORKSPACE_XMAX 110.0
	//#define WORKSPACE_YMIN -110.0
	//#define WORKSPACE_YMAX 110.0
	//#define WORKSPACE_ZMIN -110.0
	//#define WORKSPACE_ZMAX 110.0

	#define SIZE_X 22	///< Nodes in x axis of the run time created object.
	#define SIZE_Y 22	///< Nodes in y axis of the run time created object.

/// @name Obsolete
//@{ 
	#define NUM_VERTICES SIZE_X*SIZE_Y				/// Obsolete, for gstTriPolyMesh.
	#define NUM_TRIANGLES (SIZE_X-1)*(SIZE_Y-1)*2	/// Obsolete, for gstTriPolyMesh.
	#define WIN_SIZE_X 600	
	#define WIN_SIZE_Y 600	

	/// Structure of mass
	typedef struct{
		float x[3];
		float v[3];
		float f[3];
		long nail;
	}MASS;

	/// Structure of spring
	typedef struct{
		long i,j;
		float r;
	}SPRING;
//@}


#endif	// End of the .h file ///////////////////////////////////////////////