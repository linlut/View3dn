/*
 triangle_mesh.cpp
*/
#include "sysconf.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "matrix.h"
#include "ply.h"
#include "vectorall.h"
#include "dynamic_array.h"
#include "drawparms.h"
#include "triangle_mesh.h"
#include "geomath.h"

#define Vector3D Vector3f

void CTriangleMesh::_init(void)
{
	m_Vertice = NULL;
	m_pVertexNorm = NULL;
	m_Triangles = NULL;
	m_nVerticesNumber = 0;
	m_nTriangleNumber = 0;
	m_nVerticeFieldNumber=0;
	m_pVertexColor=NULL;
	m_pTriangleNorm=NULL;
	m_pRadius = NULL;
	//
	int i;
	for (i=0;i<32; i++){
		m_pAttributes[i] = NULL;
		m_strVarNames[i][0] = 0;
	}

	//
	m_pPrintFunc = NULL;
	m_ppEdgeTable = NULL;

	//
	m_pQuads = NULL;
	m_nQuads = 0;
}



CTriangleMesh::CTriangleMesh(int vertex_num, Vector3f vertex[], Vector3f normal[], int triangle_num, Vector3I *triangle)
: CMemoryMgr()
{
	_init();

	//
	m_Vertice = vertex;
	m_pVertexNorm = normal;
	m_Triangles = triangle;
	m_nVerticesNumber = vertex_num;
	m_nTriangleNumber = triangle_num;
	m_nVerticeFieldNumber=2;
	
	m_pDrawParms = NULL;
	//
	//m_pVertexColor = new unsigned char[vertex_num][4];
	//assert(m_pVertexColor!=NULL);
}


CTriangleMesh::CTriangleMesh(void): CMemoryMgr()
{
	m_pDrawParms = NULL;
	_init();
}



void CTriangleMesh::ClearAllBuffer(void)
{
	int i;

	SafeDeleteArray(m_Vertice);
	SafeDeleteArray(m_pVertexNorm);
	SafeDeleteArray(m_Triangles);
	SafeDeleteArray(m_ppEdgeTable);
	SafeDeleteArray(m_pVertexColor);
	SafeDeleteArray(m_pTriangleNorm)
	SafeDeleteArray(m_pRadius);

	//========Extensions for PLT===================
	if (m_pQuads!=NULL)
		delete [] m_pQuads;

	for (i=0;i<32; i++){
		if (m_pAttributes[i] != NULL){
			delete [] m_pAttributes[i];
			m_pAttributes[i] = NULL;
		}
		m_strVarNames[i][0] = 0;
	}

	m_pQuads = NULL;
	m_nQuads = 0;

	//==================================
	m_Vertice = NULL;
	m_pVertexNorm = NULL;
	m_Triangles = NULL;
	m_pVertexColor = NULL;

	m_nVerticesNumber = 0;
	m_nTriangleNumber = 0;	
	m_nVerticeFieldNumber = 0;
	m_pRadius = NULL;

	//Free memory buffer;
	CMemoryMgr::Free();
}


CTriangleMesh::~CTriangleMesh(void)
{
	ClearAllBuffer();
}


double CTriangleMesh::GetTrianglesTotalArea(void)
{
	double area=0;
	int v0, v1, v2;

	for (int i=0; i<m_nTriangleNumber; i++){
		v0= m_Triangles[i][0];
		v1= m_Triangles[i][1];
		v2= m_Triangles[i][2];
		area+=triangle_area(m_Vertice[v0], m_Vertice[v1], m_Vertice[v2]);
	}
	return area;
}



inline bool FIND_STRING_NAME(char *str, const char TOKEN, int& startpos, int &endpos)
{
	bool r = false;
	int i, lens = strlen(str);

	startpos = lens;
	for (i=0; i<lens; i++){
		if (str[i]==TOKEN){
			startpos = i;
			break;
		}
	}
	for (i=startpos+1; i<lens; i++){
		if (str[i]==TOKEN){
			endpos = i;
			r = true;
			break;
		}
	}
	return r;
}


static int COPY_NAMES(char* varnames, char strVarNames[32][9])
{
	const char TOKEN = 0x22; // '\"';
	int i, startpos, endpos;
	bool hasname = true;
	char *pcurname = varnames;

	//VARIABLES = "X","Y","Z","DX","DY","DZ","VX","VY","VZ","SXX","SYY","SZZ","SXY","SYZ","SZX","EPS","DM"
	memset(strVarNames, 0, sizeof(char)*32*9);
	for (i=0; hasname==true; i++){
		 hasname = FIND_STRING_NAME(pcurname, TOKEN, startpos, endpos);
		 if (hasname){
			 startpos++;
			 char *p = strVarNames[i];
			 int maxpos = _MIN_(endpos, startpos+8);
			 for (int j=startpos; j<maxpos; j++){
				 int k = j-startpos;
				 p[k]= pcurname[j];
			 }			
			 pcurname = &pcurname[endpos+1];
		 }
	}
	assert(i<=32);
	return i-1;
}


inline float READ_FORTRAN_FLOAT(char * s)
{
	ASSERT0(s[0]!=0);
	double d;
	float f;
	int i;
	int n = strlen(s);
	if (n>4){
		bool hase =false;
		bool hassign = false;
		int pos;
		for (i=0; i<n; i++){
			if (s[i]=='E' || s[i]=='e'){
				hase = true;
				break;
			}
		}
		if (!hase){
			for (i=n-4; i<n; i++){
				if (s[i]=='-' || s[i]=='+'){
					hassign = true;
					pos = i;
					break;
				}
			}
			if (hassign){
				for (i=n-1; i>=pos; i--)
					s[i+1] = s[i];
				s[pos]='E';
				s[n+1]=0;
			}
		}
	}
	sscanf(s, "%lf", &d);
	if (fabs(d)<1E-37)
		f = 0;
	else
		f = (float)d;
	return f;
}


//Load techplot .plt format file
int CTriangleMesh::LoadPltFile(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp==NULL) return 0;
	int r = LoadPltFile(fp);
	fclose(fp);
	return r;
}


//Load techplot .plt format file
int CTriangleMesh::LoadPltFile(FILE *fp)
{
	int i, j, nf, elmtype, count, color=0x00FFFF00;
	char sbuffer[512], varnames[512], selmtype[100], tmpstr[100];
	char strzon[6], *pchar, fepoint[100];

	// init
	ClearAllBuffer();
	
	// read header
	do{
		fgets(sbuffer, 500, fp);		//skip the first line, project name;
		char* foundheader = strstr(sbuffer, "TITLE = CAV");
		if (foundheader)
			break;
	}while (!feof(fp)); 
	if (feof(fp)) return 0;

	fgets(varnames, 500, fp);		//the variable names;

	//read the third line;
	fgets(sbuffer, 500, fp);		//possible also variable names;
	pchar = sbuffer;
	while (*pchar==' ') pchar++;	//filter beginning ' '
	int slength = strlen(pchar);	//filter ','
	for (i=0; i<slength; i++)		
		if (pchar[i]==',') pchar[i]= ' ';

	for (i=0; i<4; i++) strzon[i] = pchar[i];
	strzon[4]=0;
	m_fParticleRadius = 0;
	if (strcmp(strzon, "ZONE")==0)
		sscanf(pchar, "ZONE N = %d  E = %d F =%s ET =%s RAD =%f\n", &m_nVerticesNumber, &nf, fepoint, selmtype, &m_fParticleRadius);		
	else{
		fscanf(fp, "%s N = %d  E = %d F =%s ET =%s RAD =%f\n", tmpstr, &m_nVerticesNumber, &nf, fepoint, selmtype, &m_fParticleRadius);
		strcat(varnames, sbuffer);
	}

	//copy all the filed names;
	int nTotalAttrib = COPY_NAMES(varnames, m_strVarNames)-3;

	//decide mesh type;
	if (strcmp(selmtype, "QUADRILATERAL")==0){ //all the elements are quads
		elmtype = 4;
		m_nTriangleNumber=2*nf;
	}
	else if (strcmp(selmtype, "TRIANGLE")==0){ //all the elements are triangles
		elmtype = 3;
		m_nTriangleNumber=1*nf;
	}
	else if (strcmp(selmtype, "PARTICLE")==0){ //all the elements are triangles
		elmtype = 1;
		nf = m_nVerticesNumber;
		m_nTriangleNumber=1*nf;
	}
	else
		return 0;

	m_nVerticeFieldNumber=2;
	m_Vertice = new Vector3D [m_nVerticesNumber];
	m_pVertexNorm = new Vector3D [m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexColor = new unsigned char [m_nVerticesNumber][4];
	assert(m_Vertice!=NULL);
	assert(m_pVertexNorm!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexColor!=NULL);

	// some other buffer;
	for (i=0; i<nTotalAttrib; i++){
		m_pAttributes[i] = new float [m_nVerticesNumber];
		assert(m_pAttributes[i]!=NULL);
	}

	//read the vertices;
	for (i=0; i<m_nVerticesNumber; i++){		
		float v[40];
		for (j=0; j<nTotalAttrib+3; j++){
			fscanf(fp, "%s", sbuffer);
			v[j]= READ_FORTRAN_FLOAT(sbuffer);
		}
		m_Vertice[i] = Vector3D(v[0], v[1], v[2]);
		for (j=0; j<nTotalAttrib; j++){
			m_pAttributes[j][i] = v[j+3];
		}
	}

	//read the triangles;
	if (elmtype==4){ //all the elements are quads
		m_nQuads = nf;
		m_pQuads = new Vector4I[nf];
		assert(m_pQuads!=NULL);
		for (i=0; i<nf; i++){
			int n0, n1, n2, n3;
			count= fscanf(fp, "%d %d %d %d", &n0, &n1, &n2, &n3);
			n0--, n1--, n2--, n3--;
			if (count!=4 || n0<0 || n1<0 || n2<0 || n3<0){
				fprintf(stderr, "Error: reading triangle %d\n", i);
				exit(0);
			}
			j = 2*i;
			m_Triangles[j] = Vector3I(n0, n1, n2);
			m_Triangles[j+1] = Vector3I(n0, n2, n3);
			m_pQuads[i] = Vector4I(n0, n1, n2, n3);
		}
	}
	else if (elmtype==3){ //all the elements are triangles		
		for (i=0; i<nf; i++){
			int n0, n1, n2;
			count= fscanf(fp, "%d %d %d", &n0, &n1, &n2);
			n0--, n1--, n2--;
			if (count!=3 || n0<0 || n1<0 || n2<0){
				fprintf(stderr, "Error: reading triangle %d\n", i);
				exit(0);
			}
			m_Triangles[i] = Vector3I(n0, n1, n2);
		}
	}
	else if (elmtype==1){ //all the elements are particles
		m_pRadius = new float [nf];
		assert(m_pRadius!=NULL);
		for (i=0; i<nf; i++){
			int n0;
			float radius;
			fscanf(fp, "%d %f", &n0, &radius);
			n0--;
			m_Triangles[i] = Vector3I(n0, n0, n0);
			m_pRadius[i] = radius;
		}
	}
	
	//compute normals
	if (elmtype!=1){
		ComputeVertexArrayNormal();
	}
	else{
		assert(elmtype == 1);
		if (m_pVertexNorm==NULL)
			m_pVertexNorm = new Vector3D[m_nVerticesNumber];
		assert(m_pVertexNorm!=NULL);
		for (i=0; i<m_nVerticesNumber; i++){
			m_pVertexNorm[i]=Vector3D(0,0,1);
		}
	}

	return 1;
}





/*
Simple data file format:

  (#of vertices)
  (#of Triangles)
  vertices' x y z nx ny nz
  ...
  ...
  triangle vertice order
  v0 v1 v2
  ...
  ...


*/
int CTriangleMesh::LoadTXTFile(const char *filename)
{
	FILE *fp;
	int i, count, *rgb, color=0x00FFFF00;
	float x, y, z;
	float nx, ny, nz;

	//
	ClearAllBuffer();

	fp = fopen(filename, "r");
	if (fp==NULL) return 0;
	fscanf(fp, "%d", &m_nVerticesNumber);
	fscanf(fp, "%d", &m_nTriangleNumber);
	fscanf(fp, "%d", &m_nVerticeFieldNumber);
	assert(m_nVerticeFieldNumber<=3 && m_nVerticeFieldNumber>=1);

	m_Vertice = new Vector3D [m_nVerticesNumber];
	m_pVertexNorm = new Vector3D [m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexColor = new unsigned char [m_nVerticesNumber][4];

	assert(m_Vertice!=NULL);
	assert(m_pVertexNorm!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexColor!=NULL);

	for (i=0; i<m_nVerticesNumber; i++){
		if (m_nVerticeFieldNumber==1){
			count=fscanf(fp, "%f %f %f", &x,&y,&z);
			nx = 1, ny = nz=0;
			if (count!=3){
				fprintf(stderr, "Error: reading vertex %d\n", i);
				exit(0);
			}
		}
		else if (m_nVerticeFieldNumber==2){
			count=fscanf(fp, "%f %f %f %f %f %f", &x,&y,&z,&nx,&ny,&nz);
			if (count!=6){
				fprintf(stderr, "Error: reading vertex %d\n", i);
				exit(0);
			}
		}
		else{
			count=fscanf(fp, "%f %f %f %f %f %f %d", &x,&y,&z,&nx,&ny,&nz, &color);
			if (count!=7){
				fprintf(stderr, "Error: reading vertex %d\n", i);
				exit(0);
			}
		}

		m_Vertice[i].x=x, m_Vertice[i].y=y, m_Vertice[i].z=z;
		m_pVertexNorm[i] = Normalize(Vector3D(nx, ny, nz));
		rgb=(int *)&m_pVertexColor[i][0];
		*rgb = color;
	}
	for (i=0; i<m_nTriangleNumber; i++){
		int& tx = m_Triangles[i][0];
		int& ty = m_Triangles[i][1];
		int& tz = m_Triangles[i][2];
		count= fscanf(fp, "%d %d %d", &tx, &ty, &tz);
		if (count!=3 || tx<0 || ty<0 || tz<0){
			fprintf(stderr, "Error: reading triangle %d\n", i);
			exit(0);
		}
		else{
			//printf("Tri %d: %d %d %d\n", i, tx, ty, tz);
		}
	}
	fclose(fp);

	//compute the vertex normals if no input
	if (m_nVerticeFieldNumber==1){
		ComputeVertexArrayNormal();
		m_nVerticeFieldNumber =2;
	}

	return 1;
}


//load .m file, used by Hoppe for the mechanical part
int CTriangleMesh::LoadMFile(const char *filename)
{
	FILE *fp;
	int i, tmp, count, *rgb, color=0x00FFFF00;
	char instr[100];

	//
	ClearAllBuffer();

	fp = fopen(filename, "r");
	if (fp==NULL) return false;
	fscanf(fp, "%d", &m_nVerticesNumber);
	fscanf(fp, "%d", &m_nTriangleNumber);
	fscanf(fp, "%d", &m_nVerticeFieldNumber);
	assert(m_nVerticeFieldNumber==1);

	m_Vertice = new Vector3D [m_nVerticesNumber];
	m_pVertexNorm = new Vector3D [m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexColor = new unsigned char [m_nVerticesNumber][4];

	assert(m_Vertice!=NULL);
	assert(m_pVertexNorm!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexColor!=NULL);

	for (i=0; i<m_nVerticesNumber; i++){
		Vector3D *pvert = &m_Vertice[i];
		count=fscanf(fp, "%s %d %f %f %f", instr, &tmp, &pvert->x, &pvert->y, &pvert->z);
		m_pVertexNorm[i] = Vector3D(0,0,0);
		rgb=(int *)&m_pVertexColor[i][0];
		*rgb = color;
	}
	for (i=0; i<m_nTriangleNumber; i++){
		Vector3I * ptri = &m_Triangles[i];
		count= fscanf(fp, "%s %d %d %d %d", instr, &tmp, &ptri->x, &ptri->y, &ptri->z);
		ptri->x --;
		ptri->y --;
		ptri->z --;
	}
	fclose(fp);
	return true;
}




int CTriangleMesh::LoadGTSFile(const char *filename)
{
	FILE *fp;
	int i, count; // color=0x00FFFF00;
	Vector3I *pedges;
	int nedge, a, b, c;
	float x, y, z;
	Vector3D n;
	int v0, v1, v2;


	//
	ClearAllBuffer();

	fp = fopen(filename, "r");
	if (fp==NULL) return 0;
	fscanf(fp, "%d", &m_nVerticesNumber);
	fscanf(fp, "%d", &nedge);
	fscanf(fp, "%d", &m_nTriangleNumber);

	m_Vertice = new Vector3D [m_nVerticesNumber];
	m_pVertexNorm = new Vector3D [m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexColor = new unsigned char [m_nVerticesNumber][4];
	pedges = new Vector3I[nedge];

	assert(pedges!=NULL);
	assert(m_Vertice!=NULL);
	assert(m_pVertexNorm!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexColor!=NULL);

	for (i=0; i<m_nVerticesNumber; i++){
		count=fscanf(fp, "%f %f %f", &x,&y,&z);
		m_Vertice[i].x=x, m_Vertice[i].y=y, m_Vertice[i].z=z;
		m_pVertexNorm[i]=Vector3D(0,0,0);
	}
	//skip edges;
	for (i=0; i<nedge; i++){
		fscanf(fp, "%d %d", &pedges[i].x, &pedges[i].y);
		pedges[i].x --;
		pedges[i].y --;
	}

	//******************************
	for (i=0; i<m_nTriangleNumber; i++){
		fscanf(fp, "%d %d %d", &a, &b, &c);
		a--;
		b--;
		c--;
		m_Triangles[i][0]=pedges[a].x;
		m_Triangles[i][1]=pedges[a].y;

		if (pedges[b].x ==pedges[a].x){
			b =c;
		}
		if (pedges[b].x==pedges[a].y){
			m_Triangles[i][2]= pedges[b].y;
		}
		else{
			m_Triangles[i][2]=m_Triangles[i][1];
			m_Triangles[i][1]=m_Triangles[i][0];
			m_Triangles[i][0]=pedges[b].x;
		}

		v0=m_Triangles[i][0];
		v1=m_Triangles[i][1];
		v2=m_Triangles[i][2];
		n= compute_triangle_normal( m_Vertice[v0], m_Vertice[v1], m_Vertice[v2]);
		n*=-1;
		m_pVertexNorm[v0]+=n;
		m_pVertexNorm[v1]+=n;
		m_pVertexNorm[v2]+=n;
	}

	//******************************
	delete [] pedges;
	for (i=0; i<m_nVerticesNumber; i++){
		m_pVertexNorm[i].normalize();
	}

	//normal correction;
	for (i=0; i<m_nTriangleNumber; i++){
		int agree = 0;
		v0=m_Triangles[i][0];
		v1=m_Triangles[i][1];
		v2=m_Triangles[i][2];
		n= compute_triangle_normal( m_Vertice[v0], m_Vertice[v1], m_Vertice[v2]);
		n*=-1;
		if (DotProd(n, m_pVertexNorm[v0])>0)
			agree++;
		if (DotProd(n, m_pVertexNorm[v1])>0)
			agree++;
		if (DotProd(n, m_pVertexNorm[v2])>0)
			agree++;
		if (agree<2){
			printf("vertex order has to be swaped for tri %d\n!", i);
			v0 = m_Triangles[i][1];
			m_Triangles[i][1] = m_Triangles[i][2];
			m_Triangles[i][2] = v0;
		}
	}


	m_nVerticeFieldNumber = 2;

	fclose(fp);
	return 1;
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

int CTriangleMesh::LoadPLYFile(const char *filename)
{
	FILE *fp;
	int i, j;
	int elem_count;
	char *elem_name, buff[200];
	int nverts,nfaces;
	Vertex vlist;
	Face flist;
	PlyOtherProp *vert_other,*face_other;
	PlyFile *in_ply;
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
		{"vertex_indices", Int32, Int32, offsetof(Face,verts),
		1, Uint8, Uint8, offsetof(Face,nverts)},
	};

	/*** Read in the original PLY object ***/
	ClearAllBuffer();
	fp = fopen(filename, "r");
	if (fp==NULL) return 0;
	in_ply = read_ply (fp);
	if (in_ply==NULL) return 0;

	m_nVerticeFieldNumber = 2;

	for (i = 0; i < in_ply->num_elem_types; i++){
		/* prepare to read the i'th list of elements */
		elem_name = setup_element_read_ply (in_ply, i, &elem_count);
		if (equal_strings ("vertex", elem_name)){

			/* create a vertex list to hold all the vertices */
			m_nVerticesNumber = nverts = elem_count;
			m_Vertice = new Vector3D [m_nVerticesNumber];
			m_pVertexNorm = new Vector3D [m_nVerticesNumber];
			m_pVertexColor = new unsigned char [m_nVerticesNumber][4];

			assert(m_Vertice!=NULL);
			assert(m_pVertexNorm!=NULL);
			assert(m_pVertexColor!=NULL);
			memset(m_pVertexColor, 0xffffffff, m_nVerticesNumber*4*sizeof(unsigned char));

			/* set up for getting vertex elements */
			setup_property_ply (in_ply, &vert_props[0]);
			setup_property_ply (in_ply, &vert_props[1]);
			setup_property_ply (in_ply, &vert_props[2]);

			for (j = 0; j < in_ply->elems[i]->nprops; j++) {
				PlyProperty *prop;
				prop = in_ply->elems[i]->props[j];
				if (equal_strings ("nx", prop->name))
					setup_property_ply (in_ply, &vert_props[3]);
				if (equal_strings ("ny", prop->name))
					setup_property_ply (in_ply, &vert_props[4]);
				if (equal_strings ("nz", prop->name))
					setup_property_ply (in_ply, &vert_props[5]);
			}

			vert_other = get_other_properties_ply (in_ply, offsetof(Vertex,other_props));

			/* grab all the vertex elements */
			for (j = 0; j < elem_count; j++) {
				get_element_ply (in_ply, (void *) &vlist);
				m_Vertice[j].x = vlist.x;
				m_Vertice[j].y = vlist.y;
				m_Vertice[j].z = vlist.z;
				m_pVertexNorm[j].x = vlist.nx;
				m_pVertexNorm[j].y = vlist.ny;
				m_pVertexNorm[j].z = vlist.nz;

				if (vlist.nx* vlist.nx+vlist.ny*vlist.ny+vlist.nz+vlist.nz<1E-8){
					//fprintf(stderr, "Error: zero normal vector found at vertex %d!\n", j);
				}
				else{
					m_pVertexNorm[j] = Normalize(m_pVertexNorm[j]);
				}

			}
		}
		else if (equal_strings ("face", elem_name)) {
			m_nTriangleNumber = nfaces = elem_count;
			/* create a list to hold all the face elements */
			m_Triangles = new Vector3I[m_nTriangleNumber];
			assert(m_Triangles!=NULL);

			/* set up for getting face elements */
			setup_property_ply (in_ply, &face_props[0]);
			face_other = get_other_properties_ply (in_ply, offsetof(Face,other_props));

			/* grab all the face elements */
			for (j = 0; j < elem_count; j++) {
				get_element_ply (in_ply, (void *) &flist);
				if (flist.nverts !=3){
					sprintf(buff, "Found a face with %d vertices!\n", (int)flist.nverts);
					PrintMessage(buff);
				}
				m_Triangles[j][0] = flist.verts[0];
				m_Triangles[j][1] = flist.verts[1];
				m_Triangles[j][2] = flist.verts[2];
			}
		}
		else{
			get_other_element_ply (in_ply);
		}
	}

	close_ply (in_ply);
	fclose(fp);
	return 1;
}


//Load smf format file
int CTriangleMesh::LoadSMFFile(const char *filename)
{
	FILE *fp;
	char str[512];
	Vector3D v;
	Vector3I f;
	CDynamicArray<Vector3D> vv;
	CDynamicArray<Vector3I> ff;

	fp = fopen(filename, "r");
	if (fp==NULL) return false;

	m_nVerticesNumber = 0;
	m_nTriangleNumber = 0;
	while (fgets(str,512,fp)!=NULL){
		if (str[0]=='v'){
			v.x=atof(strtok(&str[2]," "));
			v.y=atof(strtok(NULL," "));
			v.z=atof(strtok(NULL," "));
			m_nVerticesNumber++;
			vv.Add(v);
		}
		else if (str[0]=='f'){
			f.x=atoi(strtok(&str[2]," "));
			f.y=atoi(strtok(NULL," "));
			f.z=atoi(strtok(NULL," "));
			m_nTriangleNumber++;
			ff.Add(f);
		}
	}

	m_Vertice = new Vector3D[m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexNorm = new Vector3D[m_nVerticesNumber];

	assert(m_Vertice!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexNorm !=NULL);

	int i;
	for (i=0; i<m_nVerticesNumber; i++){
		m_Vertice[i]=vv[i];
		m_pVertexNorm[i]= Vector3D(0, 0, 1);
	}
	for (i=0; i<m_nTriangleNumber; i++){
		m_Triangles[i].x=ff[i].x-1;
		m_Triangles[i].y=ff[i].y-1;
		m_Triangles[i].z=ff[i].z-1;
	}
	m_nVerticeFieldNumber = 2;

	fclose(fp);
	return true;
}



//Load smf format file
int CTriangleMesh::LoadOFFFile(const char *filename)
{
	FILE *fp;
	char str[512];
	int a, b, c, t;
	Vector3D n;

	fp = fopen(filename, "r");
	if (fp==NULL) return false;

	fgets(str,512,fp);
	str[3]=0;
	if (strcmp(str, "OFF")!=0){
		//not off format
		return 0;
	}

	fscanf(fp, "%d %d %d", &m_nVerticesNumber, &m_nTriangleNumber, &t);
	m_Vertice = new Vector3D[m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexNorm = new Vector3D[m_nVerticesNumber];

	assert(m_Vertice!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexNorm !=NULL);

	//read vertices;
	int i;
	for (i=0; i<m_nVerticesNumber; i++){
		fscanf(fp, "%f %f %f", &m_Vertice[i].x, &m_Vertice[i].y, &m_Vertice[i].z);
		m_pVertexNorm[i]=Vector3D(0, 0, 0);
	}

	//read triangle table
	for (i=0; i<m_nTriangleNumber; i++){
		fscanf(fp, "%d %d %d %d", &t, &a, &b, &c);
		m_Triangles[i][0]=a, 
		m_Triangles[i][1]=b, 
		m_Triangles[i][2]=c;
		n= compute_triangle_normal(m_Vertice[a], m_Vertice[b], m_Vertice[c]);
		m_pVertexNorm[a]+=n;
		m_pVertexNorm[b]+=n;
		m_pVertexNorm[c]+=n;
	}

	for (i=0; i<m_nVerticesNumber; i++){
		m_pVertexNorm[i].normalize();
	}

	m_nVerticeFieldNumber = 2;

	fclose(fp);
	return true;
}



//Load nff format file
int CTriangleMesh::LoadNFFFile(const char *filename)
{
	FILE *fp;
	char str[512];
	int  i, t, nv, ntri;
	//Vector3D n;
	float a, b, c, nx, ny, nz;

	fp = fopen(filename, _RA_);
	if (fp==NULL) return false;

	assert(0);
	fscanf(fp, "%d %d %d", &m_nTriangleNumber, &t);
	m_nVerticesNumber = 3 * m_nTriangleNumber;
	m_Vertice = new Vector3D[m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexNorm = NULL; //new Vector3D[m_nVerticesNumber];

	assert(m_Vertice!=NULL);
	assert(m_Triangles!=NULL);
	//assert(m_pVertexNorm !=NULL);

	//read triangle table
	nv = 0;
	ntri = 0;
	for (i=0; i<m_nTriangleNumber; i++){
		fscanf(fp, "%s %d", str, &t);
		assert(t==3);
		if (str[0]=='p' && str[1]!='p'){ //skip triangle;
			fscanf(fp, "%f %f %f", &a, &b, &c);
			fscanf(fp, "%f %f %f", &a, &b, &c);
			fscanf(fp, "%f %f %f", &a, &b, &c);
		}
		else
		if (str[0]=='p' && str[1]=='p'){ //triangles
			fscanf(fp, "%f %f %f %f %f %f", &a, &b, &c, &nx, &ny, &nz);
			m_Vertice[nv] = Vector3D(a, b, c), nv++;
			fscanf(fp, "%f %f %f %f %f %f", &a, &b, &c, &nx, &ny, &nz);
			m_Vertice[nv] = Vector3D(a, b, c), nv++;
			fscanf(fp, "%f %f %f %f %f %f", &a, &b, &c, &nx, &ny, &nz);
			m_Vertice[nv] = Vector3D(a, b, c), nv++;
			m_Triangles[ntri]=Vector3I(nv-3, nv-2, nv-1), 
			ntri++;
		}
		else{
			printf("Found non triangle primitives!\n");
			assert(0);
		}
	}
	m_nTriangleNumber = ntri;
	m_nVerticesNumber = nv;
	m_nVerticeFieldNumber = 2;
	ComputeVertexArrayNormal();

	fclose(fp);
	return true;
}



//Load smf format file
int CTriangleMesh::LoadTRIFile(const char *filename)
{
	FILE *fp;
	char str[512];
	int  i, nv, ntri;
	Vector3D v0, v1, v2;

	fp = fopen(filename, _RA_);
	if (fp==NULL) return false;


	fscanf(fp, "%d", &m_nTriangleNumber);
	m_nVerticesNumber = 3 * m_nTriangleNumber;
	m_Vertice = new Vector3D[m_nVerticesNumber];
	m_Triangles = new Vector3I[m_nTriangleNumber];
	m_pVertexNorm = new Vector3D[m_nVerticesNumber];

	assert(m_Vertice!=NULL);
	assert(m_Triangles!=NULL);
	assert(m_pVertexNorm !=NULL);

	//read triangle table
	nv = 0;
	ntri = 0;
	for (i=0; i<m_nTriangleNumber; i++){
		fscanf(fp, "%f %f %f %f %f %f %f %f %f %s", 
			   &v0.x, &v0.y, &v0.z, &v1.x, &v1.y, &v1.z, &v2.x, &v2.y, &v2.z, str);
		m_Vertice[nv] = v0, nv++;
		m_Vertice[nv] = v1, nv++;
		m_Vertice[nv] = v2, nv++;
		m_Triangles[ntri]=Vector3I(nv-3, nv-2, nv-1), 
		ntri++;
	}
	m_nTriangleNumber = ntri;
	m_nVerticesNumber = nv;
	m_nVerticeFieldNumber = 2;
	fclose(fp);

	OptimizeMesh();
	ComputeVertexArrayNormal();
	return true;
}





int CTriangleMesh::LoadFile(const char *filename)
{
	int len;
	int flag=0;

	len = strlen(filename);
	if (strcmp(&filename[len-4], ".txt")==0){
		flag= LoadTXTFile(filename);
	}
	else if (strcmp(&filename[len-4], ".ply")==0){
		flag= LoadPLYFile(filename);
	}
	else if (strcmp(&filename[len-4], ".smf")==0){
		flag= LoadSMFFile(filename);
	}
	else if (strcmp(&filename[len-4], ".off")==0){
		flag= LoadOFFFile(filename);
	}
	else if (strcmp(&filename[len-4], ".nff")==0){
		flag= LoadNFFFile(filename);
	}
	else if (strcmp(&filename[len-4], ".tri")==0){
		flag= LoadTRIFile(filename);
	}
	else if (strcmp(&filename[len-4], ".gts")==0){
		flag= LoadGTSFile(filename);
	}
	else if (strcmp(&filename[len-2], ".m")==0){
		flag= LoadMFile(filename);
	}
	else if (strcmp(&filename[len-4], ".plt")==0){
		flag= LoadPltFile(filename);
	}
	else{
		return 0;
	}

	if (flag){
		if (strcmp(&filename[len-4], ".plt")!=0){
			//delete some bad triangles
			ValidateTriangles();
			//change vertex order, to align normals with vertex normals
			checkNormals();
		}

		//compute bounding box;
		GetBoundingBox(lowleft, upright);
		fprintf(stderr, "Totally %d vertices, %d triangles read!\n", m_nVerticesNumber, m_nTriangleNumber);
		fprintf(stderr, "Bounding box lowleft: (%f,%f,%f)\n", (float)lowleft.x, (float)lowleft.y, (float)lowleft.z);
		fprintf(stderr, "Bounding box upright: (%f,%f,%f)\n", (float)upright.x, (float)upright.y, (float)upright.z);
		//char buff[200];
		//sprintf(buff, "Totally %d vertices, %d triangles read!\t", m_nVerticesNumber, m_nTriangleNumber);
		//PrintMessage(buff);

	}
	return flag;
}


//
void CTriangleMesh::CenteringTriangles(void)
{
	float minp[3], maxp[3];
	int i;
	Vector3D midp;

	GetBoundingBox(minp, maxp);
	for (i=0; i<3; i++) 
		midp[i]=(minp[i]+maxp[i])*0.5;

	for (i=0; i<m_nVerticesNumber; i++){
		m_Vertice[i]-=midp;
	}
}


//
void CTriangleMesh::NormalizeTriangles(void)
{
	Vector3D minp, maxp, dir, delta, dir1;
	int i;
	float l;

	GetBoundingBox(minp, maxp);
	dir = maxp - minp;
	delta = dir * 0.006327866234578909f;
	minp -= delta;
	maxp += delta;
	dir1 = maxp - minp;
	l = 1.0 /_MAX3_(dir1.x, dir1.y, dir1.z);

	for (i=0; i<m_nVerticesNumber; i++){
		Vector3D& p = m_Vertice[i];
		p -= minp;
		p.x *= l;
		p.y *= l;
		p.z *= l;
	}
}



int CTriangleMesh::isValidTriangle(int whichtriangle)
{

	int i, v0, v1, v2;
	Vector3D *p0, *p1, *p2, d1, d2, d3;
	double dot;

	i = whichtriangle;
	if (i<0 || i>= m_nTriangleNumber)
		return 0;
	v0 = m_Triangles[i][0];
	v1 = m_Triangles[i][1];
	v2 = m_Triangles[i][2];
	p0 = &m_Vertice[v0];
	p1 = &m_Vertice[v1];
	p2 = &m_Vertice[v2];

	d1 = *p1 - *p0;
	d2 = *p2 - *p1;
	d3 = CrossProd(d1, d2);
	dot = DotProd(d3,d3);

	if (fabs(dot)<1e-30){
		fprintf(stderr, "Vertex 0: %d (%f,%f,%f)\n", v0, (float)p0->x, (float)p0->y, (float)p0->z);
		fprintf(stderr, "Vertex 1: %d (%f,%f,%f)\n", v1, (float)p1->x, (float)p1->y, (float)p1->z);
		fprintf(stderr, "Vertex 2: %d (%f,%f,%f)\n", v2, (float)p2->x, (float)p2->y, (float)p2->z);
		return 0;
	}
	return 1;
}


void CTriangleMesh::ValidateTriangles(void)
{
	int i, j;
	int v0, v1, v2;

	for (i=0; i<m_nTriangleNumber; i++){
		v0 = m_Triangles[i][0];
		v1 = m_Triangles[i][1];
		v2 = m_Triangles[i][2];
		if ((v0== v1) || (v0==v2) || (v1==v2) || (!isValidTriangle(i))){
			for (j=i+1; j<m_nTriangleNumber; j++){
				m_Triangles[j-1][0]= m_Triangles[j][0];
				m_Triangles[j-1][1]= m_Triangles[j][1];
				m_Triangles[j-1][2]= m_Triangles[j][2];
			}
			m_nTriangleNumber--;
			i--;
			if (i<0) i=0;
		}
	}

}


bool CTriangleMesh::SaveFile(const char *filename)
{
	int len;
	int flag=0;

	len = strlen(filename);
	if (strcmp(&filename[len-4], ".txt")==0){
		flag= SaveTXTFile(filename);
	}
	else if (strcmp(&filename[len-4], ".smf")==0){
		flag= SaveSMFFile(filename);
	}
	else if (strcmp(&filename[len-4], ".pov")==0){
		flag= SavePOVFile(filename);
	}
	if (flag>0)
		return true;
	else
		return false;
}



bool CTriangleMesh::SaveSMFFile(const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp==NULL)
		return false;

	int i;
	for (i=0; i<m_nVerticesNumber; i++){
		fprintf(fp, "v %f %f %f\n", m_Vertice[i].x, m_Vertice[i].y, m_Vertice[i].z);
	}
	for (i=0; i<m_nTriangleNumber; i++){
		fprintf(fp, "f %d %d %d\n", m_Triangles[i].x+1, m_Triangles[i].y+1, m_Triangles[i].z+1);
	}
	fclose(fp);
	return true;
}


bool CTriangleMesh::SaveTXTFile(const char *filename)
{
	FILE *fp;
	int i, *pColor;
	char buff[256];

	fp = fopen(filename, "w");
	if (fp==NULL) return 0;
	fprintf(fp, "%d\n", m_nVerticesNumber);
	fprintf(fp, "%d\n", m_nTriangleNumber);
	fprintf(fp, "%d\n", m_nVerticeFieldNumber);

	if (m_nVerticeFieldNumber==2){
		for (i=0; i<m_nVerticesNumber; i++){
			fprintf(fp, "%f %f %f %f %f %f\n", 
					m_Vertice[i][0], m_Vertice[i][1], m_Vertice[i][2],
					m_pVertexNorm[i][0], m_pVertexNorm[i][1], m_pVertexNorm[i][2]); 
		}
	}
	else
	if (m_nVerticeFieldNumber==3){
		for (i=0; i<m_nVerticesNumber; i++){
			pColor = (int*)(&m_pVertexColor[i][0]);
			fprintf(fp, "%f %f %f %f %f %f %d\n", 
					m_Vertice[i][0], m_Vertice[i][1], m_Vertice[i][2],
					m_pVertexNorm[i][0], m_pVertexNorm[i][1], m_pVertexNorm[i][2], 
					*pColor); 
		}
	}
	else{
		fprintf(stderr, "Error: vertice filed number is %d\n", m_nVerticeFieldNumber);
		return 0;
	}

	for (i=0; i<m_nTriangleNumber; i++){
		fprintf(fp, "%d %d %d\n", m_Triangles[i][0], m_Triangles[i][1], m_Triangles[i][2]); 
	}
	fclose(fp);

	sprintf(buff, "Totally %d vertices, %d triangles write!", m_nVerticesNumber, m_nTriangleNumber);
	PrintMessage(buff);
	return 1;
}


//-----------------------------------------------------------------------
class CQsortCell
{
public:
	Vector3D m_vertex;
	Vector3D m_pVertexNorm;
	int m_nID;

public: 
	void Assign(Vector3D &vertex, Vector3D &normal, int id)
	{
		m_vertex = vertex;
		m_pVertexNorm = normal;
		m_nID = id;
	}
};


static inline int dictionary_compare(Vector3D &v1, Vector3D &v2, const float errorbound)
{
	if (fabs(v1.x - v2.x)< errorbound){
		if (fabs(v1.y - v2.y)< errorbound){
			if (fabs(v1.z - v2.z)< errorbound){
				return 0;
			}
			else{
				if (v1.z < v2.z)
					return -1;
				else 
					return 1;
			}
		}
		else{
			if (v1.y<v2.y)
				return -1;
			else
				return 1;
		}
	}
	else{
		if (v1.x < v2.x) 
			return -1;
		else
			return 1;
	}
	return 1;
}


static double SAMEPOINT_TOLERANCE;
static int compare(const void *e1, const void *e2)
{
	CQsortCell *p1, *p2;
	
	p1 = (CQsortCell *)e1;
	p2 = (CQsortCell *)e2;

	return 
		dictionary_compare(p1->m_vertex, p2->m_vertex, SAMEPOINT_TOLERANCE);
}


int CTriangleMesh::OptimizeMesh(const double TH)
{
	int i, j, count;
	CQsortCell *pBuffer;
	int *pIDBuffer;

	pBuffer = new CQsortCell[m_nVerticesNumber];
	pIDBuffer = new int[m_nVerticesNumber];
	assert(pBuffer!=NULL);
	assert(pIDBuffer!=NULL);

	for (i=0; i<m_nVerticesNumber; i++){
		pBuffer[i].Assign(m_Vertice[i], m_pVertexNorm[i], i);
	}

	{
		Vector3D diff= upright-lowleft;
		double dx = fabs(diff.x);
		double dy = fabs(diff.y);
		double dz = fabs(diff.z);
		double dim_max = _MAX3_(dx, dy, dz);
		double bsize = dx+dy+dz-dim_max;
		SAMEPOINT_TOLERANCE = bsize*0.5*TH;
		qsort(pBuffer, m_nVerticesNumber, sizeof(CQsortCell), compare);
	}

	//Assign new vertices to the array;
	pIDBuffer[pBuffer[0].m_nID] = count = 0;
	m_Vertice[0] = pBuffer[0].m_vertex;
	m_pVertexNorm[0] = pBuffer[0].m_pVertexNorm;
	for (i=1; i<m_nVerticesNumber; i++){
		if (compare(&pBuffer[i-1], &pBuffer[i])!=0){
			count++;
			m_Vertice[count] = pBuffer[i].m_vertex;
			m_pVertexNorm[count] = pBuffer[i].m_pVertexNorm;
		}
		pIDBuffer[pBuffer[i].m_nID] = count;
	}

	//Reassign face table indexs;
	for (i=0; i<m_nTriangleNumber; i++){
		for (j=0; j<3; j++){
			m_Triangles[i][j] = pIDBuffer[m_Triangles[i][j]];
		}
	}
	
	m_nVerticesNumber = count+1;
	delete [] pBuffer;
	delete [] pIDBuffer;

	//delete some bad triangles
	ValidateTriangles();

	return 1;
}


void CTriangleMesh::GetBoundingBox(float minp[3], float maxp[3])
{
	int i;
	Vector3D *v;

	lowleft = Vector3D(MAXFLOAT, MAXFLOAT, MAXFLOAT);
	upright = Vector3D(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
	for (i=0; i<m_nVerticesNumber; i++){
		v = &m_Vertice[i];
		Minimize(lowleft, *v);
		Maximize(upright, *v);
	}
	minp[0]=lowleft.x, minp[1]=lowleft.y, minp[2]=lowleft.z;
	maxp[0]=upright.x, maxp[1]=upright.y, maxp[2]=upright.z;
}



void CTriangleMesh::GetBoundingBox(int minp[3], int maxp[3])
{
	float minn[3], maxx[3];
	int i;

	GetBoundingBox(minn, maxx);
	for (i=0; i<3; i++){
		minp[i] = (int)minn[i];
		maxp[i] = (int)(maxx[i] + 0.5);
	}
}


void CTriangleMesh::ComputeVertexShortestRadius(float *pRadius)
{
	Vector3D *p0, *p;
	CEdgeTableCell* pCell;
	float r0, r;

	if (m_ppEdgeTable ==NULL){
		fprintf(stderr, "Error: edge table not created!\n");
		return;
	}
	assert(pRadius!=NULL);
	assert(m_ppEdgeTable!=NULL);

	for (int i=0; i<m_nVerticesNumber; i++){
		p0 = &m_Vertice[i];
		r0 = (float)1E38;
		pCell = m_ppEdgeTable[i];
		while (pCell!=NULL){
			p= &m_Vertice[pCell->vertex];
			pCell = pCell->pNext;
			r = Distance2(*p, *p0);
			if (r==0) 
				r=0;
			if (r<r0)
				r0 = r;
		}
		pRadius[i] = r0;
	}
}



void CTriangleMesh::checkNormals(void)
{
	int i, a, b, c;
	Vector3D n, avgn;

	if (m_pVertexNorm==NULL) return;
	for (i=0; i<m_nTriangleNumber; i++){
		n = ComputeTriangleNormal(i);
		a = m_Triangles[i][0];
		b = m_Triangles[i][1];
		c = m_Triangles[i][2];
		avgn = m_pVertexNorm[a]+m_pVertexNorm[b]+m_pVertexNorm[c];
		if (DotProd(avgn, n)<0){
			m_Triangles[i][1] = c;
			m_Triangles[i][2] = b;
		}
	}
}



int CTriangleMesh::BuildEdgeTable(void)
{
	CEdgeTableCell* p;
	int i, j;
	int cur_vertex, pre_vertex, next_vertex;

	m_ppEdgeTable = new CEdgeTableCell*[m_nVerticesNumber];
	assert(m_ppEdgeTable!=NULL);
	for (i=0; i<m_nVerticesNumber; i++){
		m_ppEdgeTable[i] = NULL;
	}

	for (i=0; i<m_nTriangleNumber; i++){
		for (j=0; j<3; j++){
			Vector3I &tri = m_Triangles[i];
			cur_vertex = tri[j];
			pre_vertex = tri[(j+2)%3];
			next_vertex= tri[(j+1)%3];
			//insert1
			if (!VertexInQueue(m_ppEdgeTable[cur_vertex], pre_vertex)){
				p = (CEdgeTableCell*)Malloc(sizeof(CEdgeTableCell));
				p->tag = 0;
				p->pNext = NULL;
				p->vertex= pre_vertex;
				m_ppEdgeTable[cur_vertex]= InsertBeforeEdgeTableQueue(m_ppEdgeTable[cur_vertex], p);
			}
			//insert2
			if (!VertexInQueue(m_ppEdgeTable[cur_vertex], next_vertex)){
				p = (CEdgeTableCell*)Malloc(sizeof(CEdgeTableCell));
				p->tag = 0;
				p->pNext = NULL;
				p->vertex= next_vertex;
				m_ppEdgeTable[cur_vertex]= InsertBeforeEdgeTableQueue(m_ppEdgeTable[cur_vertex], p);
			}
		}

	}
	return 1;
}


int CTriangleMesh::CheckVertexDegree(int i)
{
	int c=0;

	if (m_ppEdgeTable==NULL){
		BuildEdgeTable();
	}

	CEdgeTableCell *p = m_ppEdgeTable[i];
	c = 0;
	while (p!=NULL){
		c++;
		p = p->pNext;
	}
	return c;
}

void CTriangleMesh::CheckEdgeLength(int i, float &len, int &v)
{
	if (m_ppEdgeTable==NULL){
		BuildEdgeTable();
	}

	len=0;
	v=0;
	CEdgeTableCell *p = m_ppEdgeTable[i];
	Vector3D &p1 = m_Vertice[i];
	while (p!=NULL){
		Vector3D &p2 = m_Vertice[p->vertex];
		float len2 = Distance2(p1, p2);
		if (len2>len){
			len = len2;
			v = p->vertex;
		}
		p = p->pNext;
	}
}


int CTriangleMesh::CheckEdgeTable(void)
{
	int i, c , t;
	int v1, v2;
	float maxlen=0;

	if (m_ppEdgeTable==NULL){
		BuildEdgeTable();
	}
	for (i=0; i<m_nVerticesNumber; i++){
		if ((c=CheckVertexDegree(i))>=9){
			printf("Vertex %d has a connection degree of %d, possible error found!\n", i, c);
		}
		float len2;
		CheckEdgeLength(i, len2, t);
		if (len2>maxlen){
			v1 = i, v2=t;
			maxlen = len2;
		}
	}

	printf("Vertex %d and vertex %d has the longest edge, lenth is %f.\n", v1, v2, maxlen);
	return 1;
}



void CTriangleMesh::VertexNeighborAveragePositionNorm(const int vid, Vector3D &pos, Vector3D &norm)
{
	int v2, n=0;

	ASSERT0(vid>=0 && vid<m_nVerticesNumber);
	ASSERT0(m_ppEdgeTable!=NULL);

	CEdgeTableCell* p = m_ppEdgeTable[vid];
	pos=m_Vertice[vid], norm=m_pVertexNorm[vid], n++; 
	while (p!=NULL){
		v2 = p->vertex;
		pos += m_Vertice[v2];
		norm += m_pVertexNorm[v2];
		n++;
		p=p->pNext;
	}
	pos *=(1.0/n);
	norm.normalize();
}


void CTriangleMesh::VertexNeighborAveragePosition(const int vid, Vector3D &pos)
{
	int v2, n=0;

	ASSERT0(vid>=0 && vid<m_nVerticesNumber);
	ASSERT0(m_ppEdgeTable!=NULL);

	CEdgeTableCell* p = m_ppEdgeTable[vid];
	pos= m_Vertice[vid], n++; 
	while (p!=NULL){
		v2 = p->vertex;
		pos += m_Vertice[v2];
		n++;
		p=p->pNext;
	}
	pos *=(1.0/n);
}
