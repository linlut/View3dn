//FILE: meshfileio.cpp

#include <sysconf.h>
#include <vectorall.h>
#include <vector>

static int _loadQuadObjFile(FILE *fp, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad, Vector2f *&pVertexTexCoord)
{
	int i;
	const int BUFSIZE=1024;
	char buf[BUFSIZE+1];
	vector<Vector3d> pos;
	vector<Vector2f> texcoord;
	vector<Vector4i> quad;
	Vector3d v;
	const Vector3i one(1, 1, 1);

	while (!feof(fp)){
		fgets(buf, BUFSIZE, fp);
		if (buf[0]=='v' && buf[1]==' '){//vertex pos line
			sscanf(buf, "v %lg %lg %lg", &v.x, &v.y, &v.z);
			pos.push_back(v);
		}
		else if (buf[0]=='v' && buf[1]=='t'){//vertex texture coord line
			float tx, ty;
			sscanf(buf, "vt %f %f", &tx, &ty);
			texcoord.push_back(Vector2f(tx, ty));
		}
		else if (buf[0]=='f' && buf[1]==' '){//polyline
			buf[0]=' ';
			Vector3i x[4];
			sscanf(buf, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", 
				&x[0].x, &x[0].y, &x[0].z, 
				&x[1].x, &x[1].y, &x[1].z, 
				&x[2].x, &x[2].y, &x[2].z, 
				&x[3].x, &x[3].y, &x[3].z);
			x[0]-=one, x[1]-=one, x[2]-=one, x[3]-=one;
			quad.push_back(Vector4i(x[0].x, x[1].x, x[2].x, x[3].x));
		}
	}

	nv = pos.size();
	pVertex = new Vector3d[nv];
	for (i=0; i<nv; i++) pVertex[i]=pos[i];

	nquad = quad.size();
	pQuad = new Vector4i[nquad];
	for (i=0; i<nquad; i++) pQuad[i]=quad[i];

	pVertexTexCoord = new Vector2f[nv];
	for (i=0; i<nv; i++) pVertexTexCoord[i]=texcoord[i];
	return true;
}


static int _loadQuadFile(FILE *fp, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad)
{
	int i, nVerticeFieldNumber;
	double x, y, z;

	fscanf(fp, "%d", &nv);
	fscanf(fp, "%d", &nquad);
	fscanf(fp, "%d", &nVerticeFieldNumber);
	assert(nVerticeFieldNumber==1);

	pVertex = new Vector3d[nv];
	pQuad = new Vector4i[nquad];
	assert(pVertex != NULL);
	assert(pQuad != NULL);

	for (i=0; i<nv; i++){
		const int count=fscanf(fp, "%lf %lf %lf", &x,&y,&z);
		if (count != 3){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			goto CLEANUP;
		}
		pVertex[i] = Vector3d(x, y, z);
	}

	for (i=0; i<nquad; i++){
		int tx, ty, tz, tw;
		const int count= fscanf(fp, "%d %d %d %d", &tx, &ty, &tz, &tw);
		if (count!=4 || tx<0 || ty<0 || tz<0 || tw<0){
			fprintf(stderr, "Error: reading triangle %d connectivity!\n", i);
			goto CLEANUP;
		}
		pQuad[i]=Vector4i(tx, ty, tz, tw);
	}
	return 1;

CLEANUP:	//error handling
	SafeDeleteArray(pVertex);
	SafeDeleteArray(pQuad);
	pVertex = NULL;
	pQuad = NULL;
	return 0;
}

static int _loadTXTFile(FILE *fp, Vector3d *& m_pVertex, int &m_nVertexCount, Vector3i* &m_pTriangle, int &m_nPolygonCount)
{
	int i, nVerticeFieldNumber, count, color=0x00FFFF00;
	double x, y, z;
	float nx, ny, nz;

	fscanf(fp, "%d", &m_nVertexCount);
	fscanf(fp, "%d", &m_nPolygonCount);
	fscanf(fp, "%d", &nVerticeFieldNumber);
	assert(nVerticeFieldNumber<=3 && nVerticeFieldNumber>=1);

	m_pVertex = new Vector3d [m_nVertexCount];
	m_pTriangle = new Vector3i[m_nPolygonCount];
	assert(m_pVertex!=NULL);
	assert(m_pTriangle!=NULL);

	for (i=0; i<m_nVertexCount; i++){
		//read coordinates;
		count=fscanf(fp, "%lf %lf %lf", &x,&y,&z);
		nx = 1, ny = nz=0;
		if (count!=3){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			exit(0);
		}
		m_pVertex[i] = Vector3d(x, y, z);

		//read normals;
		if (nVerticeFieldNumber<2) continue;
		count=fscanf(fp, "%f %f %f", &nx,&ny,&nz);
		if (count!=3){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			exit(0);
		}

		//read colors;
		if (nVerticeFieldNumber<3) continue;
		count=fscanf(fp, "%d", &color);
		if (count!=1){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			exit(0);
		}
	}

	for (i=0; i<m_nPolygonCount; i++){
		int& tx = m_pTriangle[i][0];
		int& ty = m_pTriangle[i][1];
		int& tz = m_pTriangle[i][2];
		count= fscanf(fp, "%d %d %d", &tx, &ty, &tz);
		if (count!=3 || tx<0 || ty<0 || tz<0){
			fprintf(stderr, "Error: reading triangle %d connectivity!\n", i);
			exit(0);
		}
	}
	return 1;
}

bool loadQuadMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad, Vector2f *&pVertexTexCoord)
{
	bool rflag=false;
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return false;

	int fnamelen = strlen(fname);
	if (strcmp(&fname[fnamelen-4], ".obj")==0){
		rflag = _loadQuadObjFile(fp, pVertex, nv, pQuad, nquad, pVertexTexCoord);
	}
	else if (strcmp(&fname[fnamelen-5], ".quad")==0){
		pVertexTexCoord = NULL;
		rflag = (bool)_loadQuadFile(fp, pVertex, nv, pQuad, nquad);
	}
	fclose(fp);
	return rflag;
}

bool loadTriangleMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector3i*& pQuad, int &nquad, Vector2f *&pVertexTexCoord)
{
	pVertexTexCoord =NULL;
	FILE *fp = fopen(fname, _RA_);
	if (fp==NULL) return false;
	bool rflag = (bool)_loadTXTFile(fp, pVertex, nv, pQuad, nquad);
	fclose(fp);
	return rflag;
}

//===============Neutral mesh format, NETGEN MESH Output========================

static bool read_nodes(const char *innodefname, int& nv, Vector3d *& pVertex, int &startid)
{
	FILE *fpin;
	int i, tmp0, tmp1, tmp2;
	double x, y, z;

	fpin = fopen(innodefname, "r");
	assert(fpin!=NULL);
	fscanf(fpin, "%d %d %d %d", &nv, &tmp0, &tmp1, &tmp2);
	assert(nv>0 && tmp0==3);
	pVertex = new Vector3d[nv];
	assert(pVertex!=NULL);

	for (i=0; i<nv; i++){
		fscanf(fpin, "%d %lf %lf %lf", &tmp0, &x, &y, &z);
		if (i==0){
			assert(tmp0==0 || tmp0==1);
			startid=tmp0;
		}
		pVertex[i].x = x;
		pVertex[i].y = y;
		pVertex[i].z = z;
	}
	fclose(fpin);
	return true;
}


static bool read_face(const char *infacefname, const int startid, int& ntri, Vector3i *& pFace)
{
	int tmp0, tmp1;
	FILE *fpin = fopen(infacefname, "r");
	assert(fpin!=NULL);
	assert(startid==0 || startid==1);
	fscanf(fpin, "%d %d", &ntri, &tmp0);
	assert(ntri>0);
	assert(tmp0==0 || tmp0==1);
	pFace = new Vector3i[ntri];
	assert(pFace!=NULL);

	for (int i=0; i<ntri; i++){
		int n0, n1, n2;
		fscanf(fpin, "%d %d %d %d", &tmp1, &n0, &n1, &n2);
		if (startid==1){
			n0-=startid;
			n1-=startid;
			n2-=startid;
		}

		//reverse the triangle orientation because it is not correct!!
		pFace[i].x = n0, pFace[i].y = n2, pFace[i].z = n1; 
		if (tmp0==1) 
			fscanf(fpin, "%d", &tmp1); 
	}
	fclose(fpin);
	return true;
}


static bool read_ele(const char *inelefname, const int startid, int& ntet, Vector4i *& pFace)
{
	int tmp0, tmp1;
	//read/write elements;
	FILE *fpin = fopen(inelefname, "r");
	assert(fpin!=NULL);
	assert(startid==0 || startid==1);
	fscanf(fpin, "%d %d %d", &ntet, &tmp0, &tmp1);
	assert(ntet>0 && tmp0==4);
	pFace = new Vector4i[ntet];
	assert(pFace!=NULL);

	for (int i=0; i<ntet; i++){
		int n0, n1, n2, n3;
		fscanf(fpin, "%d %d %d %d %d", &tmp0, &n0, &n1, &n2, &n3);
		if (startid==1){
			n0-=startid;
			n1-=startid;
			n2-=startid;
			n3-=startid;		
		}
		pFace[i].x=n0;
		pFace[i].y=n1;
		pFace[i].z=n2;
		pFace[i].w=n3;
	}
	fclose(fpin);
	return true;
}



//Read neutral mesh file format generated by NETGEN
//
static Vector3d* read_netgen_neutralfile_nodes(FILE *fpin, int& nv)
{
	fscanf(fpin, "%d", &nv);
	Vector3d *pVertex = new Vector3d[nv];
	assert(pVertex!=NULL);
	for (int i=0; i<nv; i++){
		double x, y, z;
		Vector3d * v = &pVertex[i];
		fscanf(fpin, "%lf %lf %lf", &x, &y, &z);
		v->x = x, v->y = y, v->z = z;
	}
	return pVertex;
}

static int* read_netgen_neutralfile_elms(FILE *fpin, int& ntet, int &mshtype)
{
	int i, tmp0, nstep, n[8];

	fscanf(fpin, "%d", &ntet);
	assert(ntet>0);
	fscanf(fpin, "%d %d %d %d %d", &mshtype, &n[0], &n[1], &n[2], &n[3]);
	assert(mshtype==1 || mshtype==2);
	if (mshtype==1)
		nstep = 4;
	else{ 
		nstep = 8;
		fscanf(fpin, "%d %d %d %d", &n[4], &n[5], &n[6], &n[7]);
	}

	int *pFace = new int [nstep*ntet];
	assert(pFace!=NULL);
	for (i=0; i<nstep; i++) pFace[i]=n[i]-1;

	for (int j=1; j<ntet; j++, i+=nstep){
		int *a = &pFace[i];
		switch(mshtype){
		case 1:  
			fscanf(fpin, "%d %d %d %d %d", &tmp0, &a[0], &a[1], &a[2], &a[3]);
			a[0]--, a[1]--; a[2]--, a[3]--;
			break;
		case 2:
			fscanf(fpin, "%d %d %d %d %d %d %d %d %d", 
				&tmp0, &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6], &a[7]);
			a[0]--, a[1]--; a[2]--, a[3]--;
			a[4]--, a[5]--; a[6]--, a[7]--;
			break;
		default:
			assert(0);
			break;
		}
		assert(tmp0==mshtype);
	}
	return pFace;
}


static int* read_netgen_neutralfile_face(const int meshtype, FILE *fpin, int& ntri)
{
	assert(meshtype==1 || meshtype==2);
	fscanf(fpin, "%d", &ntri);
	assert(ntri>0);
	int* pFace = new int [ntri*(2+meshtype)];
	assert(pFace!=NULL);
	int c=0;
	for (int i=0; i<ntri; i++){
		int n0, n1, n2, n3, tmp1;
		if (meshtype==1){
			fscanf(fpin, "%d %d %d %d", &tmp1, &n0, &n1, &n2);
			pFace[c++] = n0-1, pFace[c++] = n1-1, pFace[c++] = n2-1; 
		}
		else{
			fscanf(fpin, "%d %d %d %d %d", &tmp1, &n0, &n1, &n2, &n3);
			pFace[c++] = n0-1, pFace[c++] = n1-1, 
			pFace[c++] = n2-1; pFace[c++] = n3-1; 
		}
	}
	return pFace;
}


static void check_elms(const Vector3d *pVertex, Vector4i *pElem, const int ntet)
{
	assert(pElem!=NULL);
	assert(pVertex!=NULL);
	for (int i=0; i<ntet; i++){
		Vector4i & tet = pElem[i];
		const Vector3d *p1 = &pVertex[tet.x];
		const Vector3d *p2 = &pVertex[tet.y];
		const Vector3d *p3 = &pVertex[tet.z];
		const Vector3d *p4 = &pVertex[tet.w];
		Vector3d e1 = (*p2) - (*p1);
		Vector3d e2 = (*p3) - (*p2);
		Vector3d e3 = (*p4) - (*p1);
		Vector3d n = CrossProd(e1, e2);
		double r = DotProd(n, e3);
		if (r<0){ //need to change orientation of the element;
			int itmp = tet.z;
			tet.z = tet.y;
			tet.y = itmp;
		}
	}
}


bool simLoadNeutralMeshFile(const char *fname, 
	Vector3d *& pVertex, int &nv, 
	int *& pFace, int &ntri,
	int *& pTet, int &ntet, int &meshtype)
{
	//read files;
	FILE *fp = fopen(fname, "r");
	if (fp==NULL) return false;
	pVertex = read_netgen_neutralfile_nodes(fp, nv);
	pTet = read_netgen_neutralfile_elms(fp, ntet, meshtype);
	assert(meshtype==1 || meshtype==2);
	pFace = read_netgen_neutralfile_face(meshtype, fp, ntri);
	fclose(fp);

	//check the orientation;
	if (meshtype==1) //tet mesh
		check_elms(pVertex, (Vector4i*)pTet, ntet);
	return true;
}



