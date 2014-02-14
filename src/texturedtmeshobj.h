//File: texturedtmeshobj.h
#ifndef __INC_TEXTURED_TRIANGLE_MESH20071214
#define __INC_TEXTURED_TRIANGLE_MESH20071214
#include <sysconf.h>
#include "oglmaterial.h"
#include "tmeshobj.h"


#define MAX_MATERIAL_NUMBER 40

class CGrasperHead;

class CTexturedTriangleObj: public CTriangleObj
{
protected:
	OglMaterial materials[MAX_MATERIAL_NUMBER];		//Mat lib
	int m_nMaterials;								//Number of materials used
	
	vector<Vector3f> *m_pTexCoord;					//Texture coordinates can be more or less than vertex number
	Vector3i *m_pTexIndex;							//Texture index for each triangle
	unsigned int m_FaceMatIndex[MAX_MATERIAL_NUMBER];//The material indice for polygons, each is encoded in 
													// (27bit position, 5bit matid)
	int m_nFaceMatIndex;							//The length of the index array, 

public:
	//===================================================================
	CGrasperHead *m_pGrasperHead;
	bool * m_pMedialBuffer; 

private:
	void _drawShadedSurfaces(const int startfaceid, const int facelen, const bool enableTexture=false);
	void _reorgnizeFaceMatIndices(const unsigned int *pFaceMatIndex, const int len);
	void _initJaws(
			const Vector3d *pVertex, const int nVertex, const int *pPoly, const int nPolygon, const int plytype, 
			const OglMaterial material[], const unsigned int faceMatIndex[], const int nFaceMatIndex);

public:	
	CTexturedTriangleObj(Vector3d *pVertex, const int nVertex, Vector3i *pBoundaryTri, const int nTri, const bool allocbuffer=false):
		CTriangleObj(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer)
	{
		m_nMaterials = 0;
		m_pTexCoord = NULL;
		m_pTexIndex = NULL;

		m_pGrasperHead = NULL;
		m_pMedialBuffer = NULL; 

	}

	virtual ~CTexturedTriangleObj(void);

	void assignTextureCoords(vector<Vector3f> *pTexCoord, Vector3i *pTexIndex)
	{
		m_pTexCoord = pTexCoord;
		m_pTexIndex = pTexIndex;
	}

	vector<Vector3f>* getTextureCoords(void)
	{
		return m_pTexCoord;
	}

	void assignMaterials(const OglMaterial* mat, const int nmat)
	{
		m_nMaterials = nmat;
		if (m_nMaterials>MAX_MATERIAL_NUMBER){
			printf("Error: too many materials input!\n");
			m_nMaterials = MAX_MATERIAL_NUMBER;
		}
		for (int i=0; i<m_nMaterials; i++)
			materials[i] = mat[i];
	}

	void assignFaceMatIndices(const unsigned int *pFaceMatIndex, const int len)
	{	

		if (len>MAX_MATERIAL_NUMBER){
			printf("Too many material indices, reorgnize them.\n");
			_reorgnizeFaceMatIndices(pFaceMatIndex, len);
		}
		else{
			m_nFaceMatIndex = len;
			for (int i=0; i<len; i++)
				m_FaceMatIndex[i] = pFaceMatIndex[i];
		}
	}

	virtual void glDraw(const CGLDrawParms &dparms);
	
	virtual bool saveMeshWavefrontObj(const char *fname, const double *matrix=NULL);

	//===========Lap. Tool functions ====================
	virtual void initJaws(void);

	virtual void updateToolHeadAngle(const int dir);

};


#endif