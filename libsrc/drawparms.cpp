//******************
//FILE: drawparms.cpp
//*********************

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <drawparms.h>
#include <colordef.h>


//=======================================================
Vector3f CGLDrawParms::m_cColorLevel[COLOR_LEVEL_BUFFLENGTH];
static int count = 0;
const int N=9;
static Vector3f colors[N]={
		COLOR_Light_Purple,
		COLOR_Green,
		COLOR_Blue,
		COLOR_Yellow,
		COLOR_Med_Purple,
		COLOR_Cyan,
		COLOR_Magenta,
		COLOR_White,
		COLOR_Pink
};

void CGLDrawParms::resetColorIndex(void)
{
	count = 0;
}

static Vector3f allocate_object_color(const int colorstep)
{
	Vector3f objcolor = colors[count];
	count = (count+colorstep)%N;
	return objcolor;
}

Vector3f CGLDrawParms::allocateNewColor()
{
	Vector3f c = allocate_object_color(1);
	return c;
}

static Vector3f preview_next_object_color(void)
{
	Vector3f objcolor = colors[count];
	return objcolor;
}

Vector3f CGLDrawParms::previewNewColor(void)
{
	Vector3f c = preview_next_object_color();
	return c;
}


//=======================================================
//LOD rendering.
//screen point to world point
void CGLDrawParms::WorldTransform(const double v[3], double vt[3])
{
	gluProject(v[0], v[1], v[2], m_Identity, m_Projection, m_Viewport, &vt[0], &vt[1], &vt[2]);
}

void CGLDrawParms::World2Screen(const GLdouble v[3], GLdouble winxyz[3])
{
	gluProject(v[0], v[1], v[2], m_ModelView, m_Projection, m_Viewport, &winxyz[0], &winxyz[1], &winxyz[2]);
}

//screen point to world point
void CGLDrawParms::Screen2World(const GLdouble winxyz[3], GLdouble v[3])
{
	gluUnProject(winxyz[0], winxyz[1], winxyz[2], m_ModelView, m_Projection, m_Viewport, &v[0], &v[1], &v[2]);
}

void CGLDrawParms::setupMaterial(void) const
{
	//get rid of artifacts in texture mapping
	Vector3f objclr(m_cObjectColor.x, m_cObjectColor.y, m_cObjectColor.z);
	if (m_bEnableTexture1D || m_bEnableTexture2D) 
		objclr = Vector3f(1,1,1);

	//only values in the range 0 --128 are accepted
	const float matShininess  = 100.0f;
	const float Ia = 0.1250f;
	const float Kd = 0.75f;
	const float Ks = 0.40f;

	float matAmbient[4] = {Ia, Ia, Ia, 1};
    float matBackAmbient[4] = {Ia, Ia, Ia, 1};
	float matAmbientDif[4] = { 0.34615f, 0.3143f, 0.0903f, 1.0f};
	float matDiffuse[4];
	float matSpecular[4];
	float matEmission[4];
	for (int i=0; i<3; i++){
		matDiffuse[i] = objclr[i]*Kd;
		matSpecular[i] = objclr[i]*Ks + 0.10f;
		matEmission[i] = 0.05f*matDiffuse[i];
	}
	matDiffuse[3]=1;
	matSpecular[3]=1;
	matEmission[3]=1;

	float matBackDif[4] = {1-0.5f*objclr.x, 1-0.5f*objclr.y, 1-0.5f*objclr.z, 1};
    float matBackSpec[4]={0.2, 0.2, 0.2, 1};	

	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT,          GL_AMBIENT,   matAmbient);
	glMaterialfv(GL_BACK,           GL_AMBIENT,   matBackAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   matAmbientDif);
	glMaterialfv(GL_FRONT,          GL_DIFFUSE,   matDiffuse);
	glMaterialfv(GL_BACK,           GL_DIFFUSE,   matBackDif);
	glMaterialfv(GL_FRONT,          GL_SPECULAR,  matSpecular);
	glMaterialfv(GL_BACK,           GL_SPECULAR,  matBackSpec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  matEmission);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

	//allow changes of color to be material;
	/*
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	if (m_bEnableTexture)
		glColor3f(1, 1, 1);
	else
		glColor3fv(matDiffuse);
	glDisable(GL_COLOR_MATERIAL);
	*/
	//lighting model
	//int gval = GL_TRUE;
	//glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, &gval);
}

/*
void CGLDrawParms::setupMaterial(void) const
{
	float matAmbient[4] = { 0.24725f, 0.2245f, 0.0645f, 1.0f };
	float matAmbientDif[4]  = { 0.34615f, 0.3143f, 0.0903f, 1.0f};
	float matDiffuse[4];
	float matBackDif[4];
    float matBackAmbient[4];
    float matBackSpec[4]={0,0,0,1};
	float matEmission[4]= {0.1f, 0.1f, 0.1f, 1};

	matDiffuse[0] = m_cObjectColor.x;
	matDiffuse[1] = m_cObjectColor.y;
	matDiffuse[2] = m_cObjectColor.z;
	matDiffuse[3] = 1;

    matBackDif[0]=0; //1-matDiffuse[0];
    matBackDif[1]=1; //-matDiffuse[1];
    matBackDif[2]=0; //1-matDiffuse[2];
    matBackDif[3]=1;

    matBackAmbient[0]=0;
    matBackAmbient[1]=0.2f;
    matBackAmbient[2]=0;
    matBackAmbient[3]=1;

	float matSpecular[4] = { 0.797357f, 0.723991f, 0.208006f, 1.0f};
	float matShininess  = 43.2f;

	for (int i=0; i<3; i++){
        matAmbient[i] *=0.5f;
		matSpecular[i] *= 0.30f;
		matDiffuse[i] *= 1.0f;
		matEmission[i] = 0.10f*matDiffuse[i];
	}

	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT,          GL_AMBIENT,   matAmbient);
	glMaterialfv(GL_BACK,           GL_AMBIENT,   matBackAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   matAmbientDif);
	glMaterialfv(GL_FRONT,          GL_DIFFUSE,   matDiffuse);
	glMaterialfv(GL_BACK,           GL_DIFFUSE,   matBackDif);
	glMaterialfv(GL_FRONT,          GL_SPECULAR,  matSpecular);
	glMaterialfv(GL_BACK,           GL_SPECULAR,  matBackSpec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  matEmission);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

	//allow changes of color to be material;
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	if (m_bEnableTexture)
		glColor3f(1, 1, 1);
	else
		glColor3fv(matDiffuse);
	glDisable(GL_COLOR_MATERIAL);

	//lighting model
	//int gval = GL_TRUE;
	//glLightModeliv(GL_LIGHT_MODEL_TWO_SIDE, &gval);
}

*/

void CGLDrawParms::reCalcLevelColor(const Vector3f& cLow, const Vector3f& cHigh, const int maxLevel)
{
	float t0, t1;
	m_cLevel0Color = cLow;
	m_cLevelNColor = cHigh;

	assert(maxLevel>=0 && maxLevel<=COLOR_LEVEL_BUFFLENGTH);
	for (int i=0; i<maxLevel; i++){
		t0 = 1.0f-((float)i)/maxLevel;
		t1 = 1.0f - t0;
		m_cColorLevel[i] = t0*m_cLevel0Color + t1*m_cLevelNColor;
	}
}


static void 
interpolate_color(unsigned char clr[][4], const int len, int c1[4], int c2[4])
{
	int i;
	float k = 1.0f/(len-1);
	for (i=0; i<len; i++){
		float t= i*k;
		float t1 = 1-t;
		unsigned char *p = clr[i];
		p[0] = t1*c1[0] + t*c2[0];
		p[1] = t1*c1[1] + t*c2[1];
		p[2] = t1*c1[2] + t*c2[2];
		p[3] = 0;
	}
}


CGLDrawParms::CGLDrawParms(const int colorstep)
{
	const int l=16;
	int i, x, y, base;
	const float K = 1.0f/255.0f;

	m_bValidDrawParms = true;		//is the class enabled/disabled??
	m_bMirrorFlag = false;

    m_bUseDispList = false;
    m_bUseVbo = false;

	m_ppColorTable = NULL;

	//setting the legend options;
	m_bShowLegend = true;
	m_nLegendLevel = 16;
	m_cLegendTextColor =  Vector3f(0.5f,0.5f,0.5f);
	m_cLegendPosition = 0;
	m_cLegendOrientation = 0;
    m_bInEPSSnag = 0;

	for (i=0; i<NAME_ARRAY_SIZE; i++)
		m_strVarNames[i] = NULL;

//	m_nDrawType = DRAW_MESH_HIDDENLINE;
	m_nDrawType = DRAW_MESH_SHADING;
	m_nDrawVertexStyle = DRAW_VERTEX_NONE;

	const float bgvalue= 0.70f;
	m_cBackgroundColor = Vector3f(bgvalue);
//	m_cBackgroundColor = Vector3f(120, 166, 240)*K;
	m_cObjectColor = allocate_object_color(colorstep);
	//m_cObjectColor = Vector3f(255, 180, 165)*K;
	//m_cObjectColor = Vector3f(200/255.0f, 120/255.0f, 128/255.0f);
	//m_cObjectColor = Vector3f(0.34615f, 0.3143f, 0.0903f);
	m_cHLineColor = Vector3f(0, 0, 0);
	m_cVertexColor = Vector3f(0.9f, 0.9f, 0.9f);
	m_cNormalColor = Vector3f(1,0,0);

	m_cLevel0Color = Vector3f(0, 0, 1);
	m_cLevelNColor = Vector3f(1, 0, 0);
	reCalcLevelColor(m_cLevel0Color, m_cLevelNColor, 16);

	m_fLineWidth =1;
	m_fAxisLineWidth = 1;
	m_fVertexSizeScale =1;
	m_fNormalLengthScale = 1;

	m_bEnableLighting = true;
	m_bEnableDepthTest = true;
	m_bEnableTexture1D = false;
	m_bEnableTexture2D = false;
	m_bEnableTexture3D = false;

	m_bEnableSmoothing = false;
	m_bEnableShadow = false;

	m_bEnableVertexTexture = false;
	m_bEnableVertexRandDir = false;
	m_bEnablePNTriangleSubdiv = false;

	m_bShowVertexNormal = false;
	m_bShowAixes = false;
	m_bShowBoudingCube = false;
	m_bShowCamera = false;
	m_bShowSphereControl = false;

	//================OCTREE====================
	m_nShowMinOctreeLevel = 6;
	m_nShowMaxOctreeLevel = 7;
	
	m_nShowOctMinX = 0;
	m_nShowOctMaxX = 1<<l;
	m_nShowOctMinY = 0;
	m_nShowOctMaxY = 1<<l;
	m_nShowOctMinZ = 0;
	m_nShowOctMaxZ = 1<<l;
	
	m_bDrawSelectedCell = true;
	m_bProjectionMode = AVL_PERSPECTIVE_PROJECTION;
	m_bUseVertexNormal = false;

	//===============view parms=================
	m_ViewPos = Vector3f(0,0,0);
	m_ViewDir = Vector3f(0,0,1);
	m_fFocalLength = 0.01;
	m_fFovy = 60;

	//=================LODR=====================
	m_Identity[0]=1, m_Identity[1]=0, m_Identity[2]=0, m_Identity[3]=0; 
	m_Identity[4]=0, m_Identity[5]=1, m_Identity[6]=0, m_Identity[7]=0; 
	m_Identity[8]=0, m_Identity[9]=0, m_Identity[10]=1, m_Identity[11]=0; 
	m_Identity[12]=0, m_Identity[13]=0, m_Identity[14]=0, m_Identity[15]=1; 

	//=================PICK====================
	m_nPickedEdgeID = -1;
	m_vPickedVertex0 = Vector3d(0,0,0);
	m_vPickedVertex1 = Vector3d(0,0,0);
	m_vPickedVertex2 = Vector3d(0,0,0);
	m_vPickedVertex3 = Vector3d(0,0,0);

	//set up the palette for mapping 
	for (i=0; i<256; i++){
		unsigned char *p = m_texture1D[i];
		p[0]=p[1]=p[2]=0, p[3]=255;
	}

	x = 2, y = 1;
	for (i=0; i<64; i++){
		base = i + 0;
		m_texture1D[base][x] = 255;
		m_texture1D[base][y] = i*4;
		m_texture1D[base][0] = 0;
	}
	for (i=0; i<64; i++){
		base = i + 64;
		m_texture1D[base][x] = 255-i*4;
		m_texture1D[base][y] = 255;
	}
	x = 1, y = 0;
	for (i=0; i<64; i++){
		base = i + 128;
		m_texture1D[base][x] = 255;
		m_texture1D[base][y] = i*4;
	}
	for (i=0; i<64; i++){
		base = i + 192;
		m_texture1D[base][x] = 255-i*4;
		m_texture1D[base][y] = 255;
	}

	m_fMinRange = 0;
	m_fMaxRange = 1;

	m_nMappingType = ScalarMappingLinear;
	m_nTextureCoorType = 0;	//the index of 1D texture attrib
	m_nTextureHandle = 0;
	m_bRecomputeTexture1DParameter = true;
	m_bRecomputeTexture1DRange = true;
	m_bShowVertexAttribValue = false; 

	//frame buffer object in the window, we will support up to 4 MRT buffers
	m_pFboMRT = NULL;		
}


bool CGLDrawParms::CellVisible(const int lvl, const Vector3I cellpos)
{
	float cellsize = 1.0f/(1<<lvl);
	Vector3d winv, cellbase, v;

	cellbase.x = cellpos.x * cellsize;
	cellbase.y = cellpos.y * cellsize;
	cellbase.z = cellpos.z * cellsize;

	v = cellbase;
	if (VertexVisible(v, winv))
		return true;
	v.x+=cellsize, v.y+=cellsize, v.z+=cellsize;
	if (VertexVisible(v, winv))
		return true;

	v=cellbase, v.x+=cellsize;
	if (VertexVisible(v, winv))
		return true;
	v=cellbase, v.y+=cellsize, v.z+=cellsize;
	if (VertexVisible(v, winv))
		return true;

	//float half=cellsize*0.5f;
	//v=cellbase, v+=Vector3D(half);
	//if (VertexVisible(v, winv))
	//	return true;

	return false;
}



bool CGLDrawParms::Qrefine2(const int level, const Vector3I& pos, const Vector3f&v, const float qerror, bool & visible)
{
	Vector3f winv;
	
	if (level>6){
		visible = true;
		World2Screen(v, winv);
	}
	else
		visible = VertexVisible(v, winv);

	if (!visible){
		if (!CellVisible(level, pos)) 
			return false;
		int ll = level;
		if (ll < 4 ) ll=4;
		float cellsize = 0.5f/(1<<ll);
		cellsize*=cellsize;
		if (qerror > cellsize)
			return true;
	}

	if (ScreenErrorHigh(winv, qerror)){
		return true;
	}
	return false;
}



bool VertexInFrustum(const Vector3d& v);
bool CellInFurstum(const int l, const int x, const int y, const int z);
bool ScreenErrorBig(const Vector3d &v, const float qerror);
/*
bool CGLDrawParms::Qrefine(const int level, const Vector3I& pos, const Vector3D&v, const float qerror, bool & visible)
{	
	if (level>7){
		visible = true;
	}
	else{
		visible = CellInFurstum(level, pos.x, pos.y, pos.z);
	}

	if (!visible){
		return false;
	}

	if (ScreenErrorBig(v, qerror)){
		return true;
	}
	return false;
}
*/


void CGLDrawParms::DrawPickedLine(void)
{
	if (m_nPickedEdgeID<0)
		return;

    glLineWidth(4);
    glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glColor3ub(255, 0, 0);
	glBegin(GL_LINES);
		glVertex3dv(&m_vPickedVertex0.x);
		glVertex3dv(&m_vPickedVertex1.x);
	glEnd();
}


static float light_pos0[3];
static float light_pos1[3];
static float light_pos2[3];

void CGLDrawParms::BeginDrawing()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixd(m_Identity);

	//===========Texture binding=======================
	if (m_bEnableTexture1D || m_bEnableVertexTexture){
		glEnable(GL_TEXTURE_1D);
		GLuint texture=0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_1D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		//GL_MODULATE, GL_DECAL, GL_BLEND, and GL_REPLACE.
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage1D(GL_TEXTURE_1D, 0, 4, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texture1D);
		m_nTextureHandle = texture;
	}

	//set material
	setupMaterial();
}


void CGLDrawParms::PostDrawing()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	if (m_bEnableTexture1D){
		if (m_nTextureHandle != 0)
			glDeleteTextures(1, &m_nTextureHandle);
	}
}


Vector3f * getColorLibrary(int & ncolor)
{
	ncolor = N;
	return colors;
}