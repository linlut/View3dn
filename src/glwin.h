
/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef CGLWin_H
#define CGLWin_H

#include <assert.h>
#include <qmainwindow.h>
#include <qgl.h>
#include <qimage.h>
#include <qtoolbar.h>
#include <qmessagebox.h>
#include <qinputdialog.h> 
#include <qaction.h>
#include <qprinter.h>
#include <triangle_mesh.h>
#include "view3d.h"
#include "cia3dobj.h"

//This application's name
#define APP_NAME "QuickView"

#define VIEW3D_MAX_OBJ_COUNT  20

class CObjectListView;


class CGLWin : public QMainWindow 
{
    Q_OBJECT

public:
	Viewer * m_pGLUIWin;

public:
    CGLWin(void);

    ~CGLWin();

	void setGoodWindowSize(const int w=100, const int h=100)
	{
		resizeAllWindow(w, h);
	}

protected:
    void closeEvent( QCloseEvent*);

private:
	QMenuBar *m_pMenuBar;
	QToolBar *m_pToolBar;
	QPrinter *m_pPrinter;
    QDockWindow *m_pFemBar; 

    QAction * _fileNewAction;
    QAction * _fileOpenAction;
	QAction * _fileSaveAction;
    QAction * _filePrintAction;

	QAction * m_pActFastDraw;
	QAction * m_pActRepeatPlay;
	QAction * m_pMirroringAction;
	QAction * m_pBGImageAction;
	QAction * m_pFloorAction;

	QAction * m_pRot90Action;
	QAction * m_pRot180Action;
	QAction * m_pRot270Action;

	CCia3dObj * _pCia3dObjs[VIEW3D_MAX_OBJ_COUNT];

	CGLDrawParms _drawParms;		//drawing configuration

	QImage _qimage;					//background image

	QString m_strFileName;			//name of the input file

	int m_nTimeDelay;				//delay mili-sec. for animation playing

	CObjectListView *m_pListViews;

private:
	//resize the OpenGL window 
	void resizeAllWindow(const int x, const int y)
	{
		assert(m_pGLUIWin!=NULL);

		QSize s1 = m_pGLUIWin->size();
		QSize s2 = this->size();
		
		int nx = s2.width() - s1.width()+ x;
		int ny = s2.height() - s1.height()+ y;
		this->resize(nx, ny);
	}

	QToolBar* _createToolBar(void);

	QDockWindow *_createDockWidgetBar(QWidget *parent, const QString& appdir);

	void prepareRotationMatrix(CGLDrawParms *p, const float arc);

	void prepareMirrorMatrix(const Vector3d &x, const Vector3d & z, const Vector3d &p, Matrix &m);

	//get the projection matrix for software usage;
	void PrepareProjectionMatrix(double mat[16]);

	bool _openCia3dFile(const QString& fname);

	void _loadNextCia3dFile(const int inc);

	void updateObjectList(void);

protected:
	virtual void keyPressEvent( QKeyEvent * e );

private slots:
	//menu: File
    void newDoc();
	void openCia3dFile();
	void openBackgroundImage();
    void save();
	void saveSTLFile(void);
	void saveTXTFile(void);
    void saveAs(void);
    void print(void);
	void loadCamera(void);
	void saveCamera(void);
	void exportPovray(void);
	void exportEps(void);

	// sampling demos
	void compute_mpart_hdf(void);
	void compute_fandisk_hdf(void);
	void compute_cube_hdf(void);
	void compute_sphere_hdf(void);
	void compute_cyl_hdf(void);
	
	// mesh gen
	void create_cylinder_mesh(void);
	void create_sphere_mesh(void);

	// rendering
	void raytracer(void);
	void hardware_rendering(void);

	// options;	
	void toggleStereo(void);
	void incEyeDistance(void);
	void decEyeDistance(void);

	void ShowBackgroundImage(void);

	void ShowFloor(void);

	void set_obj_picking(void)
	{
		assert(m_pGLUIWin!=NULL);
		m_pGLUIWin->m_SceneGraph.setPickingType(CSceneGraph::PICK_OBJECT);

	}
	void set_face_picking(void)
	{
		assert(m_pGLUIWin!=NULL);
		m_pGLUIWin->m_SceneGraph.setPickingType(CSceneGraph::PICK_FACE);
	}
	void set_line_picking(void)
	{
		assert(m_pGLUIWin!=NULL);
		m_pGLUIWin->m_SceneGraph.setPickingType(CSceneGraph::PICK_LINE);
	}

	void OptionMirroring(void);
	void OptionRendering(void);
	void OptionVolumeRendering(void);
	void OptionRotate90(void);
	void OptionRotate180(void);
	void OptionRotate270(void);

	void OptionGlobalSetting(void);
	void toggleFastDraw(void)
	{
		m_pGLUIWin->m_bFastDraw = m_pActFastDraw->isOn();
	}

	//tools
    void dupe_currentobj(void);
    void dupe_currentobjN(void);
	void show_entirescene(void);
	void create_animation(void);
	void InputRotationAxis(void);
	void InputMirrorPlane(void);
	void hidden_line(void);

	void _detail_recon(const char* PATH, const int n1, const int n2, const int step);
	void detail_recon_bar(void);
	void detail_recon_vase(void);
	void detail_recon_a4plate(void);
	void detail_recon_anyobj(void);

	void editDelayTime(void);
	void startAnimation(void);
	void stopAnimation(void);
	void toggleRepeatPlay(void)
	{
		m_pGLUIWin->m_bRepeatPlay = m_pActRepeatPlay->isOn();
	}

	//menu: View
	void setGLWinSize320x200(void);
	void setGLWinSize400x400(void);
	void setGLWinSize500x400(void);
	void setGLWinSize500x500(void);
	void setGLWinSize640x480(void);
	void setGLWinSize800x600(void);
	void setGLWinSize1024x768(void);
	void swapWidthHeight(void);
	void inputWidthHeight(void);
 
    void about();
    void aboutQt();
    
};


#endif // CGLWin_H
