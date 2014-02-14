//FILE: QtViewer.h

#ifndef _CREST_GUI_QT_QTVIEWER_H
#define _CREST_GUI_QT_QTVIEWER_H

#include <qgl.h>
#include <qtimer.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>

#include <crest/base/Vec.h>
#include <crest/base/Quat.h>
#include <crest/ogl/texture.h>
#include <crest/system/ztime.h>
#include <crest/simulation/GNode.h>
//#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include "ZQGLViewer.h"


class QtViewer :public ZQGLViewer
{
    Q_OBJECT

private:

	CGLDrawParms* m_pDrawParms;			//rendering configuration
	CSceneGraph* m_pSceneGraph;			//scengraph, objects that can be deformed in CIA3d
	CSceneGraph* m_pBgSceneGraph;		//scengraph for Background objects
	CSceneGraph* m_pLaptoolSceneGraph;	//scengraph for Laparoscopic instruments

    enum {
        TRACKBALL_MODE = 1,
        PAN_MODE = 2,
        ZOOM_MODE = 3,

        BTLEFT_MODE = 101,
        BTRIGHT_MODE = 102,
        BTMIDDLE_MODE = 103,
    };
    // Interaction
    enum {
        XY_TRANSLATION = 1,
        Z_TRANSLATION = 2,
    };

    enum { MINMOVE = 10 };


    GNode* groot;
    std::string sceneFileName;
    QTimer* timerStep;
    QTimer* timerAnimate;
    int				_W, _H;
    int				_clearBuffer;
    int				_navigationMode;
    bool			_video;
	bool			_animationOBJ; 
	int _animationOBJcounter;// save a succession of .obj indexed by _animationOBJcounter
    bool			_axis;
    int 			_background;
    bool			_shadow;
	bool			_glshadow;
    float			_zoomSpeed;
    float			_panSpeed;
    //Transformation	_sceneTransform;
    Vector3			_previousEyePos;
    GLUquadricObj*	_arrow;
    GLUquadricObj*	_tube;
    GLUquadricObj*	_sphere;
    GLUquadricObj*	_disk;
    GLuint			_numOBJmodels;
    GLuint			_materialMode;
    GLboolean		_facetNormal;
    float			_zoom;
    int				_renderingMode;
    bool			_waitForRender;
    bool			_automateDisplayed;
    ctime_t			_beginTime;
    //RayPickInteractor* interactor;
    double lastProjectionMatrix[16];
    double lastModelviewMatrix[16];
    GLint lastViewport[4];
    bool    sceneBBoxIsValid;

private:
	//Initializes the viewer OpenGL context. 
	//This method is called before the first drawing and should be overloaded to initialize some of the OpenGL flags
    void init();

	void glDrawObject(void);

	void glDrawBackgroundImage(void);

	//The actual drawing function
	void __glDraw(void);
	
	//The core method of the viewer, that draws the scene. Inherit from QGLViewer
	virtual void draw(void)
	{
		__glDraw();
	}
	
	//Draws a simplified version of the scene to guarantee interactive camera displacements.
	virtual void fastDraw(void);

	void _defaultInit(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG);

public:

    QtViewer( QWidget* parent, CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, const char* name="" );
    QtViewer( QWidget* parent);
    virtual ~QtViewer();

public slots:
    virtual void step();
    virtual void animate();
    virtual void setDt(double);
    virtual void setDt(const QString&);
    virtual void resetScene();
    virtual void resetView();
    virtual void saveView();
    virtual void showVisual(bool);
    virtual void showBehavior(bool);
    virtual void showCollision(bool);
    virtual void showBoundingCollision(bool);
    virtual void showMapping(bool);
    virtual void showMechanicalMapping(bool);
    virtual void showForceField(bool);
    virtual void showInteractionForceField(bool);
    virtual void showWireFrame(bool);
    virtual void showNormals(bool);
    virtual void screenshot();
    virtual void exportGraph();
    virtual void exportGraph(GNode*);
	virtual void exportOBJ(bool exportMTL=true);
    virtual void dumpState(bool);
    virtual void displayComputationTime(bool);
    virtual void setExportGnuplot(bool);
	virtual void setSizeW(int);
	virtual void setSizeH(int);
			
signals:
    void reload();
    void newFPS(const QString&);
    void newFPS(double);
    void newTime(const QString&);
    void newTime(double);
    void redrawn(void);
	void resizeW( int );
	void resizeH( int );

protected:

    void calcProjection();
    void ApplyShadowMap();
    void CreateRenderTexture(GLuint& textureID, int sizeX, int sizeY, int channels, int type);
    void StoreLightMatrices();

public:

	void setScene(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG);
	
	void setScene(GNode* scene, const char* filename=NULL);

    GNode* getScene()
    {
        return groot;
    }
    const std::string& getSceneFileName()
    {
        return sceneFileName;
    }
    void			SwitchToPresetView();
    void			SwitchToAutomateView();
    //void			reshape(int width, int height);
    int GetWidth()
    {
        return _W;
    };
    int GetHeight()
    {
        return _H;
    };

    void	UpdateOBJ(void);

    /////////////////
    // Interaction //
    /////////////////

    bool _mouseInteractorTranslationMode;
    bool _mouseInteractorRotationMode;
    bool _mouseInteractorMoving;
    int _mouseInteractorSavedPosX;
    int _mouseInteractorSavedPosY;
    int _translationMode;
    Quaternion _mouseInteractorCurrentQuat;
    Vector3 _mouseInteractorAbsolutePosition;
    //Trackball _mouseInteractorTrackball;
    void ApplyMouseInteractorTransformation(int x, int y);

    static Quaternion _mouseInteractorNewQuat;
    static Vector3 _mouseInteractorRelativePosition;
    static Quaternion _newQuat;
    static Quaternion _currentQuat;
    static bool _mouseTrans;
    static bool _mouseRotate;

	// Display scene from the automate
	void drawFromAutomate();
	static void	automateDisplayVM(void);

private:

    void	InitGFX(void);
    void	PrintString(void* font, char* string);
    void	Display3DText(float x, float y, float z, char* string);
    void	DrawAxis(double xpos, double ypos, double zpos, double arrowSize);
    void	DrawBox(double* minBBox, double* maxBBox, double r=0.0);
    void	DrawXYPlane(double zo, double xmin, double xmax, double ymin,
                     double ymax, double step);
    void	DrawYZPlane(double xo, double ymin, double ymax, double zmin,
                     double zmax, double step);
    void	DrawXZPlane(double yo, double xmin, double xmax, double zmin,
                     double zmax, double step);
    void	CreateOBJmodelDisplayList(int material_mode);
    //int     loadBMP(char *filename, TextureImage *texture);
    //void	LoadGLTexture(char *Filename);
    void	DrawLogo(void);
    void	DisplayOBJs(bool shadowPass = false);
    void	DisplayMenu(void);
    void	DrawScene();
    void	DrawAutomate();
    void	ApplySceneTransformation(int x, int y);
    //int		handle(int event);	// required by FLTK

protected:
    //virtual bool event ( QEvent * e );
    bool m_isControlPressed;
    bool isControlPressed() const;
    bool m_dumpState;
    bool m_displayComputationTime;
    bool m_exportGnuplot;
    std::ofstream* m_dumpStateStream;
    virtual void keyPressEvent ( QKeyEvent * e );
    virtual void keyReleaseEvent ( QKeyEvent * e );

    //virtual void mousePressEvent ( QMouseEvent * e );
    //virtual void mouseReleaseEvent ( QMouseEvent * e );
    //virtual void mouseMoveEvent ( QMouseEvent * e );
    //virtual void mouseEvent ( QMouseEvent * e );

    void eventNewStep();
    void eventNewTime();
};

#endif


