//File: texturedqmeshobj.h

#ifndef __INC_TEXTURED_QUAD_MESH20071214
#define __INC_TEXTURED_QUAD_MESH20071214

#include <sysconf.h>
#include "oglmaterial.h"
#include "qmeshobj.h"


#define MAX_MATERIAL_NUMBER 40

class CTexturedQuadObj: public CQuadObj
{
protected:
	OglMaterial materials[MAX_MATERIAL_NUMBER];		//Mat lib
	int m_nMaterials;								//Number of materials used
	
	vector<Vector3f> *m_pTexCoord;					//Texture coordinates can be more or less than vertex number
	Vector4i *m_pTexIndex;							//Texture index for each triangle
	unsigned int m_FaceMatIndex[MAX_MATERIAL_NUMBER];//The material indice for polygons, each is encoded in 
													// (27bit position, 5bit matid)
	int m_nFaceMatIndex;							//The length of the index array, 

private:
	void _drawShadedSurfaces(const int startfaceid, const int facelen, const bool enableTexture=false);
	void _reorgnizeFaceMatIndices(const unsigned int *pFaceMatIndex, const int len);


public:	

	CTexturedQuadObj(Vector3d *pVertex, const int nVertex, Vector4i *pBoundaryTri, const int nTri, const bool allocbuffer=false):
		CQuadObj(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer)
	{
		m_nMaterials = 0;
		m_pTexCoord = NULL;
		m_pTexIndex = NULL;
	}

	virtual ~CTexturedQuadObj(void)
	{
		SafeDelete(m_pTexCoord);
		SafeDelete(m_pTexIndex);
	}

	void assignTextureCoords(vector<Vector3f> *pTexCoord, Vector4i *pTexIndex)
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

};

#endif