//FILE: objrw.h
//Read/Write Wavefront OBJ file format

#ifndef _INC_WAVEFRONT_OBJRW_H_
#define _INC_WAVEFRONT_OBJRW_H_

#include <vector>
#include <string>

#include <vectorall.h>
#include <oglmaterial.h>

using namespace std;



inline unsigned int encodeMaterialIndex(const int facepos, const int matid)
{
	unsigned int x = (unsigned int)facepos;
	unsigned int y = (unsigned int)matid;
	return (x<<5) + y;
}


inline void decodeMaterialIndex(
	const int i,						//Index in the array
	const unsigned int *faceMatIndex,	//Array, where each entry-a int, (27bit position, 5bit matid)
	const int nFaceMatIndex,			//The length of the index array, 
	const int nPolygon,					//The length of the polygon buffer (# of polygons)
	int &x0, int &x1, int &matid)
{
	if (i<0 || i>=nFaceMatIndex){
		x0 = x1 = 0;
		matid = 0;
		return;
	}
	if (i==nFaceMatIndex-1){
		x0 = faceMatIndex[i]>>5;
		x1 = nPolygon;
		matid = faceMatIndex[i] & 0x1F;
		return;
	}

	x0 = faceMatIndex[i]>>5;
	x1 = faceMatIndex[i+1]>>5;
	matid = faceMatIndex[i] & 0x1F;
}


extern bool readOBJFile(const char *fname, 
	vector<OglMaterial> &materials, 
	vector<Vector3d> &vertices,
	vector<Vector3f> &normals,		
	vector<Vector3f> &texCoords, 
	vector< vector < vector <int> > > &facets,
	unsigned int *pFaceMatIndex,	//The material index buffer, it indicates which polygons are using which mats
	int &nFaceMatIndex);			//The length of the array


extern bool writeOBJFile(
		const char *filename,				//output object file name 
		const std::string& objname,
		vector<OglMaterial>& materials, 
		const Vector3d *pVertex, const int nv, 
		const Vector3f *pNormal, const int nnorm, 
		const vector<Vector3f> &vtexcoords,
		const Vector3i *triangles, const int ntri,
		const Vector3i *pTexIndex,
		const unsigned int faceMatIndex[32],//The material indice for polygons, each is encoded in (27bit position, 5bit matid)
		const int nFaceMatIndex				//The length of the index array, 
	);


#endif