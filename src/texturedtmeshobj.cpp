//File: texturedtmeshobj.cpp

#include <objrw.h>
#include <ogldrawall.h>
#include "texturedtmeshobj.h"
#include "texturedqmeshobj.h"
#include "grasper.h"


//This function draw triangle using one normal
inline void glDrawTexturedTriangle(
	const Vector3d *pVertex, const Vector3f& norm, 
	const Vector3f *t0, const Vector3f *t1, const Vector3f *t2, 
	const Vector3i &tri, const bool reversenorm=false)
{
	Vector3f n0=norm;
	if (reversenorm) n0=-n0;
	const Vector3d *v0= &pVertex[tri.x];
	const Vector3d *v1= &pVertex[tri.y];
	const Vector3d *v2= &pVertex[tri.z];

	if (t0 && t1 && t2){
		glNormal3fv(&n0.x);
		glTexCoord2fv(&t0->x);
		glVertex3dv(&v0->x);
		glTexCoord2fv(&t1->x);
		glVertex3dv(&v1->x);
		glTexCoord2fv(&t2->x);
		glVertex3dv(&v2->x);
	}
	else{
		glNormal3fv(&n0.x);
		glVertex3dv(&v0->x);
		glVertex3dv(&v1->x);
		glVertex3dv(&v2->x);
	}
}


inline void glDrawTexturedTriangle(
	const Vector3d *pVertex, const Vector3f *pNorm, 
	const Vector3f *t0, const Vector3f *t1, const Vector3f *t2, 
	const Vector3i &tri, const bool reversenorm=false)
{
	Vector3f n0=pNorm[tri.x], n1=pNorm[tri.y], n2=pNorm[tri.z];
	if (reversenorm) n0=-n0, n1=-n1, n2=-n2;
	const Vector3d *v0= &pVertex[tri.x];
	const Vector3d *v1= &pVertex[tri.y];
	const Vector3d *v2= &pVertex[tri.z];

	if (t0 && t1 && t2){
		glNormal3fv(&n0.x);
		glTexCoord2fv(&t0->x);
		glVertex3dv(&v0->x);
		glNormal3fv(&n1.x);
		glTexCoord2fv(&t1->x);
		glVertex3dv(&v1->x);
		glNormal3fv(&n2.x);
		glTexCoord2fv(&t2->x);
		glVertex3dv(&v2->x);
	}
	else{
		glNormal3fv(&n0.x);
		glVertex3dv(&v0->x);
		glNormal3fv(&n1.x);
		glVertex3dv(&v1->x);
		glNormal3fv(&n2.x);
		glVertex3dv(&v2->x);
	}
}


inline void 
glDrawTexturedTriangle(const Vector3d *pVertex, const Vector3f *pVertNorm, const Vector3f *pTexCoord, const Vector3i& tri, const Vector3i& texindex, const bool reversenorm=false)
{
	const Vector3f *t0 = &pTexCoord[texindex.x];
	const Vector3f *t1 = &pTexCoord[texindex.y];
	const Vector3f *t2 = &pTexCoord[texindex.z]; 
	glDrawTexturedTriangle(pVertex, pVertNorm, t0, t1, t2, tri, reversenorm);
}


inline void 
glDrawTexturedTriangle(const Vector3d *pVertex, const Vector3f& polyNorm, const Vector3f *pTexCoord, const Vector3i& tri, const Vector3i& texindex, const bool reversenorm=false)
{
	const Vector3f *t0 = NULL;
	const Vector3f *t1 = NULL;
	const Vector3f *t2 = NULL;
	if (pTexCoord){
		t0 = &pTexCoord[texindex.x];
		t1 = &pTexCoord[texindex.y];
		t2 = &pTexCoord[texindex.z]; 
	}
	glDrawTexturedTriangle(pVertex, polyNorm, t0, t1, t2, tri, reversenorm);
}


inline void 
glDrawTexturedTriangleArray(const Vector3d *pVertex, const Vector3f *pNorm, const Vector3f *pTexCoord, const Vector3i* pTri, const int ntri, const Vector3i *pTexIndex, const bool reversenorm=false)
{
	glBegin(GL_TRIANGLES);
	for (int i=0; i<ntri; i++){
		const Vector3i& tri = pTri[i];
		const Vector3i& texindex = pTexIndex[i];
		glDrawTexturedTriangle(pVertex, pNorm, pTexCoord, tri, texindex, reversenorm);
	}
	glEnd();
}


inline void 
glDrawTexturedTriangleArrayPolyNorm(const Vector3d *pVertex, const Vector3f *pPolyNorm, const Vector3f *pTexCoord, const Vector3i *pTri, const int ntri, const Vector3i *pTexIndex, const bool reversenorm=false)
{
	glBegin(GL_TRIANGLES);
	for (int i=0; i<ntri; i++){
		const Vector3i& tri = pTri[i];
		const Vector3i& texindex = pTexIndex[i];
		const Vector3f polyNorm = pPolyNorm[i];
		glDrawTexturedTriangle(pVertex, polyNorm, pTexCoord, tri, texindex, reversenorm);
	}
	glEnd();
}


CTexturedTriangleObj::~CTexturedTriangleObj(void)
{
	SafeDelete(m_pTexCoord);
	SafeDelete(m_pTexIndex);

	SafeDelete(m_pGrasperHead);
	SafeDeleteArray(m_pMedialBuffer); 
}


void CTexturedTriangleObj::_drawShadedSurfaces(const int startfaceid, const int facelen, const bool enableTexture)
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
	int *pPolygon = m_pPolygon + (3*startfaceid);
	if (m_pDrawParms->m_bEnableSmoothing){ //Draw per vertex normal
		if (!enableTexture){
			glDisable(GL_TEXTURE_2D);
			drawSurfaceUsingArray(m_pVertex, m_pVertexNorm, pPolygon, facelen, m_nPlyVertexCount);
		}
		else{
			glEnable(GL_TEXTURE_2D);
			const Vector3f *pTexCoord = &((*m_pTexCoord)[0]);
			const Vector3i* pTri = (const Vector3i*)pPolygon;
			const Vector3i* pTexIndex = &m_pTexIndex[startfaceid];
			const bool reversenorm=false;
			glDrawTexturedTriangleArray(m_pVertex, m_pVertexNorm, pTexCoord, pTri, facelen, pTexIndex, reversenorm);
		}
	}
	else{//Draw per polygon normal
		if (enableTexture){
			glEnable(GL_TEXTURE_2D);
		}
		else{
			glDisable(GL_TEXTURE_2D);
		}
		const Vector3f *pTexCoord = NULL;
		if (m_pTexCoord && (*m_pTexCoord).size()>0) pTexCoord = &(*m_pTexCoord)[0];
		const Vector3i* pTri = (const Vector3i*)pPolygon;
		const Vector3i* pTexIndex = &m_pTexIndex[startfaceid];
		const bool reversenorm=false;
		const Vector3f *pPolyNorm = &m_pPolyNorm[startfaceid];
		glDrawTexturedTriangleArrayPolyNorm(m_pVertex, pPolyNorm, pTexCoord, pTri, facelen, pTexIndex, reversenorm);
	}
}


void CTexturedTriangleObj::glDraw(const CGLDrawParms &dparms)
{
	m_pDrawParms = (CGLDrawParms*)&dparms;
	const bool bTexture = dparms.m_bEnableTexture2D | dparms.m_bEnableTexture1D;

	//to avoid the interferance from 1D texturing I dev. before
	glDisable(GL_TEXTURE_1D);

	if ((m_nFaceMatIndex>0) && (dparms.m_nDrawType==CGLDrawParms::DRAW_MESH_SHADING)){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		for (int i=0; i<m_nFaceMatIndex; i++){
			//decode mat pos and 
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
		return;
	}

	//default rendering method
	if (m_nFaceMatIndex<0){
		//printf("No material defined.\n");
	}
	glDisable(GL_TEXTURE_2D);
	CPolyObj::glDraw(dparms);
}

static CPolyObj *loadMeshWavefrontObj(const char *fname)
{
	int i;
	CPolyObj *p = NULL;
	vector<OglMaterial> materials; 
	vector<Vector3d> vertices;
	vector<Vector3f> normals;
	vector<Vector3f> *pTexCoord = new vector<Vector3f>;
	assert(pTexCoord!=NULL);
	vector<unsigned char> matidx;
	vector< vector < vector <int> > > facets;
	unsigned int faceMatIndex[100];
	int nFaceMatIndex;

	//load the obj file, support up to 32 materials
	bool r = readOBJFile(fname, materials, vertices, normals, *pTexCoord, facets, faceMatIndex, nFaceMatIndex);
	if (r==false){
		SafeDelete(pTexCoord);
		return p;
	}
	
	//check whether it is triangle or quad file
	const int nPoly = facets.size();
	const int ntype = facets[0][0].size();
	if (!(ntype==3 || ntype==4)){
		SafeDelete(pTexCoord);
		return p;
	}

	//copy vertices;
	const bool allocbuffer = false;
	const int nVertex = vertices.size();
	Vector3d *pVertex = new Vector3d[nVertex];
	assert(pVertex!=NULL);
	for (i=0; i<nVertex; i++) pVertex[i]=vertices[i];

	const int nTri = nPoly;
	if (ntype==3){
		//The wavefront .obj file is a triangle mesh
		//copy connectivity
		Vector3i *pBoundaryTri = new Vector3i[nTri];
		assert(pBoundaryTri!=NULL);
		Vector3i *pTexIndex = new Vector3i[nTri];
		assert(pTexIndex!=NULL);
		for (i=0; i< nTri; i++){
			vector < vector <int> > &tri = facets[i];
			const int x = tri[0][0];
			const int y = tri[0][1];
			const int z = tri[0][2];
			pBoundaryTri[i] = Vector3i(x,y,z);
			int tx = tri[1][0];
			int ty = tri[1][1];
			int tz = tri[1][2];
			pTexIndex[i] =  Vector3i(tx, ty, tz);
			//Check triangle orientation, if inverted, then reverse it
			if (normals.size()>0){
				int nx = tri[2][0];
				int ny = tri[2][1];
				int nz = tri[2][2];
				Vector3d norm = compute_triangle_normal(pVertex[x], pVertex[y], pVertex[z]);
				Vector3f normf(norm.x, norm.y, norm.z);
				Vector3f inputnormf = normals[nx] + normals[ny] + normals[nz];
				if (DotProd(normf, inputnormf)<0){
					//printf("T invert norm\n");
					pBoundaryTri[i] = Vector3i(x, z, y);
				}	
			}
		}
		CTexturedTriangleObj *pobj = new CTexturedTriangleObj(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer);
		assert(pobj!=NULL);
			
		//get material ralted stuff
		pobj->assignTextureCoords(pTexCoord, pTexIndex);
		if (materials.size()>0)
			pobj->assignMaterials(&materials[0], materials.size());
		pobj->assignFaceMatIndices(faceMatIndex, nFaceMatIndex);
		p = pobj;
	}
	else if (ntype==4){
		//The wavefront .obj file is a triangle mesh
		//copy connectivity
		Vector4i *pBoundaryTri = new Vector4i[nTri];
		assert(pBoundaryTri!=NULL);
		Vector4i *pTexIndex = new Vector4i[nTri];
		assert(pTexIndex!=NULL);
		for (i=0; i< nTri; i++){
			vector < vector <int> > &tri = facets[i];
			const int x = tri[0][0];
			const int y = tri[0][1];
			const int z = tri[0][2];
			const int w = tri[0][3];
			pBoundaryTri[i] = Vector4i(x,y,z,w);
			int tx = tri[1][0];
			int ty = tri[1][1];
			int tz = tri[1][2];
			int tw = tri[1][3];
			pTexIndex[i] =  Vector4i(tx, ty, tz, tw);
			//Check triangle orientation, if inverted, then reverse it
			if (normals.size()>0){
				int nx = tri[2][0];
				int ny = tri[2][1];
				int nz = tri[2][2];
				int nw = tri[2][3];
				Vector3d norm = compute_quad_normal(pVertex[x], pVertex[y], pVertex[z], pVertex[w]);
				Vector3f normf(norm.x, norm.y, norm.z);
				Vector3f inputnormf = normals[nx] + normals[ny] + normals[nz] + normals[nw];
				if (DotProd(normf, inputnormf)<0){
					//printf("T invert norm\n");
					pBoundaryTri[i] = Vector4i(x, w, z, y);
				}	
			}
		}
		CTexturedQuadObj *pobj = new CTexturedQuadObj(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer);
		assert(pobj!=NULL);
			
		//get material ralted stuff
		pobj->assignTextureCoords(pTexCoord, pTexIndex);
		if (materials.size()>0)
			pobj->assignMaterials(&materials[0], materials.size());
		pobj->assignFaceMatIndices(faceMatIndex, nFaceMatIndex);
		p = pobj;
	}

	return p;
}

static bool _bool_loadObj = CPolyObjLoaderFactory::AddEntry(".obj", loadMeshWavefrontObj);

static bool SaveTriangleObjMesh(CPolyObj *pobj, const char *fname, const double *matrix)
{
	CTexturedTriangleObj *p = dynamic_cast<CTexturedTriangleObj*>(pobj);
	if (p){
		const int r = p->saveMeshWavefrontObj(fname, matrix);
		return r;
	}
	return false;
}
static bool _saveTri_OBJ = CPolyObjWriterFactory::AddEntry(".obj", SaveTriangleObjMesh);


bool CTexturedTriangleObj::saveMeshWavefrontObj(const char *fname, const double *matrix)
{
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

	SafeDeleteArray(pVertex);
	return r;
}


//This function is to compress the occurance of the material references. There are cases 
//where a material is referenced several times in the chunk of a triangle list. This is not 
//optimized since we need to switch the context multiple times. Therefore, we need to perform 
//a scan on the index array to minimize the redundency. 
void CTexturedTriangleObj::_reorgnizeFaceMatIndices(const unsigned int *pFaceMatIndex, const int len)
{
	unsigned int i;
	vector<int> facecounter;
	vector<Vector3i> reordered_tris;
	reordered_tris.resize(m_nPolygonCount);
	reordered_tris.clear();
	Vector3i *ptri = (Vector3i*)m_pPolygon;

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


void CTexturedTriangleObj::_initJaws(
	const Vector3d *pVertex, const int nVertex, const int *pPoly, 
	const int nPolygon, const int plytype, 
	const OglMaterial material[], 
	const unsigned int faceMatIndex[], 
	const int nFaceMatIndex)
{
	int i;
	const Vector3i *ptri = (const Vector3i *)pPoly;
	vector<Vector3d> leftjawPos, rightjawPos;
	vector<int> leftjawIndex, rightjawIndex;

	//
	m_pMedialBuffer = new bool [nVertex];
	for (i=0; i<m_nVertexCount; i++) m_pMedialBuffer[i]=false;

	for (i=0; i<nFaceMatIndex; i++){
		int x0, x1, matid;
		decodeMaterialIndex(i, faceMatIndex, nFaceMatIndex, nPolygon, x0, x1, matid);
		const OglMaterial& m0 = material[matid];
		const Vector3f *pColor = (const Vector3f *)&m0.diffuse[0];
		const Vector3f color = *pColor;
		if (color == LEFTJAW || color == LEFTJAW_MEDIAL){
			const bool isMedial = (color==LEFTJAW_MEDIAL);
			for (int j=x0; j<x1; j++){
				const Vector3i tri = ptri[j];
				leftjawPos.push_back(pVertex[tri.x]), leftjawIndex.push_back(tri.x);
				leftjawPos.push_back(pVertex[tri.y]), leftjawIndex.push_back(tri.y);
				leftjawPos.push_back(pVertex[tri.z]), leftjawIndex.push_back(tri.z);
				if (isMedial){
					m_pMedialBuffer[tri.x]=true;
					m_pMedialBuffer[tri.y]=true;
					m_pMedialBuffer[tri.z]=true;
				}
			}
		}
		else if (color == RIGHTJAW || color == RIGHTJAW_MEDIAL){
			const bool isMedial = (color==RIGHTJAW_MEDIAL);
			for (int j=x0; j<x1; j++){
				const Vector3i tri = ptri[j];
				rightjawPos.push_back(pVertex[tri.x]), rightjawIndex.push_back(tri.x);
				rightjawPos.push_back(pVertex[tri.y]), rightjawIndex.push_back(tri.y);
				rightjawPos.push_back(pVertex[tri.z]), rightjawIndex.push_back(tri.z);
				if (isMedial){
					m_pMedialBuffer[tri.x]=true;
					m_pMedialBuffer[tri.y]=true;
					m_pMedialBuffer[tri.z]=true;
				}
			}
		}
	}

	//===============================================
	m_pGrasperHead = new CGrasperHead;
	assert(m_pGrasperHead!=NULL);
	m_pGrasperHead->setLeftJaw(nVertex, leftjawPos, leftjawIndex);
	m_pGrasperHead->setRightJaw(nVertex, rightjawPos, rightjawIndex);
	m_pGrasperHead->decideTranslationRotation();
	AxisAlignedBox box;
	this->ComputeBoundingBox(box.minp, box.maxp);
	Vector3d center = (box.minp+box.maxp)*(-0.5);
	m_pGrasperHead->addTranslation(center);
}


void CTexturedTriangleObj::initJaws(void)
{
	const Vector3d *pVertex = m_pVertex;
	const int nVertex = m_nVertexCount;
	const int *pPoly = this->m_pPolygon;
	const int nPolygon = this->m_nPolygonCount;
	const int plytype = this->m_nPlyVertexCount;
	const OglMaterial* material = materials;
	const unsigned int *faceMatIndex = m_FaceMatIndex;
	const int nFaceMatIndex = m_nFaceMatIndex;
	_initJaws(pVertex, nVertex, pPoly, nPolygon, plytype, material, faceMatIndex,nFaceMatIndex);
}


void CTexturedTriangleObj::updateToolHeadAngle(const int dir)
{
	if (!m_pGrasperHead) return;
	if (dir>=0){
		m_pGrasperHead->openJaw();
	}
	else{
		m_pGrasperHead->closeJaw();
	}
	m_pGrasperHead->applyRotation(m_pVertex);
}
