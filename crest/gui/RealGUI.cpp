
#include <qlayout.h>
#include <QFileInfo>
#include <view3dn/scenegraph.h>
#include <view3dn/cia3dobj.h>
#include <crest/simulation/truss_thread.h>
#include <double_buffer.h>
#include "RealGUI.h"

#ifdef QT_MODULE_QT3SUPPORT
#include "q3filedialog.h"
#include "qstatusbar.h"
#include "q3dockwindow.h"
#define WIDTH_OFFSET 2
#define HEIGHT_OFFSET 2
#else
#include "qlistview.h"
#include "qcheckbox.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "qlabel.h"
#include "qstatusbar.h"
#include "qfiledialog.h"
#include "qheader.h"
#include "qimage.h"
#include "qdockwindow.h"
#include "qspinbox.h"
#define WIDTH_OFFSET 0
#define HEIGHT_OFFSET 0
#endif


#ifndef QT_MODULE_QT3SUPPORT
typedef QListView Q3ListView;
typedef QListViewItem Q3ListViewItem;
typedef QFileDialog Q3FileDialog;
typedef QDockWindow Q3DockWindow;
#endif

#include "iconnode.xpm"

static const int iconWidth=8;
static const int iconHeight=10;
static const int iconMargin=6;


static QImage* classIcons = NULL;
//static const int iconMargin=7;
//static const int iconHeight=20;

//mapping; mmapping; constraint; iff; ffield; topology; mass; mstate; 
//solver; bmodel; vmodel; cmodel; pipeline; context; object; gnode;
static const int iconPos[16]=
{
	0,	// node
	1,	// object
	2,	// context
	6,	// bmodel
	4,	// cmodel
	8,	// mstate
	13, // constraint
	12, // iff
	11, // ff
	7,	// solver
	3,	// pipeline
	14, // mmap
	15, // map
	9,	// mass
	10, // topo
	5,	// vmodel
};

static QImage icons[16];

static int hexval(char c)
{
	if (c>='0' && c<='9') return c-'0';
	else if (c>='a' && c<='f') return (c-'a')+10;
	else if (c>='A' && c<='F') return (c-'A')+10;
	else return 0;
}

static QPixmap* getPixmap(GNode *obj)
{
	return NULL;
}

static QPixmap* getPixmap(cia3d::core::Base* obj)
{
	/*
	//using namespace sofa::simulation::tree::Colors;
	unsigned int flags=0;

	if (dynamic_cast<core::objectmodel::BaseNode*>(obj))
	{
		static QPixmap pixNode((const char**)iconnode_xpm);
		return &pixNode;
		//flags |= 1 << NODE;
	}
	else if (dynamic_cast<core::objectmodel::BaseObject*>(obj))
	{
		if (dynamic_cast<core::objectmodel::ContextObject*>(obj))
			flags |= 1 << CONTEXT;
		if (dynamic_cast<core::BehaviorModel*>(obj))
			flags |= 1 << BMODEL;
		if (dynamic_cast<core::CollisionModel*>(obj))
			flags |= 1 << CMODEL;
		if (dynamic_cast<core::componentmodel::behavior::BaseMechanicalState*>(obj))
			flags |= 1 << MMODEL;
		if (dynamic_cast<core::componentmodel::behavior::BaseConstraint*>(obj))
			flags |= 1 << CONSTRAINT;
		if (dynamic_cast<core::componentmodel::behavior::InteractionForceField*>(obj) &&
		    dynamic_cast<core::componentmodel::behavior::InteractionForceField*>(obj)->getMechModel1()!=dynamic_cast<core::componentmodel::behavior::InteractionForceField*>(obj)->getMechModel2())
			flags |= 1 << IFFIELD;
		else if (dynamic_cast<core::componentmodel::behavior::BaseForceField*>(obj))
			flags |= 1 << FFIELD;
		if (dynamic_cast<core::componentmodel::behavior::OdeSolver*>(obj))
			flags |= 1 << SOLVER;
		if (dynamic_cast<core::componentmodel::collision::Pipeline*>(obj)
			|| dynamic_cast<core::componentmodel::collision::Intersection*>(obj)
			|| dynamic_cast<core::componentmodel::collision::Detection*>(obj)
			|| dynamic_cast<core::componentmodel::collision::ContactManager*>(obj)
			|| dynamic_cast<core::componentmodel::collision::CollisionGroupManager*>(obj))
			flags |= 1 << COLLISION;
		if (dynamic_cast<core::componentmodel::behavior::BaseMechanicalMapping*>(obj))
			flags |= 1 << MMAPPING;
		else if (dynamic_cast<core::BaseMapping*>(obj))
			flags |= 1 << MAPPING;
		if (dynamic_cast<core::componentmodel::behavior::BaseMass*>(obj))
			flags |= 1 << MASS;
		if (dynamic_cast<core::componentmodel::topology::Topology *>(obj))
			flags |= 1 << TOPOLOGY;
		if (dynamic_cast<core::VisualModel*>(obj) && !flags)
			flags |= 1 << VMODEL;
		if (!flags)
			flags |= 1 << OBJECT;
	}
	else return NULL;

	static std::map<unsigned int, QPixmap*> pixmaps;
	if (!pixmaps.count(flags))
	{
		int nc = 0;
		for (int i=0;i<16;i++)
			if (flags & (1<<i))
				++nc;
		int nx = 2+iconWidth*nc+iconMargin;
		QImage * img = new QImage(nx,iconHeight,32);
		img->setAlphaBuffer(true);
		img->fill(qRgba(0,0,0,0));
		// Workaround for qt 3.x where fill() does not set the alpha channel
		for (int y=0 ; y < iconHeight ; y++)
			for (int x=0 ; x < nx ; x++)
				img->setPixel(x,y,qRgba(0,0,0,0));
		
		for (int y=0 ; y < iconHeight ; y++)
			img->setPixel(0,y,qRgba(0,0,0,255));
		nc = 0;
		for (int i=0;i<16;i++)
			if (flags & (1<<i))
			{
				int x0 = 1+iconWidth*nc;
				int x1 = x0+iconWidth-1;
				//QColor c(COLOR[i]);
				const char* color = COLOR[i];
				//c.setAlpha(255);
				int r = (hexval(color[1])*16+hexval(color[2]));
				int g = (hexval(color[3])*16+hexval(color[4]));
				int b = (hexval(color[5])*16+hexval(color[6]));
				int a = 255;
				for (int x=x0; x <=x1 ; x++)
				{
					img->setPixel(x,0,qRgba(0,0,0,255));
					img->setPixel(x,iconHeight-1,qRgba(0,0,0,255));
					for (int y=1 ; y < iconHeight-1 ; y++)
						//img->setPixel(x,y,c.value());
						img->setPixel(x,y,qRgba(r,g,b,a));
				}
				//bitBlt(img,nimg*(iconWidth+2),0,classIcons,iconMargin,iconPos[i],iconWidth,iconHeight);
				++nc;
			}
		for (int y=0 ; y < iconHeight ; y++)
			img->setPixel(2+iconWidth*nc-1,y,qRgba(0,0,0,255));
		pixmaps[flags] = new QPixmap(*img);
		delete img;
	}
	
	return pixmaps[flags]; */
	return NULL;
}


//using sofa::simulation::tree::Simulation;
//using sofa::simulation::tree::MutationListener;

// TODO: Move this code somewhere else

class GraphListenerQListView // :public MutationListener
{
public:
	Q3ListView* widget;
	std::map<cia3d::core::Base*, Q3ListViewItem* > items;

	GraphListenerQListView(Q3ListView* w): widget(w)
	{
	}

	Q3ListViewItem* createItem(Q3ListViewItem* parent)
	{
		Q3ListViewItem* last = parent->firstChild();
		if (last == NULL)
			return new Q3ListViewItem(parent);
		while (last->nextSibling()!=NULL)
			last = last->nextSibling();
		return new Q3ListViewItem(parent, last);
	}

	void addChild(GNode* parent, GNode* child)
	{/*
		if (items.count(child)) return;
		Q3ListViewItem* item;
		if (parent == NULL)
			item = new Q3ListViewItem(widget);
		else if (items.count(parent))
			item = createItem(items[parent]);
		else{
			//std::cerr << "Graph -> QT ERROR: Unknown parent node "<<parent->getName()<<std::endl;
			return;
		}
		if (std::string(child->getName(),0,7) != "default")
			item->setText(0, child->getName().c_str());
		QPixmap* pix = getPixmap(child);
		if (pix)
			item->setPixmap(0, *pix);
		item->setOpen(true);
		items[child] = item;
		// Add all objects and grand-children
		//MutationListener::addChild(parent, child);
		*/
	}

	void removeChild(GNode* parent, GNode* child)
	{/*
		//MutationListener::removeChild(parent, child);
		if (items.count(child))
		{
			delete items[child];
			items.erase(child);
		}
		*/
	}

	void moveChild(GNode* previous, GNode* parent, GNode* child)
	{
		/*
		if (!items.count(child) || !items.count(previous))
		{
			addChild(parent, child);
		}
		else if (!items.count(parent))
		{
			removeChild(previous, child);
		}
		else
		{
			Q3ListViewItem* itemChild = items[child];
			Q3ListViewItem* itemPrevious = items[previous];
			Q3ListViewItem* itemParent = items[parent];
			itemPrevious->takeItem(itemChild);
			itemParent->insertItem(itemChild);
		}
		*/
	}

	void addObject(GNode* parent, cia3d::core::BaseObject* object)
	{/*
		if (items.count(object)) return;
		Q3ListViewItem* item;
		if (items.count(parent))
			item = createItem(items[parent]);
		else
		{
			//std::cerr << "Graph -> QT ERROR: Unknown parent node "<<parent->getName()<<std::endl;
			return;
		}
		std::string name;// = sofa::helper::gettypename(typeid(*object));
		std::string::size_type pos = name.find('<');
		if (pos != std::string::npos)
			name.erase(pos);
		
		if (std::string(object->getName(),0,7) != "default"){
			name += "  ";
			name += object->getName();
		}
		
		item->setText(0, name.c_str());
		QPixmap* pix = getPixmap(object);
		if (pix)
			item->setPixmap(0, *pix);
		items[object] = item;
		*/
	}

	void removeObject(GNode*, cia3d::core::BaseObject* object)
	{
		/*
		if (items.count(object))
		{
			delete items[object];
			items.erase(object);
		}
		*/
	}

	void moveObject(GNode* previous, GNode* parent, cia3d::core::BaseObject* object)
	{
		/*
		if (!items.count(object) || !items.count(previous))
		{
			addObject(parent, object);
		}
		else if (!items.count(parent))
		{
			removeObject(previous, object);
		}
		else
		{
			Q3ListViewItem* itemObject = items[object];
			Q3ListViewItem* itemPrevious = items[previous];
			Q3ListViewItem* itemParent = items[parent];
			itemPrevious->takeItem(itemObject);
			itemParent->insertItem(itemObject);
		}*/
	}
};

RealGUI::RealGUI(const char* filename, const bool use_docked_windows): 
	graphListener(NULL), 
	m_DrawParms(), 
	m_SceneGraph(), 
	m_LaptoolSceneGraph(),
	m_BgSceneGraph()
{
	//=================================
    QLabel* fpsLabel = new QLabel("9999.9 FPS", this);
    fpsLabel->setAlignment(Qt::AlignRight);
    fpsLabel->setMinimumSize(fpsLabel->sizeHint());
    fpsLabel->clear();

    QLabel* timeLabel = new QLabel("T: 999.9999 s", this);
    timeLabel->setAlignment(Qt::AlignLeft);
    timeLabel->setMinimumSize(timeLabel->sizeHint());
    timeLabel->clear();

    statusBar()->addWidget(fpsLabel);
    statusBar()->addWidget(timeLabel);

	//=================================
    setScene(&m_DrawParms, &m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, filename, true);

	//signal connection
    connect(viewer, SIGNAL(reload()), this, SLOT(fileReload()));
    connect(viewer, SIGNAL(newFPS(const QString&)), fpsLabel, SLOT(setText(const QString&)));
    connect(viewer, SIGNAL(newTime(const QString&)), timeLabel, SLOT(setText(const QString&)));
    connect( dumpStateCheckBox, SIGNAL( toggled(bool) ), viewer, SLOT( dumpState(bool) ) );
    connect( exportGnuplotFilesCheckbox, SIGNAL(toggled(bool)), viewer, SLOT(setExportGnuplot(bool)) );
    connect( displayComputationTimeCheckBox, SIGNAL( toggled(bool) ), viewer, SLOT( displayComputationTime(bool) ) );
    //connect( xmlSave_pushButton, SIGNAL( pressed() ), this, SLOT( saveXML() ) );

    connect( runTetCSGAction, SIGNAL(activated()), this, SLOT(runTetCSG()) );
    connect( showEntireSceneAction, SIGNAL(activated()), this, SLOT(showEntireScene()) );
    connect( setViewerWinSizeAction, SIGNAL(activated()), this, SLOT(setViewerWinSize()) );
    connect(fileNewAction, SIGNAL(activated()), this, SLOT(fileNew()));
	
	// display size selectors
	connect(viewer, SIGNAL(resizeW(int)), sizeW, SLOT(setValue(int)));
	connect(viewer, SIGNAL(resizeH(int)), sizeH, SLOT(setValue(int)));
	viewer->updateGL();
}

//clean the buffers and object for quit/destruction
void RealGUI::clearAll(void)
{
	m_SceneGraph.RemoveAll();
	m_BgSceneGraph.RemoveAll();
	m_LaptoolSceneGraph.RemoveAll();
}

RealGUI::~RealGUI()
{
	clearAll();
}

void RealGUI::showEntireScene(void)
{
	//std::cout<< "Show entire scene!\n";
	AxisAlignedBox box1, box2, box;

	const int n1 = m_SceneGraph.SceneNodeCount();
	const int n2 = m_BgSceneGraph.SceneNodeCount();
	m_SceneGraph.GetBoundingBox(box1);
	m_BgSceneGraph.GetBoundingBox(box2);
	//box2 = box1;
	box = Union(box1, box2);
	if (n1+n2==0){
		box.minp = Vector3d(-0.5, -0.5, -0.5);
		box.maxp = Vector3d(0.5, 0.5, 0.5);		
	}

	const qglviewer::Vec minp(box.minp.x, box.minp.y, box.minp.z);
	const qglviewer::Vec maxp(box.maxp.x, box.maxp.y, box.maxp.z);
	viewer->setSceneBoundingBox(minp, maxp);
	viewer->setSceneRadius(Distance(box.minp, box.maxp)*1);
	viewer->showEntireScene();
}

void RealGUI::runTetCSG(void)
{
	//function defined in tetracsg.cpp
	void loadCSGScene(CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, CGLDrawParms &draw);

	loadCSGScene(&m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, m_DrawParms);
	setScene(&m_DrawParms, &m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, false);
	CSceneNode *p=m_LaptoolSceneGraph.GetActiveSceneNode();
	if (p==NULL){
		const int nnode = m_LaptoolSceneGraph.SceneNodeCount();
		p=m_LaptoolSceneGraph.GetSceneNode(nnode-1);
	}
	if (p!=NULL)
		viewer->setManipulatedFrame(p->m_pFrame);
	else
		viewer->setManipulatedFrame(NULL);
	showEntireScene();
}

void RealGUI::setScene(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, 
	const char* filename, bool resetView)
{
	/*
	if (viewer->getScene()!=NULL)
	{
		Simulation::unload(viewer->getScene());
		if (graphListener!=NULL)
			delete graphListener;
		graphView->clear();
	}
	*/
	setTitle(filename);
	viewer->setScene(pDraw, pSG, pLaptoolSG, pBackgroundSG);

	/*
	showVisual->setChecked(groot->getShowVisualModels());
	showBehavior->setChecked(groot->getShowBehaviorModels());
	showCollision->setChecked(groot->getShowCollisionModels());
	showBoundingCollision->setChecked(groot->getShowBoundingCollisionModels());
	showForceField->setChecked(groot->getShowForceFields());
	showInteractionForceField->setChecked(groot->getShowInteractionForceFields());
	showMapping->setChecked(groot->getShowMappings());
	showMechanicalMapping->setChecked(groot->getShowMechanicalMappings());
	showWireFrame->setChecked(groot->getShowWireFrame());
	showNormals->setChecked(groot->getShowNormals());
	startButton->setOn(groot->getContext()->getAnimate());
	dtEdit->setText(QString::number(groot->getDt()));

	graphView->setSorting(-1);
	//graphView->setTreeStepSize(10);
	graphView->header()->hide();`
	//dumpGraph(groot, new Q3ListViewItem(graphView));
	graphListener = new GraphListenerQListView(graphView);
	graphListener->addChild(NULL, groot);
*/
	if (showWireFrame->isChecked())
		pDraw->m_nDrawType = CGLDrawParms::DRAW_MESH_HIDDENLINE;
	else
		pDraw->m_nDrawType = CGLDrawParms::DRAW_MESH_SHADING;

	const bool drawTexture = showNormals->isChecked();
	pDraw->m_bEnableTexture2D = drawTexture;

	//if (resetView) showEntireScene();
}

void RealGUI::fileNew()
{
	clearAll();
}

void RealGUI::fileOpen(const char* filename, bool resetView)
{
	/*
	const int strlength = strlen(filename);
	if (strlength==0) return;
	CPolyObj *p = NULL;
	bool deformable_flag = true;

	{// load xml file
		const int b1= strcmp(filename+(strlength-4), ".xml");
		const int b2= strcmp(filename+(strlength-4), ".XML");
		if (b1==0 || b2==0){
			//return _loadPltFile(filename);
			goto FILE_ALREADY_LOADED;
		}
	}
	{// read Nuetral mesh file, generated by Netgen
		const int b1= strcmp(filename+(strlength-5), ".mesh");
		const int b2= strcmp(filename+(strlength-5), ".MESH");
		if (b1==0 || b2==0){
			p = loadNeutralMesh(filename);
			deformable_flag = true;
			goto FILE_ALREADY_LOADED;
		}
	}
	{//read polygonal objects
		CTriangleObj *pTriobj = new CTriangleObj;
		assert(pTriobj!=NULL);
		if (pTriobj->LoadFile(filename)){
			p = pTriobj;
			deformable_flag = false;
			goto FILE_ALREADY_LOADED;
		}
		else
			SafeDelete(pTriobj);
	}

FILE_ALREADY_LOADED:
	//Append into the object buffer
	if (p == NULL){
		qFatal("Failed to load %s",filename);
		return;
	}
	p->SetObjectName(filename);

	//Append into the scenegraph
	CSceneNode* pnode = NULL;
	if (deformable_flag){
		pnode = AddOneObjectToSceneGraph(p, m_SceneGraph);
	}
	else{
		pnode = AddOneObjectToSceneGraph(p, m_BgSceneGraph);
	}

	setScene(&m_DrawParms, &m_SceneGraph, &m_BgSceneGraph, filename, resetView);
	*/

}

void RealGUI::fileOpen()
{
	void loadXMLScene(const char *strfilename, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, CGLDrawParms &draw);
//    std::string filename = "c:/users/nanzhang/project/crest/data/*.xml";
    std::string filename = "./*.xml";
    QString s = Q3FileDialog::getOpenFileName(
        filename.c_str(),
        "Scenes (*.xml)",
        this,
        "open file dialog",
        "Choose a scene file" 
	);

	if (!s.isEmpty()){
		loadXMLScene(s.ascii(), &m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, m_DrawParms);
	    setScene(&m_DrawParms, &m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, false);	
		CSceneNode *p=m_LaptoolSceneGraph.GetActiveSceneNode();
		if (p==NULL){
			const int nnode = m_LaptoolSceneGraph.SceneNodeCount();
			p=m_LaptoolSceneGraph.GetSceneNode(nnode-1);
		}
		if (p!=NULL)
			viewer->setManipulatedFrame(p->m_pFrame);
		else
			viewer->setManipulatedFrame(NULL);
		showEntireScene();
	}
}

void RealGUI::fileReload()
{
    std::string filename = viewer->getSceneFileName();
    if (!filename.empty()){
        fileOpen(filename.c_str(), false);
    }
	else{
		std::cerr << "Reload failed: no file loaded.\n";
	}
}

void RealGUI::fileSaveAs()
{
    std::string filename = viewer->getSceneFileName();
    QString s = Q3FileDialog::getSaveFileName(
        filename.empty()?NULL:filename.c_str(),
        "Scenes (*.scn)",
        this,
        "save file dialog",
        "Choose where the scene will be saved" );
    if (s.length()>0)
        fileSaveAs(s);
}

void RealGUI::fileSaveAs(const char* filename)
{
//    Simulation::printXML( viewer->getScene(), filename);
}

void RealGUI::fileExit()
{
    clearAll();
	exit(0);
}

void RealGUI::saveXML()
{
    //Simulation::printXML( viewer->getScene(), "scene.scn");
}

void RealGUI::setTitle( const char* windowTitle )
{
	std::string str = "UMN Crest Simulation Platform";
	if (windowTitle && *windowTitle){
		str += " - ";
		str += windowTitle;
	}
#ifdef _WIN32
    setWindowTitle(str.c_str());
#else
    setCaption(str.c_str());
#endif
}


void RealGUI::playpause(bool value)
{
	static int count = 0;
	enum QThread::Priority prior[3]={
			QThread::HighPriority, 
			QThread::HighestPriority,
			QThread::TimeCriticalPriority};

	typedef cia3d::simulation::CDeformationThread CDeformationThread;
	CDeformationThread *pDefInst = CDeformationThread::getInstance();
	CDeformationThread &defThread = *pDefInst;
	if (defThread.isFinished()){
		std::cout << "Thread has been finished!\n";
		return;
	}

	//QString filepath("e:/demo/realtime/*");
	QString filepath("./*");
	if (value){//button is down
		if (defThread.wait(2)==true){  //Thread not started yet!
			extern int USE_INTERNAL_SIMULATOR;
			QString s("Internal simulator");
			if (!USE_INTERNAL_SIMULATOR){
				s = Q3FileDialog::getOpenFileName(
					//sceneFileName.c_str(),
					filepath,
					"Scenes (*.in)",
					this,
					"Open file dialog",
					"Choose where the scene will be opened" );
				if (s.isEmpty()) return;
				QString path = QFileInfo(s).absolutePath();
				QDir::setCurrent(path);
			}
			defThread.start(prior[0]);
		    setScene(&m_DrawParms, &m_SceneGraph, &m_LaptoolSceneGraph, &m_BgSceneGraph, s);
			count = 1;
		}
		else{
			//let the thread to run
			std::cout<<"Thread is continuing...\n";
			defThread.continueRunning();
		}
	}
	else{//button is up
		if (defThread.isRunning()){
			std::cout<<"Thread is paused...\n";
			defThread.pauseRunning();
			return;
		}
	}
}

void RealGUI::stepAnimation(void)
{
	std::cout<<"Thread runs ONE step...\n";
	typedef cia3d::simulation::CDeformationThread CDeformationThread;
	CDeformationThread *pDefInst = CDeformationThread::getInstance();
	CDeformationThread &defThread = *pDefInst;
	defThread.singleStep();
}


void RealGUI::saveSnapshot(void)
{
	if (viewer==NULL) 
		return;

	const bool automatic = false;
	const bool overwrite = false;
	viewer->setSnapshotQuality(100);
	viewer->saveSnapshot(automatic, overwrite);
}


void RealGUI::saveCamera(void)
{
	QString saveDirPath;

	//extern void ExportPovrayCamera(Camera* pcamera, FILE *fp); //in scenegraph.cpp
	QString selectedfilter = ".xml";
	if (viewer==NULL) return;
    QString fname = Q3FileDialog::getSaveFileName(
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
		//ExportPovrayCamera(m_pGLUIWin->camera(), stdout); 

		//write file
		if (hasext)
			viewer->saveToFile(fname);
		else
			viewer->saveToFile(fname+selectedfilter);
	}
}


void RealGUI::loadCamera(void)
{
	if (viewer==NULL) return;
	QString loadDirPath;

    QString fname = Q3FileDialog::getOpenFileName(
						loadDirPath,
						"Camera state file (*.xml)",
						this,
						"Open camera file dialog",
						"Choose a camera file to open");
    if ( !fname.isEmpty()){
		viewer->setStateFileName(fname);
		viewer->restoreStateFromFile();
		viewer->updateGL();  	
	}
}

bool nExportObj = false;
void RealGUI::exportGraph(void)
{
	if (nExportObj){
		nExportObj = false;
		printf("Output stoped!!...\n");
	}
	else{
		nExportObj = true;
		printf("Output started!!...\n");
	}

	/*
	int buffersize=0;
	static int count=0;
	const int nsize = m_SceneGraph.SceneNodeCount();
	if (nsize==0) return;

	extern cia3d::simulation::CDeformationThread defThread;
	extern CDualBuffer< vector<Vector3d> > defElmVertexBuffer;	//Note: space required by Intel C++ 11.0
	const bool runthread = defThread.isRunning();
	Vector3d *pvertex = NULL;
	if (runthread){	//defeormation threading accessing Critical Data
		defElmVertexBuffer.lock();
		//defElmVertexBuffer.sync();
		vector<Vector3d> *pElmVerticesBuffer = defElmVertexBuffer.getFrontBuffer();
		vector<Vector3d> & elmVerticesBuffer = *pElmVerticesBuffer;
		pvertex = & elmVerticesBuffer[0];
		buffersize = elmVerticesBuffer.size();
	}

	int nv = 0;
	for (int i=0; i<nsize; i++){
		char fname[512];
		Vector3d *pbak;
		sprintf(fname, "obj-%d-%d.stl", count, i);
		CSceneNode *pnode = m_SceneGraph.GetSceneNode(i);
		assert(pnode!=NULL);
		CPolyObj *pobj = pnode->m_pObject;		//Visual polygonal object
		if (pobj==NULL){
			pobj = pnode->m_pSimulationObject;	//Simulation solid, will drive the visual one
			pbak = pobj->m_pVertex;

		}
		else{
			pbak = pobj->m_pVertex;
			assert(pnode->m_pMapping!=NULL);
			
		}
		
		if (pobj==NULL)
			pobj = pnode->m_pCollisionObject;	//Collision object, since CIA3D doesnot support, we use it for laptools
			
		if (pobj==NULL) continue;
		const double *matrix=NULL;
		if (pnode->m_pFrame) 
			matrix = pnode->m_pFrame->matrix();

		if (pvertex){
			pobj->m_pVertex = &pvertex[nv];
			nv += pobj->m_nVertexCount;
		}
		bool r = CPolyObjWriterFactory::GetInstance()->savePolygonMesh(pobj, fname, matrix);
		pobj->m_pVertex = pbak;
		if (r)
			printf("Write %s OK!\n", fname);
		else
			printf("Write %s FAILED!!\n", fname);
	}
	count++;

	if (runthread){	//defeormation threading accessing Critical Data
		defElmVertexBuffer.unlock();
	}
	*/
}
