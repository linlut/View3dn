//lineobj.cpp
#include <math.h>
#include "lineobj.h"


inline void 
DrawLineObject(Vector3d *v, const int nv, Vector2i *l, const int nl, const Vector3f &lclr=Vector3f(0,0,0), const float lthick=1)
{
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glColor3fv(&lclr.x);
	glLineWidth(lthick);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &v[0].x);
	glDrawElements(GL_LINES, 2*nl, GL_UNSIGNED_INT, &l[0].x);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void CLineObj::glDraw(const CGLDrawParms &dparms)
{
	m_pDrawParms = (CGLDrawParms*)(&dparms);
	const Vector3f lclr=dparms.m_cHLineColor;
	const float lthick = dparms.m_fLineWidth;

	_BeginDrawing(m_pDrawParms);
		//draw the lines;
		switch (dparms.m_nDrawType){
			case CGLDrawParms::DRAW_MESH_HIDDENLINE:
			case CGLDrawParms::DRAW_MESH_LINE:
			case CGLDrawParms::DRAW_MESH_SHADING:
				DrawLineObject(m_pVertex, m_nVertexCount, 
					           (Vector2i*)m_pPolygon, m_nPolygonCount, 
							   lclr, lthick);
				break;
			case CGLDrawParms::DRAW_MESH_NONE:
				break;
		}
		
		//draw the vertices;
		if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
			if (m_fGivenRad<1e-12f) 
				m_fGivenRad=estimatedVertexRadius();
			glDrawVertices(dparms.m_cVertexColor);
		}

	_EndDrawing(m_pDrawParms);

	if (dparms.m_bShowVertexAttribValue){
		_DrawVertexAttributeText();
	}	
}



int CLineObj::exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer)
{
	assert(0);
    const int ELMTYPEID =7;   //rod element
	Vector2i*& pTriangle = (Vector2i*&)m_pPolygon;

	for (int i=0; i<m_nPolygonCount; i++){
		const int idx = tetbaseno+i;
		const Vector2i *pt = &pTriangle[i];
		const int vx = vbaseno + pt->x;
		const int vy = vbaseno + pt->y;
		fprintf(fp, "%d %d %d %d %d\n", 
					idx, ELMTYPEID, objsetid, vx, vy);
	}
	return m_nPolygonCount;
}



int CLineObj::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double thickness)
{
	extern void exportPlyVertices(FILE *fp, Vector3d *pVertex, const int nv, double _th, double *pThick, const int baseno, const double *pmatrix);
    exportPlyVertices(fp, m_pVertex, m_nVertexCount, thickness, NULL, baseno, pmatrix);
    return m_nVertexCount;
}

/*
int CLineObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nTotalAttrib)
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


int CLineObj::LoadFile(FILE *fp, const char *ftype)
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


int CLineObj::LoadFile(const char *fname)
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

*/

int CLineObj::SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
{
	int i;
	Vector3d lowleft, upright;
	for (i=0; i<bufflen; i++){
		CLineObj *p = new CLineObj;
		assert(p!=NULL);
		_copyToObject(*p, objidbuff[i]);
		p->ComputeBoundingBox(lowleft, upright);
		p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
		obj[i] = p;
	}
	return bufflen;
}


void CLineObj::_copyToObject(CLineObj & aobj, const int objid)
{
	Vector2i*& m_pTriangle = (Vector2i*&)m_pPolygon;
	int i, k, c, nply, *vindex;
	float* pid = GetVertexObjectIDPointer();
	assert(pid!=NULL);
	assert(objid>=0);

	//count number of lines;
	for (i=nply=0; i<m_nPolygonCount; i++){
		Vector2i& quad = m_pTriangle[i];
		int idx = (int)(pid[quad.x]);
		if (idx==objid) nply++;
	}
	assert(nply>0);
	aobj.m_nPolygonCount = nply;
	aobj.m_pPolygon = (int*)(new Vector2i[nply]);
	assert(aobj.m_pPolygon!=NULL);
	Vector2i*& m_pTriangle2 = (Vector2i*&)aobj.m_pPolygon;

	//copy the connectivity;
	for (i=c=0; i<m_nPolygonCount; i++){
		Vector2i& quad = m_pTriangle[i];
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
		Vector2i& quad = m_pTriangle2[i];
		k = quad.x;
		if (vindex[k]==-1) vindex[k]= c++;
		k = quad.y;
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
		Vector2i& quad = m_pTriangle2[i];
		quad.x = vindex[quad.x];
		quad.y = vindex[quad.y];
		if (quad.x==-1||quad.y==-1)
			assert(0);
	}

	//copy other polyobj class attributes;
	CPolyObj::CopyAttributes(aobj, vindex);
	delete [] vindex;
}


/*
int CLineObj::_loadTXTFile(FILE *fp)
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

int CLineObj::_loadSTLFile(FILE *fp)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	char sbuffer[501], tmp1[501], tmp2[501];
	Vector3f norm, vert, boxmin(1e30f), boxmax(-1e30f);
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
	tri.OptimizeMesh(1e-3);
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


int CLineObj::saveSTLFile(const char *fname, const double* pmatrix)
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


int CLineObj::saveTXTFile(const char *fname, const double* pmatrix)
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



void CLineObj::glDrawElementFaceGoround(const int eid, void *pvoid)
{
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	glNormal3fv(&m_pPolyNorm[eid].x);
	const Vector3i tri = m_pTriangle[eid];
	Vector3d *v0= &m_pVertex[tri.x];
	Vector3d *v1= &m_pVertex[tri.y];
	Vector3d *v2= &m_pVertex[tri.z];

	if (this->NeedTexture1D()){
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



void CLineObj::ComputeVertexNormals(void)
{
    int i;
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;

    if (m_pVertexNorm!=NULL) return;
    m_pVertexNorm = new Vector3f[m_nVertexCount];
    assert(m_pVertexNorm!=NULL);
    for (i=0; i<m_nVertexCount; i++) m_pVertexNorm[i]=Vector3f(0);

    //need the polygon normals
	if (this->m_pPolyNorm==NULL)
		ComputePolygonNormals();
 
	for (i=0; i<m_nPolygonCount; i++){
	    const Vector3i& t = m_pTriangle[i];
        m_pVertexNorm[t.x]+= m_pPolyNorm[i];
        m_pVertexNorm[t.y]+= m_pPolyNorm[i];
        m_pVertexNorm[t.z]+= m_pPolyNorm[i];
	}

    for (i=0; i<m_nVertexCount; i++)
        m_pVertexNorm[i]=Normalize(m_pVertexNorm[i]);
}

*/
//======================PICKING==========================

void CLineObj::DrawPickingObject(const int objid)
{
	SETUP_PICKING_GLENV();
    glPushName(objid);
	DrawLineObject(m_pVertex, m_nVertexCount, (Vector2i*)m_pPolygon, m_nPolygonCount);
    glPopName();
}


void CLineObj::DrawPickingObjectFace(const int)
{
	//Line object should not be picked in face mode
}


/*
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
*/
void CLineObj::DrawPickingObjectLine(const int objid)
{
	/*
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
	*/
}


void CLineObj::GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
{
	/*
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
	*/
}



void CLineObj::GetPickedLine(const int eid, int & v0, int &v1)
{
	/*
	Vector3i*& m_pTriangle = (Vector3i*&)m_pPolygon;
	const int EDGECOUNT = 3;
	assert(eid>=0 && eid<EDGECOUNT*m_nPolygonCount);
	const int t = eid / EDGECOUNT;
	const int e = eid % EDGECOUNT;
	const Vector3i tri = m_pTriangle[t];
	
	const int e1 = (e+1)%EDGECOUNT;
	v0 = tri[e];
	v1 = tri[e1];
	*/
}


void CLineObj::GetPickedPlane(const int eid, Vector3d v[4])
{
}


int CLineObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib)
{
	int i, count;

	//read vertices;
	m_nVertexCount = nv;
	LoadPltVertices(fp, nv, nattrib);

	//read the triangles;
	m_nPolygonCount = nf;
	Vector2i*& pTriangle = (Vector2i*&)m_pPolygon;
	pTriangle = new Vector2i[m_nPolygonCount];
	assert(pTriangle!=NULL);

	for (i=0; i<nf; i++){
		int n0, n1;
		count= fscanf(fp, "%d %d", &n0, &n1);
		n0--, n1--;
		if (count!=2 || n0<0 || n1<0){
			fprintf(stderr, "Error: reading triangle %d\n", i);
			exit(0);
		}
		pTriangle[i] = Vector2i(n0, n1);
	}

	return 1;
}


//Save the mesh into POVRAY's mesh2 format;
inline void getUniqueNodeName(const char *objname, const int vid, char *nodename)
{
	sprintf(nodename, "%s_V%d", objname, vid);
}

int CLineObj::exportPovrayFile(FILE *fp, const double *matrix)
{
	int i;
	const char *objname = this->GetObjectName();
	Vector3f linecolor(0, 1, 0), nodecolor(1, 0, 0);
	if (m_pDrawParms){
		linecolor = m_pDrawParms->m_cHLineColor;
		nodecolor = m_pDrawParms->m_cVertexColor;
	}

	fprintf(fp, "union {\n");
	//output vertices;
	for (i=0; i<m_nVertexCount; i++){
		double v[3];
		char nodename[256];
		TransformVertex3dToVertex3d(m_pVertex[i], matrix, v);
		getUniqueNodeName(objname, i, nodename);
		fprintf(fp, "#declare %s = <%lg, %lg, %lg>;\n", nodename, v[0], v[1], v[2]);
	}

	fprintf(fp, "union {\n");
	Vector2i* pLine = (Vector2i*)m_pPolygon;
	for (i=0; i<m_nPolygonCount; i++){
		const Vector2i t = pLine[i];
		char node1[256], node2[256];
		getUniqueNodeName(objname, t.x, node1);
		getUniqueNodeName(objname, t.y, node2);
		fprintf(fp, "cylinder{%s, %s, RL}\n", node1, node2);
	}
	fprintf(fp, "pigment{ color rgb<%f, %f, %f> }\n", linecolor.x, linecolor.y, linecolor.z);
	fprintf(fp, "}\n");

	fprintf(fp, "union {\n");
	for (i=0; i<m_nVertexCount; i++){
		char nodename[256];
		getUniqueNodeName(objname, i, nodename);
		fprintf(fp, "sphere {%s, RR}\n", nodename);
	}
	fprintf(fp, "pigment{ color rgb<%f, %f, %f> }\n", nodecolor.x, nodecolor.y, nodecolor.z);
	fprintf(fp, "}\n");

	fprintf(fp, "}\n");
	return 1;
}

void CLineObj::exportFile(FILE *fp, const char *format, const double *matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0)
		exportPovrayFile(fp, matrix);

}


