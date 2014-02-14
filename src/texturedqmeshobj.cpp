//File: texturedqmeshobj.cpp

#include <objrw.h>
#include <ogldrawall.h>
#include <geomath.h>
#include "texturedqmeshobj.h"



//This function draw a quad using one normal
inline void glDrawTexturedQuad(
	const Vector3d *pVertex, const Vector3f& norm, 
	const Vector3f *t0, const Vector3f *t1, const Vector3f *t2, const Vector3f *t3,
	const Vector4i &tri, const bool reversenorm=false)
{
	Vector3f n0=norm;
	if (reversenorm) n0=-n0;
	const Vector3d *v0= &pVertex[tri.x];
	const Vector3d *v1= &pVertex[tri.y];
	const Vector3d *v2= &pVertex[tri.z];
	const Vector3d *v3= &pVertex[tri.w];
	if (t0){
		glNormal3fv(&n0.x);
		glTexCoord2fv(&t0->x);
		glVertex3dv(&v0->x);
		glTexCoord2fv(&t1->x);
		glVertex3dv(&v1->x);
		glTexCoord2fv(&t2->x);
		glVertex3dv(&v2->x);
		glTexCoord2fv(&t3->x);
		glVertex3dv(&v3->x);
	}
	else{
		glNormal3fv(&n0.x);
		glVertex3dv(&v0->x);
		glVertex3dv(&v1->x);
		glVertex3dv(&v2->x);
		glVertex3dv(&v3->x);
	}

}


inline void glDrawTexturedQuad(
	const Vector3d *pVertex, const Vector3f *pNorm, 
	const Vector3f *t0, const Vector3f *t1, const Vector3f *t2, const Vector3f *t3, 
	const Vector4i &tri, const bool reversenorm=false)
{
	Vector3f n0=pNorm[tri.x], n1=pNorm[tri.y];
	Vector3f n2=pNorm[tri.z], n3=pNorm[tri.w];
	if (reversenorm){
		n0=-n0, n1=-n1, n2=-n2, n3=-n3;
	}
	const Vector3d *v0= &pVertex[tri.x];
	const Vector3d *v1= &pVertex[tri.y];
	const Vector3d *v2= &pVertex[tri.z];
	const Vector3d *v3= &pVertex[tri.w];
	if (t0){
		glNormal3fv(&n0.x);
		glTexCoord2fv(&t0->x);
		glVertex3dv(&v0->x);
		glNormal3fv(&n1.x);
		glTexCoord2fv(&t1->x);
		glVertex3dv(&v1->x);
		glNormal3fv(&n2.x);
		glTexCoord2fv(&t2->x);
		glVertex3dv(&v2->x);
		glNormal3fv(&n3.x);
		glTexCoord2fv(&t3->x);
		glVertex3dv(&v3->x);
	}else{
		glNormal3fv(&n0.x);
		glVertex3dv(&v0->x);
		glNormal3fv(&n1.x);
		glVertex3dv(&v1->x);
		glNormal3fv(&n2.x);
		glVertex3dv(&v2->x);
		glNormal3fv(&n3.x);
		glVertex3dv(&v3->x);
	}
}


inline void 
glDrawTexturedQuad(const Vector3d *pVertex, const Vector3f *pVertNorm, const Vector3f *pTexCoord, const Vector4i& tri, const Vector4i& texindex, const bool reversenorm=false)
{
	const Vector3f *t0 = &pTexCoord[texindex.x];
	const Vector3f *t1 = &pTexCoord[texindex.y];
	const Vector3f *t2 = &pTexCoord[texindex.z]; 
	const Vector3f *t3 = &pTexCoord[texindex.w]; 
	glDrawTexturedQuad(pVertex, pVertNorm, t0, t1, t2, t3, tri, reversenorm);
}


inline void 
glDrawTexturedQuad(const Vector3d *pVertex, const Vector3f& polyNorm, const Vector3f *pTexCoord, const Vector4i& tri, const Vector4i& texindex, const bool reversenorm=false)
{
	const Vector3f *t0 = &pTexCoord[texindex.x];
	const Vector3f *t1 = &pTexCoord[texindex.y];
	const Vector3f *t2 = &pTexCoord[texindex.z]; 
	const Vector3f *t3 = &pTexCoord[texindex.w]; 
	glDrawTexturedQuad(pVertex, polyNorm, t0, t1, t2, t3, tri, reversenorm);
}


inline void 
glDrawTexturedQuadArray(const Vector3d *pVertex, const Vector3f *pNorm, const Vector3f *pTexCoord, const Vector4i* pTri, const int ntri, const Vector4i *pTexIndex, const bool reversenorm=false)
{
	glBegin(GL_QUADS);
	for (int i=0; i<ntri; i++){
		const Vector4i tri = pTri[i];
		const Vector4i texindex = pTexIndex[i];
		glDrawTexturedQuad(pVertex, pNorm, pTexCoord, tri, texindex, reversenorm);
	}
	glEnd();
}


inline void 
glDrawTexturedQuadArrayPolyNorm(const Vector3d *pVertex, const Vector3f *pPolyNorm, const Vector3f *pTexCoord, const Vector4i *pTri, const int ntri, const Vector4i *pTexIndex, const bool reversenorm=false)
{
	glBegin(GL_QUADS);
	for (int i=0; i<ntri; i++){
		const Vector4i tri = pTri[i];
		const Vector4i texindex = pTexIndex[i];
		const Vector3f polyNormal = pPolyNorm[i];
		glDrawTexturedQuad(pVertex, polyNormal, pTexCoord, tri, texindex, reversenorm);
	}
	glEnd();
}


void CTexturedQuadObj::_drawShadedSurfaces(const int startfaceid, const int facelen, const bool enableTexture)
{
	//setup normal;
	checkNormals(false);

	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_BLEND);

	//setup lighting
	if (m_pDrawParms->m_bEnableLighting){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
	}
	else
		glDisable(GL_LIGHTING);

	//draw the obj
	int *pPolygon = m_pPolygon + (4*startfaceid);
	if (m_pDrawParms->m_bEnableSmoothing){ //Draw per vertex normal
		if (!enableTexture){
			glDisable(GL_TEXTURE_2D);
			drawSurfaceUsingArray(m_pVertex, m_pVertexNorm, pPolygon, facelen, m_nPlyVertexCount);
		}
		else{
			glEnable(GL_TEXTURE_2D);
			const Vector3f *pTexCoord = &((*m_pTexCoord)[0]);
			const Vector4i* pTri = (const Vector4i*)pPolygon;
			const Vector4i* pTexIndex = &m_pTexIndex[startfaceid];
			const bool reversenorm=false;
			glDrawTexturedQuadArray(m_pVertex, m_pVertexNorm, pTexCoord, pTri, facelen, pTexIndex, reversenorm);
		}
	}
	else{//Draw per polygon normal
		if (enableTexture)
			glEnable(GL_TEXTURE_2D);
		else{
			glDisable(GL_TEXTURE_2D);
		}
		
		Vector3f *pTexCoord=NULL;
		if (m_pTexCoord && (*m_pTexCoord).size()>0) 
			pTexCoord = &((*m_pTexCoord)[0]);
		const Vector4i* pTri = (const Vector4i*)pPolygon;
		const Vector4i* pTexIndex = &m_pTexIndex[startfaceid];
		const bool reversenorm=false;
		const Vector3f *pPolyNorm = &m_pPolyNorm[startfaceid];
		glDrawTexturedQuadArrayPolyNorm(m_pVertex, pPolyNorm, pTexCoord, pTri, facelen, pTexIndex, reversenorm);
	}
}


void CTexturedQuadObj::glDraw(const CGLDrawParms &dparms)
{
	m_pDrawParms = (CGLDrawParms*)&dparms;
	const bool bTexture = dparms.m_bEnableTexture2D;
	bool rendered = false;

	//to avoid the interferance from 1D texturing I dev. before
	glDisable(GL_TEXTURE_1D);

	if ((m_nFaceMatIndex>0) && (dparms.m_nDrawType==CGLDrawParms::DRAW_MESH_SHADING)){
		rendered = true;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		for (int i=0; i<m_nFaceMatIndex; i++){
			//decode mat pos and matid
			int x0, x1, matid;
			decodeMaterialIndex(i, m_FaceMatIndex, m_nFaceMatIndex, m_nPolygonCount, x0, x1, matid);
			const int facelen = x1 - x0;
			const bool enableTexture = bTexture && materials[matid].hasTexture();
			materials[matid].setupMaterial(enableTexture);
			_drawShadedSurfaces(x0, facelen, enableTexture);
			materials[matid].unsetupMaterial(enableTexture);
		}
		//draw the vertices if required
		if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
		    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
			CPolyObj::glDrawVertices(dparms.m_cVertexColor);
		}
		//draw the normals if required
		if (dparms.m_bShowVertexNormal){
			const float normlength = dparms.m_fNormalLengthScale;
			const Vector3f normcolor = dparms.m_cNormalColor;
			CPolyObj::glDrawVertexNormals(normlength, normcolor);
		}
	}

	//default rendering method
	if (!rendered){
		glDisable(GL_TEXTURE_2D);
		CPolyObj::glDraw(dparms);
	}
}


bool CTexturedQuadObj::saveMeshWavefrontObj(const char *fname, const double *matrix)
{
	/*
	int i;

	Vector3d *pVertex = new Vector3d[m_nVertexCount];
	assert(pVertex!=NULL);
	for (i=0; i<m_nVertexCount; i++)
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, &pVertex[i].x);

	vector<OglMaterial> mats;
	for (i=0; i<m_nMaterials; i++){
		mats.push_back(this->materials[i]);
	}

	std::string objname = GetObjectName();
	if (objname.empty()) objname+="defaultname";
	Vector3f *pNormal = m_pVertexNorm;
	if (pNormal==NULL){
		ComputeVertexNormals();
		pNormal = m_pVertexNorm;
		assert(pNormal!=NULL);
	}

	const bool r = writeOBJFile(fname, objname,
		mats, 
		pVertex, m_nVertexCount,
		pNormal, m_nVertexCount, *m_pTexCoord,
		(const Vector3i*)m_pPolygon, m_nPolygonCount, m_pTexIndex,
		m_FaceMatIndex, m_nFaceMatIndex);

	delete []pVertex;
	*/
	bool r=false;
	return r;
}


//This function is to compress the occurance of the material references. There are cases 
//where a material is referenced several times in the chunk of a triangle list. This is not 
//optimized since we need to switch the context multiple times. Therefore, we need to perform 
//a scan on the index array to minimize the redundency. 
void CTexturedQuadObj::_reorgnizeFaceMatIndices(const unsigned int *pFaceMatIndex, const int len)
{
	unsigned int i;
	vector<int> facecounter;
	vector<Vector4i> reordered_tris;
	reordered_tris.resize(m_nPolygonCount);
	reordered_tris.clear();
	Vector4i *ptri = (Vector4i*)m_pPolygon;

	//Scan the array for each material 
	for (i=0; i<MAX_MATERIAL_NUMBER; i++){
		int c = 0;
		for (int j=0; j<len; j++){
			int x0, x1, matid;
			decodeMaterialIndex(j, pFaceMatIndex, len, m_nPolygonCount, x0, x1, matid);
			if (matid==i){
				for (int k=x0; k<x1; k++)
					reordered_tris.push_back(ptri[k]);
				c += (x1-x0);
			}
		}
		facecounter.push_back(c);
	}

	//copy the re-ordered triangles
	for (i=0; i<m_nPolygonCount; i++)
		ptri[i]=reordered_tris[i];

	//regenerate the index array
	int c=0;
	m_nFaceMatIndex = 0;
	for (i=0; i<facecounter.size(); i++){
		if (facecounter[i]>0){
			m_FaceMatIndex[m_nFaceMatIndex++]=encodeMaterialIndex(c, i);
			c+=facecounter[i];
		}
	}
	assert(m_nFaceMatIndex<MAX_MATERIAL_NUMBER);
	printf("Reduce group number from %d to %d.\n", len, m_nFaceMatIndex);

}
