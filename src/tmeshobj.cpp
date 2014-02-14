//tmeshobj.cpp

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glu.h>
#include <oglmaterial.h>
#include <ply.h>
#include <maths.h>
#include <triangle_mesh.h>
#include <dynamic_array.h>
#include <objrw.h>
#include "tmeshobj.h"


int CTriangleObj::exportElemConnectivity(FILE *fp, const int objid, const int tetbaseno, const int vbaseno)
{
    const int ELMCLASSID =3;   //shell element
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;

	for (int i=0; i<m_nPolygonCount; i++){
		const int idx = tetbaseno+i;
		const Vector3i *pt = &m_pTriangle[i];
		const int vx = vbaseno + pt->x;
		const int vy = vbaseno + pt->y;
		const int vz = vbaseno + pt->z;
		fprintf(fp, "%d %d %d %d %d %d\n", 
					idx, ELMCLASSID, objid,
					vx, vy, vz);
	}
	return m_nPolygonCount;
}


void exportPlyVertices(FILE *fp, Vector3d *pVertex, const int nv, double _th, double *pThick, const int baseno, const double *pmatrix)
{ 
    char *OUT_ID_DBL3_FORMAT="%d %.14lg %.14lg %.14lg %.14lg\n";
	double matrix[4][4];    //transform
    double th=_th;          //default thickness

	if (pmatrix) vmCopy(pmatrix, matrix);
	for (int i=0; i<nv; i++){
		const int idx = baseno + i;
		const Vector3d& v = pVertex[i];
		double p1[3]={v.x, v.y, v.z}, p2[3];
		if (pmatrix){
			pmMult(p1, matrix, p2);
			VecAssign(p1, p2);
		}
        if (pThick) th=pThick[i];
		fprintf(fp, OUT_ID_DBL3_FORMAT, idx, p1[0], p1[1], p1[2], th);
	}
}


int CTriangleObj::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double thickness)
{
    exportPlyVertices(fp, m_pVertex, m_nVertexCount, thickness, NULL, baseno, pmatrix);
    return m_nVertexCount;
}


int CTriangleObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nTotalAttrib)
{
	int i, count;

	//read vertices;
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	m_nVertexCount = nv;
	LoadPltVertices(fp, nv, nTotalAttrib);

	//read the triangles;
	m_nPolygonCount = nf;
	m_pTriangle		= new Vector3i[m_nPolygonCount];
	assert(m_pTriangle!=NULL);

	for (i=0; i<nf; i++){
		int n0, n1, n2;
		count= fscanf(fp, "%d %d %d", &n0, &n1, &n2);
		n0--, n1--, n2--;
		if (count!=3 || n0<0 || n1<0 || n2<0){
			fprintf(stderr, "Error: reading triangle %d\n", i);
			exit(0);
		}
		m_pTriangle[i] = Vector3i(n0, n1, n2);
	}

	return 1;
}


int CTriangleObj::LoadFile(FILE *fp, const char *ftype)
{
	int status=0, nvert, nface, nfattrib;
	float radius;
	char chunktype[64];

	if (strcmp(ftype, ".PLT")==0 || strcmp(ftype, ".plt")==0){
		status = CPolyObj::ReadPltFileHeader(fp, chunktype, nvert, nface, radius, nfattrib, m_strFVarNames, m_strIVarNames);
		if (status == 0)
			return 0;
		status = LoadPltFileWithoutHeader(fp, nvert, nface, nfattrib);
		if (status == 0)
			return 0;
	}
	else if (strcmp(ftype, ".TXT")==0 || strcmp(ftype, ".txt")==0){
		status = _loadTXTFile(fp);
	}
	else if (strcmp(ftype, ".STL")==0 || strcmp(ftype, ".stl")==0){
		status = _loadSTLFile(fp);
	}
	else if (strcmp(ftype, ".PLY")==0 || strcmp(ftype, ".ply")==0){
		status = _loadPLYFile(fp);
	}
	//check status;
	if (status){
		AxisAlignedBox bbox;
		this->ComputeBoundingBox(bbox.minp, bbox.maxp);
		this->SetBoundingBox(bbox);
	}

	return status;
}


int CTriangleObj::LoadFile(const char *fname)
{
	FILE *fp;
	char filename[512];
	char * strExtensions[] = {".PLT", ".PLY", ".TXT", ".STL", ".OFF", ""};
	char * ftype = NULL;
	int i, flag;

	fp = fopen(fname, _RA_);
	if (fp==NULL) return 0;

	int len = strlen(fname);
	assert(len<512);
	filename[0]= fname[len-4];
	for (i=1; i<4; i++){
		int j = len-4+i;
#ifdef WIN32
		filename[i] = _toupper(fname[j]);
#else
		filename[i] = toupper(fname[j]);
#endif

	}
	filename[4]=0;

	for (i=0; strExtensions[i][0]!=0; i++){
		if (strcmp(filename, strExtensions[i])==0){
			ftype = strExtensions[i];
			break;
		}
	}

	flag = 0;	
	if (ftype)
		flag = LoadFile(fp, ftype);

	fclose(fp);
	return flag;
}

static CPolyObj* LoadTriangleMesh(const char *fname)
{
	CTriangleObj * p = new CTriangleObj;
	assert(p!=NULL);
	const int r = p->LoadFile(fname);
	if (!r){
		SafeDelete(p);
		p = NULL;
	}
	return p;
}

static bool _loadTri_PLY = CPolyObjLoaderFactory::AddEntry(".ply", LoadTriangleMesh);
static bool _loadTri_TXT = CPolyObjLoaderFactory::AddEntry(".txt", LoadTriangleMesh);
static bool _loadTri_STL = CPolyObjLoaderFactory::AddEntry(".stl", LoadTriangleMesh);
static bool _loadTri_OFF = CPolyObjLoaderFactory::AddEntry(".off", LoadTriangleMesh);


static bool SaveTriangleStlMesh(CPolyObj *pobj, const char *fname, const double *matrix)
{
	CTriangleObj *p = dynamic_cast<CTriangleObj*>(pobj);
	if (p){
		const int r = p->saveSTLFile(fname, matrix);
		return r;
	}
	return false;
}
static bool SaveTriangleTXTMesh(CPolyObj *pobj, const char *fname, const double *matrix)
{
	CTriangleObj *p = dynamic_cast<CTriangleObj*>(pobj);
	if (p){
		const int r = p->saveTXTFile(fname, matrix);
		return r;
	}
	return false;
}
static bool _saveTri_STL = CPolyObjWriterFactory::AddEntry(".stl", SaveTriangleStlMesh);
static bool _saveTri_TXT = CPolyObjWriterFactory::AddEntry(".txt", SaveTriangleTXTMesh);


int CTriangleObj::SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
{
	int i;
	Vector3d lowleft, upright;

	for (i=0; i<bufflen; i++){
		CTriangleObj *p = new CTriangleObj;
		assert(p!=NULL);
		_copyToObject(*p, objidbuff[i]);
		p->ComputeBoundingBox(lowleft, upright);
		p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
		obj[i] = p;
	}
	return bufflen;
}


void CTriangleObj::_copyToObject(CTriangleObj & aobj, const int objid)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	int i, k, c, nply, *vindex;
	float* pid = GetVertexObjectIDPointer();
	assert(pid!=NULL);
	assert(objid>=0);

	//count how many polygons;
	for (i=nply=0; i<m_nPolygonCount; i++){
		Vector3i& quad = m_pTriangle[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid) nply++;
	}
	assert(nply>0);
	aobj.m_nPolygonCount = nply;
	aobj.m_pPolygon = (int*)(new Vector3i[nply]);
	assert(aobj.m_pPolygon!=NULL);
	Vector3i*& m_pTriangle2 = (Vector3i*&)aobj.m_pPolygon;

	//copy the connectivity;
	for (i=c=0; i<m_nPolygonCount; i++){
		Vector3i& quad = m_pTriangle[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid)
			m_pTriangle2[c++] = quad;
	}
	assert(c==nply);

	//decide the new vertex buffer;
	vindex = new int [m_nVertexCount];
	assert(vindex!=NULL);
	for (i=0; i<m_nVertexCount; i++) vindex[i]=-1;
	for (i=c=0; i<nply; i++){
		Vector3i& quad = m_pTriangle2[i];
		k = quad.x;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.y;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.z;
		if (vindex[k]==-1) vindex[k]= c++;
	}

	//copy vertex buffer;
	aobj.m_nVertexCount = c;
	aobj.m_pVertex = new Vector3d[c];
	assert(aobj.m_pVertex!=NULL);
	for (i=0; i<m_nVertexCount; i++){
		k = vindex[i];
		if (k>=0)
			aobj.m_pVertex[k] = m_pVertex[i];
	}

	//fix the connectivity;
	for (i=c=0; i<nply; i++){
		Vector3i& quad = m_pTriangle2[i];
		quad.x = vindex[quad.x];
		quad.y = vindex[quad.y];
		quad.z = vindex[quad.z];
		if (quad.x==-1||quad.y==-1||quad.z==-1)
			assert(0);
	}

	//copy other polyobj class attributes;
	CPolyObj::CopyAttributes(aobj, vindex);
	delete []vindex;
}


typedef struct Vertex {
  float x,y,z;
  float nx,ny,nz;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

static int * _readTriangleStrip(FILE *fp, int &nv)
{
	fscanf(fp, "%d", &nv);
	int *ibuff = new int [nv+2];
	assert(ibuff!=NULL);
	for (int i=0; i<nv; i++)
		fscanf(fp, "%d", &ibuff[i]);
	return ibuff;
}

static Vector3i * _parseStrip2Triangles(const int *ibuff, const int nv, int &ntri)
{
	Vector3i *ptri = new Vector3i [nv];
	assert(ptri!=NULL);
	ntri = 0;
	int i=2;
	int spanlen=0;
	while (i<nv){
		const int c=ibuff[i];
		if (c==-1){
			i+=3;
			spanlen = 0;
		}
		else{
			const int a=ibuff[i-2];
			const int b=ibuff[i-1];
			if ((spanlen&0x1)==0)
				ptri[ntri]=Vector3i(a, b, c);
			else
				ptri[ntri]=Vector3i(a, c, b);
			ntri++;
			i++;
			spanlen++;
		}
	}
	return ptri;
}

int CTriangleObj::_loadPLYFile(FILE *fp)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	int i, j, elem_count, nverts, nfaces;
	char *elem_name, buff[200];
	Vertex vlist;
	Face flist;
	PlyOtherProp *vert_other,*face_other;
	PlyProperty vert_props[] = { /* list of property information for a vertex */
		{"x", Float32, Float32, offsetof(Vertex,x), 0, 0, 0, 0},
		{"y", Float32, Float32, offsetof(Vertex,y), 0, 0, 0, 0},
		{"z", Float32, Float32, offsetof(Vertex,z), 0, 0, 0, 0},
		{"nx", Float32, Float32, offsetof(Vertex,nx), 0, 0, 0, 0},
		{"ny", Float32, Float32, offsetof(Vertex,ny), 0, 0, 0, 0},
		{"nz", Float32, Float32, offsetof(Vertex,nz), 0, 0, 0, 0},
	};
	char *elem_names[] = { /* list of the kinds of elements in the user's object */
		"vertex", "face"
	};

	PlyProperty face_props[] = { /* list of property information for a face */
		{"vertex_indices", Int32, Int32, offsetof(Face,verts), 1, Uint8, Uint8, offsetof(Face,nverts)},
	};

	/*** Read in the original PLY object ***/
	Free();
	PlyFile *in_ply = read_ply (fp);
	if (in_ply==NULL) return 0;

	for (i = 0; i < in_ply->num_elem_types; i++){
		/* prepare to read the i'th list of elements */
		elem_name = setup_element_read_ply (in_ply, i, &elem_count);
		if (equal_strings ("vertex", elem_name)){
			/* create a vertex list to hold all the vertices */
			m_nVertexCount = nverts = elem_count;
			m_pVertex = new Vector3d [m_nVertexCount];
			assert(m_pVertex!=NULL);
			/* set up for getting vertex elements */
			setup_property_ply (in_ply, &vert_props[0]);
			setup_property_ply (in_ply, &vert_props[1]);
			setup_property_ply (in_ply, &vert_props[2]);
			vert_other = get_other_properties_ply (in_ply, offsetof(Vertex,other_props));
			/* grab all the vertex elements */
			for (j = 0; j < elem_count; j++) {
				get_element_ply (in_ply, (void *) &vlist);
				Vector3d *p = &m_pVertex[j];
				p->x = vlist.x;
				p->y = vlist.y;
				p->z = vlist.z;
			}
		}
		else if (equal_strings ("face", elem_name)) {
			m_nPolygonCount = nfaces = elem_count;
			/* create a list to hold all the face elements */
			m_pTriangle = new Vector3i[m_nPolygonCount];
			assert(m_pTriangle!=NULL);
			/* set up for getting face elements */
			setup_property_ply (in_ply, &face_props[0]);
			face_other = get_other_properties_ply (in_ply, offsetof(Face,other_props));
			/* grab all the face elements */
			for (j = 0; j < elem_count; j++) {
				get_element_ply (in_ply, (void *) &flist);
				if (flist.nverts !=3){
					sprintf(buff, "Found a face with %d vertices!\n", (int)flist.nverts);
					puts(buff);
				}
				Vector3i *ptri = &m_pTriangle[j];
				ptri->x = flist.verts[0];
				ptri->y = flist.verts[2];
				ptri->z = flist.verts[1];
			}
		}
		else if (equal_strings ("tristrips", elem_name)) {
			int *ibuff = _readTriangleStrip(fp, elem_count);
			m_pTriangle=_parseStrip2Triangles(ibuff, elem_count, m_nPolygonCount);
			delete [] ibuff;
		}
		else{
			get_other_element_ply (in_ply);
		}
	}

	close_ply (in_ply);
	return 1;
}


int CTriangleObj::_loadTXTFile(FILE *fp)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	int i, nVerticeFieldNumber, count, *rgb, color=0x00FFFF00;
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

	if (nVerticeFieldNumber>=2){
		m_pVertexNorm = new Vector3f [m_nVertexCount];
		assert(m_pVertexNorm!=NULL);
	}
	if (nVerticeFieldNumber>=3){
		m_pVertexColor = new unsigned char [m_nVertexCount][4];
		assert(m_pVertexColor!=NULL);
	}

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
		m_pVertexNorm[i] = Normalize(Vector3f(nx, ny, nz));

		//read colors;
		if (nVerticeFieldNumber<3) continue;
		count=fscanf(fp, "%d", &color);
		if (count!=1){
			fprintf(stderr, "Error: reading vertex %d\n", i);
			exit(0);
		}
		rgb=(int *)&m_pVertexColor[i][0];
		*rgb = color;
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

static inline bool _readSTLHeader(FILE *fp)
{
	char sbuffer[501];
	do{
		fgets(sbuffer, 500, fp);	//skip the first line, project name;
		char* foundheader = strstr(sbuffer, "solid ");
		if (foundheader)
			break;
	}while (!feof(fp)); 
	if (feof(fp)) return false;
	return true;
}

int CTriangleObj::_loadSTLFile(FILE *fp)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	char sbuffer[501], tmp1[501], tmp2[501];
	Vector3f norm, vert;
	Vector3f boxmin(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	Vector3f boxmax(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	int i, c, ntri=0, nvert=0;
	if (!_readSTLHeader(fp)) return 0;

	//read the file
	CDynamicArray<Vector3f> vbuffer(10240, 10240);
	do{
		fgets(sbuffer, 500, fp);			//fact normal xxx xxx xxx
		char* endsolid = strstr(sbuffer, "endsolid");
		if (endsolid) break;
		//read normal
		sscanf(sbuffer, "%s %s %f %f %f", tmp1, tmp2, &norm.x, &norm.y, &norm.z);
		fscanf(fp, "%s %s", tmp1, tmp2);	//outer loop
		//read the three vertices;
		double x, y, z;
		fscanf(fp, "%s %lf %lf %lf", tmp1, &x, &y, &z);
		vert.x=x, vert.y=y, vert.z=z;
		Minimize(boxmin, vert);
		Maximize(boxmax, vert);
		vbuffer.Add(vert);
		fscanf(fp, "%s %lf %lf %lf", tmp1, &x, &y, &z);
        vert.x=x, vert.y=y, vert.z=z;
		Minimize(boxmin, vert);
		Maximize(boxmax, vert);
		vbuffer.Add(vert);
		fscanf(fp, "%s %lf %lf %lf", tmp1, &x, &y, &z);
        vert.x=x, vert.y=y, vert.z=z;
		Minimize(boxmin, vert);
		Maximize(boxmax, vert);
		vbuffer.Add(vert);
		fscanf(fp, "%s", tmp1);				//endloop
		fscanf(fp, "%s\n", tmp1);				//endfacet
		nvert+=3;
	}while (!feof(fp));	
	
	//init triangle buffer
	ntri = nvert/3;
	Vector3i *ptri = new Vector3i [ntri];
	assert(ptri!=NULL);
	for (i=c=0; i<ntri; i++){
		Vector3i *pp = &ptri[i];
		pp->x = c++;
		pp->y = c++;
		pp->z = c++;
	}

	//call the optimize func. of CTriangleMesh 
	CTriangleMesh tri;
	tri.m_nVerticesNumber = nvert;
	tri.m_Vertice = vbuffer.GetBuffer();
	tri.m_pVertexNorm = tri.m_Vertice;
	tri.m_nTriangleNumber = ntri;
	tri.m_Triangles = ptri;
	tri.SetBoundingBox(boxmin, boxmax);
	tri.OptimizeMesh(3.3e-6);
	ntri = tri.m_nTriangleNumber;
	nvert = tri.m_nVerticesNumber;
	vbuffer.Resize(nvert);
	tri.m_Vertice = NULL;
	tri.m_Triangles = NULL;
	tri.m_pVertexNorm = NULL;

	//copy to the buffer;
	m_nVertexCount = nvert;
	m_nPolygonCount = ntri;
	m_pVertex = new Vector3d [m_nVertexCount];
	m_pTriangle = new Vector3i[m_nPolygonCount];
	assert(m_pVertex!=NULL);
	assert(m_pTriangle!=NULL);
    for (i=0; i<m_nVertexCount; i++){
		m_pVertex[i].x = vbuffer[i].x;
		m_pVertex[i].y = vbuffer[i].y;
		m_pVertex[i].z = vbuffer[i].z;
    }
	for (i=0; i<m_nPolygonCount; i++){
		Vector3i *p = &m_pTriangle[i];
		p->x=ptri[i].x;
		p->y=ptri[i].y;
		p->z=ptri[i].z;
	}

	return 1;
}


int CTriangleObj::saveSTLFile(const char *fname, const double* pmatrix)
{
    Vector3d p0, p1, p2;
	FILE *fp=fopen(fname, _WA_);
	if (fp==NULL) return 0;

	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	fprintf(fp, "solid Object01\n");
	for (int i=0; i<m_nPolygonCount; i++){
		const Vector3i tri = m_pTriangle[i];
		const Vector3d& _p0=m_pVertex[tri.x];
		const Vector3d& _p1=m_pVertex[tri.y];
		const Vector3d& _p2=m_pVertex[tri.z];  
		TransformVertex3dToVertex3d(_p0, pmatrix, &p0.x);
		TransformVertex3dToVertex3d(_p1, pmatrix, &p1.x);
		TransformVertex3dToVertex3d(_p2, pmatrix, &p2.x);
        const Vector3f p0f(p0.x, p0.y, p0.z);
        const Vector3f p1f(p1.x, p1.y, p1.z);
        const Vector3f p2f(p2.x, p2.y, p2.z);
		Vector3f norm = compute_triangle_normal(p0f,p1f,p2f);
		fprintf(fp, "facet normal %f %f %f\n", norm.x, norm.y, norm.z);
		fprintf(fp, "outer loop\n");
		const Vector3d *p=&p0;
		fprintf(fp, "vertex %.12lG %.12lG %.12lG\n", p->x, p->y, p->z);
		p=&p1;
		fprintf(fp, "vertex %.12lG %.12lG %.12lG\n", p->x, p->y, p->z);
		p=&p2;
		fprintf(fp, "vertex %.12lG %.12lG %.12lG\n", p->x, p->y, p->z);
		fprintf(fp, "endloop\n");
		fprintf(fp, "endfacet\n");
	}
	fprintf(fp, "endsolid Object01\n");
	fclose(fp);
	return 1;
}


int CTriangleObj::saveTXTFile(const char *fname, const double* pmatrix)
{
    int i;
	FILE *fp=fopen(fname, _WA_);
	if (fp==NULL) return 0;

	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	fprintf(fp, "%d %d\n1\n", m_nVertexCount, m_nPolygonCount);
	for (i=0; i<m_nVertexCount; i++){
        const Vector3d *_p= &m_pVertex[i];
        Vector3d p;
		TransformVertex3dToVertex3d(*_p, pmatrix, &p.x);
		fprintf(fp, "%.12lG %.12lG %.12lG\n", p.x, p.y, p.z);
    }
    for (i=0; i<m_nPolygonCount; i++){
		const Vector3i tri = m_pTriangle[i];
		fprintf(fp, "%d %d %d\n", tri.x, tri.y, tri.z);
	}
	fclose(fp);
	return 1;
}


void CTriangleObj::glDrawElementFaceGoround(const int eid, void* mirrorred)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	Vector3f normal = m_pPolyNorm[eid];
	if (mirrorred) normal = -normal;
	glNormal3fv(&normal.x);
	const Vector3i tri = m_pTriangle[eid];
	const Vector3d *v0= &m_pVertex[tri.x];
	const Vector3d *v1= &m_pVertex[tri.y];
	const Vector3d *v2= &m_pVertex[tri.z];

	if (this->NeedTexture1D() && m_pVertexTexCoor1D){
		const float tx = m_pVertexTexCoor1D[tri.x];
		const float ty = m_pVertexTexCoor1D[tri.y];
		const float tz = m_pVertexTexCoor1D[tri.z];
		glTexCoord1f(tx);
		glVertex3dv(&v0->x);
		glTexCoord1f(ty);
		glVertex3dv(&v1->x);
		glTexCoord1f(tz);
		glVertex3dv(&v2->x);
	}
	else{
		glVertex3dv(&v0->x);
		glVertex3dv(&v1->x);
		glVertex3dv(&v2->x);
	}
}


void CTriangleObj::glDrawElementFacePhong(const int eid, void *pvoid)
{
	assert(0);
}


void CTriangleObj::glDrawElementLine(const int eid, const float offset, const bool needtex)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	Vector3d p0, p1, p2, *vt = m_pVertex;
	Vector3d displacement(0,0,0);
	Vector3i &quad = m_pTriangle[eid];
	int v0=quad.x, v1=quad.y, v2=quad.z;

	if (fabs(offset)>1E-12){
		Vector3f &norm = m_pPolyNorm[eid];
		displacement.x = offset*norm.x;
		displacement.y = offset*norm.y;
		displacement.z = offset*norm.z;
		p0 = vt[v0] + displacement;
		p1 = vt[v1] + displacement;
		p2 = vt[v2] + displacement;
	}
	else{
		p0 = vt[v0];
		p1 = vt[v1];
		p2 = vt[v2];
	}
//	if (v0<v1){
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v0]);			
		glVertex3dv(&p0.x);
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v1]);			
		glVertex3dv(&p1.x);
//	}
//	if (v1<v2){
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v1]);			
		glVertex3dv(&p1.x);
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v2]);			
		glVertex3dv(&p2.x);
//	}
//	if (v2<v0){
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v2]);			
		glVertex3dv(&p2.x);
		if (needtex && m_pVertexTexCoor1D) glTexCoord1f(m_pVertexTexCoor1D[v0]);			
		glVertex3dv(&p0.x);
//	}
}


void CTriangleObj::ComputePolygonNormals(void)
{
	int f;
	if (m_pPolyNorm==NULL){
		//this is modified for deformable surface
		const int NSIZE = DETERMINE_DEFORMABLE_POLYSIZE(m_nPolygonCount);	
		m_pPolyNorm = new Vector3f[NSIZE];
		assert(m_pPolyNorm!=NULL);
	}
	for (f=0; f<m_nPolygonCount; f++){
		 m_pPolyNorm[f]=ComputeTriangleNormal(f);
	}
}


void CTriangleObj::ComputeVertexNormals(void)
{
    int i;
	Vector3f zero(0,0,0);
	Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;

	if (m_pVertexNorm==NULL){
		m_pVertexNorm = new Vector3f[m_nVertexCount];
		assert(m_pVertexNorm!=NULL);
	}
    for (i=0; i<m_nVertexCount; i++) m_pVertexNorm[i]=zero;

    //need the polygon normals
	{
		if (m_pPolyNorm==NULL){
			//this is modified for deformable surface
			const int NSIZE = DETERMINE_DEFORMABLE_POLYSIZE(m_nPolygonCount);	
			m_pPolyNorm = new Vector3f[NSIZE];
			assert(m_pPolyNorm!=NULL);
		}
		for (int f=0; f<m_nPolygonCount; f++){
			 m_pPolyNorm[f]=ComputeTriangleNormal(f, true);
		}
	}

	//==sum up
	for (i=0; i<m_nPolygonCount; i++){
	    const Vector3i& t = m_pTriangle[i];
        m_pVertexNorm[t.x]+= m_pPolyNorm[i];
        m_pVertexNorm[t.y]+= m_pPolyNorm[i];
        m_pVertexNorm[t.z]+= m_pPolyNorm[i];
	}

    for (i=0; i<m_nVertexCount; i++) m_pVertexNorm[i].normalize();
	for (i=0; i<m_nPolygonCount; i++) m_pPolyNorm[i].normalize();
}


//======================PICKING==========================


void CTriangleObj::DrawPickingObject(const int objid)
{
	int i, name = objid;
	Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;

	SETUP_PICKING_GLENV();
    glPushName(name);
	glBegin(GL_TRIANGLES);
	for (i=0; i<m_nPolygonCount; i++, name++){
		for (int t=0; t<3; t++){
			int p = m_pTriangle[i][t];
			const Vector3d& v = m_pVertex[p];
			Vector3f vf(v.x, v.y, v.z);
			glVertex3fv(&vf.x);
		}
	}
	glEnd();
    glPopName();
}

void CTriangleObj::DrawPickingObjectFace(const int objid)
{
	int i, name = objid;
	Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;
	Vector3i *ptri;
    Vector3d *v;

	//draw the triangles 
	SETUP_PICKING_GLENV();
	for (i=0; i<m_nPolygonCount; i++, name++){
		ptri = &m_pTriangle[i];
	    glPushName(name);
			glBegin(GL_TRIANGLES);
				v = &m_pVertex[ptri->x];
				glVertex3dv(&v->x);
				v = &m_pVertex[ptri->y];
				glVertex3dv(&v->x);
				v = &m_pVertex[ptri->z];
				glVertex3dv(&v->x);
			glEnd();
	    glPopName();
	}
}



inline void 
DrawPickinigTrianleEdge(Vector3d vt[], const Vector3d& disp, const int v0, const int v1, const int v2, const int Name)
{
	Vector3d p0 = vt[v0] + disp;
	Vector3d p1 = vt[v1] + disp;
	Vector3d p2 = vt[v2] + disp;

    glPushName(Name);
	glBegin(GL_LINES);
	    glVertex3dv(&p0.x);
	    glVertex3dv(&p1.x);
	glEnd();
    glPopName();

    glPushName(Name+1);
	glBegin(GL_LINES);
	    glVertex3dv(&p1.x);
	    glVertex3dv(&p2.x);
	glEnd();
    glPopName();

    glPushName(Name+2);
	glBegin(GL_LINES);
	    glVertex3dv(&p2.x);
	    glVertex3dv(&p0.x);
	glEnd();
    glPopName();
}
void CTriangleObj::DrawPickingObjectLine(const int objid)
{
	Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;
	int i, name = objid;
	SETUP_PICKING_GLENV();

	//draw the triangles first;
    glPushName(-1);
	glBegin(GL_TRIANGLES);
	for (i=0; i<m_nPolygonCount; i++){
		for (int t=0; t<3; t++){
			const int p = m_pTriangle[i][t];
			glVertex3dv(&m_pVertex[p].x);
		}
	}
	glEnd();
    glPopName();

	//then, draw the lines
	if (this->m_pPolyNorm==NULL)
		this->ComputePolygonNormals();
	const float offset = this->GetHiddenLineOffset();
    glLineWidth(2);
	for (i=0; i<m_nPolygonCount; i++, name+=3){
		Vector3f &norm = m_pPolyNorm[i];
		Vector3f disp = norm * offset; 
        Vector3d disp1(disp.x, disp.y, disp.z);
		Vector3I & tri = m_pTriangle[i];
		DrawPickinigTrianleEdge(m_pVertex, disp1, tri.x, tri.y, tri.z, name);
	}
}


void CTriangleObj::GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	const int EDGECOUNT = 3;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	int t = eid / EDGECOUNT;
	int e = eid % EDGECOUNT;
	Vector3i tri = m_pTriangle[t];
	
	int e1 = (e+1)%EDGECOUNT;
	int x = tri[e];
	int y = tri[e1];
	v0 = m_pVertex[x];
	v1 = m_pVertex[y];
}



void CTriangleObj::GetPickedLine(const int eid, int & v0, int &v1)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	const int EDGECOUNT = 3;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	const int t = eid / EDGECOUNT;
	const int e = eid % EDGECOUNT;
	const Vector3i tri = m_pTriangle[t];
	
	const int e1 = (e+1)%EDGECOUNT;
	v0 = tri[e];
	v1 = tri[e1];
}


void CTriangleObj::GetPickedPlane(const int eid, Vector3d v[4])
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	assert(eid>=0 && eid<m_nPolygonCount);
	Vector3i tri = m_pTriangle[eid];
	printf("Triangle %d (%d,%d,%d) picked.", eid, tri.x, tri.y, tri.z); 

	const int *pVertexAbsIndex = GetVertexAbsIndexPointer();
	if (pVertexAbsIndex!=NULL){
		const int v0 = pVertexAbsIndex[tri.x];
		const int v1 = pVertexAbsIndex[tri.y];
		const int v2 = pVertexAbsIndex[tri.z];
		printf(" Absolute vertex indices (%d,%d,%d).", v0, v1, v2); 
	}
	printf("\n");

	v[0] = m_pVertex[tri.x];
	v[1] = m_pVertex[tri.y];
	v[2] = m_pVertex[tri.z];
	v[3] = m_pVertex[tri.z];
}


//Save the mesh into POVRAY's mesh2 format;
int CTriangleObj::exportPovrayFile(FILE *fp, const double *matrix)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	Vector3I *t;
	Vector3f *vv, *nn;
	int i;
	double v[3];

	//output vertices;
	vv = new Vector3f [m_nVertexCount];
	nn = new Vector3f [m_nVertexCount];
	assert(vv!=NULL && nn!=NULL);
	fprintf(fp, "mesh2 {\n");
	fprintf(fp, "  vertex_vectors {\n");
	fprintf(fp, "\t%d,\n", m_nVertexCount); 
	for (i=0; i<m_nVertexCount-1; i++){
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, v);
		fprintf(fp, "\t<%lf,%lf,%lf>,\n", v[0], v[1], -v[2]);
		vv[i].x = v[0];
		vv[i].y = v[1];
		vv[i].z = v[2];
	}
	TransformVertex3dToVertex3d(m_pVertex[m_nVertexCount-1], matrix, v);
	fprintf(fp, "\t<%lf,%lf,%lf>\n  }\n", v[0], v[1], -v[2]);
/*
	//output normal vectors;
	for (i=0; i<m_nVertexCount; i++) nn[i]=Vector3f(0);
	for (i=0; i<m_nPolygonCount; i++){
		t = &m_pTriangle[i];
		Vector3f& a = m_pVertex[t->x];
		Vector3f& b = m_pVertex[t->y];
		Vector3f& c = m_pVertex[t->z];
		Vector3f norm = compute_triangle_normal(a, b, c);
		nn[t->x]+= norm;
		nn[t->y]+= norm;
		nn[t->z]+= norm;
	}
	for (i=0; i<m_nVertexCount; i++) nn[i].Normalize();
	fprintf(fp, "  normal_vectors {\n");
	fprintf(fp, "\t%d,\n", m_nVertexCount); 
	for (i=0; i<m_nVertexCount-1; i++){
		v[0]=nn[i].x;
		v[1]=nn[i].y;
		v[2]=nn[i].z;
		fprintf(fp, "\t<%lf,%lf,%lf>,\n", v[0], v[1], -v[2]);
	}
	v[0]=nn[m_nVertexCount-1].x;
	v[1]=nn[m_nVertexCount-1].y;
	v[2]=nn[m_nVertexCount-1].z;
	fprintf(fp, "\t<%lf,%lf,%lf>\n", v[0], v[1], -v[2]);
	fprintf(fp, "  }\n\n");
*/
	//output triangles
	fprintf(fp, "  face_indices {\n");
	fprintf(fp, "\t%d\n", m_nPolygonCount); 

	for (i=0; i<m_nPolygonCount-1; i++){
		t = &m_pTriangle[i];
		fprintf(fp, "\t<%d,%d,%d>,\n", t->x, t->y, t->z);
	}
	t = &m_pTriangle[m_nPolygonCount-1];
	fprintf(fp, "\t<%d,%d,%d>\n  }\n", t->x, t->y, t->z);

	fprintf(fp, "}\n");

/* An example mesh
mesh2 {
   vertex_vectors {
      24, 
      <0,0,0>, <0.5,0,0>, <0.5,0.5,0>, //1
      <0.5,0,0>, <1,0,0>, <0.5,0.5,0>, //2
      <1,0,0>, <1,0.5,0>, <0.5,0.5,0>, //3
      <1,0.5,0>, <1,1,0>, <0.5,0.5,0>, //4
      <1,1,0>, <0.5,1,0>, <0.5,0.5,0>, //5
      <0.5,1,0>, <0,1,0>, <0.5,0.5,0>, //6
      <0,1,0>, <0,0.5,0>, <0.5,0.5,0>, //7
      <0,0.5,0>, <0,0,0>, <0.5,0.5,0>  //8
   }
   normal_vectors {
      9,
      <-1,-1,0>, <0,-1,0>, <0,0,1>,
      <1,-1,0>, <1,0,0>, <1,1,0>,
      <0,1,0>, <-1,1,0>, <-1,0,0>
   }
   face_indices {
      8, 
      <0,1,2>,    <3,4,5>,       //1 2
      <6,7,8>,    <9,10,11>,     //3 4
      <12,13,14>, <15,16,17>,    //5 6
      <18,19,20>, <21,22,23>     //7 8
   }
   pigment {rgb 1}
}
*/
	delete [] vv;
	delete [] nn;
	return 1;
}


bool CTriangleObj::saveMeshWavefrontObj(const char *fname, const double *matrix)
{
	int i;

	Vector3d *pVertex = new Vector3d[m_nVertexCount];
	assert(pVertex!=NULL);
	for (i=0; i<m_nVertexCount; i++)
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, &pVertex[i].x);

	vector<OglMaterial> mats;

	std::string objname = GetObjectName();
	if (objname.empty()) objname+="defaultname";
	Vector3f *pNormal = m_pVertexNorm;
	if (pNormal==NULL){
		ComputeVertexNormals();
		pNormal = m_pVertexNorm;
		assert(pNormal!=NULL);
	}
	vector<Vector3f> texCoords;
	unsigned int faceMatIndex[2], nFaceMatIndex=0;
	const Vector3i *pTexIndex = NULL;

	const bool r = writeOBJFile(fname, objname,
		mats, 
		pVertex, m_nVertexCount,
		pNormal, m_nVertexCount, texCoords,
		(const Vector3i*)m_pPolygon, m_nPolygonCount, pTexIndex,
		faceMatIndex, nFaceMatIndex);

	delete [] pVertex;
	return r;
}


void CTriangleObj::exportFile(FILE *fp, const char *format, const double *matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0)
		exportPovrayFile(fp, matrix);

}


void CTriangleObj::flipNormal(void)
{
    //flip vertex normals and polygon normals;
    CPolyObj::flipNormal();
    //flip quad orientation
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
    for (int i=0; i<m_nPolygonCount; i++){
        Vector3i * p = &m_pTriangle[i];
        int tmp = p->y;
        p->y = p->z;
        p->z = tmp;
    }
}
