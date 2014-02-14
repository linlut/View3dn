//polyobj.cpp
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <ogldrawall.h>
#include <pntriangle.h>
#include "polyobj.h"
#include "dynamic_array.h"
#include "geomath.h"
#include "sphere16.h"

//#define NEEDQT

using namespace std;
static const char *OUT_ID_DBL3_FORMAT="%d %.14lg %.14lg %.14lg\n";
static const char *OUT_ID_DBL5_FORMAT="%d %.14lg %.14lg %.14lg %.14lg %.14lg\n";


bool *CPolyObj::getBoundaryVertexFlags(void)
{
	if (m_nVertexCount==0) return NULL;

	bool *f = new bool[m_nVertexCount];
	assert(f!=NULL);
	for (int i=0; i<m_nVertexCount; i++) f[i]=false;

	const int nsize = m_nPolygonCount*m_nPlyVertexCount;
	for (int i=0; i<nsize; i++){
		const int j = m_pPolygon[i];
		f[j]=true;
	}
	
	return f;
}


//Private functions;
void CPolyObj::_clearAllBuffer(void)
{
	CMemoryMgr::Free();

    SafeDeleteArray(m_pVertex);
    SafeDeleteArray(m_pPolygon);
    SafeDeleteArray(m_pLoad);
	SafeDeleteArray(m_pElement);

    SafeDeleteArray(m_pVertexNorm);
    SafeDeleteArray(m_pPolyNorm);
    SafeDeleteArray(m_pVertexTexCoor1D);

	for (int i=0; i<ATTRIBUTE_NUM; i++){
        SafeDeleteArray(m_pFAttributes[i]);
        SafeDeleteArray(m_pIAttributes[i]);
	}

    SafeDeleteArray(m_ppEdgeTable);
    SafeDeleteArray(m_pVertexColor);
    SafeDeleteArray(m_pFemBoundCond);

	//delete the shader object
	SafeDelete(m_pRenderingObject);
}


void CPolyObj::_init()
{
    m_dispListID = 0;
    m_vboID = 0;
    m_eboID = 0;

	m_nObjID = 0; 
	m_nObjType = OID_NULL; 
	m_nVertexCount = 0;	
	m_nPolygonCount = 0;
    m_nElementCount = 0;
    m_nPlyVertexCount = 0;      //number of vertices per polygon
    m_nElmVertexCount = 0;      //number of vertices per element

	m_pElement = NULL;
	m_pVertex = NULL;	
	m_pPolygon = NULL;
    m_pLoad = NULL;
	m_pVertexNorm = NULL;	
	m_pPolyNorm = NULL;	
	m_pVertexTexCoor1D = NULL;
	m_pFemBoundCond = NULL;

	for (int i=0; i<ATTRIBUTE_NUM; i++){
		m_pFAttributes[i] = NULL;	
		m_pIAttributes[i] = NULL;	
		m_strFVarNames[i][0] = 0;
		m_strIVarNames[i][0] = 0;
	}

	m_ppEdgeTable = NULL;
	m_pVertexColor = NULL;
	m_pDrawParms = NULL;

	m_pRenderingObject = NULL;	//a pointer to a more advanced render tech.
}


//public funcs;
CPolyObj::CPolyObj(void): 
    CMemoryMgr()
    ,CObject3D()
    ,m_meshRender()
{
	_init();
	m_fGivenRad = 0;
}

CPolyObj::~CPolyObj(void)
{
	_clearAllBuffer();

    if (m_dispListID !=0){
        glDeleteLists(m_dispListID, 1);
        m_dispListID = 0;
    }
    if (m_vboID != 0){
        glDeleteBuffers(1, &m_vboID);
        m_vboID = 0;
    }
    if (m_eboID != 0){
        glDeleteBuffers(1, &m_eboID);
        m_eboID = 0;
    }

	_init();
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


inline int copy_names(char* varnames, char strVarNames[][9], const int ATTRIBUTE_NUM)
{
	const char TOKEN = 0x22; // '\"';
	int i, startpos, endpos;
	bool hasname = true;
	char *pcurname = varnames;

	memset(strVarNames, 0, sizeof(char)*ATTRIBUTE_NUM*9);
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
	assert(i<=ATTRIBUTE_NUM);
	return i-1;
}


inline char* get_attrib_line(FILE *fp, char *sbuff, const int sbufflen)
{
	fread(sbuff, sbufflen-1, 1, fp);
	sbuff[sbufflen]=0;
	char * ptr = strstr(sbuff, "ZONE ");
	assert(ptr!=NULL);
	ptr[0]=0;

	//skip begining characters
	char *pchar = sbuff;
	while ((*pchar==' ')||(*pchar==0xA)||(*pchar==0xD)){
		pchar++;	//filter beginning ' '
	}

	//change the ',' symbols
	const int slength = strlen(pchar);	//filter ','
	for (int i=0; i<slength; i++){		
		if (pchar[i]==',') 
			pchar[i]= ' ';
	}
	return pchar;
}

int CPolyObj::ReadPltFileHeader(FILE *fp, 
	char *chunktype, int &nvert, int &nface, float &radius, 
	int &nfattrib, char fattrib[][9], char ifattrib[][9])
{
	const int SBUFFER_LENGTH = 1024;
	char sbuffer[SBUFFER_LENGTH+4], fepoint[SBUFFER_LENGTH+4];
	long int ifpos_bak = 0;

	//read plt header, the first line
	do{
		fgets(sbuffer, SBUFFER_LENGTH, fp);	  //skip the first line, project name;
		const char* foundheader = strstr(sbuffer, "TITLE =");
		if (foundheader){ 
			break;
		}
	}while (!feof(fp)); 
	if (feof(fp)) return 0;

	//read attrib line
	long int ifpos = ftell(fp); 
	if (ifpos<0){
		perror("ftell error: negtive value!\n");
		ifpos = 0;
	}
	char *pchar = get_attrib_line(fp, sbuffer, SBUFFER_LENGTH);
	nfattrib = copy_names(pchar, fattrib, ATTRIBUTE_NUM)-3;

	//read ZONE line
	fseek(fp, ifpos, SEEK_SET);
	do{
		fgets(sbuffer, SBUFFER_LENGTH, fp);	
		pchar = strstr(sbuffer, "ZONE");
		if (pchar){
			chunktype[0] = 0;  radius = 0;
			sscanf(pchar, "ZONE N = %d  E = %d F =%s ET =%s RAD =%f\n", &nvert, &nface, fepoint, chunktype, &radius);
			return 1;
		}
	}while (!feof(fp)); 
	return 0;
}


void CPolyObj::AddVertexAttributeFloat(const char *attrname, float *pattrib)
{
	for (int i=0; i<ATTRIBUTE_NUM; i++){
		if (m_pFAttributes[i]==NULL){
			//clear buffer
			const int nn = 9;
			for (int j=0; j<nn; j++) m_strFVarNames[i][j]=0;
			//copy
			m_pFAttributes[i] = pattrib;
			strncpy(m_strFVarNames[i], attrname, 8);	//attributes names, float
			m_strFVarNames[i][8]=0;
			break;
		}
	}
}


void CPolyObj::AddVertexAttributeInt(const char *attrname, int *pattrib)
{
	for (int i=0; i<ATTRIBUTE_NUM; i++){
		if (m_pIAttributes[i]==NULL){
			//clear buffer
			const int nn = 9;
			for (int j=0; j<nn; j++) m_strIVarNames[i][j]=0;
			//copy
			m_pIAttributes[i] = pattrib;
			strncpy(m_strIVarNames[i], attrname, 8);	//attributes names, float
			break;
		}
	}
}


int CPolyObj::LoadPltVertices(FILE *fp, const int nv, const int nTotalAttrib)
{
	int i, j, readCount, ip, hasIP = 0; 
	char sbuffer[1024];
	
	//alloc various buffers;
	m_nVertexCount = nv;
	m_pVertex = new Vector3d[m_nVertexCount];
	assert(m_pVertex!=NULL);
	for (i=0; i<nTotalAttrib; i++){
		m_pFAttributes[i] = new float [m_nVertexCount];
		assert(m_pFAttributes[i]!=NULL);
	}

	//read the vertices;
	if (strcmp(m_strFVarNames[nTotalAttrib-1], "IP")==0) hasIP = 1;
	readCount = nTotalAttrib + 3 - hasIP;
	double v[60];
    assert(readCount<60);
	for (i=0; i<m_nVertexCount; i++){
		for (j=0; j<readCount; j++){
			fscanf(fp, "%s", sbuffer);
			v[j]= READ_FORTRAN_FLOAT(sbuffer);
		}
		m_pVertex[i] = Vector3d(v[0], v[1], v[2]);
		for (j=0; j<nTotalAttrib-hasIP; j++){
			m_pFAttributes[j][i] = v[j+3];
		}
		if (hasIP){
			fscanf(fp, "%d", &ip);
			int *p = (int*)m_pFAttributes[nTotalAttrib-1];
			p[i]= ip-1;
		}		
	}
	return nv;
}


int CPolyObj::ObjectCount(int objidbuff[], const int bufflen)
{
	int maxid, asize;

	//find the objid field;
	float* p = GetVertexObjectIDPointer();
	if (p==NULL){
		objidbuff[0]=0;
		return 1;
	}
	
	//find max id;
	maxid = -1;
	for (int i=0; i<m_nVertexCount; ++i){
		int id = (int)p[i];
		if (id>maxid) 
			maxid = id;
	}
	assert(maxid!=-1);

	//find the number of ids using counting sort;
	for (int i=0; i<maxid+1; i++) objidbuff[i] = 0;
	for (int i=0; i<m_nVertexCount; ++i){
		int id = (int)p[i];
		objidbuff[id]++;
	}
	
	//return the buffer;
	for (int i=asize=0; i<maxid+1; ++i){
		if (objidbuff[i]>0){
			objidbuff[asize] = i;
			asize++;
		}
	}

	assert(asize>0 && asize<bufflen);
	return asize;
}


void CPolyObj::_BeginDrawing(CGLDrawParms *p)
{
	ASSERT0(p!=NULL);
	//do mapping
	if (p->m_bRecomputeTexture1DParameter && p->m_bEnableTexture1D){
		const bool recomp = p->m_bRecomputeTexture1DRange;
		const int attribIdx = p->m_nTextureCoorType;
		const int mappingType = p->m_nMappingType;
		ComputeVertexTextureCoordiates(recomp, attribIdx, mappingType, p->m_fMinRange, p->m_fMaxRange);
		p->m_bRecomputeTexture1DParameter = false;
	}
}


void CPolyObj::_EndDrawing(CGLDrawParms *pDrawParms)
{
	if (pDrawParms->m_bRecomputeTexture1DParameter){
		pDrawParms->m_bRecomputeTexture1DParameter = false;
	}
}


void CPolyObj::glDraw(const CGLDrawParms &dparms)
{
	m_pDrawParms = (CGLDrawParms*)(&dparms);

	glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glEnable(GL_NORMALIZE);		//IMPORTANT to enable it!!!

	_BeginDrawing(m_pDrawParms);
    {
		switch (dparms.m_nDrawType){
			case CGLDrawParms::DRAW_MESH_HIDDENLINE:
				_DrawHiddenLineSurfaces();
				break;

			case CGLDrawParms::DRAW_MESH_LINE:
				_DrawLineForSurfaces();
				break;
 
			case CGLDrawParms::DRAW_MESH_SHADING:
			    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
				_DrawShadedSurfaces();
				break;

			case CGLDrawParms::DRAW_MESH_NONE:
			case CGLDrawParms::DRAW_VOLUME_TRANSPARENT:
				drawVolumetricElements();
				break;
		}
		
		//draw the vertices if required
		if (dparms.m_nDrawVertexStyle!=CGLDrawParms::DRAW_VERTEX_NONE){
			if (this->m_nPlyVertexCount!=1){ //not point obj
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				_DrawVertices(dparms.m_cVertexColor);
			}
		}
		//draw the normals if required
		if (dparms.m_bShowVertexNormal){
			const float normlength = dparms.m_fNormalLengthScale;
			const Vector3f normcolor = dparms.m_cNormalColor;
			_DrawVertexNormals(normlength, normcolor);
		}
    }
	_EndDrawing(m_pDrawParms);
}


void CPolyObj::_DrawVertexAttributeText(void)
{
	int i;
	char text[200];
	float* pid = NULL;

	pid = m_pFAttributes[m_pDrawParms->m_nTextureCoorType];
	//if (pid==NULL) return;

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glColor3f(250, 250, 250);

	for (i=0; i<m_nVertexCount; i++){
		const Vector3d & pos = m_pVertex[i];
//		const float attr = pid[i];
//		sprintf(text, "%f", attr);
		sprintf(text, "%d", i);
//#ifdef NEEDQT
		extern void Draw3DText(double x, double y, double z, char *txt);
		Draw3DText(pos.x, pos.y, pos.z, text);
//#endif
	}
}

void CPolyObj::checkNormals(const bool recompute)
{
	if (m_pDrawParms==NULL) return;
	//if recompute normal every time
	if (recompute){
		if (m_pDrawParms->m_bEnableSmoothing || m_pDrawParms->m_bShowVertexNormal)
			ComputeVertexNormals();
		else
			ComputePolygonNormals();
		return;
	}
	//if no need recompute, just compute when normals are absent
	if (m_pDrawParms->m_bEnableSmoothing){ //smooth normal is needed
		if (this->m_pVertexNorm==NULL)
			ComputeVertexNormals();
	}
	else{
		if (this->m_pPolyNorm==NULL)
			ComputePolygonNormals();
	}
}


void _drawPNTriangles(
	const Vector3d *pVertex, const int nv, const Vector3i *pTri, const int ntri, const Vector3f *pNorm)
{
	const int BUFFERSIZE = 100;
	Vector3f v[BUFFERSIZE], n[BUFFERSIZE];
	Vector3i tri[BUFFERSIZE];
	int nvbase=0, nbv, nbtri;

	for (int i=0; i<ntri; i++){
		const Vector3i t = pTri[i];
		const Vector3d &p1 = pVertex[t.x];
		const Vector3d &p2 = pVertex[t.y];
		const Vector3d &p3 = pVertex[t.z];
		const Vector3f &n1 = pNorm[t.x];
		const Vector3f &n2 = pNorm[t.y];
		const Vector3f &n3 = pNorm[t.z];
		bezierPatchCoefficients(p1, p2, p3, n1, n2, n3, nvbase, v, n, nbv, tri, nbtri);
		ASSERT0(nbv<BUFFERSIZE);
		ASSERT0(nbtri<BUFFERSIZE);
		drawSurfaceUsingArray(v, n, &tri[0].x, nbtri, 3);
	}
}

inline void __CheckGLError(){
    GLenum error; 
    const char* format = "GL error at file %s, line %d: %s\n";
    while (( error = glGetError() ) != GL_NO_ERROR){
        //fprintf (stderr, format, __FILE__, __LINE__, gluErrorString(error)); 
    }
}
//#define CheckGLError __CheckGLError();
#define CHECK_GL_ERRORS __CheckGLError();

GLuint createVBO(size_t vertexSize, const float* vertex, const float *normal, const float* color)
{
    const GLenum target = GL_ARRAY_BUFFER;
    const GLenum usage = GL_STREAM_DRAW_ARB;
    GLuint id = 0;                               // 0 is reserved, 
    glGenBuffers(1, &id);                        // returns non-zero id if success
    glBindBuffer(target, id);                    // activate vbo id to use
    CHECK_GL_ERRORS;

    size_t totalSize = vertexSize * 3;
    if (color == NULL) totalSize = vertexSize * 2;
    glBufferData(target, totalSize, 0, usage);
    size_t offset = 0;
    glBufferSubData(target, offset, vertexSize, vertex);
    offset += vertexSize;
    glBufferSubData(target, offset, vertexSize, normal);
    offset += vertexSize;
    if (color != NULL){
        glBufferSubData(target, offset, vertexSize, color);
    }
    CHECK_GL_ERRORS;

	glBindBuffer(target, 0);
    return id;      // return VBO id
}

GLuint createEBO(const void* data, int dataSize)
{
    const GLenum target = GL_ELEMENT_ARRAY_BUFFER;
    const GLenum usage = GL_STATIC_DRAW;

    GLuint id = 0;                               // 0 is reserved
    glGenBuffers(1, &id);                        // return non-zero id if success
    glBindBuffer(target, id);                    // activate vbo id to use
    glBufferData(target, dataSize, data, usage); // upload data to video card
    CHECK_GL_ERRORS;

    // check data size in VBO is same as input array, if not return 0 and delete VBO
    int bufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
    if(dataSize != bufferSize)
    {
        glDeleteBuffers(1, &id);
        id = 0;
        std::cout << "[createVBO()] Data size is mismatch with input array\n";
    }

	glBindBuffer(target, 0);
    return id;      // return VBO id
}

void DrawModel(GLuint vboId, GLuint eboId, int nv, int nelm, int vert_per_elm)
{
    const size_t vertexSize = nv * 3 * sizeof(GLfloat);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
	const int offset = vertexSize;
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glNormalPointer(GL_FLOAT, 0, (GLvoid*)offset);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

    const size_t indexSize = nelm * vert_per_elm;
    GLenum mode = GL_TRIANGLES;
    if (vert_per_elm == 4) mode = GL_QUADS;
	glDrawElements(mode, indexSize, GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void CPolyObj:: _DrawShadedSurfaces(void)
{
    CHECK_GL_ERRORS;
	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_BLEND);

	//setup normal;
	checkNormals(false);

	//setup lighting
	if (m_pDrawParms->m_bEnableLighting){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}
	else
		glDisable(GL_LIGHTING);

	//setup texture 1d
	if (m_pDrawParms->m_bEnableTexture1D)
		glEnable(GL_TEXTURE_1D);
	else
		glDisable(GL_TEXTURE_1D);

	//setup the mirror flag;
	void *pMirrorTag = NULL;
	const bool mirrFlag = m_pDrawParms->m_bMirrorFlag;
	if (mirrFlag) pMirrorTag = (void*)0x1;

	//draw the obj
	if (m_nPlyVertexCount==3 || m_nPlyVertexCount==4){
		if (m_pDrawParms->m_bEnableSmoothing){
			if (mirrFlag){
				for (int i=0; i<m_nVertexCount; i++) 
					m_pVertexNorm[i] = -m_pVertexNorm[i];
			}
			if (!m_pDrawParms->m_bEnableTexture1D){
                if (!m_pDrawParms->m_bUseVbo){
                    drawSurfaceUsingArray(m_pVertex, m_pVertexNorm, m_pPolygon, m_nPolygonCount, m_nPlyVertexCount);
                }
                else{
                    /*
                    if (0 == m_vboID){
                        std::vector<float> vertex;
                        vertex.resize(m_nVertexCount * 3);
                        Vector3f *p = (Vector3f*)&vertex[0];
                        Vector3d *src = (Vector3d*)m_pVertex;
                        for (int i=0; i<m_nVertexCount; i++){
                            const Vector3d& v = src[i];
                            p[i] = Vector3f(v.x, v.y, v.z);
                        }
                        const size_t vertexSize = m_nVertexCount * 3 *sizeof(float);
                        m_vboID = createVBO(vertexSize, &p[0].x, &m_pVertexNorm[0].x, NULL);
                        const size_t elementSize = m_nPolygonCount * m_nPlyVertexCount * sizeof(int);
                        m_eboID = createEBO(m_pPolygon, elementSize);
                    }
                    DrawModel(m_vboID, m_eboID, m_nVertexCount, m_nPolygonCount, m_nPlyVertexCount);
                    */
                    if (0 == m_meshRender.vbo()){
                        CHECK_GL_ERRORS;
                        m_meshRender.setVertexPointer(&m_pVertex[0].x, m_nVertexCount);
                        m_meshRender.setNormalPointer(&m_pVertexNorm[0].x);
                        m_meshRender.setElementPointer(m_pPolygon, m_nPolygonCount, m_nPlyVertexCount);
                        m_meshRender.setElementNormalPointer(&m_pPolyNorm[0].x);
                        m_meshRender.createGPUBuffers();
                    }
                    //m_meshRender.drawFlatShaded(&m_pDrawParms->m_cObjectColor.x);
                    //m_meshRender.drawGouraudShaded(&m_pDrawParms->m_cObjectColor.x);
                    //m_meshRender.drawPhongShaded(&m_pDrawParms->m_cObjectColor.x);
                    //m_meshRender.drawWireFrame(&m_pDrawParms->m_cObjectColor.x, m_pDrawParms->m_fLineWidth);
                    m_meshRender.drawFlatHiddenLine(&m_pDrawParms->m_cObjectColor.x, &m_pDrawParms->m_cHLineColor.x, m_pDrawParms->m_fLineWidth, m_pDrawParms->m_Viewport[2], m_pDrawParms->m_Viewport[3]);
                    //m_meshRender.drawGouraudHiddenLine(&m_pDrawParms->m_cObjectColor.x, &m_pDrawParms->m_cHLineColor.x, m_pDrawParms->m_fLineWidth, m_pDrawParms->m_Viewport[2], m_pDrawParms->m_Viewport[3]);
                    //m_meshRender.drawPhongHiddenLine(&m_pDrawParms->m_cObjectColor.x, &m_pDrawParms->m_cHLineColor.x, m_pDrawParms->m_fLineWidth, m_pDrawParms->m_Viewport[2], m_pDrawParms->m_Viewport[3]);
                }

				//if (m_nPlyVertexCount==3 && m_pDrawParms->m_bEnablePNTriangleSubdiv){
				//if (m_nPlyVertexCount==3 ){
				//	_drawPNTriangles(m_pVertex, m_nVertexCount, (Vector3i*)m_pPolygon, m_nPolygonCount, m_pVertexNorm);
				//}
				//else{
				//	drawSurfaceUsingArray(m_pVertex, m_pVertexNorm, m_pPolygon, m_nPolygonCount, m_nPlyVertexCount);
                //}
			}
			else{
				if (m_pVertexTexCoor1D)
					draw1DTexturedSurfaceUsingArray(m_pVertex, m_pVertexNorm, 
						m_pVertexTexCoor1D, m_pPolygon, m_nPolygonCount, m_nPlyVertexCount);
			}
			if (mirrFlag){
				for (int i=0; i<m_nVertexCount; i++) 
					m_pVertexNorm[i] = -m_pVertexNorm[i];
			}
		}
		else{
			GLenum nGLElementType=GL_QUADS;
            if (m_nPlyVertexCount==3) nGLElementType = GL_TRIANGLES;
            if (m_pDrawParms->m_bUseDispList){
                if (0 == m_dispListID){
            	    m_dispListID = glGenLists(1);
            	    glNewList(m_dispListID, GL_COMPILE);
                    {
                        for (int i=0; i<m_nPolygonCount; ++i){
                            const int j = i & 2047; //2^11 -1
    			            if (j==0) glBegin(nGLElementType);
			                glDrawElementFaceGoround(i, pMirrorTag);
    			            if (j==2047) glEnd();
                        }
                        if ((m_nPolygonCount&2047)!=0) glEnd();
                    }
            	    glEndList();
                }
      		    glCallList(m_dispListID);
            }
            else{
               for (int i=0; i<m_nPolygonCount; ++i){
                    const int j = i & 2047; //2^11 -1
                    if (j==0) glBegin(nGLElementType);
		            glDrawElementFaceGoround(i, pMirrorTag);
		            if (j==2047) glEnd();
               }
               if ((m_nPolygonCount&2047)!=0) glEnd();
            }
		}
	}
	else if (m_nPlyVertexCount==1){
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_DEPTH_TEST);      
		CGLDrawParms::glDrawVertexStyle tmp = m_pDrawParms->m_nDrawVertexStyle;
		if (m_pDrawParms->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_NONE)
			m_pDrawParms->m_nDrawVertexStyle=CGLDrawParms::DRAW_VERTEX_SPHERE;
		_DrawVertices(m_pDrawParms->m_cObjectColor);
		m_pDrawParms->m_nDrawVertexStyle = tmp;
	}
}


void CPolyObj:: _DrawLineForSurfaces(const float offset)
{
	bool needtex = false;
	void * pMirrorTag = NULL;
	if (m_pDrawParms->m_bMirrorFlag) pMirrorTag = (void*)0x1;
	
	//require face normal
	if (this->m_pPolyNorm==NULL)
		ComputePolygonNormals();

	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

	if (m_pDrawParms->m_bEnableTexture1D &&
		m_pDrawParms->m_nDrawType!=CGLDrawParms::DRAW_MESH_HIDDENLINE){
		needtex=true;
		glEnable(GL_TEXTURE_1D);
	}
	else
		glDisable(GL_TEXTURE_1D);

	if (m_nPlyVertexCount==3 || m_nPlyVertexCount==4){
		glDisable(GL_LIGHTING);
		glColor3fv(&m_pDrawParms->m_cHLineColor.x);
		glLineWidth(m_pDrawParms->m_fLineWidth);
		glBegin(GL_LINES);
		for (int f=0; f<m_nPolygonCount; f++)
			glDrawElementLine(f, offset, needtex);
		glEnd();
	}
	else if (m_nPlyVertexCount==1){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_DEPTH_TEST);      
		for (int f=0; f<m_nVertexCount; f++)
			glDrawElementFaceGoround(f, pMirrorTag);
	}
}


void CPolyObj::_DrawHiddenLineSurfaces(void)
{
	//In case of hiddenline snag rendering
	if (m_pDrawParms->m_bInEPSSnag){
		_DrawShadedSurfaces();
		float offset = GetHiddenLineOffset();
		_DrawLineForSurfaces(offset);
		return;
	}

	//prevent points being drawed as hline
	const char *pname = this->Description();
	if (strcmp(pname, "pnt")==0){
		_DrawShadedSurfaces();
		return;
	}

	//draw 1st pass, face
	//hline color and width;
	bool bakLighting = m_pDrawParms->m_bEnableLighting;
	bool bakTexture = m_pDrawParms->m_bEnableTexture1D;

	m_pDrawParms->m_bEnableLighting = false;
	m_pDrawParms->m_bEnableTexture1D = false;
	glEnable(GL_COLOR_MATERIAL);
	glColor3fv(&m_pDrawParms->m_cHLineColor.x);
	glDisable(GL_COLOR_MATERIAL);
	glLineWidth(m_pDrawParms->m_fLineWidth);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_DrawShadedSurfaces();
	m_pDrawParms->m_bEnableLighting = bakLighting;
	m_pDrawParms->m_bEnableTexture1D = bakTexture;

	//draw 2nd pass, line
	m_pDrawParms->setupMaterial();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	_DrawShadedSurfaces();
	glDisable(GL_POLYGON_OFFSET_FILL);
}


static void computeTextureCoordinates(
    float *pArray, 
	const int nArray, 
	float * pTexParm1D, 
	const ScalarToScalarMappingType mappingfunc,
	const float minval, 
	const float maxval)
{
	float dx = maxval - minval; 
	if (dx < 1e-30f)
		dx = 1e-30f;
	dx = 1 / dx;

	for (int i=0; i<nArray; i++){
		const float x = pArray[i];
		float &t = pTexParm1D[i];
		switch(mappingfunc){
			case ScalarMappingLinear:
				t= (x - minval) * dx;
				break;
			case ScalarMappingLog2:
				t= (x - minval) * dx;
				t = sqrt(t);
				break;
			case ScalarMappingLog10:
				t *= t;
				break;
		default:
			assert(0);
		}
		const float FTOL = 0.001f;
		const float FTOL1 = 1.0f - FTOL;
		if (t<FTOL) 
			t=FTOL;
		else if (t>FTOL1)
			t=FTOL1;
	}
}

static void 
computeAttribRange(float *pArray, const int nArray, float &minval, float &maxval)
{
	maxval = -1e30f; minval = +1e30f;
	for (int i=0; i<nArray; i++){
		const float x = pArray[i];
		if (x>maxval)
			maxval = x;
		if (x<minval)
			minval = x;
	}
	const float diff = maxval - minval;
	const float FZERO = 1e-30f;
	if (fabs(diff)<FZERO){
		maxval += FZERO;
	}
}


void CPolyObj::ComputeVertexTextureCoordiates(
	const bool useNewRange, const int attribIdx, const int mappingType, float& minval, float& maxval)
{
	const int nv =m_nVertexCount;
	const int idx = attribIdx;
	if (idx<0 || idx>=ATTRIBUTE_NUM) return;

	float *pArray = m_pFAttributes[idx];
	char *strName = m_strFVarNames[idx];
	if (pArray==NULL || strName[0]==0)
		return;

	if (m_pVertexTexCoor1D==NULL) 
		m_pVertexTexCoor1D  = new float [nv];
	assert(m_pVertexTexCoor1D!=NULL);

	if (useNewRange){
		computeAttribRange(pArray, nv, minval, maxval);
		printf("Mesh property %d range is [%g, %g]\n", attribIdx, minval, maxval);
	}

	ScalarToScalarMappingType emappingType = (ScalarToScalarMappingType)mappingType;
	computeTextureCoordinates(pArray, nv, m_pVertexTexCoor1D, emappingType, minval, maxval);
}


void CPolyObj::_DrawVertices(const Vector3f &spherecolor)
{
	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_2D);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);

	//setup lighting
	if (m_pDrawParms->m_bEnableLighting){
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}
	else{
		glDisable(GL_LIGHTING);
	}

	//setup texture 1D
	if ((this->m_nPlyVertexCount==1 && (m_pDrawParms->m_bEnableTexture1D || m_pDrawParms->m_bEnableTexture2D)) || 
		m_pDrawParms->m_bEnableVertexTexture){
		glEnable(GL_TEXTURE_1D);
		glEnable(GL_COLOR_MATERIAL);
		glColor3f(1, 1, 1);
	}
	else{
		glDisable(GL_TEXTURE_1D);
		glEnable(GL_COLOR_MATERIAL);
		glColor3fv(&spherecolor.x);
	}
	glDisable(GL_COLOR_MATERIAL);
	glMatrixMode(GL_MODELVIEW);

    //prepare vertex sphere radius
	if (m_fGivenRad==0) 
        m_fGivenRad=estimatedVertexRadius();

	switch(m_pDrawParms->m_nDrawVertexStyle){
	case CGLDrawParms::DRAW_VERTEX_SPHERE:
		{
		const bool reversenorm = m_pDrawParms->m_bMirrorFlag;
		const bool saveflag = m_pDrawParms->m_bEnableTexture1D;
		m_pDrawParms->m_bEnableTexture1D |= m_pDrawParms->m_bEnableVertexTexture;
		for (int f=0; f<m_nVertexCount; ++f){
			if (this->m_nPlyVertexCount==1)
				glDrawElementFaceGoround(f, NULL);
			else
	 		    glDrawVertexSphere(f, reversenorm);
		}
		m_pDrawParms->m_bEnableTexture1D = saveflag;
		break;
		}
	default:
	case CGLDrawParms::DRAW_VERTEX_POINT:
		{
		glDisable(GL_LIGHTING);
		glPointSize(4*m_pDrawParms->m_fVertexSizeScale);
		glBegin(GL_POINTS);
		    for (int f=0; f<m_nVertexCount; ++f) 
				drawVertexPoint(f);
		glEnd();
		break;
		}
	case CGLDrawParms::DRAW_VERTEX_TETRA:
		for (int f=0; f<m_nVertexCount; f++) 
			drawVertexTetra(f);
		break;
	case CGLDrawParms::DRAW_VERTEX_CUBE:
		for (int f=0; f<m_nVertexCount; f++) 
			drawVertexCube(f);
		break;
	};
}


void CPolyObj::_DrawVertexNormals(const float scalefactor, const Vector3f &color)
{
	if (m_pVertexNorm==NULL)
		ComputeVertexNormals();

	glEnable(GL_DEPTH_TEST);      
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    //glDisable(GL_TEXTURE_3D);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

    //prepare normal length
    const float normlen = estimatedVertexRadius() * scalefactor;

	glColor3fv(&color.x);
	glLineWidth(m_pDrawParms->m_fLineWidth);
	glBegin(GL_LINES);
	for (int i=0; i<m_nVertexCount; i++){
		const Vector3d vd = m_pVertex[i];
		const Vector3f p0 = Vector3f(vd.x, vd.y, vd.z);
		const Vector3f p1 = p0 + m_pVertexNorm[i]*normlen; 
		glVertex3fv(&p0.x);
		glVertex3fv(&p1.x);
	}
	glEnd();
}


void CPolyObj::CopyAttributes(CPolyObj &aobj, int *vindex)
{
	const int newvlen = aobj.m_nVertexCount;
	float *pdst, *psrc;

	assert(newvlen>0);
	//copy some other attribute pointers;
	for (int i=0; i<ATTRIBUTE_NUM; i++){
		strcpy(aobj.m_strFVarNames[i], m_strFVarNames[i]);
		aobj.m_pFAttributes[i] = NULL;
		psrc = m_pFAttributes[i];
		if (psrc){
			aobj.m_pFAttributes[i] = pdst = new float [newvlen];
			assert(pdst!=NULL);
			for (int j=0; j<m_nVertexCount; j++){
				int k = vindex[j];
				if (k>=0) pdst[k] = psrc[j];
			}			
		}
	}

	aobj.m_nPlyVertexCount = m_nPlyVertexCount;		//OpenGL element type
	aobj.m_fGivenRad = m_fGivenRad;					//Sphere radius if we render each vertex using a ball
}	


void CPolyObj::glDrawVertexSphere(const int eid, const bool reversenorm)
{
	ASSERT0(eid>=0 && eid<m_nVertexCount);
	if (this->NeedTexture1D() && m_pVertexTexCoor1D){
		const float tx = m_pVertexTexCoor1D[eid];
		glTexCoord1f(tx);
	}

	float r = m_fGivenRad * m_pDrawParms->m_fVertexSizeScale;
	CSphere16::getInstance().glDraw(m_pVertex[eid], r, reversenorm);
}


inline void DrawSolidCube(const Vector3f& v0, const Vector3f& v7)
{
	const Vector3f v1(v7.x, v0.y, v0.z);
	const Vector3f v2(v0.x, v7.y, v0.z);
	const Vector3f v3(v7.x, v7.y, v0.z);
	const Vector3f v4(v0.x, v0.y, v7.z);
	const Vector3f v5(v7.x, v0.y, v7.z);
	const Vector3f v6(v0.x, v7.y, v7.z);

	glBegin(GL_QUADS);
		glNormal3f(0, 0, -1);
		glVertex3fv((&v0.x));
		glVertex3fv((&v1.x));
		glVertex3fv((&v3.x));
		glVertex3fv((&v2.x));

		glNormal3f(0, -1, 0);
		glVertex3fv((&v0.x));
		glVertex3fv((&v1.x));
		glVertex3fv((&v5.x));
		glVertex3fv((&v4.x));

		glNormal3f(1, 0, 0);
		glVertex3fv((&v1.x));
		glVertex3fv((&v3.x));
		glVertex3fv((&v7.x));
		glVertex3fv((&v5.x));

		glNormal3f(0, 0, 1);
		glVertex3fv((&v4.x));
		glVertex3fv((&v5.x));
		glVertex3fv((&v7.x));
		glVertex3fv((&v6.x));

		glNormal3f(0, 1, 0);
		glVertex3fv((&v2.x));
		glVertex3fv((&v3.x));
		glVertex3fv((&v7.x));
		glVertex3fv((&v6.x));

		glNormal3f(-1, 0, 0);
		glVertex3fv((&v0.x));
		glVertex3fv((&v2.x));
		glVertex3fv((&v6.x));
		glVertex3fv((&v4.x));
	glEnd();
}


inline void DrawSolidTetrahedron(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
	Vector3f n0 = compute_triangle_normal(v0, v1, v3);
	n0.normalize();
	Vector3f n1 = compute_triangle_normal(v1, v2, v3);
	n1.normalize();
	Vector3f n2 = compute_triangle_normal(v0, v3, v2);
	n2.normalize();
	Vector3f n3 = compute_triangle_normal(v0, v2, v1);
	n3.normalize();

	//render using the OpengGL array pointers
	Vector3f norms[4]={n0, n1, n2, n3};
	Vector3f vertices[4]={v0, v1, v2, v3};
	static int tris[12]={0, 1, 3, 1, 2, 3, 0, 3, 2, 0, 2, 1};
	drawSurfaceUsingArray(vertices, norms, tris, 4, 3); return;
	
	glBegin(GL_TRIANGLES);
		glNormal3fv(&n0.x);
		glVertex3fv(&v0.x);
		glVertex3fv(&v1.x);
		glVertex3fv(&v3.x);

		glNormal3fv(&n1.x);
		glVertex3fv(&v1.x);
		glVertex3fv(&v2.x);
		glVertex3fv(&v3.x);

		glNormal3fv(&n2.x);
		glVertex3fv(&v0.x);
		glVertex3fv(&v3.x);
		glVertex3fv(&v2.x);

		glNormal3fv(&n3.x);
		glVertex3fv(&v0.x);
		glVertex3fv(&v2.x);
		glVertex3fv(&v1.x);
	glEnd();
}


void CPolyObj::drawVertexTetra(const int eid)
{
	const float r = m_fGivenRad * m_pDrawParms->m_fVertexSizeScale*2;
	Vector3d& v = m_pVertex[eid];

	glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		if (m_pDrawParms->m_bEnableVertexRandDir){
			float x = rand();
			float y = rand();
			float z = rand();
			float angle = rand();
			glRotatef(angle, x, y, z);
		}
		if (this->NeedTexture1D() && m_pVertexTexCoor1D)
			glTexCoord1f(m_pVertexTexCoor1D[eid]);
		Vector3f v0(0,0,0), v1(0,0,0), v2(0,0,0), v3(0,0,0);
		v1.x+=r;
		v2.y+=r;
		v3.z+=r;
		DrawSolidTetrahedron(v0, v1, v2, v3);
	glPopMatrix();
}


void CPolyObj::drawVertexCube(const int eid)
{
	const Vector3d v = m_pVertex[eid];
	const float r = m_fGivenRad * m_pDrawParms->m_fVertexSizeScale *0.75f;

	glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		if (m_pDrawParms->m_bEnableVertexRandDir){
			float x=rand(), y=rand(), z=rand();
			float angle=rand();
			glRotatef(angle, x, y, z);
		}
		if (this->NeedTexture1D() && m_pVertexTexCoor1D)
			glTexCoord1f(m_pVertexTexCoor1D[eid]);
		const Vector3f v7(r, r, r);
		const Vector3f v0 = -v7;
		DrawSolidCube(v0, v7);
	glPopMatrix();
}


void CPolyObj::drawVertexPoint(const int eid)
{
	if (this->NeedTexture1D() && m_pVertexTexCoor1D){
		const float tx = m_pVertexTexCoor1D[eid];
		glTexCoord1f(tx);
	}
	glVertex3dv(&m_pVertex[eid].x);
}


int CPolyObj::exportElemNodes(FILE *fp, const int baseno, const double *pmatrix, const double)
{
	double matrix[4][4];
	if (pmatrix) vmCopy(pmatrix, matrix);
	for (int i=0; i<m_nVertexCount; i++){
		const int idx = baseno + i;
		const Vector3d& v = m_pVertex[i];
		double p1[3]={v.x, v.y, v.z}, p2[3];
		if (pmatrix){
			pmMult(p1, matrix, p2);
			VecAssign(p1, p2);
		}
		fprintf(fp, OUT_ID_DBL3_FORMAT, idx, p1[0], p1[1], p1[2]);
	}
	return m_nVertexCount;
}


int CPolyObj::exportElemNodeDisplacement(FILE *fp, const int baseno, const Vector3d & displ)
{
	const double dx=displ.x;
	const double dy=displ.y;
	const double dz=displ.z;
	for (int i=0; i<m_nVertexCount; i++){
		const int idx = baseno + i;		
		fprintf(fp, OUT_ID_DBL3_FORMAT, idx, dx, dy, dz);
	}
	return m_nVertexCount;
}


int CPolyObj::exportElemNodeDisplacementAndAngle(FILE *fp, const int baseno, const Vector3d & displ)
{
	const double dx=displ.x;
	const double dy=displ.y;
	const double dz=displ.z;
    const double angdispx=0;
    const double angdispy=0;

	for (int i=0; i<m_nVertexCount; i++){
		const int idx = baseno + i;		
		fprintf(fp, OUT_ID_DBL5_FORMAT, idx, dx, dy, dz, angdispx, angdispy);
	}
	return m_nVertexCount;
}


int CPolyObj::exportElemNodeVelocity(FILE *fp, const int baseno, const Vector3d & velocity)
{
	const double dx=velocity.x;
	const double dy=velocity.y;
	const double dz=velocity.z;
	for (int i=0; i<m_nVertexCount; i++){
		const int idx = baseno + i;		
		fprintf(fp, OUT_ID_DBL3_FORMAT, idx, dx, dy, dz);
	}
	return m_nVertexCount;
}


int CPolyObj::exportElemNodeVelocityAndAngle(FILE *fp, const int baseno, const Vector3d & velocity)
{
	const double dx=velocity.x;
	const double dy=velocity.y;
	const double dz=velocity.z;
    const double anglevx = 0;
    const double anglevy = 0;

	for (int i=0; i<m_nVertexCount; i++){
		const int idx = baseno + i;		
		fprintf(fp, OUT_ID_DBL5_FORMAT, idx, dx, dy, dz, anglevx, anglevy);
	}
	return m_nVertexCount;
}


int CPolyObj::exportElemNodeLoad(FILE *fp, const int baseno, const Vector3d & objload)
{
    double lx=objload.x, ly=objload.y, lz=objload.z;

	for (int i=0; i<m_nVertexCount; i++){
	    const int idx = baseno + i;	
        if (m_pLoad!=NULL){   
	        lx=m_pLoad[i].x;
	        ly=m_pLoad[i].y;
	        lz=m_pLoad[i].z;
	    }
        if ((lx!=0) || (ly!=0) || (lz!=0))
            fprintf(fp, OUT_ID_DBL3_FORMAT, idx, lx, ly, lz);
    }
	return m_nVertexCount;
}


void CPolyObj::transformVertexArray(const double matrix[4][4], Vector3d *pvertex)
{
	Vector3d *pdst = pvertex;
	if (pdst==NULL) pdst = m_pVertex;

	for (int i=0; i<m_nVertexCount; i++){
		Vector3d& v= m_pVertex[i];
		double point[3]={v.x, v.y, v.z};
		double p2[3];
		pmMult(point, matrix, p2);
		Vector3d& v2= pdst[i];
		v2.x = p2[0], v2.y=p2[1], v2.z=p2[2];
	}
}

//================================================

class CTriangleListItem
{
public:
	int m_nTriangleID;
	CTriangleListItem * m_pNext;

public:
	CTriangleListItem(void)
	{
		m_pNext = NULL;
		m_nTriangleID = -1;
	}

	~CTriangleListItem(){}
};

static void _buildTriangleTabForVertices(CTriangleListItem **pVertex, const int nv, const int *ptri, const int ntri, const int nstep, CMemoryMgr &mgr)
{
	int i, j;
	for (i=0; i<nv; i++) pVertex[i]=NULL;
	for (i=0; i<ntri; i++){
		const int* a = &ptri[i*nstep];
		for (j=0; j<nstep; j++){
			const int v0 = a[j];
			CTriangleListItem * pListItem = (CTriangleListItem*)mgr.Malloc(sizeof(CTriangleListItem));
			pListItem->m_nTriangleID = i;
			pListItem->m_pNext = pVertex[v0];
			pVertex[v0] = pListItem;
		}
	}
}

static inline void MARK_VERTEX_INQUEUE(unsigned char *bitbuff, const int v)
{
	unsigned char & b = bitbuff[v];
	b |= 0x80;
}
static inline Vector3f GET_TRIANGLE_NORM(const int tid, const int *pTriangle, const int nstep, Vector3d *pVertex)
{
	const int* ptri = &pTriangle[tid*nstep];
	const Vector3d& a=pVertex[ptri[0]];
	const Vector3d& b=pVertex[ptri[1]];
	const Vector3d& c=pVertex[ptri[2]];
	const Vector3d n = compute_triangle_normal(a, b, c);
	const Vector3f norm(n.x, n.y, n.z);
	return norm;
}
void CPolyObj::propergateFemBoundSeed(const int tid, const float THRESHOLD, const int *ptri, const int nstep)
{
	assert(tid>=0 && tid< m_nPolygonCount);

	// Build the tirangle list buffer;
	CTriangleListItem **pVertex = new CTriangleListItem * [m_nVertexCount];
	assert(pVertex!=NULL);
	CMemoryMgr mgr;
	_buildTriangleTabForVertices(pVertex, m_nVertexCount, ptri, m_nPolygonCount, nstep, mgr);

	//init, or setup before propergation
	setupBoundaryConditionBufferBeforePropergation();

	bool *pVisited = new bool [m_nPolygonCount];
	assert(pVisited!=NULL);
	for (int i=0; i<m_nPolygonCount; i++) pVisited[i]=false;
	vector<int> que(20000);
	que.push_back(tid);
	pVisited[tid]=true;
	const Vector3f norm0=GET_TRIANGLE_NORM(tid, ptri, nstep, m_pVertex);

	while (que.size()>0){
		const int nsize = que.size();
		const int t = que[nsize-1];
		que.pop_back();
		//process this triangle;
		const Vector3f norm1=GET_TRIANGLE_NORM(t, ptri, nstep, m_pVertex);
		const float dotr = (DotProd(norm0, norm1));
		if (dotr < 1-THRESHOLD) continue;
		const int* ta = &ptri[t*nstep];
		//propergate its neighbours;
		for (int i=0; i<nstep; i++){
			const int v0 = ta[i];
			MARK_VERTEX_INQUEUE(m_pFemBoundCond, v0);
			CTriangleListItem *phead = pVertex[v0];
			while (phead!=NULL){
				const int x = phead->m_nTriangleID;
				if (!pVisited[x]){
					que.push_back(x);
					pVisited[x]=true;
				}
				phead = phead->m_pNext;
			}
		}
	}

	//free spaces
	delete [] pVisited;
	delete [] pVertex;
}


void CPolyObj::exportBoundaryConditions(FILE *fp, const int baseno)
{
	unsigned char *pBound = this->m_pFemBoundCond;
	if (pBound==NULL) return;
	char str[8][20] = { "0 0 0", 
						"1 0 0", 
						"0 1 0", 
						"1 1 0", 
						"0 0 1", 
						"1 0 1", 
						"0 1 1", 
						"1 1 1"};

	for (int i=0; i<m_nVertexCount; i++){
		const unsigned char b = pBound[i]&0x07;
		if (b)
			fprintf(fp, "%d %s 0 0 0\n", i+baseno, str[b]);
	}
}

CObjectBase* runGPUTetIsoRender(CObjectBase *pRenderingObj, CPolyObj *pobj, CGLDrawParms *pdrawparm)
{
	return NULL;
}

void CPolyObj::drawVolumetricElements(void)
{
	extern CObjectBase* runGPUTetIsoRender(CObjectBase *pRenderingObj, CPolyObj *pobj, CGLDrawParms *pdrawparm);
	CObjectBase* p;

	p = runGPUTetIsoRender(m_pRenderingObject, this, m_pDrawParms);
	if (p!=NULL)
		m_pRenderingObject = p;
}
