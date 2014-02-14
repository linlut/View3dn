#include "GUI.h"

#include <QInputDialog>
#include <qvariant.h>
#include "QtViewer.h"


void convertImage(QImage& q, const int cx, const int cy, const int cz)
{
	q.setAlphaBuffer(true);
	int w = q.width();
	int h = q.height();
	for (int y=0; y<h; y++){
		uint *p = (uint *)q.scanLine(y);
		for (int x=0; x<w; x++){
			QRgb rgb = q.pixel(x, y);
			int r = qRed(rgb);
			QRgb rgb2 = qRgba (cx, cy, cz, r);
			*(p+x) = rgb2;
		}
	}
}

void createBleedingImages(void)
{
	QString pathname("C:/ams09/media/materials/textures/bloodLoop/");
	QString fname("bloodLoopFRAME ");
	for (int i=0; i<90; i++){
		char buff[20];
		sprintf(buff, "%d.png", i+1);
		QString filename = pathname + fname + buff;
		QImage q;
		q.load(filename);
		convertImage(q, 255, 0, 0);

		sprintf(buff, "test_%d.png", i);		
		QString filename2 = pathname + buff;
		q.save(filename2, "png", 100);
	}
}


void createBubbleImages(void)
{
	//QString pathname("C:/ams09/media/materials/textures/bubbleAlphaFRAMES/");
	QString pathname("C:/ams09/media/materials/textures/moreCameraBubblesFRAMES/");

	QString fname("moreCameraBubblesFRAME.");
	for (int i=0; i<90; i++){
		char buff[20];
		sprintf(buff, "%d.png", i+40);
		QString filename = pathname + fname + buff;
		QImage q;
		q.load(filename);
		convertImage(q, 255, 255, 255);

		sprintf(buff, "test_%d.png", i);		
		QString filename2 = pathname + buff;
		q.save(filename2, "png", 100);
	}
}

void convertImage(QImage& c, QImage& a)
{
	c.setAlphaBuffer(true);
	int w = c.width();
	int h = c.height();
	for (int y=0; y<h; y++){
		uint *p = (uint *)c.scanLine(y);
		for (int x=0; x<w; x++){
			QRgb rgb = c.pixel(x, y);
			QRgb alpha = a.pixel(x, y);
			int r = qRed(rgb);
			int g = qGreen(rgb);
			int b = qBlue(rgb);
			int k = qRed(alpha);
			QRgb rgb2 = qRgba (r, g, b, k);
			*(p+x) = rgb2;
		}
	}
}

void createFiberImages(void)
{
	QString pathname("E:/incoming/danBurke/ams7_09/");
	QString fname1("fiber709_COLOR.png");
	QString fname2("fiber709_TRANS.png");

	QImage c, a;
	c.load(pathname+fname1);
	a.load(pathname+fname2);
	convertImage(c, a);
	c.save(pathname+"fiberall.png", "png", 100);
}


void createLaserImages(void)
{
	QString pathname("C:/ams09/media/materials/textures/fireColorFRAMES/");
	QString fname("fireColorFRAME ");
	for (int i=0; i<90; i++){
		char buff[20];
		sprintf(buff, "%d.png", i+1);
		QString filename = pathname + fname + buff;
		QImage q;
		q.load(filename);
		//convertImage(q);
		sprintf(buff, "test_%d.png", i);		
		QString filename2 = pathname + buff;
		q.save(filename2, "png", 100);
	}
}


void Ui_GUI::setupUi(Q3MainWindow *GUI)
{
    GUI->setObjectName(QString::fromUtf8("GUI"));
    GUI->resize(QSize(886, 542).expandedTo(GUI->minimumSizeHint()));
    GUI->setMinimumSize(QSize(251, 542));
    fileNewAction = new QAction(GUI);
    fileNewAction->setObjectName(QString::fromUtf8("fileNewAction"));
    fileNewAction->setName("fileNewAction");
    fileNewAction->setIcon(icon(image2_ID));
    fileOpenAction = new QAction(GUI);
    fileOpenAction->setObjectName(QString::fromUtf8("fileOpenAction"));
    fileOpenAction->setName("fileOpenAction");
    fileOpenAction->setIcon(icon(image3_ID));
    fileReloadAction = new QAction(GUI);
    fileReloadAction->setObjectName(QString::fromUtf8("fileReloadAction"));
    fileReloadAction->setName("fileReloadAction");
    fileSaveAction = new QAction(GUI);
    fileSaveAction->setObjectName(QString::fromUtf8("fileSaveAction"));
    fileSaveAction->setName("fileSaveAction");
    fileSaveAction->setEnabled(false);
    fileSaveAction->setIcon(icon(image4_ID));
    fileSaveAsAction = new QAction(GUI);
    fileSaveAsAction->setObjectName(QString::fromUtf8("fileSaveAsAction"));
    fileSaveAsAction->setName("fileSaveAsAction");
    filePrintAction = new QAction(GUI);
    filePrintAction->setObjectName(QString::fromUtf8("filePrintAction"));
    filePrintAction->setName("filePrintAction");
    filePrintAction->setEnabled(false);
    filePrintAction->setIcon(icon(image5_ID));
    fileExitAction = new QAction(GUI);
    fileExitAction->setObjectName(QString::fromUtf8("fileExitAction"));
    fileExitAction->setName("fileExitAction");

    editUndoAction = new QAction(GUI);
    editUndoAction->setObjectName(QString::fromUtf8("editUndoAction"));
    editUndoAction->setName("editUndoAction");
    editUndoAction->setEnabled(false);
    editUndoAction->setIcon(icon(image6_ID));
    editRedoAction = new QAction(GUI);
    editRedoAction->setObjectName(QString::fromUtf8("editRedoAction"));
    editRedoAction->setName("editRedoAction");
    editRedoAction->setEnabled(false);
    editRedoAction->setIcon(icon(image7_ID));
    editCutAction = new QAction(GUI);
    editCutAction->setObjectName(QString::fromUtf8("editCutAction"));
    editCutAction->setName("editCutAction");
    editCutAction->setEnabled(false);
    editCutAction->setIcon(icon(image8_ID));
    editCopyAction = new QAction(GUI);
    editCopyAction->setObjectName(QString::fromUtf8("editCopyAction"));
    editCopyAction->setName("editCopyAction");
    editCopyAction->setEnabled(false);
    editCopyAction->setIcon(icon(image9_ID));
    editPasteAction = new QAction(GUI);
    editPasteAction->setObjectName(QString::fromUtf8("editPasteAction"));
    editPasteAction->setName("editPasteAction");
    editPasteAction->setEnabled(false);
    editPasteAction->setIcon(icon(image10_ID));
    editFindAction = new QAction(GUI);
    editFindAction->setObjectName(QString::fromUtf8("editFindAction"));
    editFindAction->setName("editFindAction");
    editFindAction->setEnabled(false);
    editFindAction->setIcon(icon(image11_ID));
    helpContentsAction = new QAction(GUI);
    helpContentsAction->setObjectName(QString::fromUtf8("helpContentsAction"));
    helpContentsAction->setName("helpContentsAction");
    helpContentsAction->setEnabled(false);
    helpIndexAction = new QAction(GUI);
    helpIndexAction->setObjectName(QString::fromUtf8("helpIndexAction"));
    helpIndexAction->setName("helpIndexAction");
    helpIndexAction->setEnabled(false);
    helpAboutAction = new QAction(GUI);
    helpAboutAction->setObjectName(QString::fromUtf8("helpAboutAction"));
    helpAboutAction->setName("helpAboutAction");
    helpAboutAction->setEnabled(false);
    exportGraphAction = new QAction(GUI);
    exportGraphAction->setObjectName(QString::fromUtf8("exportGraphAction"));
    exportGraphAction->setName("exportGraphAction");
    exportGraphAction->setIcon(icon(image0_ID));

	//Nan Zhang added
    showEntireSceneAction = new QAction(GUI);
    showEntireSceneAction->setObjectName(QString::fromUtf8("showEntireSceneAction"));
    showEntireSceneAction->setName("showEntireSceneAction");

    runTetCSGAction = new QAction(GUI);
    runTetCSGAction->setObjectName(QString::fromUtf8("runTetCSGAction"));
    runTetCSGAction->setName("runTetCSGAction");

    setViewerWinSizeAction = new QAction(GUI);
    setViewerWinSizeAction->setObjectName(QString::fromUtf8("setViewerWinSizeAction"));
    setViewerWinSizeAction->setName("setViewerWinSize");

    //exportGraphAction->setIcon(icon(image0_ID));

    widget = new QWidget(GUI);
    widget->setObjectName(QString::fromUtf8("widget"));
    vboxLayout = new QVBoxLayout(widget);
    vboxLayout->setSpacing(2);
    vboxLayout->setMargin(5);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    splitter2 = new QSplitter(widget);
    splitter2->setObjectName(QString::fromUtf8("splitter2"));
    splitter2->setOrientation(Qt::Horizontal);
    layout5 = new QWidget(splitter2);
    layout5->setObjectName(QString::fromUtf8("layout5"));
    gridLayout = new QGridLayout(layout5);
    gridLayout->setSpacing(2);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    SaveViewButton = new QPushButton(layout5);
    SaveViewButton->setObjectName(QString::fromUtf8("SaveViewButton"));
    SaveViewButton->setEnabled(true);

    gridLayout->addWidget(SaveViewButton, 2, 1, 1, 1);

    tabs = new QTabWidget(layout5);
    tabs->setObjectName(QString::fromUtf8("tabs"));
    tabView = new QWidget();
    tabView->setObjectName(QString::fromUtf8("tabView"));
    vboxLayout1 = new QVBoxLayout(tabView);
    vboxLayout1->setSpacing(2);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    showVisual = new QCheckBox(tabView);
    showVisual->setObjectName(QString::fromUtf8("showVisual"));
    showVisual->setChecked(true);

    vboxLayout1->addWidget(showVisual);

    showBehavior = new QCheckBox(tabView);
    showBehavior->setObjectName(QString::fromUtf8("showBehavior"));
    showBehavior->setChecked(true);

    vboxLayout1->addWidget(showBehavior);

    showCollision = new QCheckBox(tabView);
    showCollision->setObjectName(QString::fromUtf8("showCollision"));
    showCollision->setChecked(true);

    vboxLayout1->addWidget(showCollision);

    showBoundingCollision = new QCheckBox(tabView);
    showBoundingCollision->setObjectName(QString::fromUtf8("showBoundingCollision"));
    showBoundingCollision->setChecked(true);

    vboxLayout1->addWidget(showBoundingCollision);

    showMapping = new QCheckBox(tabView);
    showMapping->setObjectName(QString::fromUtf8("showMapping"));
    showMapping->setChecked(true);
    vboxLayout1->addWidget(showMapping);

    showMechanicalMapping = new QCheckBox(tabView);
    showMechanicalMapping->setObjectName(QString::fromUtf8("showMechanicalMapping"));
    showMechanicalMapping->setChecked(true);
    vboxLayout1->addWidget(showMechanicalMapping);

    showForceField = new QCheckBox(tabView);
    showForceField->setObjectName(QString::fromUtf8("showForceField"));
    showForceField->setChecked(true);
    vboxLayout1->addWidget(showForceField);

    showInteractionForceField = new QCheckBox(tabView);
    showInteractionForceField->setObjectName(QString::fromUtf8("showInteractionForceField"));
    showInteractionForceField->setChecked(true);
    vboxLayout1->addWidget(showInteractionForceField);

    spacerItem = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    vboxLayout1->addItem(spacerItem);

    showWireFrame = new QCheckBox(tabView);
    showWireFrame->setObjectName(QString::fromUtf8("showWireFrame"));
    showWireFrame->setEnabled(true);
    showWireFrame->setChecked(false);
    vboxLayout1->addWidget(showWireFrame);

    showNormals = new QCheckBox(tabView);
    showNormals->setObjectName(QString::fromUtf8("showNormals"));
    showNormals->setChecked(true);
    vboxLayout1->addWidget(showNormals);

    spacerItem1 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    vboxLayout1->addItem(spacerItem1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(2);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    spacerItem2 = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem2);

    sizeW = new QSpinBox(tabView);
    sizeW->setObjectName(QString::fromUtf8("sizeW"));
    sizeW->setMaximum(3000);
    hboxLayout->addWidget(sizeW);

    textLabel_sizeX = new QLabel(tabView);
    textLabel_sizeX->setObjectName(QString::fromUtf8("textLabel_sizeX"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(textLabel_sizeX->sizePolicy().hasHeightForWidth());
    textLabel_sizeX->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(textLabel_sizeX);

    sizeH = new QSpinBox(tabView);
    sizeH->setObjectName(QString::fromUtf8("sizeH"));
    sizeH->setMaximum(3000);

    hboxLayout->addWidget(sizeH);

    spacerItem3 = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem3);


    vboxLayout1->addLayout(hboxLayout);

    spacerItem4 = new QSpacerItem(20, 90, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout1->addItem(spacerItem4);

    tabs->addTab(tabView, QApplication::translate("GUI", "View", 0, QApplication::UnicodeUTF8));
    TabStats = new QWidget();
    TabStats->setObjectName(QString::fromUtf8("TabStats"));
    vboxLayout2 = new QVBoxLayout(TabStats);
    vboxLayout2->setSpacing(2);
    vboxLayout2->setMargin(0);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    dumpStateCheckBox = new QCheckBox(TabStats);
    dumpStateCheckBox->setObjectName(QString::fromUtf8("dumpStateCheckBox"));

    vboxLayout2->addWidget(dumpStateCheckBox);

    displayComputationTimeCheckBox = new QCheckBox(TabStats);
    displayComputationTimeCheckBox->setObjectName(QString::fromUtf8("displayComputationTimeCheckBox"));

    vboxLayout2->addWidget(displayComputationTimeCheckBox);

    exportGnuplotFilesCheckbox = new QCheckBox(TabStats);
    exportGnuplotFilesCheckbox->setObjectName(QString::fromUtf8("exportGnuplotFilesCheckbox"));

    vboxLayout2->addWidget(exportGnuplotFilesCheckbox);

    spacerItem5 = new QSpacerItem(20, 24, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout2->addItem(spacerItem5);

    tabs->addTab(TabStats, QApplication::translate("GUI", "Stats", 0, QApplication::UnicodeUTF8));
    TabGraph = new QWidget();
    TabGraph->setObjectName(QString::fromUtf8("TabGraph"));
    vboxLayout3 = new QVBoxLayout(TabGraph);
    vboxLayout3->setSpacing(2);
    vboxLayout3->setMargin(0);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    ExportGraphButton = new QPushButton(TabGraph);
    ExportGraphButton->setObjectName(QString::fromUtf8("ExportGraphButton"));
    ExportGraphButton->setIcon(icon(image0_ID));

    vboxLayout3->addWidget(ExportGraphButton);

    graphView = new Q3ListView(TabGraph);
    graphView->addColumn(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    graphView->header()->setClickEnabled(false, graphView->header()->count() - 1);
    graphView->header()->setResizeEnabled(false, graphView->header()->count() - 1);
    graphView->setObjectName(QString::fromUtf8("graphView"));
    graphView->setRootIsDecorated(true);
    graphView->setTreeStepSize(15);

    vboxLayout3->addWidget(graphView);

    tabs->addTab(TabGraph, QApplication::translate("GUI", "Graph", 0, QApplication::UnicodeUTF8));
    tabHelp = new QWidget();
    tabHelp->setObjectName(QString::fromUtf8("tabHelp"));
    vboxLayout4 = new QVBoxLayout(tabHelp);
    vboxLayout4->setSpacing(2);
    vboxLayout4->setMargin(0);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    textEdit1 = new Q3TextEdit(tabHelp);
    textEdit1->setObjectName(QString::fromUtf8("textEdit1"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(7));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(textEdit1->sizePolicy().hasHeightForWidth());
    textEdit1->setSizePolicy(sizePolicy1);
    textEdit1->setMinimumSize(QSize(120, 0));
    textEdit1->setMaximumSize(QSize(32767, 32767));
    textEdit1->setWordWrap(Q3TextEdit::WidgetWidth);
    textEdit1->setReadOnly(true);

    vboxLayout4->addWidget(textEdit1);

    tabs->addTab(tabHelp, QApplication::translate("GUI", "Help", 0, QApplication::UnicodeUTF8));

    gridLayout->addWidget(tabs, 4, 0, 1, 2);

    stepButton = new QPushButton(layout5);
    stepButton->setObjectName(QString::fromUtf8("stepButton"));
    stepButton->setAutoRepeat(true);

    gridLayout->addWidget(stepButton, 0, 1, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(2);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    dtLabel = new QLabel(layout5);
    dtLabel->setObjectName(QString::fromUtf8("dtLabel"));

    hboxLayout1->addWidget(dtLabel);

    dtEdit = new QLineEdit(layout5);
    dtEdit->setObjectName(QString::fromUtf8("dtEdit"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(dtEdit->sizePolicy().hasHeightForWidth());
    dtEdit->setSizePolicy(sizePolicy2);
    dtEdit->setMaximumSize(QSize(80, 32767));

    hboxLayout1->addWidget(dtEdit);


    gridLayout->addLayout(hboxLayout1, 1, 1, 1, 1);

    screenshotButton = new QPushButton(layout5);
    screenshotButton->setObjectName(QString::fromUtf8("screenshotButton"));

    gridLayout->addWidget(screenshotButton, 3, 0, 1, 2);

    ResetSceneButton = new QPushButton(layout5);
    ResetSceneButton->setObjectName(QString::fromUtf8("ResetSceneButton"));
    ResetSceneButton->setEnabled(true);

    gridLayout->addWidget(ResetSceneButton, 1, 0, 1, 1);

    ResetViewButton = new QPushButton(layout5);
    ResetViewButton->setObjectName(QString::fromUtf8("ResetViewButton"));
    ResetViewButton->setEnabled(true);

    gridLayout->addWidget(ResetViewButton, 2, 0, 1, 1);

    startButton = new QPushButton(layout5);
    startButton->setObjectName(QString::fromUtf8("startButton"));
    startButton->setCheckable(true);

    gridLayout->addWidget(startButton, 0, 0, 1, 1);

    splitter2->addWidget(layout5);
    viewer = new QtViewer(splitter2);
    viewer->setObjectName(QString::fromUtf8("viewer"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy3.setHorizontalStretch(100);
    sizePolicy3.setVerticalStretch(1);
    sizePolicy3.setHeightForWidth(viewer->sizePolicy().hasHeightForWidth());
    viewer->setSizePolicy(sizePolicy3);
    viewer->setMinimumSize(QSize(0, 0));
    viewer->setCursor(QCursor(static_cast<Qt::CursorShape>(2)));
    viewer->setMouseTracking(true);
    viewer->setFocusPolicy(Qt::StrongFocus);
    splitter2->addWidget(viewer);

    vboxLayout->addWidget(splitter2);

    GUI->setCentralWidget(widget);
    menubar = new QMenuBar(GUI);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setEnabled(true);
    fileMenu = new QMenu(menubar);
    fileMenu->setObjectName(QString::fromUtf8("fileMenu"));
    editMenu = new QMenu(menubar);
    editMenu->setObjectName(QString::fromUtf8("editMenu"));
    viewMenu = new QMenu(menubar);
    viewMenu->setObjectName(QString::fromUtf8("viewMenu"));
    helpMenu = new QMenu(menubar);
    helpMenu->setObjectName(QString::fromUtf8("helpMenu"));

    fileMenu->addAction(fileNewAction);
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileReloadAction);
    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileSaveAsAction);
    fileMenu->addAction(exportGraphAction);
    fileMenu->addSeparator();
    fileMenu->addAction(filePrintAction);
    fileMenu->addSeparator();
    fileMenu->addAction(fileExitAction);
    editMenu->addAction(editUndoAction);
    editMenu->addAction(editRedoAction);
    editMenu->addSeparator();
    editMenu->addAction(editCutAction);
    editMenu->addAction(editCopyAction);
    editMenu->addAction(editPasteAction);
    editMenu->addSeparator();
    editMenu->addAction(editFindAction);

	//View menu add actions
    viewMenu->addAction(runTetCSGAction);
    viewMenu->addSeparator();
    viewMenu->addAction(showEntireSceneAction);
    viewMenu->addAction(setViewerWinSizeAction);
    viewMenu->addSeparator();

    helpMenu->addAction(helpContentsAction);
    helpMenu->addAction(helpIndexAction);
    helpMenu->addSeparator();
    helpMenu->addAction(helpAboutAction);

	//add menus into the bar
    menubar->addAction(fileMenu->menuAction());
    menubar->addAction(editMenu->menuAction());
    menubar->addAction(viewMenu->menuAction());
    menubar->addAction(helpMenu->menuAction());
    retranslateUi(GUI);

    QMetaObject::connectSlotsByName(GUI);
} // setupUi


void Ui_GUI::retranslateUi(Q3MainWindow *GUI)
{
    GUI->setWindowTitle(QApplication::translate("GUI", "UMN CREST Simulator", 0, QApplication::UnicodeUTF8));
	//Nan Zhang added new actions
    showEntireSceneAction->setIconText(QApplication::translate("GUI", "Show Entire Scene", 0, QApplication::UnicodeUTF8));
    showEntireSceneAction->setText(QApplication::translate("GUI", "&Show Entire Scene", 0, QApplication::UnicodeUTF8));
    setViewerWinSizeAction->setIconText(QApplication::translate("GUI", "Set Viewer Size", 0, QApplication::UnicodeUTF8));
    setViewerWinSizeAction->setText(QApplication::translate("GUI", "&Set Viewer Size", 0, QApplication::UnicodeUTF8));
    runTetCSGAction->setIconText(QApplication::translate("GUI", "Run Tetrahedron CSG", 0, QApplication::UnicodeUTF8));
    runTetCSGAction->setText(QApplication::translate("GUI", "&Run Tetrahedron CSG", 0, QApplication::UnicodeUTF8));

    //showEntireSceneAction->setShortcut(QApplication::translate("GUI", "Ctrl+N", 0, QApplication::UnicodeUTF8));

    fileNewAction->setIconText(QApplication::translate("GUI", "New", 0, QApplication::UnicodeUTF8));
    fileNewAction->setText(QApplication::translate("GUI", "&New", 0, QApplication::UnicodeUTF8));
    fileNewAction->setShortcut(QApplication::translate("GUI", "Ctrl+N", 0, QApplication::UnicodeUTF8));
    fileOpenAction->setIconText(QApplication::translate("GUI", "Open", 0, QApplication::UnicodeUTF8));
    fileOpenAction->setText(QApplication::translate("GUI", "&Open...", 0, QApplication::UnicodeUTF8));
    fileOpenAction->setShortcut(QApplication::translate("GUI", "Ctrl+O", 0, QApplication::UnicodeUTF8));
    fileReloadAction->setIconText(QApplication::translate("GUI", "Reload", 0, QApplication::UnicodeUTF8));
    fileReloadAction->setText(QApplication::translate("GUI", "&Reload", 0, QApplication::UnicodeUTF8));
    fileReloadAction->setShortcut(QApplication::translate("GUI", "Ctrl+R", 0, QApplication::UnicodeUTF8));
    fileSaveAction->setIconText(QApplication::translate("GUI", "Save", 0, QApplication::UnicodeUTF8));
    fileSaveAction->setText(QApplication::translate("GUI", "&Save", 0, QApplication::UnicodeUTF8));
    fileSaveAction->setShortcut(QApplication::translate("GUI", "Ctrl+S", 0, QApplication::UnicodeUTF8));
    fileSaveAsAction->setIconText(QApplication::translate("GUI", "Save As", 0, QApplication::UnicodeUTF8));
    fileSaveAsAction->setText(QApplication::translate("GUI", "Save &As...", 0, QApplication::UnicodeUTF8));
    fileSaveAsAction->setShortcut(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    filePrintAction->setIconText(QApplication::translate("GUI", "Print", 0, QApplication::UnicodeUTF8));
    filePrintAction->setText(QApplication::translate("GUI", "&Print...", 0, QApplication::UnicodeUTF8));
    filePrintAction->setShortcut(QApplication::translate("GUI", "Ctrl+P", 0, QApplication::UnicodeUTF8));
    fileExitAction->setIconText(QApplication::translate("GUI", "Exit", 0, QApplication::UnicodeUTF8));
    fileExitAction->setText(QApplication::translate("GUI", "E&xit", 0, QApplication::UnicodeUTF8));
    fileExitAction->setShortcut(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    editUndoAction->setIconText(QApplication::translate("GUI", "Undo", 0, QApplication::UnicodeUTF8));
    editUndoAction->setText(QApplication::translate("GUI", "&Undo", 0, QApplication::UnicodeUTF8));
    editUndoAction->setShortcut(QApplication::translate("GUI", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
    editRedoAction->setIconText(QApplication::translate("GUI", "Redo", 0, QApplication::UnicodeUTF8));
    editRedoAction->setText(QApplication::translate("GUI", "&Redo", 0, QApplication::UnicodeUTF8));
    editRedoAction->setShortcut(QApplication::translate("GUI", "Ctrl+Y", 0, QApplication::UnicodeUTF8));
    editCutAction->setIconText(QApplication::translate("GUI", "Cut", 0, QApplication::UnicodeUTF8));
    editCutAction->setText(QApplication::translate("GUI", "&Cut", 0, QApplication::UnicodeUTF8));
    editCutAction->setShortcut(QApplication::translate("GUI", "Ctrl+X", 0, QApplication::UnicodeUTF8));
    editCopyAction->setIconText(QApplication::translate("GUI", "Copy", 0, QApplication::UnicodeUTF8));
    editCopyAction->setText(QApplication::translate("GUI", "C&opy", 0, QApplication::UnicodeUTF8));
    editCopyAction->setShortcut(QApplication::translate("GUI", "Ctrl+C", 0, QApplication::UnicodeUTF8));
    editPasteAction->setIconText(QApplication::translate("GUI", "Paste", 0, QApplication::UnicodeUTF8));
    editPasteAction->setText(QApplication::translate("GUI", "&Paste", 0, QApplication::UnicodeUTF8));
    editPasteAction->setShortcut(QApplication::translate("GUI", "Ctrl+V", 0, QApplication::UnicodeUTF8));
    editFindAction->setIconText(QApplication::translate("GUI", "Find", 0, QApplication::UnicodeUTF8));
    editFindAction->setText(QApplication::translate("GUI", "&Find...", 0, QApplication::UnicodeUTF8));
    editFindAction->setShortcut(QApplication::translate("GUI", "Ctrl+F", 0, QApplication::UnicodeUTF8));
    helpContentsAction->setIconText(QApplication::translate("GUI", "Contents", 0, QApplication::UnicodeUTF8));
    helpContentsAction->setText(QApplication::translate("GUI", "&Contents...", 0, QApplication::UnicodeUTF8));
    helpContentsAction->setShortcut(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    helpIndexAction->setIconText(QApplication::translate("GUI", "Index", 0, QApplication::UnicodeUTF8));
    helpIndexAction->setText(QApplication::translate("GUI", "&Index...", 0, QApplication::UnicodeUTF8));
    helpIndexAction->setShortcut(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    helpAboutAction->setIconText(QApplication::translate("GUI", "About", 0, QApplication::UnicodeUTF8));
    helpAboutAction->setText(QApplication::translate("GUI", "&About", 0, QApplication::UnicodeUTF8));
    helpAboutAction->setShortcut(QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    exportGraphAction->setIconText(QApplication::translate("GUI", "Export &Graph...", 0, QApplication::UnicodeUTF8));
    exportGraphAction->setText(QApplication::translate("GUI", "Export &Graph...", 0, QApplication::UnicodeUTF8));
    SaveViewButton->setText(QApplication::translate("GUI", "Save Vie&w", 0, QApplication::UnicodeUTF8));
    SaveViewButton->setShortcut(QApplication::translate("GUI", "Alt+W", 0, QApplication::UnicodeUTF8));
    showVisual->setText(QApplication::translate("GUI", "Visual Model", 0, QApplication::UnicodeUTF8));
    showBehavior->setText(QApplication::translate("GUI", "Behavior Model", 0, QApplication::UnicodeUTF8));
    showCollision->setText(QApplication::translate("GUI", "Collision Model", 0, QApplication::UnicodeUTF8));
    showBoundingCollision->setText(QApplication::translate("GUI", "Bounding Tree", 0, QApplication::UnicodeUTF8));
    showMapping->setText(QApplication::translate("GUI", "Mapping", 0, QApplication::UnicodeUTF8));
    showMechanicalMapping->setText(QApplication::translate("GUI", "Mechanical Mapping", 0, QApplication::UnicodeUTF8));
    showForceField->setText(QApplication::translate("GUI", "Force Field", 0, QApplication::UnicodeUTF8));
    showInteractionForceField->setText(QApplication::translate("GUI", "Interaction", 0, QApplication::UnicodeUTF8));
    showWireFrame->setText(QApplication::translate("GUI", "Wire Frame", 0, QApplication::UnicodeUTF8));
    showNormals->setText(QApplication::translate("GUI", "Textures", 0, QApplication::UnicodeUTF8));
    textLabel_sizeX->setText(QApplication::translate("GUI", "<p align=\"center\">x</p>", 0, QApplication::UnicodeUTF8));
    tabs->setTabText(tabs->indexOf(tabView), QApplication::translate("GUI", "View", 0, QApplication::UnicodeUTF8));
    dumpStateCheckBox->setText(QApplication::translate("GUI", "Dump State", 0, QApplication::UnicodeUTF8));
    dumpStateCheckBox->setProperty("toolTip", QVariant(QApplication::translate("GUI", "record the state at each time step in file \"dumpState.data\"", "This is used to plut curves fter a simulation", QApplication::UnicodeUTF8)));
    displayComputationTimeCheckBox->setText(QApplication::translate("GUI", "Log Time", 0, QApplication::UnicodeUTF8));
    exportGnuplotFilesCheckbox->setText(QApplication::translate("GUI", "export state as gnuplot files", 0, QApplication::UnicodeUTF8));
    tabs->setTabText(tabs->indexOf(TabStats), QApplication::translate("GUI", "Stats", 0, QApplication::UnicodeUTF8));
    ExportGraphButton->setText(QApplication::translate("GUI", "E&xport Graph...", 0, QApplication::UnicodeUTF8));
    ExportGraphButton->setShortcut(QApplication::translate("GUI", "Alt+X", 0, QApplication::UnicodeUTF8));
    graphView->header()->setLabel(0, QApplication::translate("GUI", "", 0, QApplication::UnicodeUTF8));
    tabs->setTabText(tabs->indexOf(TabGraph), QApplication::translate("GUI", "Graph", 0, QApplication::UnicodeUTF8));

	textEdit1->setText(QApplication::translate("GUI", "-----\n"
		"TO NAVIGATE: use MOUSE.\n"
		"\n"
		"", 0, QApplication::UnicodeUTF8));

    tabs->setTabText(tabs->indexOf(tabHelp), QApplication::translate("GUI", "Help", 0, QApplication::UnicodeUTF8));
    stepButton->setText(QApplication::translate("GUI", "S&TEP", 0, QApplication::UnicodeUTF8));
    stepButton->setShortcut(QApplication::translate("GUI", "Alt+T", 0, QApplication::UnicodeUTF8));
    dtLabel->setText(QApplication::translate("GUI", "DT:", 0, QApplication::UnicodeUTF8));
    screenshotButton->setText(QApplication::translate("GUI", "Save &Screenshot", 0, QApplication::UnicodeUTF8));
    screenshotButton->setShortcut(QApplication::translate("GUI", "Alt+S", 0, QApplication::UnicodeUTF8));
    ResetSceneButton->setText(QApplication::translate("GUI", "&Reset Scene", 0, QApplication::UnicodeUTF8));
    ResetSceneButton->setShortcut(QApplication::translate("GUI", "Alt+R", 0, QApplication::UnicodeUTF8));
    ResetViewButton->setText(QApplication::translate("GUI", "&Load View", 0, QApplication::UnicodeUTF8));
    ResetViewButton->setShortcut(QApplication::translate("GUI", "Alt+L", 0, QApplication::UnicodeUTF8));
    startButton->setText(QApplication::translate("GUI", "&ANIM", 0, QApplication::UnicodeUTF8));
    startButton->setShortcut(QApplication::translate("GUI", "Alt+A", 0, QApplication::UnicodeUTF8));

    fileMenu->setTitle(QApplication::translate("GUI", "&File", 0, QApplication::UnicodeUTF8));
    editMenu->setTitle(QApplication::translate("GUI", "&Edit", 0, QApplication::UnicodeUTF8));
    viewMenu->setTitle(QApplication::translate("GUI", "&View", 0, QApplication::UnicodeUTF8));
    helpMenu->setTitle(QApplication::translate("GUI", "&Help", 0, QApplication::UnicodeUTF8));
} // retranslateUi



/*
 *  Constructs a GUI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
GUI::GUI(QWidget* parent, const char* name, Qt::WFlags fl)
    : Q3MainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(fileNewAction, SIGNAL(activated()), this, SLOT(fileNew()));
    connect(fileOpenAction, SIGNAL(activated()), this, SLOT(fileOpen()));
    connect(fileReloadAction, SIGNAL(activated()), this, SLOT(fileReload()));
    connect(fileSaveAction, SIGNAL(activated()), this, SLOT(fileSave()));
    connect(fileSaveAsAction, SIGNAL(activated()), this, SLOT(fileSaveAs()));
    connect(filePrintAction, SIGNAL(activated()), this, SLOT(filePrint()));
    connect(fileExitAction, SIGNAL(activated()), this, SLOT(fileExit()));
    connect(editUndoAction, SIGNAL(activated()), this, SLOT(editUndo()));
    connect(editRedoAction, SIGNAL(activated()), this, SLOT(editRedo()));
    connect(editCutAction, SIGNAL(activated()), this, SLOT(editCut()));
    connect(editCopyAction, SIGNAL(activated()), this, SLOT(editCopy()));
    connect(editPasteAction, SIGNAL(activated()), this, SLOT(editPaste()));
    connect(editFindAction, SIGNAL(activated()), this, SLOT(editFind()));
    connect(helpIndexAction, SIGNAL(activated()), this, SLOT(helpIndex()));
    connect(helpContentsAction, SIGNAL(activated()), this, SLOT(helpContents()));
    connect(helpAboutAction, SIGNAL(activated()), this, SLOT(helpAbout()));
    connect(startButton, SIGNAL(toggled(bool)), this, SLOT(playpause(bool)));
    connect(ResetSceneButton, SIGNAL(clicked()), viewer, SLOT(resetScene()));
    connect(dtEdit, SIGNAL(textChanged(const QString&)), viewer, SLOT(setDt(const QString&)));
    connect(ResetSceneButton, SIGNAL(clicked()), viewer, SLOT(resetScene()));
    connect(ResetViewButton, SIGNAL(clicked()), this, SLOT(loadCamera()));
    connect(SaveViewButton, SIGNAL(clicked()), this, SLOT(saveCamera()));

    connect(showVisual, SIGNAL(toggled(bool)), viewer, SLOT(showVisual(bool)));
    connect(showBehavior, SIGNAL(toggled(bool)), viewer, SLOT(showBehavior(bool)));
    connect(showCollision, SIGNAL(toggled(bool)), viewer, SLOT(showCollision(bool)));
    connect(showBoundingCollision, SIGNAL(toggled(bool)), viewer, SLOT(showBoundingCollision(bool)));
    connect(showMapping, SIGNAL(toggled(bool)), viewer, SLOT(showMapping(bool)));
    connect(showMechanicalMapping, SIGNAL(toggled(bool)), viewer, SLOT(showMechanicalMapping(bool)));
    connect(showForceField, SIGNAL(toggled(bool)), viewer, SLOT(showForceField(bool)));
    connect(showInteractionForceField, SIGNAL(toggled(bool)), viewer, SLOT(showInteractionForceField(bool)));
    connect(showWireFrame, SIGNAL(toggled(bool)), viewer, SLOT(showWireFrame(bool)));
    connect(showNormals, SIGNAL(toggled(bool)), viewer, SLOT(showNormals(bool)));
    connect(stepButton, SIGNAL(clicked()), this, SLOT(stepAnimation()));
    connect(screenshotButton, SIGNAL(clicked()), this, SLOT(saveSnapshot()));
    connect(ExportGraphButton, SIGNAL(clicked()), this, SLOT(exportGraph()));
    connect(exportGraphAction, SIGNAL(activated()), this, SLOT(exportGraph()));
    connect(sizeW, SIGNAL(valueChanged(int)), viewer, SLOT(setSizeW(int)));
    connect(sizeH, SIGNAL(valueChanged(int)), viewer, SLOT(setSizeH(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
GUI::~GUI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void GUI::languageChange()
{
    retranslateUi(this);
}

void GUI::fileNew()
{
    qWarning("GUI::fileNew(): Not implemented yet");
}

void GUI::fileOpen()
{
    qWarning("GUI::fileOpen(): Not implemented yet");
}

void GUI::fileReload()
{
    qWarning("GUI::fileReload(): Not implemented yet");
}

void GUI::fileSave()
{
    qWarning("GUI::fileSave(): Not implemented yet");
}

void GUI::fileSaveAs()
{
    qWarning("GUI::fileSaveAs(): Not implemented yet");
}

void GUI::filePrint()
{
    qWarning("GUI::filePrint(): Not implemented yet");
}

void GUI::fileExit()
{
    qWarning("GUI::fileExit(): Not implemented yet");
}

void GUI::editUndo()
{
    qWarning("GUI::editUndo(): Not implemented yet");
}

void GUI::editRedo()
{
    qWarning("GUI::editRedo(): Not implemented yet");
}

void GUI::editCut()
{
    qWarning("GUI::editCut(): Not implemented yet");
}

void GUI::editCopy()
{
    qWarning("GUI::editCopy(): Not implemented yet");
}

void GUI::editPaste()
{
    qWarning("GUI::editPaste(): Not implemented yet");
}

void GUI::editFind()
{
    qWarning("GUI::editFind(): Not implemented yet");
}

void GUI::helpIndex()
{
    qWarning("GUI::helpIndex(): Not implemented yet");
}

void GUI::helpContents()
{
    qWarning("GUI::helpContents(): Not implemented yet");
}

void GUI::helpAbout()
{
    qWarning("GUI::helpAbout(): Not implemented yet");
}

void GUI::saveXML()
{
    qWarning("GUI::saveXML(): Not implemented yet");
}

void GUI::showEntireScene(void){}

void GUI::runTetCSG(void){}

void GUI::playpause(bool){}

void GUI::stepAnimation(void){}

void GUI::saveSnapshot(void){}

void GUI::saveCamera(void){}

void GUI::loadCamera(void){}

void GUI::exportGraph(void){}

void GUI::setViewerWinSize(void)
{
	static QString text("640 480");
	int x, y;
	bool ok;
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
