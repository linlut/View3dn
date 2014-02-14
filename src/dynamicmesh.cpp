//dynamicmesh.cpp

#include <QGLViewer/qglviewer.h>
#include <triangle_mesh.h>

#include "tetraobj.h"
#include "dynamicmesh.h"
#include "cia3dobj.h"
#include "pointobj.h"


float THdmg = 0.01f - 1e-4f;

using namespace qglviewer;


static inline void SEEK_GROUPn(FILE *fp, const char *group1)
{
	char sbuffer[512];
	do{
		fgets(sbuffer, 512, fp);
		if (strstr(sbuffer, group1)!=NULL)
			break;
	}while(!feof(fp));
	assert(!feof(fp));
}


CDynamicMesh::CDynamicMesh(const char *fname): 
	m_Triangles(60*1024, 30*1024), 
	m_SplitTets(40*1024, 20*1024),
	m_SplitTetEdges(40*1024, 20*1024),
	m_FractVertex(100240, 20240),
	m_FractTri(60240, 20240),
	m_FractVertex2(100240, 20240),
	m_FractTri2(60240, 20240)				
{
	const int ELEMTYPE_BUFFLENGTH=40;
	int i, elenodecount[ELEMTYPE_BUFFLENGTH];
	const char group1[]="GROUP 1";
	const char group3[]="GROUP 3";
	char sbuffer[512];
	FILE *fp = fopen(fname, _RA_);
	assert(fp!=NULL);

	//allloc space for memory management;
	m_pSSurfaceMgr = new CMemoryMgr;
	m_pDSurfaceMgr = new CMemoryMgr;	
	assert(m_pSSurfaceMgr!=NULL && m_pDSurfaceMgr!=NULL);

	//init flags
	m_bExportTextureCoor = false;		//donot export 3d texture coor.
	m_bReconstrucSurface = true;
	for (i=0; i<20; i++){				
		m_bExportBoundingSph[i]=false;	//donot exprot bouding spheres
		m_bDisableObject[i] = false;	//export the objects
	}

	//read vertex count and element count
	SEEK_GROUPn(fp, group1);
    int nPoint, nElem, nMat, nStep, nAlgo, nMiter, nType;
	float fTol;
	fscanf(fp, "%d %d %d %d %d %d %f %d\n", 
			    &nPoint, &nElem, &nMat, &nStep, &nAlgo, &nMiter, &fTol, &nType);
	m_nVertexCount = nPoint;	
	m_nTetraCount = nElem;
	m_pStaticVertex = new CVertexInfo[m_nVertexCount];
	m_pTetra = new Vector4i[m_nTetraCount];
	assert(m_pTetra!=NULL);
	assert(m_pStaticVertex!=NULL);
	assert(nMat<ELEMTYPE_BUFFLENGTH);
	m_nMaterialCount = nMat;	

	//read node count for each element type
	fgets(sbuffer, 512, fp);
	fgets(sbuffer, 512, fp);
	fgets(sbuffer, 512, fp);
	fgets(sbuffer, 512, fp);
	for (i=0; i<nMat; i++){
		int tmp1, tmp2, tmp3, tmp4, tmp5, n;
		fscanf(fp, "%d %d %d %d %d %d", &tmp1, &tmp2, &n, &tmp3, &tmp4, &tmp5);
		elenodecount[tmp1-1] = n; 
	}

	//read mesh connectivity
	SEEK_GROUPn(fp, group3);
	for (i=0; i<m_nTetraCount; i++){
		int id, matid;
		int t0, t1, t2, t3, t4, t5, t6, t7;
		Vector4i *p = &m_pTetra[i];
		fscanf(fp, "%d %d %d", &id, &t0, &matid);
		matid --;
		int nodecount = elenodecount[matid];
		switch(nodecount){
		case 4:
			fscanf(fp, "%d %d %d %d", &p->x, &p->y, &p->z, &p->w);
			p->x --; p->y --;
			p->z --; p->w --;
			m_pStaticVertex[p->x].m_nMaterialID = matid;
			m_pStaticVertex[p->y].m_nMaterialID = matid;
			m_pStaticVertex[p->z].m_nMaterialID = matid;
			m_pStaticVertex[p->w].m_nMaterialID = matid;
			break;
		case 8:
			fscanf(fp, "%d %d %d %d %d %d %d %d", &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
			p->x = p->y = p->z = p->w = -1;
			t0--, t1--, t2--, t3--;
			t4--, t5--, t6--, t7--;
			m_pStaticVertex[t0].m_nMaterialID = matid;
			m_pStaticVertex[t1].m_nMaterialID = matid;
			m_pStaticVertex[t2].m_nMaterialID = matid;
			m_pStaticVertex[t3].m_nMaterialID = matid;
			m_pStaticVertex[t4].m_nMaterialID = matid;
			m_pStaticVertex[t5].m_nMaterialID = matid;
			m_pStaticVertex[t6].m_nMaterialID = matid;
			m_pStaticVertex[t7].m_nMaterialID = matid;
			break;
		default:
			assert(0);
		}
	}

	//build an index buffer for actively split tetrahedra
	m_pTetIndex = new int [m_nTetraCount];
	assert(m_pTetIndex!=NULL);
	for (i=0; i<m_nTetraCount; i++) m_pTetIndex[i]=-1;

	//read all the vertices;
	for (i=0; i<m_nVertexCount; i++){
		int id;
		float thick;
		Vector3f *p = &(m_pStaticVertex[i].m_vInitPosition);
		fscanf(fp, "%d %f %f %f %f", &id, &p->x, &p->y, &p->z, &thick);
	}

	//compute bounding box for each material
	for (i=0; i<nMat; i++){
		ComputeStaticMeshBoundingBox(i, m_pStaticVertex, m_nVertexCount, m_bbox[i]);
	}

	//over;
	fclose(fp);
	printf("Reading file %s OK!\n", fname);

}


CDynamicMesh::~CDynamicMesh()
{
	if (m_pSSurfaceMgr) delete m_pSSurfaceMgr;
	if (m_pDSurfaceMgr) delete m_pDSurfaceMgr;

	if (m_pStaticVertex) delete [] m_pStaticVertex;
	if (m_pTriangleBoundary) delete [] m_pTriangleBoundary;	
	if (m_pTetra) delete [] m_pTetra;	
	if (m_pTetIndex) delete [] m_pTetIndex;
}


static inline 
int _getSurfaceComponentCount(CVertexInfo * pbuff, const int nCount)
{
	int i, c;
	for (i=0; i<nCount; i++)
		pbuff[i].setVisitFlag(false);
	for (i=c=0; i<nCount; i++){
		CVertexInfo *ptr = &pbuff[i];
		if (ptr->getVisitFlag()) 
			continue;
		if (ptr->m_pNext == ptr->m_pPre)	//ignore particles and inner vertices
			continue;						//or <=2 set
		ptr->setVisitFlag(true);
		CVertexInfo * phead = (CVertexInfo*)ptr->m_pNext;
		while (phead!=ptr){
			phead->setVisitFlag(true);
			phead = (CVertexInfo*)phead->m_pNext;
		}
		c++;								// inc set count
	}
	return c;
}

void CDynamicMesh::_buildBoundaryVertexClusters(Vector3i *pTriBuff, const int tribufflen)
{
	//Clear the linklist pointers;
	int i;
	for (i=0; i<m_nVertexCount; i++){
		CVertexInfo * p = &m_pStaticVertex[i];	
		p->m_pPre = p->m_pNext = p;
		p->m_nIndex = i;
	}

	for (i=0; i<tribufflen; i++){
		Vector3i tri = pTriBuff[i];
		int v0 = tri.x;
		int v1 = tri.y;
		int v2 = tri.z;
		CVertexInfo *p0 = &m_pStaticVertex[v0];
		CVertexInfo *p1 = &m_pStaticVertex[v1];
		CVertexInfo *p2 = &m_pStaticVertex[v2];
		p0->mergeList(p1);
		p1->mergeList(p2);
	}

	//Do statistics, show the count of connected components;
	int c= _getSurfaceComponentCount(m_pStaticVertex, m_nVertexCount);
	printf("Totally %d connected surface components!\n", c);
}

static inline 
float STRESS_J2(const float sxx, const float syy, const float szz, 
				const float sxy, const float syz, const float szx)
{
	const float n11 = sxx;
	const float n12 = sxy;
	const float n13 = szx;
	const float n21 = n12;
	const float n22 = syy;
	const float n23 = syz;
	const float n31 = n13;
	const float n32 = n23;
	const float n33 = szz;

	float j2 =	n22*n33-n23*n23 +
				n11*n33-n13*n13 +
				n11*n22-n12*n12;
	return j2;
}

void CDynamicMesh::_updateVertexPositions(CCia3dObj & obj)
{
	int i, j;

	for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		CObject3D * p = obj.m_pObjList[i];
		if (p==NULL) continue;
		const char * str = p->Description();
		if (strcmp(str, "pnt")==0  || 
			strcmp(str, "tri")==0  ||
			strcmp(str, "qad")==0 ){
			CPolyObj *ply = (CPolyObj*)p;
			int np = ply->m_nVertexCount;
			int *pVertexAbsIndex = ply->GetVertexAbsIndexPointer();
			//ASSERT0(pVertexAbsIndex!=NULL);
			Vector3d *pVertex = ply->m_pVertex;
			ASSERT0(pVertex!=NULL);

			//
			int idxstress = ply->GetFAttributeIndexByName("SXX");
			int idxdmg = ply->GetFAttributeIndexByName("DM");
			assert(idxstress>=0);
			assert(idxdmg>=0);
			//"SXX","SYY","SZZ","SXY","SYZ","SZX"
			float *psxx = ply->m_pFAttributes[idxstress];
			float *psyy = ply->m_pFAttributes[idxstress+1];
			float *pszz = ply->m_pFAttributes[idxstress+2];
			float *psxy = ply->m_pFAttributes[idxstress+3];
			float *psyz = ply->m_pFAttributes[idxstress+4];
			float *pszx = ply->m_pFAttributes[idxstress+5];
			float *pdmg = ply->m_pFAttributes[idxdmg];
			for (j=0; j<np; j++){
				int idx = j;
				if (pVertexAbsIndex)
					idx = pVertexAbsIndex[j];
				ASSERT0(idx>=0 && idx< m_nVertexCount);
                Vector3d *pp = &pVertex[j];
				m_pStaticVertex[idx].m_vCurrPosition = Vector3f(pp->x, pp->y, pp->z);

				//========================
				//const float j2 = STRESS_J2(psxx[j], psyy[j], pszz[j], psxy[j], psyz[j], pszx[j]);
				const float damage = pdmg[j];
				m_pStaticVertex[idx].J2 = _MAX_(damage, m_pStaticVertex[idx].J2);
			}
		}
	}	
}


void CDynamicMesh::_updateBoundrayMeshConnectivity(CCia3dObj & obj)
{
	int i;

	//clear the buffer;
	m_Triangles.RemoveAll();

	//for each triangle mesh object, add the triangles;
	for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		CObject3D * p = obj.m_pObjList[i];
		if (p==NULL) continue;
		const char * str = p->Description();
		if (strcmp(str, "tri")!=0) 
			continue;

		//Only deal with trimesh object
		CTriangleObj *ply = (CTriangleObj*)p;
		int nt = ply->m_nPolygonCount;
		int *pVertexAbsIndex = ply->GetVertexAbsIndexPointer();
		//ASSERT0(pVertexAbsIndex!=NULL);
		Vector3i *pTri = (Vector3i*)ply->m_pPolygon;
		ASSERT0(pTri!=NULL);

		//Add triangle and update the dynamic boundary flag
		for (int j=0; j<nt; j++){
			Vector3i tri = pTri[j];
			Vector3i tri2 = tri;
			if (pVertexAbsIndex){
				tri2.x = pVertexAbsIndex[tri.x];
				tri2.y = pVertexAbsIndex[tri.y];
				tri2.z = pVertexAbsIndex[tri.z];
			}
			m_Triangles.Add(tri2);			
		}
	}

	//build the boundary triangle list for each vertex;
	m_pDSurfaceMgr->Free();
	Vector3i *ptri = m_Triangles.GetBuffer();
	int ntri = m_Triangles.GetSize();
	BuildDynamicTriangleTabForVertices(m_pStaticVertex, m_nVertexCount, ptri, ntri, *m_pDSurfaceMgr);
}


//Return: the array index of the tet;
inline int CDynamicMesh::_addMarchingTetAndEdgesToQueue(const int tetindex, int vbuff[], const int vlen)
{
	Vector4i tet=m_pTetra[tetindex];
	int tetpos = m_pTetIndex[tetindex];
	if (tetpos<0){  //if at least one particle, no need to insert tet
		m_pTetIndex[tetindex] = tetpos = m_SplitTets.GetSize();
		CMarchingTet martet(tet);
		m_SplitTets.Add(martet);
	}
	//maintain a split order and add edges into the edge queue;
	CMarchingTet& t = m_SplitTets[tetpos];
	for (int i=0; i<vlen; i++){
		const int vid = vbuff[i];
		t.AddOneSplitVertex(vid);
		t.AddThreeSplitEdgesForVertex(vid, &m_SplitTetEdges);
	}
	return tetpos;
}

//==============================================================

void CDynamicMesh::__checkParticleStatusChanges(
	 const Vector4i& tet, bool* pParticleFlag, int* vbuff, int &vlen, 
	 bool& hasParticleBefore, bool& hasParticleNow)
{
	vlen = 0;
	const bool ax = m_pStaticVertex[tet.x].getParticleFlag();
	const bool ay = m_pStaticVertex[tet.y].getParticleFlag();
	const bool az = m_pStaticVertex[tet.z].getParticleFlag();
	const bool aw = m_pStaticVertex[tet.w].getParticleFlag();
	hasParticleBefore = ax|ay|az|aw;
	const bool bx = pParticleFlag[tet.x];
	const bool by = pParticleFlag[tet.y];
	const bool bz = pParticleFlag[tet.z];
	const bool bw = pParticleFlag[tet.w];
	hasParticleNow = bx|by|bz|bw;

	if (hasParticleNow==false){
		ASSERT0(hasParticleBefore==false);
		return;
	}

	//now, there is at least one particle for the tet
	const int * ptr = &tet.x;
	bool c[4]={ax ^ bx, ay ^ by, az ^ bz, aw ^ bw}; 
	for (int i=0; i<4; i++){
		if (c[i]) vbuff[vlen++] = ptr[i];
	}
}


static inline
void _checkEdgeConnectivity3(const int v[3], CVertexInfo *pVertInfo, int vbuff[4], int &nsplit)
{
	const int v0=v[0];
	const int v1=v[1];
	const int v2=v[2];
	const Vector3i* r01 = _lsearchDynamicEdge(v0, v1, pVertInfo);
	const Vector3i* r02 = _lsearchDynamicEdge(v0, v2, pVertInfo);
	if (r01){
		if (r02) // 3 of them are in one set
			return;
		else
			vbuff[nsplit++] = v2;
	}
	else{
		if (r02)
			vbuff[nsplit++] = v1;
		else{
			const Vector3i* r12 = _lsearchDynamicEdge(v1, v2, pVertInfo);
			if (r12)
				vbuff[nsplit++] = v0;
			else
				vbuff[nsplit++] = v0, vbuff[nsplit++] = v1;
		}
	}
}

static inline
void _checkEdgeConnectivity22(const int v0[2], const int v1[2], CVertexInfo *pVertInfo, int vbuff[4], int &nsplit)
{
	const Vector3i* r01 = _lsearchDynamicEdge(v0[0], v0[1], pVertInfo);
	const Vector3i* r23 = _lsearchDynamicEdge(v1[0], v1[1], pVertInfo);
	if (r01){
		if (r23){
			vbuff[nsplit++]=v0[0];
			vbuff[nsplit++]=v0[1];		
		}
		else{
			vbuff[nsplit++]=v1[0];
			vbuff[nsplit++]=v1[1];		
		}
	}
	else{
		if (r23){
			vbuff[nsplit++]=v0[0];
			vbuff[nsplit++]=v0[1];		
		}
		else{
			vbuff[nsplit++]=v0[0];
			vbuff[nsplit++]=v0[1];		
			vbuff[nsplit++]=v1[0];
		}
	}
}


void CDynamicMesh::__checkTetConnectivityChanges(const Vector4i& tet, bool* pParticleFlag, int* vbuff, int &vlen)
{
	const int *p = &tet.x;
	int i, c, a[4];
	vlen = c = 0;

	//first, find all the non particle vertices;
	for (i=0; i<4; i++){
		const int v = p[i];
		if (!pParticleFlag[v])
			a[c++]= v;
	}

	//if only one vertex left, no special treat required;
	switch(c){
	 case 0:
	 case 1: return;

	 case 2:
			if (!_lsearchDynamicEdge(a[0], a[1], m_pStaticVertex)){
				vbuff[0] = a[0];
				vlen = 1;
			}
			break;
	 case 3:
			_checkEdgeConnectivity3(a, m_pStaticVertex, vbuff, vlen);
			break;
	 case 4:
			printf("Not implemented!\n");
			assert(0);
			break;

	 default:
		assert(0);
	}
}


static inline 
int FromSetToSplitVertices(const int v[4][4], const int vlen[4], const int nset, CVertexInfo *pVertInfo, int splitbuff[4])
{
	int i, nsplit = 0;
	switch(nset){
		case 2:
			if (vlen[0]==1){
				splitbuff[0] = v[0][0], nsplit=1;
				_checkEdgeConnectivity3(v[1], pVertInfo, splitbuff, nsplit);
			}
			else if (vlen[0]==2){
				_checkEdgeConnectivity22(v[0], v[1], pVertInfo, splitbuff, nsplit);
			}
			else{
				ASSERT0(vlen[0]==3);
				splitbuff[0] = v[1][0], nsplit=1;
				_checkEdgeConnectivity3(v[0], pVertInfo, splitbuff, nsplit);
			}
			break;
		case 3:
			for (i=0; i<3; i++){
				if (vlen[i]!=2)
					splitbuff[nsplit++] = v[i][0];
			}			
			for (i=0; i<3; i++){
				if (vlen[i]==2){
					const int x = v[i][0];
					const int y = v[i][1];
					if (!_lsearchDynamicEdge(x, y, pVertInfo)){
						splitbuff[nsplit++] = x;
						splitbuff[nsplit++] = y;
					}
					break;
				}
			}
			break;
		default:
			printf("Not implemented yet!\n");
			assert(0);
	}
	return nsplit;
}


int debtm = 200;
void CDynamicMesh::_updateParticles(const int tm, CCia3dObj & obj)
{
	int i, nAllMesh=0, nNewParticle=0;
	bool *pParticleFlag = new bool [m_nVertexCount];
	assert(pParticleFlag!=NULL);

	//init an array for particle flag
	for (i=0; i<m_nVertexCount; i++) pParticleFlag[i]=false;
	for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
		CObject3D * p = obj.m_pObjList[i];
		if (p==NULL) continue;
		const char * str = p->Description();
		if (strcmp(str, "pnt")!=0) continue;
		CPointObj *pnt = (CPointObj*)p;
		int *pVertexAbsIndex = pnt->GetVertexAbsIndexPointer();
		ASSERT0(pVertexAbsIndex!=NULL);
		for (int j=0; j<pnt->m_nVertexCount; j++){
			if (pVertexAbsIndex)
				pParticleFlag[pVertexAbsIndex[j]]=true;
			else
				pParticleFlag[j]=true;
		}
	}

	//traverse the tet array for new tets to split
	for (i=0; i<m_nTetraCount; i++){
		const Vector4i tet = m_pTetra[i];			
		if (tet.x==-1) continue;

#ifdef _DEBUG
		if (tm==debtm ){
			int aga=1;
		}
		Vector3i debpos(211, 1478, 3518);
		debpos.bubbleSort();
		Vector4i tet2 = tet; 
		//tet2.BubbleSort();
		if ((tet2.x==debpos.x && tet2.y==debpos.y && tet2.z==debpos.z)||
			(tet2.x==debpos.x && tet2.y==debpos.y && tet2.w==debpos.z)||
			(tet2.y==debpos.x && tet2.z==debpos.y && tet2.w==debpos.z)){
			if (m_pStaticVertex[debpos.x].getDynBoundaryFlag() &&
				m_pStaticVertex[debpos.y].getDynBoundaryFlag() &&
				m_pStaticVertex[debpos.z].getDynBoundaryFlag())
				int agag=1;
		}
#endif

		//check whether all the vertices are on the mesh;
		int vbuff[8], vbuff2[8], vlen1=0, vlen2=0;
		if (_onDynBoundarySurface(tet)){
			if (_verticesSameSurfaceComponent(tet.x, tet.y, tet.z, tet.w))
				continue;
			//check connectivity of tet. Assert that the vertices are in just 2 sets
			int v[4][4], slen[4];
			int nset = ClassifyVerticesOfTet(tet, m_pStaticVertex, v, slen);
			vlen2 = FromSetToSplitVertices(v, slen, nset, m_pStaticVertex, vbuff);
			nAllMesh ++;
			//continue;
			//_dumpTetAsPovTriangles(tet);
		}
		else{
			//check status change for particles;
			bool hasParticleBefore, hasParticleNow;
			__checkParticleStatusChanges(tet, pParticleFlag, vbuff, vlen1, hasParticleBefore, hasParticleNow);
			//check connectivity change for mesh vertices;
			if (hasParticleBefore||hasParticleNow)
				__checkTetConnectivityChanges(tet, pParticleFlag, &vbuff[vlen1], vlen2);
		}

		//add the tetrahedron into the buffer;
		int vlen = vlen1+vlen2;
		if (vlen==0) continue;
		
		//adjust the split order using the dmg information
		vbuff2[0]=vbuff[0], vbuff2[1]=vbuff[1], vbuff2[2]=vbuff[2], vbuff2[3]=vbuff[3]; 
		if (m_pTetIndex[i]<0 && vlen>1){ //a new tet, first split insert
			if (vlen==3){
				vbuff[0] = tet.x, vbuff[1] = tet.y;
				vbuff[2] = tet.z, vbuff[3] = tet.w;
				vlen=4;
			}
			SetFirstVertexWithMaxEdgeIntersections(tet, vbuff, vlen, m_pStaticVertex);
		}

		//add this tet
		const int tetpos =_addMarchingTetAndEdgesToQueue(i, vbuff, vlen);

		//if the virtual tet is composed of 4 new particles, we do an immediate split
		CMarchingTetEdge *pSlitEdges = m_SplitTetEdges.GetBuffer();
		CMarchingTet & mart = m_SplitTets[tetpos];
		if (vlen1==4){
			mart.PreSlitAllEdges(tm, pSlitEdges, m_pStaticVertex);
		}
		else{
			for (int j=0; j<vlen1; j++){
				const int v = vbuff2[j];
				mart.LockVertexAttachedEdges(tm, v, pSlitEdges);
			}				
		}
	}

	//set new particle flags;
	for (i=0; i<m_nVertexCount; i++){
		if (pParticleFlag[i]==m_pStaticVertex[i].getParticleFlag()) //status no change?
			continue;
		m_pStaticVertex[i].setParticleFlag(true);
		nNewParticle++;
	}

	//print out statistic info.
	delete [] pParticleFlag;
	printf("New particle number is %d\n", nNewParticle);
	int TetTypeCount[5]={0,0,0,0,0};
	for (i=0; i<m_SplitTets.GetSize(); i++){
		const int c = m_SplitTets[i].GetSplitVertexCount();
		ASSERT0((c>0) && (c<=4));
		TetTypeCount[c]++;
	}
	printf("Particle in tet: 0:%d,1:%d,2:%d,3:%d,4:%d\n", 
			TetTypeCount[0], TetTypeCount[1], TetTypeCount[2], TetTypeCount[3], TetTypeCount[4]);
	printf("Tets total %d, all mesh vertices but to split are %d.\n", m_nTetraCount, nAllMesh);
}



void CDynamicMesh::_checkFlatCuts(void)
{
	int i, ntet = m_SplitTets.GetSize();
	CMarchingTet *ptet = m_SplitTets.GetBuffer();

	for (i=0; i<m_nVertexCount; i++)
		m_pStaticVertex[i].setHasFlatCutFlag(false);

	for (i=0; i<ntet; i++){
		CMarchingTet * t = &ptet[i];
		int v0 = t->GetVerticeByExtractionSequence(0);
		m_pStaticVertex[v0].setHasFlatCutFlag(true);
	}
}


//==============================================================
void CDynamicMesh::_extractFracturedMaterialInterface(const int tm, const int MATID, FILE *fp, int *tindex)
{
	const int ntet = m_SplitTets.GetSize();
	CMarchingTet *ptet = m_SplitTets.GetBuffer();
	CMarchingTetEdge *pedge = m_SplitTetEdges.GetBuffer();
	//clear the buffers;
	m_FractVertex.RemoveAll();
	m_FractTri.RemoveAll();
	m_FractVertex2.RemoveAll();
	m_FractTri2.RemoveAll();

	for (int i=0; i<ntet; i++){
		CMarchingTet *p = &ptet[i];
		int matid = p->MaterialOfTet(m_pStaticVertex);
		if (matid!=MATID)
			continue;

#ifdef _DEBUG
		if (tm==12 )
			int aaaa=1;
		Vector4i tet = p->m_tet; 
		//tet.BubbleSort();
		Vector4i debpos(211, 3378, 1478, 3518);
		//debpos.BubbleSort();
		if (tet.x==debpos.x && tet.y==debpos.y && tet.z==debpos.z && tet.w==debpos.w)
			int agag=1;
#endif

		if (tindex) tindex[i]=m_FractTri.GetSize();
		p->ExtractSurface(tm, pedge, m_pStaticVertex);
	}
	if (tindex) tindex[ntet]=m_FractTri.GetSize();
}


void CDynamicMesh::__getBoundingBoxOfObject(const int matid, Vector3f& lowleft, Vector3f& upright)
{
	const float MAXR=1e22f;
	lowleft=Vector3f(MAXR, MAXR, MAXR);
	upright=Vector3f(-MAXR, -MAXR, -MAXR);
	for (int i=0; i<m_nVertexCount; i++){
		CVertexInfo *p = &m_pStaticVertex[i];
		const int mm =p->getMaterialID();
		if (mm!=matid) continue;
		Minimize(lowleft, p->m_vCurrPosition);
		Maximize(upright, p->m_vCurrPosition);
	}
}

void CDynamicMesh::_extractMissingTriangles(const int matid)
{
	for (int i=0; i<m_nTriangleCount; i++){
		Vector3i tri = m_pTriangleBoundary[i]; 
		const int v0=tri.x;
		const int v1=tri.y;
		const int v2=tri.z;
		const int mm =m_pStaticVertex[v0].getMaterialID();
		if (mm!=matid) continue;
		if (m_pStaticVertex[v0].getParticleFlag())
			continue;
		if (m_pStaticVertex[v1].getParticleFlag())
			continue;
		if (m_pStaticVertex[v2].getParticleFlag())
			continue;
		Vector3i* p=_lsearchDynamicTri(v0, v1, v2, m_pStaticVertex);
		if (p!=NULL)
			continue;
		if (!_verticesSameSurfaceComponent(v0, v1, v2)){
			continue;
			/*
			//test the scaling, if not too far way
			const float TH = 1.05f;
			float dx, dy;
			const Vector3f p0= m_pStaticVertex[v0].m_vCurrPosition;
			const Vector3f p1= m_pStaticVertex[v1].m_vCurrPosition;
			const Vector3f p2= m_pStaticVertex[v2].m_vCurrPosition;
			const Vector3f q0= m_pStaticVertex[v0].m_vInitPosition;
			const Vector3f q1= m_pStaticVertex[v1].m_vInitPosition;
			const Vector3f q2= m_pStaticVertex[v2].m_vInitPosition;
			dx = Distance2(p0, p1); 
			dy = Distance2(q0, q1); 
			if (dx/dy>TH) continue;
			dx = Distance2(p0, p2); 
			dy = Distance2(q0, q2); 
			if (dx/dy>TH) continue;
			dx = Distance2(p1, p2); 
			dy = Distance2(q1, q2); 
			if (dx/dy>TH) continue;
			*/
		}
		m_Triangles.Add(Vector3i(v0, v1, v2));
	}	
}


void CDynamicMesh::_exportUnPatchedMesh(const int matid, FILE *fp)
{
	int i, tricount=0, vertcount=0;
	int *pVertIndex = new int [m_nVertexCount];
	assert(pVertIndex!=NULL);
	for (i=0; i<m_nVertexCount; i++) pVertIndex[i]=-1;

	const int ntri1 = m_Triangles.GetSize();
	//output missing triangles caused due to single element break;
	if (m_bReconstrucSurface) _extractMissingTriangles(matid);
	const int ntri2 = m_Triangles.GetSize();
	printf("Find missing triangles, totally output %d.\n", ntri2-ntri1);

	//scan the triangle buffer for tricount and vertcount;
	const Vector3i *ptri = m_Triangles.GetBuffer();
	for (i=0; i<ntri2; i++){
		const Vector3i & tri = ptri[i];
		const int v0=tri.x;	const int v1=tri.y; const int v2=tri.z;
		const int m0 = m_pStaticVertex[v0].m_nMaterialID;
		const int m1 = m_pStaticVertex[v1].m_nMaterialID;
		const int m2 = m_pStaticVertex[v2].m_nMaterialID;
		ASSERT0(m0==m1 && m1==m2);
		if (m0==matid){
			tricount++;
			if (pVertIndex[v0]<0) pVertIndex[v0]=vertcount++;
			if (pVertIndex[v1]<0) pVertIndex[v1]=vertcount++;
			if (pVertIndex[v2]<0) pVertIndex[v2]=vertcount++;
		}		
	}

	//output a tinny triangle in case of empty object;
	if (tricount==0){
		fprintf(fp, "mesh{\n");
		fprintf(fp, "triangle{<0,0,0>,<0,0,1e-15>,<0,1e-15,0>}\n"); 
		fprintf(fp, "}\n\n\n");
		delete [] pVertIndex;
		return;
	}

	//output a mesh2 object
	Vector3f *N = new Vector3f [vertcount];
	assert(N!=NULL);
	for (i=0; i<m_nVertexCount; i++){
		const int v0 = pVertIndex[i];
		if (v0<0) continue;
		N[v0] = m_pStaticVertex[i].m_vCurrPosition;		
	}
	fprintf(fp, "mesh2{\n");
	fprintf(fp, "vertex_vectors{\n");
	fprintf(fp, "%d,\n", vertcount);
	for (i=0; i<vertcount; i++){
		const Vector3f *p = &N[i];
		fprintf(fp, "<%f,%f,%f>\n", p->x, p->y, -p->z);
	}
	fprintf(fp, "}\n");

	if (m_bExportTextureCoor){
		for (i=0; i<m_nVertexCount; i++){
			const int v0 = pVertIndex[i];
			if (v0<0) continue;
			N[v0] = m_pStaticVertex[i].m_vInitPosition;
		}
		fprintf(fp, "normal_vectors{\n");
		fprintf(fp, "%d,\n", vertcount);
		for (i=0; i<vertcount; i++){
			const Vector3f *p = &N[i];
			fprintf(fp, "<%f,%f,%f>\n", p->x, p->y, -p->z);
		}
		fprintf(fp, "}\n");
	}

	bool printed=false;
	fprintf(fp, "face_indices{\n");
	fprintf(fp, "%d,\n", tricount);
	for (i=0; i<ntri2; i++){
		const Vector3i & tri = ptri[i];
		const int v0=tri.x;	const int v1=tri.y; const int v2=tri.z;
		const int m0 = m_pStaticVertex[v0].m_nMaterialID;
		const int m1 = m_pStaticVertex[v1].m_nMaterialID;
		const int m2 = m_pStaticVertex[v2].m_nMaterialID;
		ASSERT0(m0==m1 && m1==m2);
		if (m0!=matid) continue;
		if ((!printed) && (i>=ntri1)){
			fprintf(fp, "//Find missing triangles for material %d.\n", matid);
			printed=true;
		}
		fprintf(fp, "<%d,%d,%d>\n", pVertIndex[v0], pVertIndex[v1], pVertIndex[v2]);
	}
	fprintf(fp, "//End missing triangles, total output %d.\n", ntri2-ntri1);
	fprintf(fp, "}\n");

	//close mesh
	fprintf(fp, "}\n\n\n");
	delete [] N;
	delete [] pVertIndex;
	m_Triangles.Resize(ntri1);
}


void CDynamicMesh::_exportPatchedMesh(FILE *fp, const int *tindex)
{
	if (m_FractTri.GetSize()==0){ //output a blank mesh to avoid errors
		fprintf(fp, "mesh{\n");
		fprintf(fp, "triangle{<0,0,0>,<0,1e-12,0>,<0,0,1e-12>}\n"); 
		fprintf(fp, "}\n\n");
		return;
	}
	//=================OUTPUT the ARRAY===================
	int i;
	const CGeneratedVertex *pVertex = m_FractVertex.GetBuffer();
	const int nv = m_FractVertex.GetSize();
	fprintf(fp, "//=======Fracture triangles begin======\n");
	fprintf(fp, "mesh2{\n");
	fprintf(fp, "vertex_vectors{\n");
	fprintf(fp, "%d,\n", nv); 
	for (i=0; i<nv; i++){
		const Vector3f& v=pVertex[i].v;
		fprintf(fp, "<%lf,%lf,%lf>\n", v[0], v[1], -v[2]);
	}
	fprintf(fp, "}\n"); 

	if (m_bExportTextureCoor){ //export the 3D texture coor, using normal field
		const CGeneratedVertex *pTexCoor = m_FractVertex.GetBuffer();
		fprintf(fp, "normal_vectors{\n");
		fprintf(fp, "%d,\n", nv); 
		for (i=0; i<nv; i++){
			const Vector3f& v=pTexCoor[i].t;
			fprintf(fp, "<%f,%f,%f>\n", v[0], v[1], -v[2]);
		}
		fprintf(fp, "}\n"); 
	}

	//output triangles
	const int ntri = m_FractTri.GetSize();
	const Vector3i *ptri = m_FractTri.GetBuffer();
	fprintf(fp, "face_indices{\n");
	fprintf(fp, "%d,\n", ntri); 
	if (tindex){
		for (i=0; i<m_SplitTets.GetSize(); i++){
			fprintf(fp, "//Tet%d\n", i);
			for (int j=tindex[i]; j<tindex[i+1]; j++){
				const Vector3i* t = &ptri[j];
				fprintf(fp, "<%d,%d,%d>\n", t->x, t->y, t->z);
			}
		}
	}
	else{
		for (i=0; i<ntri; i++){
			const Vector3i* t = &ptri[i];
			fprintf(fp, "<%d,%d,%d>\n", t->x, t->y, t->z);
		}
	}
	fprintf(fp, "}\n");

	fprintf(fp, "//=======Fracture triangles end, total %d======\n", ntri);
	fprintf(fp, "}\n\n\n");	//end mesh2
}


static inline void PRINT_TRIANGLE(FILE *fp, const CGeneratedVertex *pVertex, const Vector3i &tri)
{
	const Vector3i* t = &tri;
	const Vector3f *px = &pVertex[t->x].v;
	const Vector3f *py = &pVertex[t->y].v;
	const Vector3f *pz = &pVertex[t->z].v;
	fprintf(fp, "triangle{<%f,%f,%f>,<%f,%f,%f>,<%f,%f,%f>}\n", 
			px->x, px->y, -px->z, 
			py->x, py->y, -py->z, 
			pz->x, pz->y, -pz->z);
}

void CDynamicMesh::_exportPatchedMeshAlt(FILE *fp, const int *tindex)
{
	if (m_FractTri.GetSize()==0){ //output a blank mesh to avoid errors
		fprintf(fp, "mesh{\n");
		fprintf(fp, "triangle{<0,0,0>,<0,1e-12,0>,<0,0,1e-12>}\n"); 
		fprintf(fp, "}\n\n");
		return;
	}
	//=================OUTPUT the ARRAY===================
	const CGeneratedVertex *pVertex = m_FractVertex.GetBuffer();
	const Vector3i *ptri = m_FractTri.GetBuffer();
	const int nv = m_FractVertex.GetSize();
	const int ntri = m_FractTri.GetSize();

	fprintf(fp, "//=======Fracture triangles begin======\n");
	fprintf(fp, "mesh{\n");
	if (tindex){
		for (int i=0; i<m_SplitTets.GetSize(); i++){
			fprintf(fp, "//Tet%d\n", i);
			for (int j=tindex[i]; j<tindex[i+1]; j++)
				PRINT_TRIANGLE(fp, pVertex, ptri[j]);
		}
	}
	else{
		for (int i=0; i<ntri; i++)
			PRINT_TRIANGLE(fp, pVertex, ptri[i]);
	}

	fprintf(fp, "}\n");
	fprintf(fp, "//=======Fracture triangles end, total %d======\n\n\n", ntri);
}


#define POVOBJ_NAMEFORMAT "OBJ_MATERIAL%d_SUB"
void CDynamicMesh::_exportPovrayMesh(const int tm, CCia3dObj & obj, FILE *fp)
{
	char objname0[256], objname1[256], objname2[256];

	for (int i=0; i<m_nMaterialCount; i++){
		//if object i is disabled, no need to output
		if (m_bDisableObject[i])
			continue;

		//=============1st part, the original =====================
		sprintf(objname0, POVOBJ_NAMEFORMAT"0", i);
		fprintf(fp, "#declare %s=\n", objname0);

		//in some cases, we just need to render the bounding sphere of the object, 
		//such as when the simmulated object is a sphere. It will look much better.
		if (m_bExportBoundingSph[i]){
			Vector3f lowleft, upright, c, dis;
			__getBoundingBoxOfObject(i, lowleft, upright);
			c = (lowleft+upright)*0.5f;
			dis = upright - lowleft;
			const float r = _MAX3_(dis.x, dis.y, dis.z);
			fprintf(fp, "sphere {<%f,%f,%f>, %f}\n", c.x, c.y, c.z, r);
		}
		else //output the mesh
			_exportUnPatchedMesh(i, fp);

		//=============2nd part, the patched =====================
		sprintf(objname1, POVOBJ_NAMEFORMAT"1", i);
		fprintf(fp, "#declare %s=\n", objname1);

		if (m_bExportBoundingSph[i]){
			fprintf(fp, "sphere {<0,0,0>, 1e-30}\n");
		}
		else{
			if (m_bReconstrucSurface){
				//output the fracture triangles;
				int *tindex = NULL; // new int [m_SplitTets.GetSize()+10];
				_extractFracturedMaterialInterface(tm, i, fp, tindex);
				//simplify the surface
				const int nv0 = m_FractVertex.GetSize();
				const int ntri0=m_FractTri.GetSize();
				_simplifyFracturedSurface(i);
				const int nv1 = m_FractVertex.GetSize();
				const int ntri1=m_FractTri.GetSize();
				printf("Simplify vertex %d->%d, triangle %d->%d\n", nv0, nv1, ntri0, ntri1);
				//output the patched mesh part;
				_exportPatchedMesh(fp, tindex);
				if (tindex) delete []tindex;
			}
			else{
				fprintf(fp, "union{\n");
				fprintf(fp, "sphere{<0,0,0>,1e-30}\n");
				fprintf(fp, "sphere{<1,0,0>,1e-30}\n");
				int cc = 0;
				for (int j=0; j<CIA3D_SUBOBJ_LIMIT; j++){
					CObject3D * p = obj.m_pObjList[j];
					if (p==NULL) continue;
					const char * str = p->Description();
					if (strcmp(str, "pnt")!=0) continue;
					CPointObj *pnt= (CPointObj*)p;
					const float* pobjid= pnt->GetVertexObjectIDPointer();
					for (int k=0; k<pnt->m_nVertexCount; k++){
						if (((int)pobjid[k])!=i) continue;
						const Vector3d *p = &pnt->m_pVertex[k];
						const float r=pnt->m_pRadius[k]*0.80f;
						fprintf(fp, "sphere{<%lg,%lg,%lg>,%f}\n", p->x, p->y, -p->z, r);
						cc++;
					}
				}
				fprintf(fp, "//Total output %d spheres.\n", cc);
				fprintf(fp, "}\n");
			}
		}

		sprintf(objname2, POVOBJ_NAMEFORMAT, i);
		fprintf(fp, "#declare %s=\n", objname2);
		fprintf(fp, "union{object{%s} object{%s}}\n", objname0, objname1);
		fprintf(fp, "//===============================================\n\n\n\n\n");
	}
}


void CDynamicMesh::_exportPovrayFile(const int tm, void *pwin, CCia3dObj & obj, const char *fname)
{
	//---------Main file---------------
	extern void ExportPovrayMaterial(FILE *fp);
	extern void ExportPovrayCamera(Camera* pcamera, FILE *fp);
	extern void CreateIncFileName(const char *fname, char * povincfname, char* povincfullfname);
	assert(pwin!=NULL);
	FILE *fp = fopen(fname, _WA_);
	assert(fp!=NULL);

	//---------Main FILE---------------
	ExportPovrayMaterial(fp);
	QGLViewer *pviewer = (QGLViewer*)pwin;
	qglviewer::Camera *pcamera = pviewer->camera();
	ExportPovrayCamera(pcamera, fp);

	//include user editable file
	char povincfname[512], povincfullfname[512];
	CreateIncFileName(fname, povincfname, povincfullfname);
	fprintf(fp, "#include \"%s\"\n\n\n", povincfname);

	//export obj def.
	for (int i=0; i<m_nMaterialCount; i++){
		char objname[512];
		sprintf(objname, POVOBJ_NAMEFORMAT, i);
		fprintf(fp, "object {\n");
		fprintf(fp, "\t%s\n", objname);
		fprintf(fp, "\ttexture { CIA3DTex%d }\n", i);
		fprintf(fp, "}\n");
	}
	fclose(fp);

	//---------MESH FILE---------------
	fp=fopen(povincfullfname, _WA_);
	assert(fp!=NULL);
	_exportPovrayMesh(tm, obj, fp);
	fclose(fp);
}


//==============================================================


bool CDynamicMesh::LoadNextTimeStepPlt(const int tm, const char *infname, const char *ofname, void *pwin)
{
	char fnamebuffer[512];
	const double scale=1;

	//load plt0 using the cia3d data structure.
	CCia3dObj obj;
	obj.LoadFile(infname, scale);

	//Update the vertex positions;
	_updateVertexPositions(obj);

	//Connectivity, update and translate 
	_updateBoundrayMeshConnectivity(obj);

	if (m_bReconstrucSurface){//Reconstruct the surface is required
		//Find the connected surface components;
		_buildBoundaryVertexClusters(m_Triangles.GetBuffer(), m_Triangles.GetSize());
		//Particles, generate the exact shape for them;
		printf("Updating particles...\n");
		_updateParticles(tm, obj);
		//mark out the vertices which are first cut
		_checkFlatCuts();
	}

	//write a povray file
	printf("Writing pov file...\n");
	if (m_bReconstrucSurface)
		sprintf(fnamebuffer, "%s.pov", ofname);
	else
		sprintf(fnamebuffer, "%sB.pov", ofname);
	_exportPovrayFile(tm, pwin, obj, fnamebuffer);

	return true;
}



void CDynamicMesh::DecideBoundaryVertices(const char *fname)
{
	int i, j, count; 
	const double scale = 1;

	//load plt0 using the cia3d data structure.
	CCia3dObj obj;
	obj.LoadFile(fname, scale);
	int nobj = obj.UnsplitObjectCount();

	m_nTriangleCount = obj.GetTotalTriangleCount();
	m_pTriangleBoundary = new Vector3i[m_nTriangleCount];
	assert(m_pTriangleBoundary!=NULL);

	//copy the connectivity array
	count = 0;
	for (i=0; i<nobj; i++){
		CObject3D * p = obj.m_pObjList[i];
		const char *des = p->Description();
		if (strcmp(des, "tri") == 0){  //a triangle mesh;
			CTriangleObj *ptri = (CTriangleObj*)p;
			Vector3i*& pTriangle2 = (Vector3i*&)ptri->m_pPolygon;
			int *pVertexAbsIndex = ptri->GetVertexAbsIndexPointer();
			if (pVertexAbsIndex==NULL)
				fprintf(stderr, "Vertex absolute index pointer is NULL!\n");
			int ntri = ptri->m_nPolygonCount;
			for (j=0; j<ntri; j++, count++){
				Vector3i *v = &pTriangle2[j];
				int x, y, z;
				if (pVertexAbsIndex){
					x = pVertexAbsIndex[v->x];
					y = pVertexAbsIndex[v->y];
					z = pVertexAbsIndex[v->z];
				}
				else{
					x = v->x;
					y = v->y;
					z = v->z;
				}
				ASSERT0(x>=0 && x<m_nVertexCount);
				ASSERT0(y>=0 && y<m_nVertexCount);
				ASSERT0(z>=0 && z<m_nVertexCount);
				m_pTriangleBoundary[count] = Vector3i(x, y, z);
				m_pTriangleBoundary[count].bubbleSort();
			}
		}
		else if (strcmp(des, "qad") == 0){  //a quad mesh;
			//quad mesh is used as boundary conditions, no use here
		}
		else if (strcmp(des, "pnt") == 0){  //a point mesh;
			//impossible to have a point set
			assert(0);
		}
		else{
			assert(0);
		}
	}

	//sor the array;
	assert(count==m_nTriangleCount);
	qsort(m_pTriangleBoundary, count, sizeof(Vector3i), _vec3i_compare);

	//build the static boundary list for each vertex;
	m_pSSurfaceMgr->Free();
	BuildStaticTriangleTabForVertices(m_pStaticVertex, m_nVertexCount, m_pTriangleBoundary, count, *m_pSSurfaceMgr);
}


//====================SIMPLIFICATION======================

int CDynamicMesh::_getridDoubleSidedTriangle(Vector3i *ptri, const int nv, const int ntri)
{
	int i, c;
	CTriangleListItem ** ppList = new CTriangleListItem *[nv];
	assert(ppList!=NULL);
	for (i=0; i<nv; i++) ppList[i]=NULL;

	for (i=0; i<ntri; i++){
		const Vector3i tri = ptri[i];
		const int v0=_MAX3_(tri.x, tri.y, tri.z);
		CTriangleListItem *&p = ppList[v0];
		CTriangleListItem *q = HasOpsiteSideTriangle(tri, v0, p);
		if (!q){
			CTriangleListItem *newp=(CTriangleListItem *)m_pDSurfaceMgr->Malloc(sizeof(CTriangleListItem));
			int v1, v2;
			if (v0==tri.x)
				v1=tri.y, v2=tri.z;
			else if (v0==tri.y)
				v1=tri.z, v2=tri.x;
			else
				v1=tri.x, v2=tri.y;
			newp->m_v0 = v1, newp->m_v1=v2;
			newp->m_pNext = p;
			p = newp;
		}
		else
			q->m_v0 = q->m_v1 = -1;
	}
	for (i=c=0; i<nv; i++){
		CTriangleListItem *p= ppList[i];
		while(p!=NULL){
			if (p->m_v0!=-1)
				ptri[c++] = Vector3i(i, p->m_v0, p->m_v1);
			p=p->m_pNext;
		}
	}
	return c;
}


void CDynamicMesh::__simplifyOneFracturedSurface(CDynamicArray<CGeneratedVertex>& fractVertex, CDynamicArray<Vector3i> &fractTri, const float TH)
{
	extern void VertexClusterGVertices(CDynamicArray<CGeneratedVertex>& fractVertex, CDynamicArray<Vector3i> &fractTri, const float TH);
	const int ntri = fractTri.GetSize();
	const int nv = fractVertex.GetSize();
	if (ntri==0 || nv==0) return;
	VertexClusterGVertices(fractVertex, fractTri, TH);

	//remove the double sided tri.
	Vector3i *ptri = fractTri.GetBuffer();
	const int c = _getridDoubleSidedTriangle(ptri, fractVertex.GetSize(), fractTri.GetSize());
	fractTri.Resize(c);
}


void CDynamicMesh::_simplifyFracturedSurface(const int matid)
{
	extern void SmoothingVertices(CGeneratedVertex* pvert, const int nv, const Vector3i *ptri, const int ntri, CMemoryMgr &mgr, CVertexInfo *pVertInfo, const float S);
	int i;
	const Vector3d lowleft = m_bbox[matid].Min();
	const Vector3d upright = m_bbox[matid].Max();
	const Vector3d dist = upright-lowleft;
	const double TH =sqrt(DotProd(dist, dist))*1e-5;

	//simplify mesh surface
	__simplifyOneFracturedSurface(m_FractVertex, m_FractTri, TH);

	//simplify particle surface
	__simplifyOneFracturedSurface(m_FractVertex2, m_FractTri2, TH);

	//do mesh smoothing
	{
		CGeneratedVertex *pvert = m_FractVertex.GetBuffer();
		Vector3i *ptri = m_FractTri.GetBuffer();
		const int nv   = m_FractVertex.GetSize();
		const int ntri = m_FractTri.GetSize();
		SmoothingVertices(pvert, nv, ptri, ntri, *m_pDSurfaceMgr, m_pStaticVertex, 0.25f);
	}
	{
		CGeneratedVertex *pvert = m_FractVertex2.GetBuffer();
		Vector3i *ptri = m_FractTri2.GetBuffer();
		const int nv   = m_FractVertex2.GetSize();
		const int ntri = m_FractTri2.GetSize();
		SmoothingVertices(pvert, nv, ptri, ntri, *m_pDSurfaceMgr, m_pStaticVertex, 0.00f);
	}

	//finally, copy vertices and triangles for output;
	CGeneratedVertex *pvert = m_FractVertex2.GetBuffer();
	const int nv1 = m_FractVertex.GetSize();
	const int nv2 = m_FractVertex2.GetSize();
	for (i=0; i<nv2; i++) m_FractVertex.Add(pvert[i]);
	assert(nv1+nv2==m_FractVertex.GetSize());

	Vector3i *ptri = m_FractTri2.GetBuffer();
	const int ntri = m_FractTri2.GetSize();
	const int base = nv1;
	for (i=0; i<ntri; i++){
		Vector3i tri = ptri[i];
		tri.x+=base, tri.y+=base, tri.z+=base;
		m_FractTri.Add(tri);
	}
}


/*
	//call the progressive mesh simplification
	extern void MeshSimplification(float pVertex[][3], float pTexCoor[][3], const int nv, int pTri[][3], const int ntri, const float simpratio, int &newnv, int &newntri);
	typedef float FP3[3];	
	typedef int IP3[3];	
	ntri = m_FractTri.GetSize();
	nv = m_FractVertex.GetSize();
	int newnv, newntri;
	MeshSimplification((FP3*)pvert, (FP3*)ptexcoor, nv, (IP3*)ptri, ntri, 0.99f, newnv, newntri);
	m_FractTri.Resize(newntri);
	m_FractVertex.Resize(newnv);	
	m_FractTexCoor.Resize(newnv);	
*/
