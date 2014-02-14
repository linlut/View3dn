//FILE: volumemeshio.h

#ifndef _INC_VOLUMEMESH_IO_H_
#define _INC_VOLUMEMESH_IO_H_

#include <vector3d.h>

//Neutral Mesh is exported by NetGen
bool loadNeutralMeshFile(const char *fname, 
	Vector3d *& pVertex, int &nv, 
	int *& pFace, int &ntri,
	int *& pTet, int &ntet, int &meshtype);
	
//HMAscii is a file format used in Amira for hyper mesh data export?
bool loadHMAsciiMeshFile(const char *fname, 
	Vector3d *& pVertex, int &nv, 
	int *& pFace, int &ntri,
	int *& pTet, int &ntet, int &meshtype);

//The off file format for 3D volumetric meshes
bool loadOffMeshFile(const char *fname, 
	Vector3d *& pVertex, int &nv, 
	float *& pIsoValue,
	int *& pFace, int &ntri, int *& pTet, int &ntet, 
	int &meshtype);


#endif