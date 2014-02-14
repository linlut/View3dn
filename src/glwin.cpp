//File: glwin.cpp
//Author: Nan Zhang   June 13. 2011

#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qinputdialog.h> 
#include <qtabdialog.h>
#include <qstring.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qtoolbox.h>

#include "glwin.h"
#include "mirrordialogex.h"
#include "dparmdialogex.h"
#include "animcontroldialogex.h"
#include "hlineview.h"
#include "dynamicmesh.h"
#include "scenegraphstate.h"
#include "objectlistview.h"

#include <images/filenew.xpm>
#include <images/fileopen.xpm>
#include <images/filesave.xpm>
#include <images/fileprint.xpm>
#include <images/play.xpm>


extern CDynamicMesh *pDynMesh;
static QString saveDirPath = "";
static QString loadDirPath = "";


static void COPY_MATRIX_TO_ARRAY(const Matrix& m, double *p)
{
	p[0] = m.x[0][0];
	p[1] = m.x[0][1];
	p[2] = m.x[0][2];
	p[3] = m.x[0][3];
	p[4] = m.x[1][0];
	p[5] = m.x[1][1];
	p[6] = m.x[1][2];
	p[7] = m.x[1][3];
	p[8] = m.x[2][0];
	p[9] = m.x[2][1];
	p[10] = m.x[2][2];
	p[11] = m.x[2][3];
	p[12] = m.x[3][0];
	p[13] = m.x[3][1];
	p[14] = m.x[3][2];
	p[15] = m.x[3][3];
}


QToolBar* CGLWin::_createToolBar(void)
{
    _fileNewAction = new QAction( QPixmap(filenew_xpm), "&Clear Window", CTRL+Key_N, this);
    connect(_fileNewAction, SIGNAL(activated()), this, SLOT(newDoc()));

    _fileOpenAction = new QAction( QPixmap(fileopen), "&Open...", CTRL+Key_O, this);
    connect(_fileOpenAction, SIGNAL(activated()), this, SLOT(openCia3dFile()));

    _fileSaveAction = new QAction( QPixmap(filesave), "&Save snapshot", CTRL+Key_S, this);
    connect(_fileSaveAction, SIGNAL(activated()), this, SLOT(save()));

    _filePrintAction = new QAction( QPixmap(fileprint), "&Print", CTRL+Key_Q, this);
    connect(_filePrintAction, SIGNAL(activated()), this, SLOT(print()));

    // populate a tool bar with some actions
    QToolBar * fileTools = new QToolBar( this, "file operations" );
    fileTools->setLabel( "File Operations" );
    _fileNewAction->addTo( fileTools );
    _fileOpenAction->addTo( fileTools );
    _fileSaveAction->addTo( fileTools );
    _filePrintAction->addTo( fileTools );
	return fileTools;
}


QDockWindow* CGLWin::_createDockWidgetBar(QWidget *parent, const QString& appdir)
{
    QDockWindow* p = new QDockWindow( QDockWindow::InDock, this );
    p->setResizeEnabled(true);
    p->setCloseMode(QDockWindow::Always);
    addToolBar( p, Qt::DockLeft );

    QToolBox* pToolBox = new QToolBox(p);
    p->setWidget(pToolBox);
    p->setFixedExtentWidth( 200 );
    p->setCaption("Object List");
    setDockEnabled( p, Qt::DockTop, FALSE );
    setDockEnabled( p, Qt::DockBottom, FALSE );

    //first node
    m_pListViews = new CObjectListView(m_pGLUIWin->m_SceneGraph, pToolBox);
    pToolBox->addItem(m_pListViews, "Objects");

	/*
    //second node
	m_pMaterialTree = new CMaterialTree(appdir, pActiveSceneNode, pToolBox);
    pToolBox->addItem(m_pMaterialTree, "Materials");
	*/
    return p;    
}


//Create a CGLWin widget
CGLWin::CGLWin(void):
	QMainWindow( 0, "View3D", WDestructiveClose), 
	m_pListViews(NULL)
{
	int i;

	// Data members;
	CGLDrawParms tDraw;
	_drawParms = tDraw;

	m_nTimeDelay = 0;

	for (i=0; i<VIEW3D_MAX_OBJ_COUNT; i++) _pCia3dObjs[i] = NULL;

	// setup printer
	m_pPrinter = new QPrinter;

	// GUI members, create a toolbar;
	m_pToolBar = _createToolBar();		

    // populate a menu with all actions
	m_pMenuBar = menuBar();
	assert(m_pMenuBar!=NULL);

	//====================File options ==========================
	{
		QPopupMenu * file = new QPopupMenu( this );
		menuBar()->insertItem( "&File  ", file );
		_fileNewAction->addTo( file );
		_fileOpenAction->addTo( file );
		_fileSaveAction->addTo( file );
		file->insertSeparator();
		file->insertItem( "Open Background", this,  SLOT(openBackgroundImage()), NULL);
		file->insertItem( "Load Camera", this,  SLOT(loadCamera()), NULL);
		file->insertItem( "Save Camera", this,  SLOT(saveCamera()), NULL);
		file->insertSeparator();	//===========================
		file->insertItem( "Export Povray", this,  SLOT(exportPovray()), NULL);
		file->insertItem( "Export STL", this,  SLOT(saveSTLFile()), NULL);
		file->insertItem( "Export TXT", this,  SLOT(saveTXTFile()), NULL);
		file->insertItem( "Export EPS", this,  SLOT(exportEps()), NULL);
	    file->insertSeparator();	//===========================
		_filePrintAction->addTo( file );
	    file->insertSeparator();	//===========================
		file->insertItem( "E&xit", qApp,  SLOT(closeAllWindows()), NULL);
	}


	//================ Tools Options ================================
	{
		QPopupMenu * pMenuTools = new QPopupMenu( this );
		menuBar()->insertItem( "&Tools", pMenuTools );
		pMenuTools->insertItem( "Show Entire Scene", this,  SLOT(show_entirescene()));
		pMenuTools->insertItem( "Dupe Current Object", this,  SLOT(dupe_currentobj()));
		pMenuTools->insertItem( "Dupe Current Object N", this,  SLOT(dupe_currentobjN()));

		{// Animation options
			pMenuTools->insertSeparator();	//=======================
			QPopupMenu * pAnimOption = new QPopupMenu( this );
			pMenuTools->insertItem( "&Prepare Animation", pAnimOption);
			pAnimOption->insertItem( "Setup", this,  SLOT(create_animation()));

		    QAction* startAction = new QAction(QPixmap(play_xpm), "&Start Animation", Qt::Key_F10, this);
			connect(startAction, SIGNAL(activated()), this, SLOT(startAnimation()));
		    startAction->addTo( pAnimOption );

			pAnimOption->insertItem( "Stop Animation", this,  SLOT(stopAnimation()), Qt::Key_F11);
			pAnimOption->insertItem( "Change Delay Time", this,  SLOT(editDelayTime()));

			{//toggle fast draw mode
				m_pActRepeatPlay = new QAction("Repeat", CTRL+Key_T, this);
				connect(m_pActRepeatPlay, SIGNAL(activated()), this, SLOT(toggleRepeatPlay()));
				m_pActRepeatPlay->setToggleAction(true);
				m_pActRepeatPlay->setOn(false);
				m_pActRepeatPlay->addTo( pAnimOption );
			}
		}
		pMenuTools->insertSeparator();	//=======================
		{// Stereo options
			QPopupMenu * pStereoOption = new QPopupMenu( this );
			pMenuTools->insertItem( "&Stereo", pStereoOption);

			QAction* m_pActToggleStereo = new QAction("Stereo Mode", Qt::Key_F12, this);
			connect(m_pActToggleStereo, SIGNAL(activated()), this, SLOT(toggleStereo()));
			m_pActToggleStereo->setToggleAction(true);
			m_pActToggleStereo->setOn(false);
			m_pActToggleStereo->addTo( pStereoOption );

			pStereoOption->insertItem( "Increase Eye Distance", this,  SLOT(incEyeDistance()), Qt::Key_Plus);
			pStereoOption->insertItem( "Decrease Eye Distance", this,  SLOT(decEyeDistance()), Qt::Key_Minus);
		}

		//pMenuTools->insertItem( "Rotation Axis", this,  SLOT(InputRotationAxis()), NULL);
		//pMenuTools->insertItem( "Mirror Plane", this,  SLOT(InputMirrorPlane()), NULL);
		//pMenuTools->insertSeparator();
		//pMenuTools->insertItem( "Vector Hidden Line Removal", this,  SLOT(hidden_line()), CTRL+Key_H);
		//pMenuTools->insertSeparator();	//=======================

		pMenuTools->insertSeparator();	//=======================
		pMenuTools->insertItem( "Set Mirror Plane", this,  SLOT(InputMirrorPlane()));
		pMenuTools->insertItem( "Set Rotation Axis", this,  SLOT(InputRotationAxis()));
	}

	//================ Rendering options ================================
	{
		QPopupMenu * pMenuOption = new QPopupMenu( this );
		menuBar()->insertItem( "&Options", pMenuOption );
		pMenuOption->insertItem( "Global Setting", this,  SLOT(OptionGlobalSetting()));
		pMenuOption->insertItem( "Surface Rendering", this,  SLOT(OptionRendering()));
		pMenuOption->insertItem( "Volume Rendering", this,  SLOT(OptionVolumeRendering()));
		pMenuOption->insertSeparator();	//=======================
		{
			QPopupMenu * pPickOption = new QPopupMenu( this );
			pMenuOption->insertItem( "&Picking Types", pPickOption);
			pPickOption->insertItem( "Pick an Object", this,  SLOT(set_obj_picking()));
			pPickOption->insertItem( "Pick a Polygon", this,  SLOT(set_face_picking()));
			pPickOption->insertItem( "Pick a Line", this,  SLOT(set_line_picking()));
		}

		QAction* act_mirroring = new QAction("Mirroring", 0, this);
		connect( act_mirroring, SIGNAL(activated()), this, SLOT(OptionMirroring()));
		act_mirroring->setToggleAction(true);
		act_mirroring->setOn(false);
		act_mirroring->addTo( pMenuOption );
		m_pMirroringAction = act_mirroring;

		QPopupMenu * pRotOption = new QPopupMenu( this );
		pMenuOption->insertItem( "&Rotation Types", pRotOption);
		QAction* act_rot0obj = new QAction("Rotate 90 Degree", 0, this);
		connect( act_rot0obj, SIGNAL(activated()), this, SLOT(OptionRotate90()));
		act_rot0obj->addTo( pRotOption );
		QAction* act_rot1obj = new QAction("Rotate 180 Degree", 0, this);
		connect( act_rot1obj, SIGNAL(activated()), this, SLOT(OptionRotate180()));
		act_rot1obj->addTo( pRotOption );
		QAction* act_rot2obj = new QAction("Rotate 270 Degree", 0, this);
		connect( act_rot2obj, SIGNAL(activated()), this, SLOT(OptionRotate270()));
		act_rot2obj->addTo( pRotOption );
		m_pRot90Action = act_rot0obj;
		m_pRot180Action = act_rot1obj;
		m_pRot270Action = act_rot2obj;
		m_pRot90Action->setToggleAction(true);
		m_pRot90Action->setOn(false);
		m_pRot180Action->setToggleAction(true);
		m_pRot180Action->setOn(false);
		m_pRot270Action->setToggleAction(true);
		m_pRot270Action->setOn(false);

		//show background image buttion
		QAction* act_showbgimg = new QAction("Background Image", 0, this);
		connect( act_showbgimg, SIGNAL(activated()), this, SLOT(ShowBackgroundImage()));
		act_showbgimg->setToggleAction(true);
		act_showbgimg->setOn(false);
		act_showbgimg->addTo( pMenuOption );
		m_pBGImageAction = act_showbgimg;

		//show floor plane buttion
		QAction* act_showfloor = new QAction(QString::fromLocal8Bit("Floor Plane"), 0, this);
		connect( act_showfloor, SIGNAL(activated()), this, SLOT(ShowFloor()));
		act_showfloor->setToggleAction(true);
		act_showfloor->setOn(false);
		act_showfloor->addTo( pMenuOption );
		m_pFloorAction = act_showfloor;

		{//Toggle fast draw mode
			m_pActFastDraw = new QAction("Fast Drawing", 0, this);
			connect(m_pActFastDraw, SIGNAL(activated()), this, SLOT(toggleFastDraw()));
			m_pActFastDraw->setToggleAction(true);
			m_pActFastDraw->setOn(false);
			m_pActFastDraw->addTo( pMenuOption );
		}

	}


	//========change opengl window size ==========================
	{
		QPopupMenu * pMenuGLWinsize = new QPopupMenu( this );
		menuBar()->insertItem( "&Window", pMenuGLWinsize );
		pMenuGLWinsize->insertItem( "320x200", this,  SLOT(setGLWinSize320x200()), CTRL+Qt::Key_3);
		pMenuGLWinsize->insertItem( "400x400", this,  SLOT(setGLWinSize400x400()), CTRL+Qt::Key_4);
		pMenuGLWinsize->insertItem( "500x400", this,  SLOT(setGLWinSize500x400()), CTRL+Qt::Key_5);
		pMenuGLWinsize->insertItem( "500x500", this,  SLOT(setGLWinSize500x500()), CTRL+Qt::Key_2 );
		pMenuGLWinsize->insertItem( "640x480", this,  SLOT(setGLWinSize640x480()), CTRL+Qt::Key_6);
		pMenuGLWinsize->insertItem( "800x600", this,  SLOT(setGLWinSize800x600()), CTRL+Qt::Key_8);
		pMenuGLWinsize->insertItem( "1024x768", this,  SLOT(setGLWinSize1024x768()), CTRL+Qt::Key_1);
		pMenuGLWinsize->insertSeparator();
		pMenuGLWinsize->insertItem( "Swap Width/Height", this,  SLOT(swapWidthHeight()), CTRL+Key_W);
		pMenuGLWinsize->insertItem( "Input Width/Height", this,  SLOT(inputWidthHeight()), CTRL+Key_I);
	}


/*
	{
		QPopupMenu * pPopMenu = new QPopupMenu( this );
		menuBar()->insertItem( "&My Research", pPopMenu );
		pPopMenu->insertItem( "Bar Recons", this,  SLOT(detail_recon_bar()));
		pPopMenu->insertItem( "Vase Recons", this,  SLOT(detail_recon_vase()));
		pPopMenu->insertSeparator();
		pPopMenu->insertItem( "Letter Size Plate", this,  SLOT(detail_recon_a4plate()));
		pPopMenu->insertSeparator();
		pPopMenu->insertItem( "Detail Recon Dialog", this,  SLOT(detail_recon_anyobj()));
	} 
*/

	//================ end Rendering options ================================
	QGLFormat format;
	format.setStereo( true );
    m_pGLUIWin = new Viewer(format, this, "Viewer3D"); 
	assert(m_pGLUIWin!=NULL);
    statusBar()->setFixedHeight( fontMetrics().height() + 6 );
    statusBar()->message( "Viewer ready", 3000 );

    setCentralWidget(m_pGLUIWin);
	m_pGLUIWin->setDrawingData(&_drawParms);
	m_pGLUIWin->m_SceneGraph.hookObject(m_pGLUIWin);
	m_pGLUIWin->m_SceneGraph.hookGLWindow(m_pGLUIWin);

	//================ dock window==================
	QString appDir;
    m_pFemBar=_createDockWidgetBar(this, appDir);

}



/*!
  Release allocated resources
*/

CGLWin::~CGLWin()
{
	int i;
	for (i=0; i<VIEW3D_MAX_OBJ_COUNT; i++){
	}

	SafeDelete(m_pPrinter);
}


void CGLWin::newDoc()
{
	m_pGLUIWin->m_SceneGraph.RemoveAll();		

	for (int i=0; i<VIEW3D_MAX_OBJ_COUNT; i++){
		CCia3dObj * p = _pCia3dObjs[i];
		SafeDelete(p);
		_pCia3dObjs[i] = NULL;
	}

	CGLDrawParms::resetColorIndex();
	//
	m_pListViews->update(m_pGLUIWin->m_SceneGraph);

	m_pGLUIWin->updateGL(); 
}


void CGLWin::toggleStereo(void)
{
	m_pGLUIWin->toggleStereoDisplay();
}

void CGLWin::incEyeDistance(void)
{
	m_pGLUIWin->incEyeDistance(+1);
}

void CGLWin::decEyeDistance(void)
{
	m_pGLUIWin->incEyeDistance(-1);
}

void CGLWin::keyPressEvent( QKeyEvent * e )
{
	int key = e->key();

	switch(key){
		case Qt::Key_PageUp:
			_loadNextCia3dFile(-1);
			m_pListViews->update(m_pGLUIWin->m_SceneGraph);
			break;
		case Qt::Key_PageDown:
			_loadNextCia3dFile(+1);
			m_pListViews->update(m_pGLUIWin->m_SceneGraph);
			break;
		case Qt::Key_F9:		//Full screen
			m_pGLUIWin->toggleFullScreen();
			break;
		case Qt::Key_F12:		//Toggle stereo display
		case Qt::Key_S:
			m_pGLUIWin->toggleStereoDisplay();
			break;
		default:
			QMainWindow::keyPressEvent(e);
			break;
	}
}


void CGLWin::_loadNextCia3dFile(const int inc)
{
	char *fileExtBuffer[2]={"%d.plt", "%d.obj"};
	char *fileext = fileExtBuffer[0];
    if (m_strFileName.isEmpty()) return;
	const int BLENGTH = 255;
	char buff[BLENGTH+1];
	const char * pchar = m_strFileName.ascii();
	const int slen = strlen(pchar);
	int i, j, c;

	bool isPLT = (strcmp(&pchar[slen-4], ".plt")==0) || (strcmp(&pchar[slen-4], ".PLT")==0);
	bool isOBJ = (strcmp(&pchar[slen-4], ".obj")==0) || (strcmp(&pchar[slen-4], ".OBJ")==0);
	if (isPLT || isOBJ){
		if (isOBJ) fileext = fileExtBuffer[1];
		c = 0;
		i = slen-5;
		while (pchar[i]>='0' && pchar[i]<='9'){
			buff[BLENGTH-c] = pchar[i];
			i--;
			c++;
		}
		for (j=0; j<c; j++) buff[j]=buff[BLENGTH-c+j+1];
		buff[c]=0;
		//get a new number;
		int n = atoi(buff) + inc;
		if (n<0) return; 

		//set a new file name;
		strcpy(buff, pchar);
		i++;
		sprintf(&buff[i], fileext, n);
	}
	else
		return;

	QString fname(buff);
	if (_openCia3dFile(fname)){
		m_strFileName = fname;	
		setCaption(fname);
		m_pGLUIWin->updateGL();  	
	}
}



bool CGLWin::_openCia3dFile(const QString& fname)
{
	static unsigned short loadcount=0;
	CSceneGraph &scenegraph = m_pGLUIWin->m_SceneGraph;
	const double scale = 1;

	//save state
	CSceneGraphState state(scenegraph);
		if (!LoadAndSetMultipleObjs(fname.ascii(), _pCia3dObjs, VIEW3D_MAX_OBJ_COUNT, scale))
			return false;
		scenegraph.RemoveAll();
		AddObjectsToSceneGraph(_pCia3dObjs, VIEW3D_MAX_OBJ_COUNT, m_pGLUIWin->m_SceneGraph);
		state.useSceneGraphState();
		scenegraph.DumpNodeNames();
	state.clearSceneGraphState();

	//set active scene node
	scenegraph.SetDefaultActiveSceneNode();

	//set boudning box
	AxisAlignedBox bbox;
	scenegraph.GetBoundingBox(bbox);
	m_pGLUIWin->m_Floor.SetBoundingBox(bbox);
	Vector3d dd = Vector3d(0, 0, 0); //(bbox.maxp - bbox.minp)*0.50f;
	Vector3d minp = bbox.minp - dd;
	Vector3d maxp = bbox.maxp + dd;
	Vec box1(minp.x,minp.y,minp.z), box2(maxp.x,maxp.y,maxp.z);
	m_pGLUIWin->setSceneBoundingBox(box1, box2);
	{	Camera *pcam=m_pGLUIWin->camera();
		const float s = pcam->flySpeed();
		pcam->setFlySpeed(s*0.02f);
	}

	if (loadcount==0)
		m_pGLUIWin->showEntireScene();

	//set caption
	loadcount ++;
	return true;
}


//--------------------------------
void CGLWin::openCia3dFile()
{
	assert(m_pGLUIWin!=NULL);
	static QString dirPath = "";

    QString fname = QFileDialog::getOpenFileName(
		dirPath,
		//"Surface mesh(*.txt *.ply *.plt *.stl *.obj)",
		"Tecplot Output (*.plt)\nTriangle Meshes (*.obj *.ply *.stl *.txt)\nTetrahedral Meshes (*.mesh *.hmascii *.offv)\nAll Files (*.*)",
		this,
		"Open File Dialog",
		"Choose a mesh file to open"
		);
	//no filename as input
    if (fname.isEmpty()) return;
	if (_openCia3dFile(fname)){
		m_strFileName = fname;	
		setCaption(fname);
	}

	//update left side list
	updateObjectList();
}
//--------------------------------



void CGLWin::save()
{
	if (m_pGLUIWin==NULL) 
		return;

	bool automatic = false;
	bool overwrite = false;
	m_pGLUIWin->setSnapshotQuality(100);
	m_pGLUIWin->saveSnapshot(automatic, overwrite);
}


void CGLWin::saveSTLFile()
{
	CSceneNode *p = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
	if (p==NULL) return;
	CPolyObj *pobj=(CPolyObj*)p->m_pObject;
    if (!pobj->isTriangleBoundary()) return;
	CTriangleObj *ptri = (CTriangleObj*)pobj;

    QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"STL file (*.stl)",
						this,
						"Save STL file dialog",
						"Choose an STL file to save"
						);

    if (fname.isEmpty()) return;
	ptri->saveSTLFile(fname.ascii());
}


void CGLWin::saveTXTFile()
{
	CSceneNode *p = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
	if (p==NULL) return;
	CPolyObj *pobj=(CPolyObj*)p->m_pObject;
    if (!pobj->isTriangleBoundary()) return;
    QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"TXT file (*.txt)",
						this,
						"Save TXT file dialog",
						"Choose an TXT file to save"
						);

    if (fname.isEmpty()) return;
	CTriangleObj *ptri = (CTriangleObj*)pobj;
    ptri->saveTXTFile(fname.ascii());
}


void CGLWin::saveAs(void)
{

}


void CGLWin::print(void)
{
	if (m_pPrinter==NULL) return;

	//parpare printer
    if (m_pPrinter->setup(this))	
		statusBar()->message( tr("Printing...") );
	else
		return;

	QPainter p;
	if(!p.begin(m_pPrinter)) 
	    return;

	//p.setFont( e->font() );
	//QFontMetrics fm = p.fontMetrics();
	//QPaintDeviceMetrics metrics( m_pPrinter ); 
	p.end();				
	
	statusBar()->message( tr("Printing completed"), 2000 );
}



void CGLWin::loadCamera(void)
{
	if (m_pGLUIWin==NULL) return;

    QString fname = QFileDialog::getOpenFileName(
						loadDirPath,
						"Camera state file (*.xml)",
						this,
						"Open camera file dialog",
						"Choose a camera file to open");
    if ( !fname.isEmpty()){
		m_pGLUIWin->setStateFileName(fname);
		m_pGLUIWin->restoreStateFromFile();
		m_pGLUIWin->updateGL();  	
	}
}


void CGLWin::saveCamera(void)
{
	extern void ExportPovrayCamera(Camera* pcamera, FILE *fp); //in scenegraph.cpp
	QString selectedfilter = ".xml";
	if (m_pGLUIWin==NULL) return;
    QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"Camera state file (*.xml)",
						this,
						"Save camera file dialog",
						"Choose a camera file to save"
						);
    if ( !fname.isEmpty()){
		bool hasext = false;
		const char *strName = fname.ascii();
		int len = strlen(strName);
		if (len>4){
			if (strcmp(&strName[len-4], ".xml")==0)
				hasext = true;
			else if (strcmp(&strName[len-4], ".XML")==0)
				hasext = true;
		}
		//write a povray camera 
		ExportPovrayCamera(m_pGLUIWin->camera(), stdout); 

		//write file
		if (hasext)
			m_pGLUIWin->saveToFile(fname);
		else
			m_pGLUIWin->saveToFile(fname+selectedfilter);
	}
}


void CGLWin::exportPovray(void)
{
	assert(m_pGLUIWin!=NULL);
	const int n = m_pGLUIWin->m_SceneGraph.size();
	if (n==0) return;
    QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"Povray file (*.pov)",
						this,
						"Save povray file dialog",
						"Choose a povray file to save"
						);

    if (fname.isEmpty()) return;
	m_pGLUIWin->m_SceneGraph.exportPovrayFile(m_pGLUIWin, fname.ascii());
}


static void glwRedrawFunc(QGLViewer* w,void * user_data)
{
	glEnable(GL_DEPTH_TEST);
	Viewer *vw = (Viewer*)w;
	vw->draw();
}

void CGLWin::exportEps(void)
{
	//no empty scene;
	const int n = m_pGLUIWin->m_SceneGraph.size();
	if (n==0) return;

	//get file name to export
	int i;
	extern bool ExportEPSFile(const char *fname, QGLViewer* win, void *drawfunc);
	QString selectedfilter = ".eps";
    QString fname = QFileDialog::getSaveFileName(
						saveDirPath,
						"EPS file (*.eps)",
						this,
						"Save EPS file dialog",
						"Choose an EPS file to save"
						);
    if (fname.isEmpty()) return;

	//push state
	for (i=0; i<n; i++){
		CSceneNode * p = m_pGLUIWin->m_SceneGraph.GetSceneNode(i);
		p->m_DrawParms.m_bInEPSSnag = true;
	}

	//export data
	ExportEPSFile(fname.ascii(), m_pGLUIWin, (void*)glwRedrawFunc);

	//pop state
	for (i=0; i<n; i++){
		CSceneNode * p = m_pGLUIWin->m_SceneGraph.GetSceneNode(i);
		p->m_DrawParms.m_bInEPSSnag = false;
	}
}


void CGLWin::closeEvent( QCloseEvent* ce )
{
/*	int f = QMessageBox::information( this, "Qt Application Example",
				      "The document has been changed since "
				      "the last save.",
				      "Save Now", "Cancel", "Leave Anyway",
				      0, 1 );
	*/
	int f = 2;
    switch(f){
		case 0:
			save();
			ce->accept();
			break;
		case 1:
			default: // just for sanity
			ce->ignore();
			break;
		case 2:
			exit(0);
			break;
    }
}


//============================================

void CGLWin::setGLWinSize320x200(void)
{
	const int x=320;
	const int y=200;
	resizeAllWindow(x, y);
}


void CGLWin::setGLWinSize400x400(void)
{
	const int x=400;
	const int y=400;
	resizeAllWindow(x, y);
}


void CGLWin::setGLWinSize500x400(void)
{
	const int x=500;
	const int y=400;
	resizeAllWindow(x, y);
}


void CGLWin::setGLWinSize500x500(void)
{
	const int x=500;
	const int y=500;
	resizeAllWindow(x, y);
}


void CGLWin::setGLWinSize640x480(void)
{
	const int x=640;
	const int y=480;
	resizeAllWindow(x, y);
}

void CGLWin::setGLWinSize800x600(void)
{
	const int x=800;
	const int y=600;
	resizeAllWindow(x, y);
}

void CGLWin::setGLWinSize1024x768(void)
{
	const int x=1024;
	const int y=768;
	resizeAllWindow(x, y);
}


void CGLWin::swapWidthHeight(void)
{
	int w = m_pGLUIWin->width();
	int h = m_pGLUIWin->height();
	resizeAllWindow(h, w);
}


void CGLWin::inputWidthHeight(void)
{
	static QString text("640 480");
	int x, y;
	bool ok;
	Viewer *viewer = m_pGLUIWin;;
	if (viewer==NULL) return;

	text = QInputDialog::getText(
            "Input window size", 
			"Please input window size in format: sizex sizey", 
			QLineEdit::Normal,
            text, &ok, this );
	if ( !ok || text.isEmpty() )  return;

	const char *str = text.ascii();
	sscanf(str, "%d %d", &x, &y);
	QSize s1 = viewer->size();
	QSize s2 = this->size();
		
	int nx = s2.width() - s1.width()+ x;
	int ny = s2.height() - s1.height()+ y;
	this->resize(nx, ny);
}


//======================================
void CGLWin::raytracer(void)
{
}

void CGLWin::hardware_rendering(void)
{
}


void CGLWin::about()
{
    QMessageBox::about( this, "View3dn",
			"This program is for postprocessing FEM simulation results.\n"
			"Author: Nan Zhang, Copyright 2011");
}


void CGLWin::aboutQt()
{
    //QMessageBox::aboutQt( this, "HDF sampling and rendering " );
}


//======================================================

void CGLWin::OptionRendering(void)
{
	CSceneNode* pnode=0;
	char dlgname[256];
	pnode = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
	if (pnode==NULL)
		return;

	pnode->m_pObject->CopyAttribNames2DrawParms(pnode->m_DrawParms);
	const char *nodename = pnode->GetObjectName();
	sprintf(dlgname, "Rendering Config for Scene Node %s", nodename);
    CDGDrawParms tabdialog(&pnode->m_DrawParms, this, dlgname);
    tabdialog.exec();
}

void CGLWin::OptionVolumeRendering(void)
{
	CSceneNode * pnode;
	char dlgname[256];
	pnode = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
	if (pnode==NULL)
		return;

	pnode->m_pObject->CopyAttribNames2DrawParms(pnode->m_DrawParms);
	const char *nodename = pnode->GetObjectName();
	sprintf(dlgname, "Rendering Config for Scene Node %s", nodename);
    CDGDrawParms tabdialog(&pnode->m_DrawParms, this, dlgname);
    tabdialog.exec();
}

void CGLWin::prepareMirrorMatrix(const Vector3d &x, const Vector3d & z, const Vector3d &p, Matrix &mat)
{
	Vector3d X = Normalize(x);
	Vector3d Z = Normalize(z);
	Vector3d Y = CrossProd(Z, X);
	Y.normalize();

	Matrix t0; SetTranslationMatrix(-p, t0);
	Matrix r0; GenRotationMatrix(X, Y, Z, r0);
	Matrix m0; MirrorZ(m0);
	Matrix r1 = r0; 
	r1.transpose();
	Matrix rr = r1*r0;
	Matrix t1; SetTranslationMatrix(p, t1);
	mat = t0*r0*m0*r1*t1;
}


void CGLWin::OptionMirroring(void)
{
	CSceneGraph &scenegraph = m_pGLUIWin->m_SceneGraph;
	const int nn = scenegraph.size();

	if ((nn==0) || (!scenegraph.mirrorTriangleBeenSet())){
		m_pMirroringAction->setOn(false);
		scenegraph.RemoveAllMirroredObjects();
		return;
	}

	if (scenegraph.HasMirroredObjects()){
		m_pMirroringAction->setOn(false);
		scenegraph.RemoveAllMirroredObjects();
		return;
	}

	//turn switch on
	m_pMirroringAction->setOn(true);
	addMirroredSceneNodes(scenegraph);
}


void CGLWin::ShowBackgroundImage(void)
{
	if (m_pBGImageAction->isOn() && (!_qimage.isNull()))
		m_pGLUIWin->m_pBGImage = &_qimage;
	else
		m_pGLUIWin->m_pBGImage = NULL;

}


void CGLWin::ShowFloor(void)
{
	if (m_pFloorAction->isOn()){
		m_pGLUIWin->m_Floor.SetDrawZ(true);
		
	}
	else{
		m_pGLUIWin->m_Floor.SetDrawX(false);
		m_pGLUIWin->m_Floor.SetDrawY(false);
		m_pGLUIWin->m_Floor.SetDrawZ(false);
		m_pGLUIWin->m_Floor.SetDraw_X(false);
		m_pGLUIWin->m_Floor.SetDraw_Y(false);
		m_pGLUIWin->m_Floor.SetDraw_Z(false);
	}
}



void CGLWin::InputRotationAxis(void)
{
	bool ok;
	Vector3f v[2];
	Vector3f &v0=v[0], &v1=v[1];
	QString text, s0, s1;
	char str0[100]="", str1[100]="";
	CSceneGraph &scene = m_pGLUIWin->m_SceneGraph;

	{//first, get the info from the active line
		CSceneNode *pnode = NULL;
		scene.GetActiveLine(pnode, v0, v1);
		if (pnode){
			sprintf(str0, "%f %f %f ", v0.x, v0.y, v0.z);
			sprintf(str1, "%f %f %f ", v1.x, v1.y, v1.z);
		}
	}

	//first point;
	text = QInputDialog::getText(
            "Input rotation axis", "Please input the first point in (x y z):", QLineEdit::Normal,
            QString(str0), &ok, this );
	if ( ok && (!text.isEmpty()) ) {
		const char *str = text.ascii();
		sscanf(str, "%f %f %f", &v0.x, &v0.y, &v0.z);
	} 
	else        
		return;
	//second point;
	text = QInputDialog::getText(
            "Input rotation axis", "Please input the second point in (x y z):", QLineEdit::Normal,
            QString(str1), &ok, this );
	if ( ok && (!text.isEmpty()) ) {
		const char *str = text.ascii();
		sscanf(str, "%f %f %f", &v1.x, &v1.y, &v1.z);
	} 
	else        
		return;

	//save the line
	Vector3d vv[2];
	vv[0] = Vector3d(v0.x, v0.y, v0.z);
	vv[1] = Vector3d(v1.x, v1.y, v1.z);
	scene.setRotationAxis(vv);

}


void CGLWin::InputMirrorPlane(void)
{
	bool ok;
	Vector3f v[4];
	Vector3f &v0=v[0];
	Vector3f &v1=v[1];
	Vector3f &v2=v[2];
	QString text, s0, s1, s2;
	char str0[100]="", str1[100]="", str2[100]="";
	CSceneGraph &scene = m_pGLUIWin->m_SceneGraph;

	{//first, get the info from the active face
		int nnode;
		CSceneNode *pnode = NULL;
		scene.GetActiveFace(pnode, v, nnode);
		if (pnode){
			sprintf(str0, "%f %f %f ", v0.x, v0.y, v0.z);
			sprintf(str1, "%f %f %f ", v1.x, v1.y, v1.z);
			sprintf(str2, "%f %f %f ", v2.x, v2.y, v2.z);
		}
		else{
			if (scene.mirrorTriangleBeenSet()){
				Vector3d vv[3];
				scene.getMirrorTriangle(vv);
				v0 = Vector3f(vv[0].x, vv[0].y, vv[0].z);
				v1 = Vector3f(vv[1].x, vv[1].y, vv[1].z);
				v2 = Vector3f(vv[2].x, vv[2].y, vv[2].z);
				sprintf(str0, "%f %f %f ", v0.x, v0.y, v0.z);
				sprintf(str1, "%f %f %f ", v1.x, v1.y, v1.z);
				sprintf(str2, "%f %f %f ", v2.x, v2.y, v2.z);
			}
		}
	}

	//first point;
	text = QInputDialog::getText(
            "Input mirror plane", "Please input the first point in (x y z):", QLineEdit::Normal,
            QString(str0), &ok, this );
	if ((ok) && (!text.isEmpty())){
		const char *str = text.ascii();
		sscanf(str, "%f %f %f", &v0.x, &v0.y, &v0.z);
	} 
	else return;

	//second point;
	text = QInputDialog::getText(
            "Input mirror plane", "Please input the second point in (x y z):", QLineEdit::Normal,
            QString(str1), &ok, this );
	if ( ok && (!text.isEmpty()) ) {
		const char *str = text.ascii();
		sscanf(str, "%f %f %f", &v1.x, &v1.y, &v1.z);
	} 
	else return;

	//3rd point;
	text = QInputDialog::getText(
            "Input mirror plane", "Please input the 3rd point in (x y z):", QLineEdit::Normal,
            QString(str2), &ok, this );
	if ( ok && (!text.isEmpty()) ) {
		const char *str = text.ascii();
		sscanf(str, "%f %f %f", &v2.x, &v2.y, &v2.z);
	} 
	else return;

	//save the triangle
	Vector3d vv[3];
	vv[0] = Vector3d(v0.x, v0.y, v0.z);
	vv[1] = Vector3d(v1.x, v1.y, v1.z);
	vv[2] = Vector3d(v2.x, v2.y, v2.z);
	scene.setMirrorTriangle(vv);
}


void CGLWin::OptionRotate90(void)
{
	CSceneGraph &scenegraph = m_pGLUIWin->m_SceneGraph;
	const int nn = scenegraph.size();
	if ((nn==0) || (!scenegraph.rotationAxisBeenSet())){
		m_pRot90Action->setOn(false);
		scenegraph.RemoveAllRot90Objects();
		return;
	}

	if (scenegraph.HasRot90Objects()){
		m_pRot90Action->setOn(false);
		scenegraph.RemoveAllRot90Objects();
		return;
	}

	//turn switch on
	m_pRot90Action->setOn(true);
	addRotAngleSceneNodes(scenegraph, 90);
}


void CGLWin::OptionRotate180(void)
{
	CSceneGraph &scenegraph = m_pGLUIWin->m_SceneGraph;
	const int nn = scenegraph.size();
	if ((nn==0) || (!scenegraph.rotationAxisBeenSet())){
		m_pRot180Action->setOn(false);
		scenegraph.RemoveAllRot180Objects();
		return;
	}

	if (scenegraph.HasRot180Objects()){
		m_pRot180Action->setOn(false);
		scenegraph.RemoveAllRot180Objects();
		return;
	}

	//turn switch on
	m_pRot180Action->setOn(true);
	addRotAngleSceneNodes(scenegraph, 180);
}


void CGLWin::OptionRotate270(void)
{
	CSceneGraph &scenegraph = m_pGLUIWin->m_SceneGraph;
	const int nn = scenegraph.size();
	if ((nn==0) || (!scenegraph.rotationAxisBeenSet())){
		m_pRot270Action->setOn(false);
		scenegraph.RemoveAllRot270Objects();
		return;
	}

	if (scenegraph.HasRot270Objects()){
		m_pRot270Action->setOn(false);
		scenegraph.RemoveAllRot270Objects();
		return;
	}

	//turn switch on
	m_pRot270Action->setOn(true);
	addRotAngleSceneNodes(scenegraph, 270);
}


void CGLWin::prepareRotationMatrix(CGLDrawParms* p0, const float arc)
{
	if (p0==NULL)
		return;
	/*
	CGLDrawParms &_drawParms = _pObjDrawParms[0];
	if (_drawParms.m_nPickedEdgeID<0)
		return;
	
	Vector3D base = _pObjDrawParms[0].m_vPickedVertex0;
	Vector3D axis = _pObjDrawParms[0].m_vPickedVertex1 - base;
	axis = Normalize(axis);
	Matrix m = RotationAxisMatrix(axis, arc);
    Matrix t1 = TranslationMatrix(-base);
    Matrix t2 = TranslationMatrix(base);
    Matrix mt = (t1 * m) * t2;
	//mt.Transpose();
	COPY_MATRIX_TO_ARRAY(mt, p0->m_Identity);
	*/
}


void CGLWin::startAnimation(void)
{
	const int nn = m_pGLUIWin->m_SceneGraph.size();
	if (nn==0) return;
	m_pGLUIWin->resetCurrentAnimationCounter();
	m_pGLUIWin->m_bRepeatPlay = m_pActRepeatPlay->isOn();
	m_pGLUIWin->doAnimation();
}

void CGLWin::stopAnimation(void)
{
	m_pGLUIWin->invalidCurrentAnimationCounter();
	m_pGLUIWin->m_bRepeatPlay = false;
}

void CGLWin::editDelayTime(void)
{
	bool ok;
	char strval[100];

	sprintf(strval, "%d", m_pGLUIWin->m_nTimeDelay);
	QString text = QInputDialog::getText(
            "Input delay time", "Please input the delay time (ms.):", QLineEdit::Normal,
            strval, &ok, this);
	if (ok && (!text.isEmpty())){
		const char *str = text.ascii();
		sscanf(str, "%d", &m_pGLUIWin->m_nTimeDelay);
	} 
}


static int _find_commstrlen(const char *startfname, const char *endfname)
{
	//found the max common name 
	int i;
	int len = _MIN_(strlen(startfname), strlen(endfname));
	for (i=0; i<len; i++){
		if (startfname[i]!=endfname[i])
			break;
	}
	int c = i-1;
	while (startfname[c]>='0' && startfname[c]<='9')
		c --;
	c++;
	return c;
}


void CGLWin::create_animation(void)
{
	const int SBUFFER_LENGTH = 512;
	static char startfname[SBUFFER_LENGTH]="";
	static char endfname[SBUFFER_LENGTH]="";
	static char outputfname[SBUFFER_LENGTH]="";
	static int counter =1;
	const int nn = m_pGLUIWin->m_SceneGraph.size();
	if (nn==0) return;
    CAnimControlDialogEx diag(this, "Animation control",
				startfname,
				endfname,
				&counter, 
				outputfname);
    if (diag.exec() != QDialog::Accepted) 
		return;
	if (startfname[0]==0 || endfname[0]==0 || counter<=0)
		return;

	//found the max common name 
	int n1, n2, c=_find_commstrlen(startfname, endfname);
	sscanf(&startfname[c], "%d", &n1);
	sscanf(&endfname[c], "%d", &n2);
	char fnamein[SBUFFER_LENGTH];
	for (int i=0; i<c; i++) fnamein[i] = startfname[i];
	fnamein[c++]='%';
	fnamein[c++]='d';
	fnamein[c++]='.';
	const int sslen = strlen(startfname);
	fnamein[c++]=startfname[sslen-3];
	fnamein[c++]=startfname[sslen-2];
	fnamein[c++]=startfname[sslen-1];
	fnamein[c] = 0;
	m_pGLUIWin->setAnimationParameters(fnamein, outputfname, n1, n2, counter);

	//call animation;
    QObject::connect(m_pGLUIWin, SIGNAL(printMessage(const QString &)),
                     statusBar(), SLOT(message(const QString &)));
    statusBar()->message("Playing animation...");	// Initial message
    m_pGLUIWin->doAnimation();
    statusBar()->message("Done.", 2000);			// Final message for 2 seconds
}


void CGLWin::show_entirescene(void)
{
	m_pGLUIWin->showEntireScene();
}


void CGLWin::openBackgroundImage()
{
	static QString dirPath = "";
    QString fname = QFileDialog::getOpenFileName(
						dirPath,
						"Image fiile (*.jpg *.bmp *.png)",
						this,
						"Open file dialog",
						"Choose a background image");
    if (fname.isEmpty())
		return;

	QImage image;
	if (!image.load(fname))
		return;

	_qimage = m_pGLUIWin->convertToGLFormat(image);;
	m_pGLUIWin->m_pBGImage = &_qimage;
	m_pBGImageAction->setOn(true);
	m_pGLUIWin->updateGL();  	
}


void CGLWin::OptionGlobalSetting(void)
{
    CDGDrawParms tabdialog(&_drawParms, this, "Global Setting");
    tabdialog.exec();	
	m_pGLUIWin->updateGL();  	
}


void CGLWin::hidden_line(void)
{/*
	assert(m_pGLUIWin!=NULL);
	const int nsize = m_pGLUIWin->m_SceneGraph.size();
	if (nsize==0) return;

	double mat[16];
	PrepareProjectionMatrix(mat);
	const int w = m_pGLUIWin->width();
	const int h = m_pGLUIWin->height();
    CHlineViewer *p = new CHlineViewer(&_drawParms, &m_pGLUIWin->m_SceneGraph, mat, w, h, NULL, caption()); 
	if (p) p->show();
    */
}


void CGLWin::PrepareProjectionMatrix(double mat[16])
{
	Camera *pcam = m_pGLUIWin->camera();
	GLdouble mv[16];
	GLdouble proj[16];
	pcam->getModelViewMatrix(mv);
	pcam->getProjectionMatrix(proj);
	
	for (unsigned short i=0; i<4; ++i){
		for (unsigned short j=0; j<4; ++j){
			double sum = 0.0;
			for (unsigned short k=0; k<4; ++k)
				sum += proj[i+4*k]*mv[k+4*j];
			mat[i+4*j] = sum;
		}
	}
}


void CGLWin::detail_recon_bar(void)
{
#define PATH "g:/siggraph06/plate0/test-frict02/"
	int i;
	char infile[256], outfile[256];
	const char *fname_c3d1in		= PATH"c3d1.in";
	const char *fname_plt0			= PATH"ciaout0.plt";
	const char *finname_template	= PATH"ciaout%d.plt";
	const char *foutname_template	= PATH"ciaout%04d";

	CDynamicMesh mesh(fname_c3d1in);
	pDynMesh = & mesh;

	mesh.DecideBoundaryVertices(fname_plt0);
	//mesh.SetExportBoudningSphFlag(0, true);
	mesh.SetExportTextureCoor(true);
	mesh.SetReconstrucSurface(true);

	for (i=5; i<41; i++){
#ifdef _DEBUG
		if (i==20){
			int asdag=1;
		}
#endif
		sprintf(infile, finname_template, i);
		sprintf(outfile, foutname_template, i);
		printf("Loading file %s\n", infile);
		mesh.LoadNextTimeStepPlt(i, infile, outfile, m_pGLUIWin);
	}

#undef PATH

}



void CGLWin::detail_recon_vase(void)
{
//#define PATH "G:/PLT/vase3-v25/"
//#define PATH "G:/Siggraph06/vase/vase3-v25new/"
#define PATH "G:/Siggraph06/vase/vase3-v25/"
	int i;
	char infile[256], outfile[256];
	const char *fname_c3d1in		= PATH"c3d1.in";
	const char *fname_plt0			= PATH"ciaout0.plt";
	const char *finname_template	= PATH"ciaout%d.plt";
	const char *foutname_template	= PATH"ciaout%04d";

	CDynamicMesh mesh(fname_c3d1in);
	pDynMesh = & mesh;
	mesh.SetExportTextureCoor(true);
	mesh.SetReconstrucSurface(true);

	mesh.DecideBoundaryVertices(fname_plt0);

	for (i=1; i<635;i++){
#ifdef _DEBUG
		if (i==130){
			int asdag=1;
		}
#endif

		sprintf(infile, finname_template, i);
		sprintf(outfile, foutname_template, i);
		printf("Loading file %s\n", infile);
		mesh.LoadNextTimeStepPlt(i, infile, outfile, m_pGLUIWin);
	}
#undef PATH

}



void CGLWin::detail_recon_a4plate(void)
{
#define PATH "g:/siggraph06/plate0/a4plate2/"
	char infile[256], outfile[256];
	const char *fname_c3d1in		= PATH"c3d1.in";
	const char *fname_plt0			= PATH"ciaout0.plt";
	const char *finname_template	= PATH"ciaout%d.plt";
	const char *foutname_template	= PATH"ciaout%04d";

	CDynamicMesh mesh(fname_c3d1in);
	mesh.DecideBoundaryVertices(fname_plt0);
	pDynMesh = & mesh;

	mesh.SetExportTextureCoor(true);
//	mesh.SetReconstrucSurface(false);

	for (int i=10; i<101; i++){
#ifdef _DEBUG
		if (i==20){
			int asdag=1;
		}
#endif
		sprintf(infile, finname_template, i);
		sprintf(outfile, foutname_template, i);
		printf("Loading file %s\n", infile);
		mesh.LoadNextTimeStepPlt(i, infile, outfile, m_pGLUIWin);
	}

#undef PATH

}


void CGLWin::_detail_recon(const char* PATH, const int n1, const int n2, const int step)
{
	char infile[256]="", outfile[256]="";
	char fname_c3d1in[512]=""; 
	char fname_plt0[512]="";
	char finname_template[512]="";
	char foutname_template[512]="";

	strcat(fname_c3d1in, PATH);
	strcat(fname_c3d1in, "c3d1.in");
	strcat(fname_plt0, PATH);
	strcat(fname_plt0, "ciaout0.plt");
	strcat(finname_template, PATH);
	strcat(finname_template, "ciaout%d.plt");
	strcat(foutname_template, PATH);
	strcat(foutname_template, "ciaout%04d");

	CDynamicMesh mesh(fname_c3d1in);
	pDynMesh = & mesh;
	mesh.SetExportTextureCoor(true);
	mesh.SetReconstrucSurface(false);
	mesh.DecideBoundaryVertices(fname_plt0);

	for (int i=n1; i<n2; i+=step){
		sprintf(infile, finname_template, i);
		sprintf(outfile, foutname_template, i);
		printf("Loading file %s\n", infile);
		mesh.LoadNextTimeStepPlt(i, infile, outfile, m_pGLUIWin);
	}
}


static void _extract_path(const char *startfname, const int c, char *PATH)
{
	int pos=c;
	for (int i=c-1; i>=0; i--){
		if (startfname[i]=='/' || startfname[i]=='\\'){
			pos = i; break;
		}
	}
	if (pos==c){
		PATH[0]='/', PATH[1]=0;
	}
	else{
		for (int i=0; i<=pos; i++) 
			PATH[i]=startfname[i];
		PATH[pos+1]=0;
	}
}

void CGLWin::detail_recon_anyobj(void)
{
	static char startfname[256]="";
	static char endfname[256]="";
	static char outputfname[256]="";
	static int counter =1;
	const int nn = m_pGLUIWin->m_SceneGraph.size();
	if (nn==0) return;

    CAnimControlDialogEx diag(this, "Reconstruction Dialog",
				startfname,
				endfname,
				&counter, 
				outputfname);
    if (diag.exec() != QDialog::Accepted)
		return;

	//until this dialog return;
	if (startfname[0]==0 || endfname[0]==0 || counter<=0)
		return;

	//found the max common name 
	const int c=_find_commstrlen(startfname, endfname);
	int n1, n2;
	sscanf(&startfname[c], "%d", &n1);
	sscanf(&endfname[c], "%d", &n2);
	char PATH[512];
	_extract_path(startfname, c, PATH);
	
    QObject::connect(m_pGLUIWin, SIGNAL(printMessage(const QString &)), statusBar(), SLOT(message(const QString &)));
    statusBar()->message("Converting files...");	// Initial message
	_detail_recon(PATH, n1, n2, counter);
    statusBar()->message("Done.", 2000);			// Final message for 2 seconds
}


void CGLWin::updateObjectList(void)
{
	if (m_pGLUIWin==NULL || m_pListViews==NULL) 
		return;

	m_pListViews->update(m_pGLUIWin->m_SceneGraph);
}

void CGLWin::dupe_currentobj()
{
    if (m_pGLUIWin==NULL || m_pGLUIWin->m_SceneGraph.GetActiveSceneNode()==NULL){
        return;
    }
    CSceneNode *pnode = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
    CPolyObj *pobj = pnode->m_pObject;
    Vector3d lowleft, upright;
    pobj->ComputeBoundingBox(lowleft, upright);
    const Vector3d objsize = upright - lowleft;
    const double *matrix = pnode->m_pFrame->matrix();
    Vector3d step = objsize;

    const int N = 9;
    CPolyObj * newobj = NULL;
    const int nv = pobj->m_nVertexCount;
    const Vector3d *pvert = pobj->m_pVertex;
    const int ntri = pobj->m_nPolygonCount;

    if (pobj->m_nPlyVertexCount == 3){
        const Vector3i *ptri = (const Vector3i*)pobj->m_pPolygon;
        CTriangleObj * ptriobj = dynamic_cast<CTriangleObj*>(pobj);
        if (ptriobj != NULL){
            Vector3d *p2 = new Vector3d[N*nv];
            Vector3d *p3 = p2;
            for (int j=0; j<N; j++, step+=objsize, p3+=nv){
                for (int i=0; i<nv; i++){
            		TransformVertex3dToVertex3d(pvert[i], matrix, &p3[i].x);
                    p3[i].x += step.x;
                }
            }
            Vector3i *ptri2, *ptri3;
            ptri2 = ptri3 = new Vector3i[ntri*N];
            Vector3i offset(0, 0, 0);
            for (int j=0; j<N; j++, ptri3+=ntri, offset+=Vector3i(nv)){
                for (int i=0; i<ntri; i++)
                    ptri3[i] = ptri[i] + offset;
            }
            newobj = new CTriangleObj(p2, nv*N, ptri2, ntri*N, false);
        }
    }
    else if (pobj->m_nPlyVertexCount == 4){
            /*
            CQuadObj * ptriobj = dynamic_cast<CQuadObj*>(pobj);
            if (ptriobj!=NULL){
                newobj = new CQuadObj(*ptriobj);
            }
            */
    }

    if (newobj != NULL){
        newobj->ComputeBoundingBox(lowleft, upright);
        newobj->SetBoundingBox(AxisAlignedBox(lowleft, upright));        
        CSceneNode *p = AddOneObjectToSceneGraph(newobj, m_pGLUIWin->m_SceneGraph);
        p->m_DrawParms.m_bUseDispList = true;
        p->m_DrawParms.m_bUseVbo = true;
        p->m_DrawParms.m_bEnableSmoothing = true;
    }
    
}


void CGLWin::dupe_currentobjN()
{
    if (m_pGLUIWin==NULL || m_pGLUIWin->m_SceneGraph.GetActiveSceneNode()==NULL){
        return;
    }
    CSceneNode *pnode = m_pGLUIWin->m_SceneGraph.GetActiveSceneNode();
    CPolyObj *pobj = pnode->m_pObject;
    Vector3d lowleft, upright;
    pobj->ComputeBoundingBox(lowleft, upright);
    const Vector3d objsize = upright - lowleft;
    const double *matrix = pnode->m_pFrame->matrix();
    Vector3d step = objsize;

    QString text = "1";
    /*
    bool ok;
	text = QInputDialog::getText("Input object number", 
            "Please input number of objects to copy, must be [1..100]",
			QLineEdit::Normal, text, &ok );
 	//if ( !ok ) return;
    int N = text.toInt();
    */
    int N = 9;
    //this->statusBar()->message(QString("Input is: ") + text, 3000);
    //if (N<1 || N>100) return;

    const int nv = pobj->m_nVertexCount;
    const Vector3d *pvert = pobj->m_pVertex;
    const int ntri = pobj->m_nPolygonCount;
    CPolyObj * newobj = NULL;

    for (int k=0; k<N; k++, step+=objsize){
        if (pobj->m_nPlyVertexCount == 3){
            const Vector3i *ptri = (const Vector3i*)pobj->m_pPolygon;
            CTriangleObj * ptriobj = dynamic_cast<CTriangleObj*>(pobj);
            if (ptriobj != NULL){
                Vector3d *p2 = new Vector3d[nv];
                for (int i=0; i<nv; i++){
            		 TransformVertex3dToVertex3d(pvert[i], matrix, &p2[i].x);
                     p2[i].x += step.x;
                }
                Vector3i *ptri2 = new Vector3i[ntri];
                for (int i=0; i<ntri; i++) ptri2[i] = ptri[i];
                newobj = new CTriangleObj(p2, nv, ptri2, ntri, false);
            }
        }
        else if (pobj->m_nPlyVertexCount == 4){
        }
        if (newobj != NULL){
            newobj->ComputeBoundingBox(lowleft, upright);
            newobj->SetBoundingBox(AxisAlignedBox(lowleft, upright));        
            CSceneNode *p = AddOneObjectToSceneGraph(newobj, m_pGLUIWin->m_SceneGraph);
            p->m_DrawParms.m_bUseDispList = true;
            p->m_DrawParms.m_bUseVbo = true;
            p->m_DrawParms.m_bEnableSmoothing = true;
        }
    }    
}