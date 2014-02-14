//FILE: QtViewer.cpp

#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#include <GL/glaux.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <qevent.h>

#include <double_buffer.h>
#include <view3dn/gllight.h>
#include "QtViewer.h"
#include "GenGraphForm.h"
#include "Main2.h"
#include <crest/system/ztime.h>
#include <crest/simulation/truss_thread.h>
#include <crest/GPU/Cuda/cuda0.h>
#include <crest/collision/collision-interface.h>
#include <view3dn/texturedtmeshobj.h>

using std::cout;
using std::endl;
//using namespace qglviewer;

// Mouse Interactor
bool QtViewer::_mouseTrans = false;
bool QtViewer::_mouseRotate = false;
Quaternion QtViewer::_mouseInteractorNewQuat;
Quaternion QtViewer::_newQuat;
Quaternion QtViewer::_currentQuat;
Vector3 QtViewer::_mouseInteractorRelativePosition(0,0,0);

// Shadow Mapping parameters

// These store our width and height for the shadow texture
enum { SHADOW_WIDTH = 512 };
enum { SHADOW_HEIGHT = 512 };
enum { SHADOW_MASK_SIZE = 2048 };

// This is used to set the mode with glTexParameteri() for comparing depth values.
// We use GL_COMPARE_R_TO_TEXTURE_ARB as our mode.  R is used to represent the depth value.
#define GL_TEXTURE_COMPARE_MODE_ARB       0x884C

// This is used to set the function with glTexParameteri() to tell OpenGL how we
// will compare the depth values (we use GL_LEQUAL (less than or equal)).
#define GL_TEXTURE_COMPARE_FUNC_ARB       0x884D

// This mode is what will compare our depth values for shadow mapping
#define GL_COMPARE_R_TO_TEXTURE_ARB       0x884E

// The texture array where we store our image data
GLuint g_DepthTexture;

// This is our global shader object that will load the shader files
//CShader g_Shader;

//float g_DepthOffset[2] = { 3.0f, 0.0f };
float g_DepthOffset[2] = { 10.0f, 0.0f };
float g_DepthBias[2] = { 0.0f, 0.0f };

// These are the light's matrices that need to be stored
float g_mProjection[16] = {0};
float g_mModelView[16] = {0};
//float g_mCameraInverse[16] = {0};

GLuint ShadowTextureMask;

// End of Shadow Mapping Parameters

// ---------------------------------------------------------
// --- Constructor
// ---------------------------------------------------------
const QGLWidget* shareWidget=NULL;
QtViewer::QtViewer( QWidget* parent): ZQGLViewer(parent, shareWidget)
{
	_defaultInit(NULL, NULL, NULL, NULL);
}

QtViewer::QtViewer( QWidget* parent, CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, const char* name )
	: ZQGLViewer(parent, shareWidget)
{
	_defaultInit(pDraw, pSG, pLaptoolSG, pBackgroundSG);
}

void QtViewer::setScene(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG)
{
	m_pDrawParms = pDraw;				//rendering configuration
	m_pSceneGraph = pSG;				//scengraph, objects that can be deformed in CIA3d
	m_pLaptoolSceneGraph = pLaptoolSG;	//scengraph, objects that can be deformed in CIA3d
	m_pBgSceneGraph = pBackgroundSG;	//scengraph for Background objects, including lap. tools
}

static Vector3i _getBackgroundColor(CGLDrawParms* m_pDrawParms)
{
	Vector3i color(0,0,0);
  	if (m_pDrawParms){
		const int r = m_pDrawParms->m_cBackgroundColor.x * 255;
		const int g = m_pDrawParms->m_cBackgroundColor.y * 255;
		const int b = m_pDrawParms->m_cBackgroundColor.z * 255;
		color = Vector3i(r,g,b);
	}
	return color;
}

void QtViewer::_defaultInit(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG)
{
	setScene(pDraw, pSG, pLaptoolSG, pBackgroundSG);
	
	groot = NULL;				
	//initTexturesDone = false;
	// setup OpenGL mode for the window
	//Fl_Gl_Window::mode(FL_RGB | FL_DOUBLE | FL_DEPTH | FL_ALPHA);
	timerStep = new QTimer(this);
	connect( timerStep, SIGNAL(timeout()), this, SLOT(updateGL()) );
	//----------------------------------------------------
	//int QTimer::start ( int msec, bool sshot = FALSE )
	//----------------------------------------------------
	timerStep->start(33.33);		//30HZ FPS timeout for display rendering
	//timerAnimate = new QTimer(this);
	//timerAnimate->start(200);	//10HZ FPS timeout
	//connect( timerAnimate, SIGNAL(timeout()), this, SLOT(animate()) );

	_previousEyePos = Vector3(0.0, 0.0, 0.0);
	_zoom = 1.0;
	_zoomSpeed = 250.0;
	_panSpeed = 25.0;
	_navigationMode = TRACKBALL_MODE;
//	_spinning = false;
//	_moving = false;
	_video = false;
	_animationOBJ = false;
	_axis = false;
	_background = 0;
	_shadow = false;
	_numOBJmodels = 0;
	_materialMode = 0;
	_facetNormal = GL_FALSE;
	_renderingMode = GL_RENDER;
	_waitForRender = false;
	sceneBBoxIsValid = false;
	//texLogo = NULL;

	_automateDisplayed = false;

	/*_surfaceModel = NULL;
	_springMassView = NULL;
	_mapView = NULL;
	sphViewer = NULL;
	*/
	/*
	_arrow = gluNewQuadric();
	gluQuadricDrawStyle(_arrow, GLU_FILL);
	gluQuadricOrientation(_arrow, GLU_OUTSIDE);
	gluQuadricNormals(_arrow, GLU_SMOOTH);

	_tube = gluNewQuadric();
	gluQuadricDrawStyle(_tube, GLU_FILL);
	gluQuadricOrientation(_tube, GLU_OUTSIDE);
	gluQuadricNormals(_tube, GLU_SMOOTH);

	_sphere = gluNewQuadric();
	gluQuadricDrawStyle(_sphere, GLU_FILL);
	gluQuadricOrientation(_sphere, GLU_OUTSIDE);
	gluQuadricNormals(_sphere, GLU_SMOOTH);

	_disk = gluNewQuadric();
	gluQuadricDrawStyle(_disk, GLU_FILL);
	gluQuadricOrientation(_disk, GLU_OUTSIDE);
	gluQuadricNormals(_disk, GLU_SMOOTH);
	*/
	// init trackball rotation matrix / quaternion
	//_newTrackball.ComputeQuaternion(0.0, 0.0, 0.0, 0.0);
	//_newQuat = _newTrackball.GetQuaternion();

	////////////////
	// Interactor //
	////////////////
    m_isControlPressed = false;
	_mouseInteractorMoving = false;
	_mouseInteractorTranslationMode = false;
	_mouseInteractorRotationMode = false;
	_mouseInteractorSavedPosX = 0;
	_mouseInteractorSavedPosY = 0;
	//_mouseInteractorTrackball.ComputeQuaternion(0.0, 0.0, 0.0, 0.0);
	//_mouseInteractorNewQuat = _mouseInteractorTrackball.GetQuaternion();

	//interactor = NULL;
        
    //////////////////////
    m_dumpState = false;
	m_dumpStateStream = 0;
    m_displayComputationTime = false;
    m_exportGnuplot = false;
}


// ---------------------------------------------------------
// --- Destructor
// ---------------------------------------------------------
QtViewer::~QtViewer()
{
}

// -----------------------------------------------------------------
// --- OpenGL initialization method - includes light definitions, 
// --- color tracking, etc.
// -----------------------------------------------------------------

void QtViewer::init(void)
{
	//call parent class's init function
	ZQGLViewer::init();

	//this class's init
	static bool	initialized	= false;
	const Vector3i bgcolor = _getBackgroundColor(m_pDrawParms);
	setBackgroundColor(QColor(bgcolor.x, bgcolor.y, bgcolor.z));
	camera()->frame()->setSpinningSensitivity(4);

	if (!initialized){
		{
			int ligID = 0;
			qglviewer::Vec viewpos = camera()->position();
			Vector3f lightpos0(viewpos.x, viewpos.y, viewpos.z);
			CPointLight light0(lightpos0, 1.00);
			//light0.applyLight(ligID);
			ligID=1;
			Vector3f lightpos1(-1000, -1000, -1000);
			CPointLight light1(lightpos1, 0.10f);
			//light1.applyLight(ligID++);
			Vector3f lightpos2(-1000, -1000, 0);
			CPointLight light2(lightpos2, 0.20f);
			light2.applyLight(ligID++);
		}

		//glShadeModel(GL_SMOOTH);
		// Here we allocate memory for our depth texture that will store our light's view
		//CreateRenderTexture(g_DepthTexture, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
		//CreateRenderTexture(ShadowTextureMask, SHADOW_MASK_SIZE, SHADOW_MASK_SIZE, GL_LUMINANCE, GL_LUMINANCE);
		/*
		if (_glshadow = CShader::InitGLSL())
		{
			// Here we pass in our new vertex and fragment shader files to our shader object.
			g_Shader.InitShaders("../share/shaders/ShadowMappingPCF.vert", "../share/shaders/ShadowMappingPCF.frag");
		}else
		{
			printf("WARNING QtViewer : shadows are not supported !\n");
			_shadow = false;
		}*/

		// change status so we only do this stuff once
		initialized = true;
		//_beginTime = CTime::getTime();
		printf("\n");
	}

	// switch to preset view
	//SwitchToPresetView();
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------

///////////////////////////////// STORE LIGHT MATRICES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function positions our view from the light for shadow mapping
/////
///////////////////////////////// STORE LIGHT MATRICES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void QtViewer::StoreLightMatrices()
{	
	//	_lightPosition[0] =  _sceneTransform.translation[0] + 10;//*cosf(TT);
	//	_lightPosition[1] =  _sceneTransform.translation[1] + 10;//*sinf(2*TT);
	//	_lightPosition[2] =  _sceneTransform.translation[2] + 35;//

	//_lightPosition[0] =  1;
	//_lightPosition[1] =  -10;
	//_lightPosition[2] =  0;

	// Reset our current light matrices
	memset(g_mModelView, 0, sizeof(float)*16);
	memset(g_mProjection, 0, sizeof(float)*16);

	g_mModelView[0] = 1; // identity
	g_mModelView[5] = 1;
	g_mModelView[10] = 1;
	g_mModelView[15] = 1;

	// Using perpective shadow map for the "miner lamp" case ( i.e. light.z == 0 )
	// which is just a "rotation" in sceen space

	float lx;// = -_lightPosition[0] * lastProjectionMatrix[0] - _lightPosition[1] * lastProjectionMatrix[4] + lastProjectionMatrix[12];
	float ly;// = -_lightPosition[0] * lastProjectionMatrix[1] - _lightPosition[1] * lastProjectionMatrix[5] + lastProjectionMatrix[13];
	float lz;// = -_lightPosition[0] * lastProjectionMatrix[2] - _lightPosition[1] * lastProjectionMatrix[6] + lastProjectionMatrix[14];
	//float lw = -_lightPosition[0] * lastProjectionMatrix[3] - _lightPosition[1] * lastProjectionMatrix[7] + lastProjectionMatrix[15];
	//std::cout << "lx = "<<lx<<" ly = "<<ly<<" lz = "<<lz<<" lw = "<<lw<<std::endl;

	Vector3 l(-lx,-ly,-lz);
	Vector3 y;
	y = l.cross(Vector3(1,0,0));
	Vector3 x;
	x = y.cross(l);
	l.normalize();
	y.normalize();
	x.normalize();

	g_mProjection[ 0] = x[0]; g_mProjection[ 4] = x[1]; g_mProjection[ 8] = x[2]; g_mProjection[12] =    0;
	g_mProjection[ 1] = y[0]; g_mProjection[ 5] = y[1]; g_mProjection[ 9] = y[2]; g_mProjection[13] =    0;
	g_mProjection[ 2] = l[0]; g_mProjection[ 6] = l[1]; g_mProjection[10] = l[2]; g_mProjection[14] =    0;
	g_mProjection[ 3] =    0; g_mProjection[ 7] =    0; g_mProjection[11] =    0; g_mProjection[15] =    1;

	g_mProjection[ 0] = x[0]; g_mProjection[ 4] = y[0]; g_mProjection[ 8] = l[0]; g_mProjection[12] =    0;
	g_mProjection[ 1] = x[1]; g_mProjection[ 5] = y[1]; g_mProjection[ 9] = l[1]; g_mProjection[13] =    0;
	g_mProjection[ 2] = x[2]; g_mProjection[ 6] = y[2]; g_mProjection[10] = l[2]; g_mProjection[14] =    0;
	g_mProjection[ 3] =    0; g_mProjection[ 7] =    0; g_mProjection[11] =    0; g_mProjection[15] =    1;

	glPushMatrix();{

		glLoadIdentity();
		glScaled(1.0/(fabs(g_mProjection[0])+fabs(g_mProjection[4])+fabs(g_mProjection[8])),
			1.0/(fabs(g_mProjection[1])+fabs(g_mProjection[5])+fabs(g_mProjection[9])),
			1.0/(fabs(g_mProjection[2])+fabs(g_mProjection[6])+fabs(g_mProjection[10])));
		glMultMatrixf(g_mProjection);
		glMultMatrixd(lastProjectionMatrix);
		
		// Grab the current matrix that will be used for the light's projection matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, g_mProjection);

		// Go back to the original matrix
	}glPopMatrix();

	/*
	// Let's push on a new matrix so we don't change the rest of the world
	glPushMatrix();{

		// Reset the current modelview matrix
		glLoadIdentity();
		
		// This is where we set the light's position and view.
		gluLookAt(_lightPosition[0],  _lightPosition[1],  _lightPosition[2], 
		_sceneTransform.translation[0],	   _sceneTransform.translation[1],	    _sceneTransform.translation[2],		0, 1, 0);

		// Now that we have the light's view, let's save the current modelview matrix.
		glGetFloatv(GL_MODELVIEW_MATRIX, g_mModelView);

		// Reset the current matrix
		glLoadIdentity();

		// Set our FOV, aspect ratio, then near and far planes for the light's view
		gluPerspective(90.0f, 1.0f, 4.0f, 250.0f);

		// Grab the current matrix that will be used for the light's projection matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, g_mProjection);

		// Go back to the original matrix
	}glPopMatrix();
	*/
}

/////////////////////////////// CREATE RENDER TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function creates a blank texture to render to
/////
/////////////////////////////// CREATE RENDER TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void QtViewer::CreateRenderTexture(GLuint& textureID, int sizeX, int sizeY, int channels, int type)
{
	glGenTextures(1, &textureID);								
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Create the texture and store it on the video card
	glTexImage2D(GL_TEXTURE_2D, 0, channels, sizeX, sizeY, 0, type, GL_UNSIGNED_INT, NULL);

	// Set the texture quality
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

//////////////////////////////// APPLY SHADOW MAP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function applies the shadow map to our world data
/////
//////////////////////////////// APPLY SHADOW MAP \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void QtViewer::ApplyShadowMap()
{				
	/*
	// Let's turn our shaders on for doing shadow mapping on our world
	g_Shader.TurnOn();

	// Turn on our texture unit for shadow mapping and bind our depth texture
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D); 
	glBindTexture(GL_TEXTURE_2D, g_DepthTexture);

	// Give GLSL our texture unit that holds the shadow map
	g_Shader.SetInt(g_Shader.GetVariable("shadowMap"), 1);
	//g_Shader.SetInt(g_Shader.GetVariable("tex"), 0);

	// Here is where we set the mode and function for shadow mapping with shadow2DProj().

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
		GL_COMPARE_R_TO_TEXTURE_ARB);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);

	// Create our bias matrix to have a 0 to 1 ratio after clip space
	const float mBias[] = {0.5, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5+g_DepthBias[0], 0.0,
		0.5, 0.5, 0.5+g_DepthBias[1], 1.0};

	glMatrixMode(GL_TEXTURE);

	glLoadMatrixf(mBias);			// The bias matrix to convert to a 0 to 1 ratio
	glMultMatrixf(g_mProjection);	// The light's projection matrix
	glMultMatrixf(g_mModelView);	// The light's modelview matrix
	//glMultMatrixf(g_mCameraInverse);// The inverse modelview matrix

	glMatrixMode(GL_MODELVIEW);			// Switch back to normal modelview mode

	glActiveTextureARB(GL_TEXTURE0_ARB);

	// Render the world that needs to be shadowed

	glPushMatrix();
	{
		glLoadIdentity();
		_sceneTransform.Apply();
		glGetDoublev(GL_MODELVIEW_MATRIX,lastModelviewMatrix);
		DisplayOBJs();
	}
	glPopMatrix();

	// Reset the texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	// Turn the first multi-texture pass off
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);

	// Light expected, we need to turn our shader off since we are done
	g_Shader.TurnOff();
	*/
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
void QtViewer::PrintString(void* font, char* string)
{
	int	len, i;

	len = (int) strlen(string);
	for (i = 0; i < len; i++)
	{
		//glutBitmapCharacter(font, string[i]);
	}
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
void QtViewer::Display3DText(float x, float y, float z, char* string)
{
	char*	c;

	glPushMatrix();
	glTranslatef(x, y, z);
	for (c = string; *c != '\0'; c++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	}
	glPopMatrix();
}

// ---------------------------------------------------
// --- 
// --- 
// ---------------------------------------------------
void QtViewer::DrawAxis(double xpos, double ypos, double zpos, double arrowSize)
{
	/*
	float	fontScale	= (float) (arrowSize / 600.0);

	Enable<GL_DEPTH_TEST> depth;
	Enable<GL_LIGHTING> lighting;
	Enable<GL_COLOR_MATERIAL> colorMat;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);

	// --- Draw the "X" axis in red
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	glTranslated(xpos, ypos, zpos);
	glRotatef(90.0f, 0.0, 1.0, 0.0);
	gluCylinder(_tube, arrowSize / 50.0, arrowSize / 50.0, arrowSize, 10, 10);
	glTranslated(0.0, 0.0, arrowSize);
	gluCylinder(_arrow, arrowSize / 15.0, 0.0, arrowSize / 5.0, 10, 10);
	// ---- Display a "X" near the tip of the arrow
	glTranslated(-0.5 * fontScale * (double)
		glutStrokeWidth(GLUT_STROKE_ROMAN, 88),
		arrowSize / 15.0, arrowSize /
		5.0);
	glLineWidth(3.0);
	glScalef(fontScale, fontScale, fontScale);
	glutStrokeCharacter(GLUT_STROKE_ROMAN, 88);
	glScalef(1.0f / fontScale, 1.0f / fontScale, 1.0f / fontScale);
	glLineWidth(1.0f);
	// --- Undo transforms
	glTranslated(-xpos, -ypos, -zpos);
	glPopMatrix();

	// --- Draw the "Y" axis in green
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glTranslated(xpos, ypos, zpos);
	glRotatef(-90.0f, 1.0, 0.0, 0.0);
	gluCylinder(_tube, arrowSize / 50.0, arrowSize / 50.0, arrowSize, 10, 10);
	glTranslated(0.0, 0.0, arrowSize);
	gluCylinder(_arrow, arrowSize / 15.0, 0.0, arrowSize / 5.0, 10, 10);
	// ---- Display a "Y" near the tip of the arrow
	glTranslated(-0.5 * fontScale * (double)
		glutStrokeWidth(GLUT_STROKE_ROMAN, 89),
		arrowSize / 15.0, arrowSize /
		5.0);
	glLineWidth(3.0);
	glScalef(fontScale, fontScale, fontScale);
	glutStrokeCharacter(GLUT_STROKE_ROMAN, 89);
	glScalef(1.0f / fontScale, 1.0f / fontScale, 1.0f / fontScale);
	glLineWidth(1.0);
	// --- Undo transforms
	glTranslated(-xpos, -ypos, -zpos);
	glPopMatrix();

	// --- Draw the "Z" axis in blue
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	glTranslated(xpos, ypos, zpos);
	glRotatef(0.0f, 1.0, 0.0, 0.0);
	gluCylinder(_tube, arrowSize / 50.0, arrowSize / 50.0, arrowSize, 10, 10);
	glTranslated(0.0, 0.0, arrowSize);
	gluCylinder(_arrow, arrowSize / 15.0, 0.0, arrowSize / 5.0, 10, 10);
	// ---- Display a "Z" near the tip of the arrow
	glTranslated(-0.5 * fontScale * (double)
		glutStrokeWidth(GLUT_STROKE_ROMAN, 90),
		arrowSize / 15.0, arrowSize /
		5.0);
	glLineWidth(3.0);
	glScalef(fontScale, fontScale, fontScale);
	glutStrokeCharacter(GLUT_STROKE_ROMAN, 90);
	glScalef(1.0f / fontScale, 1.0f / fontScale, 1.0f / fontScale);
	glLineWidth(1.0);
	// --- Undo transforms
	glTranslated(-xpos, -ypos, -zpos);
	glPopMatrix();
	*/
}

// ---------------------------------------------------
// --- 
// --- 
// ---------------------------------------------------
void QtViewer::DrawBox(double* minBBox, double* maxBBox, double r)
{
	/*
    //std::cout << "box = < " << minBBox[0] << ' ' << minBBox[1] << ' ' << minBBox[2] << " >-< " << maxBBox[0] << ' ' << maxBBox[1] << ' ' << maxBBox[2] << " >"<< std::endl;
    if (r==0.0)
        r = (Vector3(maxBBox) - Vector3(minBBox)).norm() / 500;
#if 0
    {
        Enable<GL_DEPTH_TEST> depth;
        Disable<GL_LIGHTING> lighting;
        glColor3f(0.0, 1.0, 1.0);
        glBegin(GL_LINES);
            for (int corner=0; corner<4; ++corner)
            {
                glVertex3d(           minBBox[0]           ,
                           (corner&1)?minBBox[1]:maxBBox[1],
                           (corner&2)?minBBox[2]:maxBBox[2]);
                glVertex3d(           maxBBox[0]           ,
                           (corner&1)?minBBox[1]:maxBBox[1],
                           (corner&2)?minBBox[2]:maxBBox[2]);
            }
            for (int corner=0; corner<4; ++corner)
            {
                glVertex3d((corner&1)?minBBox[0]:maxBBox[0],
                                      minBBox[1]           ,
                           (corner&2)?minBBox[2]:maxBBox[2]);
                glVertex3d((corner&1)?minBBox[0]:maxBBox[0],
                                      maxBBox[1]           ,
                           (corner&2)?minBBox[2]:maxBBox[2]);
            }
        
            // --- Draw the Z edges
            for (int corner=0; corner<4; ++corner)
            {
                glVertex3d((corner&1)?minBBox[0]:maxBBox[0],
                           (corner&2)?minBBox[1]:maxBBox[1],
                                      minBBox[2]           );
                glVertex3d((corner&1)?minBBox[0]:maxBBox[0],
                           (corner&2)?minBBox[1]:maxBBox[1],
                                      maxBBox[2]           );
            }
        glEnd();
        return;
    }
#endif
    Enable<GL_DEPTH_TEST> depth;
    Enable<GL_LIGHTING> lighting;
    Enable<GL_COLOR_MATERIAL> colorMat;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);

    // --- Draw the corners
    glColor3f(0.0, 1.0, 1.0);
    for (int corner=0; corner<8; ++corner)
    {
        glPushMatrix();
        glTranslated((corner&1)?minBBox[0]:maxBBox[0],
                     (corner&2)?minBBox[1]:maxBBox[1],
                     (corner&4)?minBBox[2]:maxBBox[2]);
        gluSphere(_sphere,2*r,20,10);
        glPopMatrix();
    }

    glColor3f(1.0, 1.0, 0.0);
    // --- Draw the X edges
    for (int corner=0; corner<4; ++corner)
    {
        glPushMatrix();
        glTranslated(           minBBox[0]           ,
                     (corner&1)?minBBox[1]:maxBBox[1],
                     (corner&2)?minBBox[2]:maxBBox[2]);
        glRotatef(90,0,1,0);
        gluCylinder(_tube, r, r, maxBBox[0] - minBBox[0], 10, 10);
        glPopMatrix();
    }

    // --- Draw the Y edges
    for (int corner=0; corner<4; ++corner)
    {
        glPushMatrix();
        glTranslated((corner&1)?minBBox[0]:maxBBox[0],
                                minBBox[1]           ,
                     (corner&2)?minBBox[2]:maxBBox[2]);
        glRotatef(-90,1,0,0);
        gluCylinder(_tube, r, r, maxBBox[1] - minBBox[1], 10, 10);
        glPopMatrix();
    }

    // --- Draw the Z edges
    for (int corner=0; corner<4; ++corner)
    {
        glPushMatrix();
        glTranslated((corner&1)?minBBox[0]:maxBBox[0],
                     (corner&2)?minBBox[1]:maxBBox[1],
                                minBBox[2]           );
        gluCylinder(_tube, r, r, maxBBox[2] - minBBox[2], 10, 10);
        glPopMatrix();
    }
	*/
}


// ----------------------------------------------------------------------------------
// --- Draw a "plane" in wireframe. The "plane" is parallel to the XY axis
// --- of the main coordinate system 
// ----------------------------------------------------------------------------------
void QtViewer::DrawXYPlane(double zo, double xmin, double xmax, double ymin,
	double ymax, double step)
{
	register double x, y;

	//Enable<GL_DEPTH_TEST> depth;

	glBegin(GL_LINES);
	for (x = xmin; x <= xmax; x += step)
	{
		glVertex3d(x, ymin, zo);
		glVertex3d(x, ymax, zo);
	}
	glEnd();

	glBegin(GL_LINES);
	for (y = ymin; y <= ymax; y += step)
	{
		glVertex3d(xmin, y, zo);
		glVertex3d(xmax, y, zo);
	}
	glEnd();
}


// ----------------------------------------------------------------------------------
// --- Draw a "plane" in wireframe. The "plane" is parallel to the XY axis
// --- of the main coordinate system 
// ----------------------------------------------------------------------------------
void QtViewer::DrawYZPlane(double xo, double ymin, double ymax, double zmin,
	double zmax, double step)
{/*
	register double y, z;
	Enable<GL_DEPTH_TEST> depth;

	glBegin(GL_LINES);
	for (y = ymin; y <= ymax; y += step)
	{
		glVertex3d(xo, y, zmin);
		glVertex3d(xo, y, zmax);
	}
	glEnd();

	glBegin(GL_LINES);
	for (z = zmin; z <= zmax; z += step)
	{
		glVertex3d(xo, ymin, z);
		glVertex3d(xo, ymax, z);
	}
	glEnd();
*/
}


// ----------------------------------------------------------------------------------
// --- Draw a "plane" in wireframe. The "plane" is parallel to the XY axis
// --- of the main coordinate system 
// ----------------------------------------------------------------------------------
void QtViewer::DrawXZPlane(double yo, double xmin, double xmax, double zmin,
	double zmax, double step)
{/*
	register double x, z;
	Enable<GL_DEPTH_TEST> depth;

	glBegin(GL_LINES);
	for (x = xmin; x <= xmax; x += step)
	{
		glVertex3d(x, yo, zmin);
		glVertex3d(x, yo, zmax);
	}
	glEnd();

	glBegin(GL_LINES);
	for (z = zmin; z <= zmax; z += step)
	{
		glVertex3d(xmin, yo, z);
		glVertex3d(xmax, yo, z);
	}
	glEnd();
	*/
}

// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void QtViewer::DrawLogo()
{
	int w = 0;
	int h = 0; 
/*
	if (texLogo && texLogo->getImage()) {
		h = texLogo->getImage()->getHeight();
		w = texLogo->getImage()->getWidth();
	}
	else return;

	Enable <GL_TEXTURE_2D> tex;
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5, _W, -0.5, _H, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (texLogo)
		texLogo->bind();

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex3d((_W-w)/2, (_H-h)/2, 0.0);

	glTexCoord2d(1.0, 0.0);
	glVertex3d( _W-(_W-w)/2, (_H-h)/2, 0.0);

	glTexCoord2d(1.0, 1.0);
	glVertex3d( _W-(_W-w)/2, _H-(_H-h)/2, 0.0);

	glTexCoord2d(0.0, 1.0);
	glVertex3d((_W-w)/2, _H-(_H-h)/2, 0.0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);	

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	*/
}

// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void QtViewer::DisplayOBJs(bool shadowPass)
{
    /*
    //Enable<GL_LIGHTING> light;
    //Enable<GL_DEPTH_TEST> depth;
    
    glShadeModel(GL_SMOOTH);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor4f(1,1,1,1);
    glDisable(GL_COLOR_MATERIAL);
    
    if (!initTexturesDone)
    {
        std::cout << "-----------------------------------> initTexturesDone\n";
        //---------------------------------------------------
        //Simulation::initTextures(groot);
        //---------------------------------------------------
        initTexturesDone = true;
    }
    
    if (!groot->getMultiThreadSimulation())
    {
        if (shadowPass)
            Simulation::drawShadows(groot);
        else
            Simulation::draw(groot);
        if (_axis)
        {
            DrawAxis(0.0, 0.0, 0.0, 10.0);
            if (sceneMinBBox[0] < sceneMaxBBox[0])
                DrawBox(sceneMinBBox.ptr(), sceneMaxBBox.ptr());
        }
    }
    else
        automateDisplayVM();
*/
    // glDisable(GL_COLOR_MATERIAL);
}

// -------------------------------------------------------
// ---
// -------------------------------------------------------
void QtViewer::DisplayMenu(void)
{
	/*
	Disable<GL_LIGHTING> light;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5, _W, -0.5, _H, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.3f, 0.7f, 0.95f);
	glRasterPos2i(_W / 2 - 5, _H - 15);
	//sprintf(buffer,"FPS: %.1f\n", _frameRate.GetFPS());
	//PrintString(GLUT_BITMAP_HELVETICA_12, buffer);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	*/
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
void QtViewer::DrawScene(void)
{	
/*
	_newQuat.buildRotationMatrix(_sceneTransform.rotation);
	calcProjection();

#if 1
	if (_shadow)
	{
		//glGetDoublev(GL_MODELVIEW_MATRIX,lastModelviewMatrix);

		// Update the light matrices for it's current position
		StoreLightMatrices();

		// Set the current viewport to our texture size
		glViewport(0, 0, (int)SHADOW_WIDTH, (int)SHADOW_HEIGHT);

		// Clear the screen and depth buffer so we can render from the light's view
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Now we just need to set the matrices for the light before we render
		glMatrixMode(GL_PROJECTION);

		// Push on a matrix to make sure we can restore to the old matrix easily
		glPushMatrix();{
			// Set the current projection matrix to our light's projection matrix
			glLoadMatrixf(g_mProjection);

			// Load modelview mode to set our light's modelview matrix
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();{
				// Load the light's modelview matrix before we render to a texture
				glLoadMatrixf(g_mModelView);

				// Since we don't care about color when rendering the depth values to
				// the shadow-map texture, we disable color writing to increase speed.
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 

				// This turns of the polygon offset functionality to fix artifacts.
				// Comment this out and run the program to see what artifacts I mean.
				glEnable(GL_POLYGON_OFFSET_FILL);
				glDisable(GL_BLEND);

				// Eliminate artifacts caused by shadow mapping
				//	glPolygonOffset(1.0f, 0.10f);
				glPolygonOffset(g_DepthOffset[0], g_DepthOffset[1]);

				_sceneTransform.Apply();
				// Render the world according to the light's view
				DisplayOBJs(true);

				// Now that the world is rendered, save the depth values to a texture
				glDisable(GL_BLEND);
				//glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, g_DepthTexture);

				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, (int)SHADOW_WIDTH, (int)SHADOW_HEIGHT);

				// We can turn color writing back on since we already stored the depth values
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 

				// Turn off polygon offsetting
				glDisable(GL_POLYGON_OFFSET_FILL);

			}glPopMatrix();
			// Go back to the projection mode and restore the original matrix
			glMatrixMode(GL_PROJECTION);
			// Restore the original projection matrix
		}glPopMatrix();

		// Go back to modelview model to start drawing like normal
		glMatrixMode(GL_MODELVIEW);

		// Restore our normal viewport size to our screen width and height
		glViewport(0, 0, GetWidth(), GetHeight());

		// Clear the color and depth bits and start over from the camera's view
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glPushMatrix();{
		//	glLoadIdentity();
		//	_sceneTransform.ApplyInverse();
		//	glGetFloatv(GL_MODELVIEW_MATRIX, g_mCameraInverse);
		//}glPopMatrix();

		glLightfv( GL_LIGHT0, GL_POSITION, _lightPosition );


		// Render the world and apply the shadow map texture to it
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(lastProjectionMatrix);
		glMatrixMode(GL_MODELVIEW);
		ApplyShadowMap();
		{ // NICO
			Enable<GL_TEXTURE_2D> texture_on;
				glDisable(GL_BLEND);
			glBindTexture(GL_TEXTURE_2D, ShadowTextureMask);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GetWidth(), GetHeight());
		}
		if (_background==0)
			glClearColor(0.0589f, 0.0589f, 0.0589f, 1.0f);
		else if (_background==1)
			glClearColor(0.0f,0.0f,0.0f,0.0f);
		else if (_background==2)
			glClearColor(1.0f,1.0f,1.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (_background==0)
			DrawLogo();
		glPushMatrix();
		_sceneTransform.Apply();
		DisplayOBJs();
		glPopMatrix();

		{
			float ofu = GetWidth()/(float)SHADOW_MASK_SIZE;
			float ofv = GetHeight()/(float)SHADOW_MASK_SIZE;
			//glActiveTextureARB(GL_TEXTURE0_ARB);
			//glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,ShadowTextureMask);
			//glTexEnvi(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,  GL_REPLACE);
			Disable<GL_DEPTH_TEST> dtoff;
			Disable<GL_LIGHTING> dlight;
			Enable<GL_BLEND> blend_on;
			glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
			glColor3f(1,1,1);
			glViewport(0, 0, GetWidth(), GetHeight());
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0,1,0,1,-1,1); 
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();{
				glLoadIdentity();
				glBegin(GL_QUADS);{
					glTexCoord2f(0,0);
					glVertex2f(0,0);
					glTexCoord2f(0,ofv);
					glVertex2f(0,1);
					glTexCoord2f(ofu,ofv);
					glVertex2f(1,1);
					glTexCoord2f(ofu,0);
					glVertex2f(1,0);
				}glEnd();
			}glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glViewport(0, 0, GetWidth(), GetHeight());
			glDisable(GL_TEXTURE_2D);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	
	}
	else
#endif
	{
		if (_background==0)
			DrawLogo();

		glLoadIdentity();
		_sceneTransform.Apply();

		glGetDoublev(GL_MODELVIEW_MATRIX,lastModelviewMatrix);

		if (_renderingMode == GL_RENDER)
		{
		// Initialize lighting
		glPushMatrix();
		glLoadIdentity();
		glLightfv(GL_LIGHT0, GL_POSITION, _lightPosition);
		glPopMatrix();
		//Enable<GL_LIGHT0> light0;

		glColor3f(0.5f, 0.5f, 0.6f);
		//	DrawXZPlane(-4.0, -20.0, 20.0, -20.0, 20.0, 1.0);
		//	DrawAxis(0.0, 0.0, 0.0, 10.0);

		DisplayOBJs();

		DisplayMenu();		// always needs to be the last object being drawn
		}
	}
	*/
}


void QtViewer::DrawAutomate(void)
{
/*
	std::cout << "DrawAutomate\n";
	_newQuat.buildRotationMatrix(_sceneTransform.rotation);

	glLoadIdentity();
	_sceneTransform.Apply();
*/

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5, 12.5, -10, 10, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
/*
	for(int i = 0; i < (int) Automate::getInstance()->tabNodes.size(); i++)
	{
		for(int j = 0; j < (int) Automate::getInstance()->tabNodes[i]->tabOutputs.size(); j++)
		{
			Automate::getInstance()->tabNodes[i]->tabOutputs[j]->draw();
		}

		Automate::getInstance()->tabNodes[i]->draw();
	}
*/
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	update();
}

// ---------------------------------------------------------
// --- Reshape of the window, reset the projection
// ---------------------------------------------------------
void QtViewer::calcProjection()
{/*
    int width = _W;
    int height = _H;
    double xNear, yNear, zNear, zFar, xOrtho, yOrtho;
    double xFactor = 1.0, yFactor = 1.0;
    double offset;
    double xForeground, yForeground, zForeground, xBackground, yBackground,
           zBackground;

    //if (!sceneBBoxIsValid)
    {
        Simulation::computeBBox(groot, sceneMinBBox.ptr(), sceneMaxBBox.ptr());
        sceneBBoxIsValid = true;
    }
    //std::cout << "Scene BBox = "<<sceneMinBBox<<" - "<<sceneMaxBBox<<"\n";
    if (sceneMinBBox[0] > sceneMaxBBox[0])
    {
        zNear = 1.0;
        zFar = 1000.0;
    }
    else
    {
        zNear = 1e10;
        zFar = -1e10;
        double minBBox[3] = {sceneMinBBox[0], sceneMinBBox[1], sceneMinBBox[2] };
        double maxBBox[3] = {sceneMaxBBox[0], sceneMaxBBox[1], sceneMaxBBox[2] };
        if (_axis)
        {
            for (int i=0;i<3;i++)
            {
                if (minBBox[i]>-2) minBBox[i] = -2;
                if (maxBBox[i]<14) maxBBox[i] = 14;
            }
        }

        for (int corner=0;corner<8;++corner)
        {
            Vector3 p((corner&1)?minBBox[0]:maxBBox[0],
                      (corner&2)?minBBox[1]:maxBBox[1],
                      (corner&4)?minBBox[2]:maxBBox[2]);
            p = _sceneTransform * p;
            double z = -p[2];
            if (z < zNear) zNear = z;
            if (z > zFar) zFar = z;
        }
        if (zFar <= 0 || zFar >= 1000)
        {
            zNear = 1;
            zFar = 1000.0;
        }
        else
        {
            zNear *= 0.9; // add some margin
            zFar *= 1.1;
            if (zNear < zFar*0.01)
                    zNear = zFar*0.01;
            if (zNear < 1.0) zNear = 1.0;
            if (zFar < 2.0) zFar = 2.0;
        }
        //std::cout << "Z = ["<<zNear<<" - "<<zFar<<"]\n";
    }
    xNear = 0.35*zNear;
    yNear = 0.35*zNear;
    offset = 0.001*zNear;		// for foreground and background planes

    xOrtho = fabs(_sceneTransform.translation[2]) * xNear / zNear;
    yOrtho = fabs(_sceneTransform.translation[2]) * yNear / zNear;

    if ((height != 0) && (width != 0))
    {
        if (height > width)
        {
            xFactor = 1.0;
            yFactor = (double) height / (double) width;
        }
        else
        {
            xFactor = (double) width / (double) height;
            yFactor = 1.0;
        }
    }

    lastViewport[0] = 0;
    lastViewport[1] = 0;
    lastViewport[2] = width;
    lastViewport[3] = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    zForeground = -zNear - offset;
    zBackground = -zFar + offset;

    glFrustum(-xNear * xFactor, xNear * xFactor, -yNear * yFactor,
               yNear * yFactor, zNear, zFar);
    xForeground = -zForeground * xNear / zNear;
    yForeground = -zForeground * yNear / zNear;
    xBackground = -zBackground * xNear / zNear;
    yBackground = -zBackground * yNear / zNear;

    xForeground *= xFactor;
    yForeground *= yFactor;
    xBackground *= xFactor;
    yBackground *= yFactor;

	glGetDoublev(GL_PROJECTION_MATRIX,lastProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	*/
}

void QtViewer::eventNewTime()
{
	/*
    double time = groot->getTime();
    //cerr<<"QtViewer::eventNewTime()"<<endl;
    emit newTime(time);
    char buf[100];
    sprintf(buf, "T: %.3f s", time);
    emit newTime(buf);
	*/
}

void QtViewer::eventNewStep()
{
	/*
	static ctime_t beginTime[10];
	static const ctime_t timeTicks = CTime::getRefTicksPerSec();
	static int frameCounter = 0;
	if (frameCounter==0)
	{
		ctime_t t = CTime::getRefTime();
		for (int i=0;i<10;i++)
			beginTime[i] = t;
	}
	++frameCounter;
	if ((frameCounter%10) == 0)
	{
		ctime_t curtime = CTime::getRefTime();
		int i = ((frameCounter/10)%10);
		double fps = ((double)timeTicks / (curtime - beginTime[i]))*(frameCounter<100?frameCounter:100);
		emit newFPS(fps);
		char buf[100];
		sprintf(buf, "%.1f FPS", fps);
		//GUI->fpsLabel->value(buf);
		emit newFPS(buf);
		beginTime[i] = curtime;
		//frameCounter = 0;
	}
	if (m_displayComputationTime && (frameCounter%100) == 0 && groot!=NULL)
	{
		std::cout << "========== ITERATION " << frameCounter << " ==========\n";
		const simulation::tree::GNode::NodeTimer& total = groot->getTotalTime();
		const std::map<std::string, simulation::tree::GNode::NodeTimer>& times = groot->getActionTime();
		const std::map<std::string, std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer> >& objtimes = groot->getObjectTime();
		const double fact = 1000000.0 / (100*groot->getTimeFreq());
		for (std::map<std::string, simulation::tree::GNode::NodeTimer>::const_iterator it = times.begin(); it != times.end(); ++it)
		{
			std::cout << "TIME "<<it->first<<": " << ((int)(fact*it->second.tTree+0.5))*0.001 << " ms (" << (1000*it->second.tTree/total.tTree)*0.1 << " %).\n";
			//std::map<std::string, std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer> >::const_iterator it1 = objtimes.find(it->first);
			if (it1 != objtimes.end())
			{
				for (std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
				{
					std::cout << "  "<< sofa::helper::gettypename(typeid(*(it2->first)))<<" "<< it2->first->getName() <<": "
						<< ((int)(fact*it2->second.tObject+0.5))*0.001 << " ms (" << (1000*it2->second.tObject/it->second.tTree)*0.1 << " %).\n";
				}
			}
		}
		for (std::map<std::string, std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer> >::const_iterator it = objtimes.begin(); it != objtimes.end(); ++it)
		{
			if (times.count(it->first)>0) continue;
			ctime_t ttotal = 0;
			for (std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				ttotal += it2->second.tObject;
			std::cout << "TIME "<<it->first<<": " << ((int)(fact*ttotal+0.5))*0.001 << " ms (" << (1000*ttotal/total.tTree)*0.1 << " %).\n";
			if (ttotal > 0)
				for (std::map<sofa::core::objectmodel::BaseObject*, simulation::tree::GNode::ObjectTimer>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
				{
					std::cout << "  "<< sofa::helper::gettypename(typeid(*(it2->first)))<<" "<< it2->first->getName() <<": "
						<< ((int)(fact*it2->second.tObject+0.5))*0.001 << " ms (" << (1000*it2->second.tObject/ttotal)*0.1 << " %).\n";
				}
		}
		std::cout << "TOTAL TIME: " << ((int)(fact*total.tTree+0.5))*0.001 << " ms (" << ((int)(100/(fact*total.tTree*0.000001)+0.5))*0.01 << " FPS).\n";
		groot->resetTime();
	}
	*/
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
/*
void QtViewer::animate(void)
{
	if (_spinning)
	{
		_newQuat = _currentQuat + _newQuat;
	}

	// update the entire scene
	update();
}
*/

static bool output_snapshot=false;
void QtViewer::animate(void)
{
	static int counter = 0;
	if (output_snapshot){
		setSnapshotCounter(counter);
		//char sbuff[300];
		//sprintf(sbuff, "out%04d.png", counter);
		setSnapshotQuality(100);
		saveSnapshot(true, true);
		counter++;
	}
}

// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
void QtViewer::ApplySceneTransformation(int x, int y)
{
/*
	float	x1, x2, y1, y2;
	float	xshift, yshift, zshift;
	if (_moving)
	{
		if (_navigationMode == TRACKBALL_MODE)
		{
			x1 = (2.0f * _W / 2.0f - _W) / _W;
			y1 = (_H - 2.0f * _H / 2.0f) / _H;
			x2 = (2.0f * (x + (-_mouseX + _W / 2.0f)) - _W) / _W;
			y2 = (_H - 2.0f * (y + (-_mouseY + _H / 2.0f))) / _H;
			_currentTrackball.ComputeQuaternion(x1, y1, x2, y2);
			_currentQuat = _currentTrackball.GetQuaternion();
			_savedMouseX = _mouseX;
			_savedMouseY = _mouseY;
			_mouseX = x;
			_mouseY = y;
			_newQuat = _currentQuat + _newQuat;
			update();
		}
		else if (_navigationMode == ZOOM_MODE)
		{
			zshift = (2.0f * y - _W) / _W - (2.0f * _mouseY - _W) / _W;
			_sceneTransform.translation[2] = _previousEyePos[2] -
				_zoomSpeed * zshift;
			update();
		}
		else if (_navigationMode == PAN_MODE)
		{
			xshift = (2.0f * x - _W) / _W - (2.0f * _mouseX - _W) / _W;
			yshift = (2.0f * y - _W) / _W - (2.0f * _mouseY - _W) / _W;
			_sceneTransform.translation[0] = _previousEyePos[0] +
				_panSpeed * xshift;
			_sceneTransform.translation[1] = _previousEyePos[1] -
				_panSpeed * yshift;
			update();
		}
	}
	*/
}


// ---------------------------------------------------------
// ---
// ---------------------------------------------------------
void QtViewer::ApplyMouseInteractorTransformation(int x, int y)
{/*
	// Mouse Interaction
	double coeffDeplacement = 0.025;
	Quaternion conjQuat, resQuat, _newQuatBckUp;

	float x1, x2, y1, y2;

	if (_mouseInteractorMoving)
	{
		if (_mouseInteractorRotationMode)
		{
			if ((_mouseInteractorSavedPosX != x) || (_mouseInteractorSavedPosY != y))
			{
				x1 = 0;
				y1 = 0;
				x2 = (2.0f * (x + (-_mouseInteractorSavedPosX + _W / 2.0f)) - _W) / _W;
				y2 = (_H - 2.0f * (y + (-_mouseInteractorSavedPosY + _H / 2.0f))) / _H;

				_mouseInteractorTrackball.ComputeQuaternion(x1, y1, x2, y2);
				_mouseInteractorCurrentQuat = _mouseInteractorTrackball.GetQuaternion();
				_mouseInteractorSavedPosX = x;
				_mouseInteractorSavedPosY = y;

				_mouseInteractorNewQuat = _mouseInteractorCurrentQuat + _mouseInteractorNewQuat;
				_mouseRotate = true;
			}
			else
			{
				_mouseRotate = false;
			}

			update();
		}
		else if (_mouseInteractorTranslationMode)
		{
			_mouseInteractorAbsolutePosition =  Vector3(0,0,0);
			_mouseInteractorRelativePosition =  Vector3(0,0,0);

			if (_translationMode == XY_TRANSLATION)
			{
				_mouseInteractorAbsolutePosition[0] = coeffDeplacement * (x - _mouseInteractorSavedPosX);
				_mouseInteractorAbsolutePosition[1] = -coeffDeplacement * (y - _mouseInteractorSavedPosY);

				_mouseInteractorSavedPosX = x;
				_mouseInteractorSavedPosY = y;
			}
			else if (_translationMode == Z_TRANSLATION)
			{
				_mouseInteractorAbsolutePosition[2] = coeffDeplacement * (y - _mouseInteractorSavedPosY);

				_mouseInteractorSavedPosX = x;
				_mouseInteractorSavedPosY = y;
			}

			_newQuatBckUp[0] = _newQuat[0];
			_newQuatBckUp[1] = _newQuat[1];
			_newQuatBckUp[2] = _newQuat[2];
			_newQuatBckUp[3] = _newQuat[3];

			_newQuatBckUp.normalize();

			// Conjugate calculation of the scene orientation quaternion
			conjQuat[0] = -_newQuatBckUp[0];
			conjQuat[1] = -_newQuatBckUp[1];
			conjQuat[2] = -_newQuatBckUp[2];
			conjQuat[3] = _newQuatBckUp[3];

			conjQuat.normalize();

			resQuat = _newQuatBckUp.quatVectMult(_mouseInteractorAbsolutePosition) * conjQuat;

			_mouseInteractorRelativePosition[0] = resQuat[0];
			_mouseInteractorRelativePosition[1] = resQuat[1];
			_mouseInteractorRelativePosition[2] = resQuat[2];

			_mouseTrans = true;
			update();
		}
	}
	*/
}


// ----------------------------------------
// --- Handle events (mouse, keyboard, ...)
// ----------------------------------------

bool QtViewer::isControlPressed() const {
    return m_isControlPressed;
}

void updateGrasperAngle(CSceneGraph *pLapTool, const int dir)
{
	if (pLapTool==NULL) return;
	CSceneNode *pnode = pLapTool->GetActiveSceneNode();
	if (pnode==NULL) return;
	CTexturedTriangleObj *pobj = dynamic_cast<CTexturedTriangleObj *>(pnode->m_pObject);
	if (pobj==NULL) return;
	pobj->updateToolHeadAngle(dir);
}

void QtViewer::keyPressEvent ( QKeyEvent * e )
{
	switch(e->key()){
	case Qt::Key_1:
		updateGrasperAngle(m_pLaptoolSceneGraph, 1);
		break;	
	case Qt::Key_2:
		updateGrasperAngle(m_pLaptoolSceneGraph, -1);
		break;	
	case Qt::Key_V:
		{
			if (output_snapshot){
				output_snapshot = false;
				printf("Snapshot output is disabled!\n");
			}
			else{
				output_snapshot = true;
				printf("Snapshot output is enabled!\n");
			}
		}
		break;	
	case Qt::Key_P:
		{
			static int count=0;
			count++;
			qglviewer::Camera *p = camera();
			if ((count&0x1)==1){
				p->setType((qglviewer::Camera::Type)(qglviewer::Camera::ORTHOGRAPHIC)); //should be ORTHOGRAPHIC, here for compatible issue;
			}
			else
				p->setType((qglviewer::Camera::Type)(qglviewer::Camera::PERSPECTIVE)); //should be ORTHOGRAPHIC, here for compatible issue;
		}
		break;
	default:
		QGLViewer::keyPressEvent(e);
		break;
	}
	/*
    if( isControlPressed() ) // pass event to the scene data structure
    {
        //cerr<<"QtViewer::keyPressEvent, key = "<<e->key()<<" with Control pressed "<<endl;
        sofa::core::objectmodel::KeypressedEvent keyEvent(e->key());
        groot->propagateEvent(&keyEvent);
    }
    else  // control the GUI
        switch(e->key())
	{

	case Qt::Key_S:
		// --- save screenshot
		{
			screenshot();
			break;
		}
	case Qt::Key_V:
		// --- save video
		{
			_video = !_video;
			capture.setCounter();
			break;
		}
	case Qt::Key_W:
		// --- save current view
		{
			saveView();
			break;
		}

	case Qt::Key_O:
		// --- export to OBJ
		{
			exportOBJ();
			break;
		}
	case Qt::Key_P:
		// --- export to a succession of OBJ to make a video
		{
			_animationOBJ = !_animationOBJ;
			_animationOBJcounter = 0;
			break;
		}
	case Qt::Key_R:
		// --- draw axis
		{
			_axis = !_axis;
			update();
			break;
		}
	case Qt::Key_B:
		// --- change background
		{
			_background = (_background+1)%3;
			update();
			break;
		}

	case Qt::Key_L:
		// --- draw shadows
		{
			if (_glshadow)
			{
				_shadow = !_shadow;
				update();
			}else
			{
				printf("WARNING QtViewer : shadows are not supported !\n");
				_shadow=false;
			}
			break;
		}

	case Qt::Key_A:
		// --- switch automate display mode
		{
			bool multi = false;
			if (groot)
				multi = groot->getContext()->getMultiThreadSimulation();
			//else
			//	multi = Scene::getInstance()->getMultiThreadSimulation();
			if (multi)
			{
				if (!_automateDisplayed)
				{
					_automateDisplayed = true;
					//Fl::add_idle(displayAutomateCB);
					SwitchToAutomateView();
					sofa::helper::gl::glfntInit();
				}
				else
				{
					_automateDisplayed = false;
					//Fl::remove_idle(displayAutomateCB);
					SwitchToPresetView();
					sofa::helper::gl::glfntClose();
				}
			}

			update();
			break;
		}

	case Qt::Key_Escape:
		{
			exit(0);
			break;
		}

	case Qt::Key_C:
		{
			// --- switch interaction mode
			if (!_mouseInteractorTranslationMode)
			{
				std::cout << "Interaction Mode ON\n";
				_mouseInteractorTranslationMode = true;
				_mouseInteractorRotationMode = false;
			}
			else
			{
				std::cout << "Interaction Mode OFF\n";
				_mouseInteractorTranslationMode = false;
				_mouseInteractorRotationMode = false;
			}	
			break;
		}
	case Qt::Key_F5:
		{
			emit reload();
			//if (!sceneFileName.empty())
			//{
			//	std::cout << "Reloading "<<sceneFileName<<std::endl;
			//	std::string filename = sceneFileName;
			//	Quaternion q = _newQuat;
			//	Transformation t = _sceneTransform;
			//	simulation::tree::GNode* newroot = Simulation::load(filename.c_str());
			//	if (newroot == NULL)
			//	{
			//		std::cout << "Failed to load "<<filename<<std::endl;
			//		std::string s = "Failed to load ";
			//		s += filename;
			//		qFatal(s.c_str());
			//		break;
			//	}
			//	setScene(newroot, filename.c_str());
			//	_newQuat = q;
			//	_sceneTransform = t;
			//}

			break;
		}
            case Qt::Key_Control:
            {
                m_isControlPressed = true;
                //cerr<<"QtViewer::keyPressEvent, CONTROL pressed"<<endl;
		break;
            }
            default:
            {
                e->ignore();
            }
	}
	*/
	/*
	if (Fl::get_key(FL_Control_L) || Fl::get_key(FL_Control_R))
	{
	if ((_mouseInteractorTranslationMode) && (!_mouseInteractorRotationMode))
	{
	_mouseInteractorRotationMode = true;
	}
	}
	else
	{
	_mouseInteractorRotationMode = false;
	}
	*/
//	 QGLViewer::keyPressEvent(e);
}


void QtViewer::keyReleaseEvent(QKeyEvent * e)
{
    //cerr<<"QtViewer::keyReleaseEvent, key = "<<e->key()<<endl;
    switch(e->key())
    {
        case Qt::Key_Control:
        {
            m_isControlPressed = false;
            //cerr<<"QtViewer::keyPressEvent, CONTROL released"<<endl;
        }
        default:
        {
            e->ignore();
        }
    }
}
/*
void QtViewer::mousePressEvent ( QMouseEvent * e )
{
	mouseEvent(e);
}

void QtViewer::mouseReleaseEvent ( QMouseEvent * e )
{
	mouseEvent(e);
}

void QtViewer::mouseMoveEvent ( QMouseEvent * e )
{
	mouseEvent(e);
}

void QtViewer::mouseEvent ( QMouseEvent * e )
{
}
*/
// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void QtViewer::SwitchToPresetView(){}


// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void QtViewer::SwitchToAutomateView()
{/*
	_sceneTransform.translation[0] = -10.0;
	_sceneTransform.translation[1] = 0.0;
	_sceneTransform.translation[2] = -50.0;
	_newQuat[0] = 0.0;
	_newQuat[1] = 0.0;
	_newQuat[2] = 0.0;
	_newQuat[3] = 0.0;
	*/
}

void QtViewer::step()
{
	/*
    if (groot->getContext()->getMultiThreadSimulation())
    {
        static Node* n = NULL;

        if(ExecBus::getInstance() != NULL)
        {
            n = ExecBus::getInstance()->getNext("displayThread", n);

            if (n)
            {
                n->execute("displayThread");
            }
        }
    }
    else
    {
        if (_waitForRender) return;
        //groot->setLogTime(true);
        //Simulation::animate(groot);
        
        if( m_dumpState )
          Simulation::dumpState( groot, *m_dumpStateStream );
        if( m_exportGnuplot )
          Simulation::exportGnuplot( groot, groot->getTime() );
        

        _waitForRender = true;
        eventNewStep();
        eventNewTime();

        update();
    }
	
    if (_animationOBJ)
    {
#ifdef CAPTURE_PERIOD
        static int counter = 0;
        if ((counter++ % CAPTURE_PERIOD)==0)
#endif
        {
            exportOBJ(false);
            ++_animationOBJcounter;
        }
    }
	*/
}

void QtViewer::dumpState(bool value)
{
	/*
    m_dumpState = value;
    if( m_dumpState )
    {
        m_dumpStateStream = new std::ofstream("dumpState.data");
    }
    else if( m_dumpStateStream!=NULL )
    {
        delete m_dumpStateStream;
        m_dumpStateStream = 0;
    }*/
}

void QtViewer::displayComputationTime(bool value)
{
	/*
    m_displayComputationTime = value;
    if (groot)
    {
        groot->setLogTime(m_displayComputationTime);
    }
	*/
}

void QtViewer::setDt(double value)
{
	/*
	if (value > 0.0)
	{
		if (groot)
			groot->getContext()->setDt(value);
	}
	*/
}

void QtViewer::setDt(const QString& value)
{
	//setDt(value.toDouble());
}

void QtViewer::resetScene()
{
	/*
	if (groot)
	{
		Simulation::reset(groot);
		eventNewTime();
		update();
	}
	*/
}

void QtViewer::resetView()
{
	SwitchToPresetView();
	update();
}

void QtViewer::saveView()
{
	/*
	if (!sceneFileName.empty())
	{
		std::string viewFileName = sceneFileName+".view";
		std::ofstream out(viewFileName.c_str());
		if (!out.fail())
		{
			out << _sceneTransform.translation[0] << " " << _sceneTransform.translation[1] << " " << _sceneTransform.translation[2] << "\n";
			out << _newQuat[0] << " " << _newQuat[1] << " " << _newQuat[2] << " " << _newQuat[3] << "\n";
			out.close();
		}
		std::cout << "View parameters saved in "<<viewFileName<<std::endl;
	}
	*/
}

void QtViewer::showVisual(bool value)
{/*
	if (groot)
	{
		groot->getContext()->setShowVisualModels(value);
		Simulation::updateContext(groot);
	} */
	update();
}

void QtViewer::showBehavior(bool value)
{
	/*
	if (groot)
	{
		groot->getContext()->setShowBehaviorModels(value);
		Simulation::updateContext(groot);
	}*/
	update();
}

void QtViewer::showCollision(bool value)
{
	/*
	if (groot)
	{
		groot->getContext()->setShowCollisionModels(value);
		Simulation::updateContext(groot);
	}
	*/
	update();
}

void QtViewer::showBoundingCollision(bool value)
{
	/*
	if (groot)
	{
		groot->getContext()->setShowBoundingCollisionModels(value);
		Simulation::updateContext(groot);
	} */
	update();
}

void QtViewer::showMapping(bool value)
{/*
	if (groot)
	{
		groot->getContext()->setShowMappings(value);
		Simulation::updateContext(groot);
	}*/
	update();
}

void QtViewer::showMechanicalMapping(bool value)
{/*
	if (groot)
	{
		groot->getContext()->setShowMechanicalMappings(value);
		Simulation::updateContext(groot);
	} */
	update();
}

void QtViewer::showForceField(bool value)
{/*
	if (groot)
	{
		groot->getContext()->setShowForceFields(value);
		Simulation::updateContext(groot);
	} */
	update();
}

void QtViewer::showInteractionForceField(bool value)
{ /*
	if (groot)
	{
		groot->getContext()->setShowInteractionForceFields(value);
		Simulation::updateContext(groot);
	} */
	update();
}


static void setWireFrame(CSceneGraph &sg, const bool value)
{
	const int nsize = sg.SceneNodeCount();
	CGLDrawParms::glDrawMeshStyle drawtype;
	if (value)
		drawtype = CGLDrawParms::DRAW_MESH_HIDDENLINE;
	else
		drawtype  = CGLDrawParms::DRAW_MESH_SHADING;

	//draw all the objects;
	for (int i=0; i<nsize; i++){
		CSceneNode *node = sg.GetSceneNode(i);
		CGLDrawParms &drawparms = node->m_DrawParms;
		drawparms.m_nDrawType = drawtype;
	}
}

static void setTexture(CSceneGraph &sg, const bool value)
{
	const int nsize = sg.SceneNodeCount();
	//draw all the objects;
	for (int i=0; i<nsize; i++){
		CSceneNode *node = sg.GetSceneNode(i);
		CGLDrawParms &drawparms = node->m_DrawParms;
		drawparms.m_bEnableTexture2D = value;
	}
}

void QtViewer::showWireFrame(bool value)
{
	if (m_pDrawParms!=NULL){
		setWireFrame(*m_pSceneGraph, value);
		setWireFrame(*m_pBgSceneGraph, value);
		setWireFrame(*m_pLaptoolSceneGraph, value);
		update();
		printf("Wire frame value is %d\n", value);
	}
}

void QtViewer::showNormals(bool value)
{
	if (m_pDrawParms!=NULL){
		setTexture(*m_pSceneGraph, value);
		setTexture(*m_pBgSceneGraph, value);
		setTexture(*m_pLaptoolSceneGraph, value);
		update();
		printf("Texture value is %d\n", value);
	}
}

void QtViewer::screenshot()
{
	//capture.saveScreen();
}

void QtViewer::exportOBJ(bool exportMTL)
{/*
	if (!groot) return;
	std::ostringstream ofilename;
	if (!sceneFileName.empty())
	{
		const char* begin = sceneFileName.c_str();
		const char* end = strrchr(begin,'.');
		if (!end) end = begin + sceneFileName.length();
		ofilename << std::string(begin, end);
	}
	else
		ofilename << "scene";
// 	double time = groot->getTime();
// 	ofilename << '-' << (int)(time*1000);
	
	std::stringstream oss;
	oss.width(5);
	oss.fill('0');
	oss << _animationOBJcounter;
	
	ofilename << '_' << (oss.str().c_str());
	ofilename << ".obj";
	std::string filename = ofilename.str();
	std::cout << "Exporting OBJ Scene "<<filename<<std::endl;
	Simulation::exportOBJ(groot, filename.c_str(),exportMTL);
	*/
}

void QtViewer::setScene(GNode* scene, const char* filename)
{/*
	std::ostringstream ofilename;

	sceneFileName = (filename==NULL)?"":filename;
	if (!sceneFileName.empty())
	{
		const char* begin = sceneFileName.c_str();
		const char* end = strrchr(begin,'.');
		if (!end) end = begin + sceneFileName.length();
		ofilename << std::string(begin, end);
		ofilename << "_";
	}
	else
		ofilename << "scene_";

	capture.setPrefix(ofilename.str());
	if (scene != groot)
	{
		//SwitchToPresetView();
		if (interactor != NULL)
			interactor = NULL;
	}
	groot = scene;
	initTexturesDone = false;
	sceneBBoxIsValid = false;
	eventNewTime();
	*/
	update();
}

void QtViewer::exportGraph()
{
	exportGraph(getScene());
}

void QtViewer::exportGraph(GNode* root)
{
	/*
	if (root == NULL) return;
	sofa::gui::qt::GenGraphForm* form = new sofa::gui::qt::GenGraphForm;
	form->setScene(root);
	form->show();
	*/
}

/// Render Scene called during multiThread simulation using automate
void QtViewer::drawFromAutomate()
{
	update();
}

void QtViewer::automateDisplayVM(void)
{
}

void QtViewer::setExportGnuplot( bool exp )
{
	/*
  m_exportGnuplot = exp;
  if( m_exportGnuplot ){
    Simulation::initGnuplot( groot );
    Simulation::exportGnuplot( groot, groot->getTime() );
  } */
}

void QtViewer::setSizeW( int size )
{
	resizeGL( size, _H );
	updateGL();
}

void QtViewer::setSizeH( int size )
{
	resizeGL( _W, size );
	updateGL();
}
/*
void QtViewer::glDrawBackgroundImage(void)
{
	static QImage bgimage("c:/users/nanzhang/include/images/bwbackground.png");
	QImage *m_pBGImage = &bgimage;
	if (m_pBGImage->isNull())
		return;
	int depth = m_pBGImage->depth();
	if (depth!=32) return;

	setBackgroundColor(QColor(0,0,0));
	uchar * pix = m_pBGImage->bits();
	int imagew = m_pBGImage->width();
	int imageh = m_pBGImage->height();
	int winw = this->width();
	int winh = this->height();
	double xf = 1.0*winw/imagew;
	double yf = 1.0*winh/imageh;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	setTextIsEnabled(true);
	startScreenCoordinatesSystem();
	glRasterPos2i(0,winh);
	glPixelZoom(xf, yf);
	glDrawPixels(imagew, imageh, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	stopScreenCoordinatesSystem();
}
*/
void QtViewer::glDrawBackgroundImage(void)
{
	const int winw = this->width();
	const int winh = this->height();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	startScreenCoordinatesSystem();
	Vector3f black(0.2f, 0.2f, 0.2f);
	Vector3f white(0.9f, 0.9f, 0.9f);
	glBegin(GL_QUADS);
		glColor3fv(&black.x);  
		glVertex2i(0, 0);
		glColor3fv(&black.x);  
		glVertex2i(winw, 0);
		glColor3fv(&white.x);  
		glVertex2i(winw, winh);
		glColor3fv(&white.x);  
		glVertex2i(0, winh);
	glEnd();
	stopScreenCoordinatesSystem();
}

//The actual drawing function
void QtViewer::__glDraw(void)
{
	static int render_mutex = 0;
	if (render_mutex==1) return;
	render_mutex=1;
	const Vector3i bgcolor = _getBackgroundColor(m_pDrawParms);
	setBackgroundColor(QColor(bgcolor.x, bgcolor.y, bgcolor.z));
	//glClearColor(0, 0, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT);
	if (m_pDrawParms)
		setAxisIsDrawn(m_pDrawParms->m_bShowAixes);
	//draw the obj;
	//glDrawBackgroundImage();

	//test Tet CSG functions, NZ 2009/07/17
	extern void runCSGCut(CSceneGraph* pSG, CSceneGraph* pLaptoolSG);
	runCSGCut(m_pSceneGraph, m_pLaptoolSceneGraph);

	glDrawObject();

	render_mutex=0;
}
		
//Draws a simplified version of the scene to guarantee interactive camera displacements.
void QtViewer::fastDraw(void)
{
	__glDraw();
}


void QtViewer::glDrawObject(void)
{
	{//setup a light at the view position;
	qglviewer::Vec pos=camera()->position();
	qglviewer::Vec dir=camera()->viewDirection();
	m_pDrawParms->m_ViewPos = Vector3f(pos.x, pos.y, pos.z);
	m_pDrawParms->m_ViewDir = Vector3f(dir.x, dir.y, dir.z);
	Vector3f lightpos0 = m_pDrawParms->m_ViewPos;
	CPointLight light0(lightpos0, 1.00f);
	light0.applyLight(0);
	}

	//draw the scene using the deformed vertices
	typedef cia3d::simulation::CDeformationThread CDeformationThread;
	CDeformationThread *pDefInst = CDeformationThread::getInstance();
	CDeformationThread& defThread = *pDefInst;
	extern CDualBuffer< vector<Vector3d> > defElmVertexBuffer;	//Note: space required by Intel C++ 11.0
	extern CDualBuffer< vector<Vector4i> > defElmSurfaceBuffer;	//Note: space required by Intel C++ 11.0
	extern CDualBuffer< vector<int> > defElmSurfaceCountBuffer; //Note: space required by Intel C++ 11.0

	const bool runthread = defThread.isRunning();
	vector<Vector3d> *pElmVerticesBuffer=NULL;
	vector<Vector4i> *pElmSurfacesBuffer=NULL;
	vector<int> *pElmSurfacesCountBuffer = NULL;
	if (runthread){							//defeormation threading accessing Critical Data
		defElmVertexBuffer.lock();
		defElmVertexBuffer.sync();			//sync the buffers
		defElmSurfaceBuffer.sync();
		defElmSurfaceCountBuffer.sync();
		pElmVerticesBuffer = defElmVertexBuffer.getFrontBuffer();
		pElmSurfacesBuffer = defElmSurfaceBuffer.getFrontBuffer();
		pElmSurfacesCountBuffer = defElmSurfaceCountBuffer.getFrontBuffer();
		defElmVertexBuffer.unlock();
	}

	extern bool nExportObj;
	m_pDrawParms->m_pFboMRT = this->getFrameBufferObjectPointer();
	//The drawing process includes several stages
	//1. draw the deformed objects scene graph
	m_pSceneGraph->glDraw(*m_pDrawParms, 
		pElmVerticesBuffer, 
		pElmSurfacesBuffer, 
		pElmSurfacesCountBuffer, 
		nExportObj);
	//2. draw the background (undeformed) objects scene graph
	m_pBgSceneGraph->glDraw(*m_pDrawParms, NULL);
	//3. draw the laparoscopic tools scene graph
	//update the mouse grabber
	extern void checkGrabberTools(CSceneGraph &laptool, vector<Vector3d> *deformedVertices);
	checkGrabberTools(*m_pLaptoolSceneGraph, pElmVerticesBuffer);
	//we draw them after the coordinates are updated
	m_pLaptoolSceneGraph->glDraw(*m_pDrawParms, NULL);

	//================Collision and Mouse drag===================
	if (pElmVerticesBuffer){
		CCollisionResponseBuffer colbuf;
		colbuf.beginCollision();
			//mouse drag test
			getMouseGrabberPositionConstraints(*m_pLaptoolSceneGraph, pElmVerticesBuffer, colbuf);
			//collison handling by calling Shen's code
			extern double physicaltime;
			const double cur_time = physicaltime;
			collisonInstrumentTissue(cur_time,
									 *m_pSceneGraph, 
									 *m_pLaptoolSceneGraph, 
									 pElmVerticesBuffer,
									 pElmSurfacesBuffer,
									 pElmSurfacesCountBuffer,
									 colbuf);
		colbuf.endCollision();
		CCollisionResponseBuffer *pcolbuf = CCollisionResponseBuffer::getInstance();
		*pcolbuf = colbuf;	//double buffer
	}
	//========================================================

	//if (nExportObj) nExportObj=false;
}
