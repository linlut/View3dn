/*
*/

#ifndef _CREST_GUI_H_
#define _CREST_GUI_H_

#include <qvariant.h>

class QtViewer;

#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3MainWindow>
#include <Qt3Support/Q3TextEdit>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QSplitter>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "QtViewer.h"

class Ui_GUI
{
public:
    QAction *fileNewAction;
    QAction *fileOpenAction;
    QAction *fileReloadAction;
    QAction *fileSaveAction;
    QAction *fileSaveAsAction;
    QAction *filePrintAction;
    QAction *fileExitAction;
    QAction *editUndoAction;
    QAction *editRedoAction;
    QAction *editCutAction;
    QAction *editCopyAction;
    QAction *editPasteAction;
    QAction *editFindAction;
    QAction *helpContentsAction;
    QAction *helpIndexAction;
    QAction *helpAboutAction;
    QAction *exportGraphAction;

	//Nan Zhang added some widgets
    QAction *runTetCSGAction;
    QAction *showEntireSceneAction;
    QAction *setViewerWinSizeAction;
    //QAction *exportGraphAction;

    QWidget *widget;
    QVBoxLayout *vboxLayout;
    QSplitter *splitter2;
    QWidget *layout5;
    QGridLayout *gridLayout;
    QPushButton *SaveViewButton;
    QTabWidget *tabs;
    QWidget *tabView;
    QVBoxLayout *vboxLayout1;
    QCheckBox *showVisual;
    QCheckBox *showBehavior;
    QCheckBox *showCollision;
    QCheckBox *showBoundingCollision;
    QCheckBox *showMapping;
    QCheckBox *showMechanicalMapping;
    QCheckBox *showForceField;
    QCheckBox *showInteractionForceField;
    QSpacerItem *spacerItem;
    QCheckBox *showWireFrame;
    QCheckBox *showNormals;
    QSpacerItem *spacerItem1;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem2;
    QSpinBox *sizeW;
    QLabel *textLabel_sizeX;
    QSpinBox *sizeH;
    QSpacerItem *spacerItem3;
    QSpacerItem *spacerItem4;
    QWidget *TabStats;
    QVBoxLayout *vboxLayout2;
    QCheckBox *dumpStateCheckBox;
    QCheckBox *displayComputationTimeCheckBox;
    QCheckBox *exportGnuplotFilesCheckbox;
    QSpacerItem *spacerItem5;
    QWidget *TabGraph;
    QVBoxLayout *vboxLayout3;
    QPushButton *ExportGraphButton;
    Q3ListView *graphView;
    QWidget *tabHelp;
    QVBoxLayout *vboxLayout4;
    Q3TextEdit *textEdit1;
    QPushButton *stepButton;
    QHBoxLayout *hboxLayout1;
    QLabel *dtLabel;
    QLineEdit *dtEdit;
    QPushButton *screenshotButton;
    QPushButton *ResetSceneButton;
    QPushButton *ResetViewButton;
    QPushButton *startButton;
    QtViewer *viewer;
    QMenuBar *menubar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    void setupUi(Q3MainWindow *GUI);

    void retranslateUi(Q3MainWindow *GUI);

protected:
    enum IconID
    {
        image0_ID,
        image1_ID,
        image2_ID,
        image3_ID,
        image4_ID,
        image5_ID,
        image6_ID,
        image7_ID,
        image8_ID,
        image9_ID,
        image10_ID,
        image11_ID,
        unknown_ID
    };
    static QPixmap icon(IconID id)
    {
    static const char* const image0_data[] = { 
"16 16 38 1",
"j c #000000",
"g c #000400",
"z c #080808",
"x c #080c08",
"q c #101010",
"d c #101410",
"b c #181c18",
"w c #202420",
"I c #292829",
"o c #313031",
"m c #313431",
"F c #393c39",
"y c #414041",
"l c #4a484a",
"D c #4a4c4a",
"r c #525052",
"E c #525552",
"v c #5a595a",
"a c #626562",
"c c #6a696a",
"A c #6a6d6a",
"e c #8b8d8b",
"C c #9c999c",
"J c #9c9d9c",
"h c #a4a1a4",
"B c #a4a5a4",
"t c #acaaac",
"k c #acaeac",
"G c #c5c2c5",
"n c #cdcacd",
"H c #cdcecd",
"s c #d5d6d5",
"u c #dedede",
"p c #e6e2e6",
"i c #f6f2f6",
"# c #f6f6f6",
"f c #fffaff",
". c #ffffff",
".....#abba#.....",
".....cdeedc.....",
"....fgh..hgf....",
"....ijk..kji....",
".....lmnnol.....",
".....pqjjqp.....",
".....njhhjn.....",
".....ro..or.....",
"....sjt..tjs....",
"..fuvw....wvuf..",
".txjjy....yjjxt.",
"pzAuBzh..hzBuAzp",
"Cz...vD..Dv...zC",
"Bg#..lE..El..#gB",
"ibFhcjG..GjchFbi",
".HydIJ....JIdyH."};


    static const unsigned char image1_data[] = { 
    0x78, 0x9c, 0x53, 0x4e, 0x49, 0x4d, 0xcb, 0xcc, 0x4b, 0x55, 0x48, 0x29,
    0xcd, 0xcd, 0xad, 0x8c, 0x2f, 0xcf, 0x4c, 0x29, 0xc9, 0x50, 0x30, 0xe0,
    0x52, 0x46, 0x11, 0xcd, 0x48, 0xcd, 0x4c, 0xcf, 0x28, 0x01, 0x0a, 0x17,
    0x97, 0x24, 0x96, 0x64, 0x26, 0x2b, 0x24, 0x67, 0x24, 0x16, 0x41, 0xa5,
    0x92, 0x32, 0x4b, 0x8a, 0xa3, 0x63, 0x15, 0x6c, 0x15, 0xaa, 0xb9, 0x14,
    0x14, 0x6a, 0xad, 0xb9, 0x00, 0x67, 0x11, 0x1b, 0x1f
};

    static const unsigned char image2_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0x86, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xed, 0x95, 0x31, 0x0a, 0x80,
    0x30, 0x0c, 0x45, 0x7f, 0xa5, 0x43, 0xae, 0xd0, 0xdd, 0x25, 0xd0, 0x25,
    0x05, 0x0f, 0xa0, 0x87, 0xf6, 0x1a, 0x2e, 0xde, 0x27, 0x4e, 0x3a, 0xd5,
    0xb6, 0xa4, 0x38, 0x08, 0x7d, 0x90, 0x25, 0xd0, 0x97, 0x4f, 0x08, 0x14,
    0xf8, 0x08, 0x97, 0xe9, 0x69, 0xc7, 0xdb, 0x87, 0x29, 0xd7, 0x54, 0xd5,
    0xd7, 0x02, 0x00, 0x89, 0x52, 0x0d, 0x90, 0x15, 0xd7, 0x10, 0x11, 0x70,
    0xe4, 0xa2, 0xdc, 0x5b, 0xc4, 0x1c, 0x19, 0xe4, 0x09, 0x04, 0xc2, 0x71,
    0x1e, 0x8a, 0xcc, 0x5a, 0x4c, 0x89, 0xb7, 0x75, 0x43, 0x98, 0x43, 0x31,
    0xb9, 0x29, 0x71, 0x5a, 0x12, 0x08, 0x84, 0xdd, 0xef, 0xaf, 0xc9, 0x4d,
    0x62, 0xe7, 0x8a, 0x07, 0x61, 0x13, 0xdf, 0x97, 0x51, 0x1b, 0x66, 0xda,
    0x71, 0x0b, 0x43, 0x3c, 0xc4, 0x43, 0x5c, 0xa0, 0xe7, 0x07, 0x69, 0xf5,
    0xfd, 0x84, 0x0b, 0xdc, 0x13, 0x2a, 0xa6, 0xf1, 0xf6, 0x44, 0xfe, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image3_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xb1, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xed, 0x94, 0xbb, 0x12, 0x02,
    0x21, 0x10, 0x04, 0x7b, 0x29, 0x02, 0xd2, 0xfb, 0x58, 0x23, 0x23, 0xcb,
    0x0f, 0x36, 0x35, 0xb8, 0xaa, 0x35, 0x00, 0x14, 0x0e, 0x0e, 0x81, 0xc2,
    0xcc, 0x49, 0x96, 0x67, 0xef, 0x30, 0x01, 0xf0, 0x57, 0x90, 0x4c, 0xdc,
    0xd1, 0x2f, 0x0c, 0x9d, 0xe1, 0x1e, 0xa1, 0x71, 0x4d, 0x8f, 0xe3, 0x19,
    0xc7, 0x3d, 0x4d, 0xc5, 0xfc, 0x00, 0x0a, 0x80, 0x3d, 0xd9, 0x18, 0x79,
    0x49, 0x2d, 0x1e, 0x04, 0x50, 0xd5, 0xb8, 0x2f, 0x80, 0x22, 0xd2, 0xcd,
    0x3d, 0x3d, 0x68, 0x3f, 0x4d, 0xe5, 0x5d, 0x7d, 0x27, 0x29, 0xd6, 0x47,
    0x9a, 0x9b, 0xc4, 0xf8, 0x60, 0x6d, 0xcb, 0xd4, 0x1c, 0xf7, 0xd5, 0xb6,
    0xaa, 0x19, 0x4f, 0x64, 0x5d, 0xa8, 0x9a, 0x71, 0x0a, 0xbd, 0xdf, 0x60,
    0xb3, 0xf0, 0xd8, 0xc1, 0x59, 0x78, 0xee, 0xed, 0xf9, 0xe5, 0x9a, 0x45,
    0xd1, 0x80, 0x3a, 0x7f, 0x69, 0x73, 0x01, 0xd2, 0x98, 0x47, 0x28, 0x20,
    0x21, 0x8a, 0x75, 0x4e, 0x03, 0x28, 0x77, 0xbc, 0xc2, 0x69, 0x3a, 0x50,
    0xd5, 0x75, 0x4e, 0xa3, 0x8c, 0x0f, 0x64, 0x9d, 0xd3, 0xcc, 0xf1, 0x4a,
    0xa7, 0x05, 0x78, 0x52, 0x2b, 0xbf, 0xdd, 0x3e, 0xbd, 0x00, 0x8c, 0x80,
    0x91, 0xc0, 0x84, 0x8d, 0x35, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image4_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x01,
    0x35, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xb5, 0x95, 0xb1, 0x6a, 0xc3,
    0x30, 0x10, 0x86, 0x3f, 0x17, 0x0f, 0xd7, 0xcd, 0x1a, 0xf3, 0x1c, 0x86,
    0x2e, 0x06, 0x3f, 0x51, 0xf7, 0x4e, 0xa5, 0x84, 0x90, 0xa1, 0x64, 0xed,
    0x13, 0x05, 0xbc, 0x74, 0xcf, 0x33, 0x78, 0x29, 0x38, 0x5b, 0x6e, 0x10,
    0xb4, 0x83, 0x22, 0xc7, 0xb6, 0xe2, 0x48, 0x6e, 0xd3, 0x5b, 0x74, 0x3f,
    0xd2, 0xfd, 0xf7, 0xf3, 0x73, 0x3e, 0xc3, 0x3f, 0x45, 0x76, 0x3e, 0xbf,
    0xef, 0xcd, 0x9b, 0xfb, 0xec, 0xe3, 0x1d, 0x3a, 0x85, 0xd7, 0x37, 0x68,
    0xf6, 0x0d, 0x85, 0x14, 0xa8, 0x2a, 0x08, 0xa8, 0xea, 0x4d, 0x5c, 0x3e,
    0x95, 0x1c, 0x3e, 0x0f, 0xa8, 0x2a, 0x65, 0x5d, 0x02, 0xf0, 0xe0, 0x89,
    0x3b, 0x05, 0x73, 0x6e, 0x23, 0xb9, 0x70, 0xd4, 0x23, 0xe4, 0x8e, 0x24,
    0x86, 0x81, 0x1e, 0xfb, 0xe8, 0x53, 0x23, 0x8e, 0x1c, 0x00, 0x0b, 0x22,
    0x32, 0x52, 0x36, 0xc5, 0x2b, 0xbb, 0xa2, 0xa5, 0xa5, 0x90, 0x02, 0x80,
    0xaa, 0xae, 0x46, 0x5e, 0x8c, 0x14, 0x8b, 0x6f, 0x33, 0x50, 0xa6, 0x5f,
    0x20, 0x6a, 0x02, 0xa5, 0x6d, 0xde, 0x5e, 0x30, 0xb0, 0x59, 0xc3, 0x6e,
    0x7b, 0x85, 0xd8, 0x08, 0xa8, 0x75, 0xb9, 0x5a, 0xa7, 0x0c, 0x0b, 0x46,
    0x1e, 0x51, 0x7b, 0xea, 0xb1, 0x88, 0x8c, 0xee, 0x45, 0x24, 0xa8, 0x4f,
    0xf2, 0xb8, 0xcd, 0x5b, 0x28, 0x34, 0xea, 0xf1, 0xb0, 0x3e, 0x50, 0xdc,
    0xf9, 0x8e, 0x33, 0xca, 0x66, 0xf1, 0xb4, 0x3e, 0xc5, 0xe3, 0xa4, 0xe9,
    0x98, 0xd6, 0xa7, 0x78, 0x9c, 0xa2, 0xfc, 0x9a, 0xc7, 0x7d, 0x0f, 0xef,
    0xd1, 0x66, 0x1d, 0x8e, 0x4e, 0x2c, 0x76, 0xdb, 0xd0, 0xe3, 0x60, 0x8e,
    0x8d, 0x38, 0xf2, 0x21, 0x5e, 0x72, 0xce, 0x7a, 0xec, 0x3b, 0xff, 0x16,
    0xdf, 0x54, 0xfc, 0xfc, 0xb2, 0xc8, 0x09, 0xe0, 0xb2, 0x6b, 0x02, 0xe2,
    0x61, 0xe7, 0x94, 0x68, 0xf6, 0x8d, 0xfb, 0x32, 0xad, 0x52, 0xd5, 0x55,
    0x7c, 0x8e, 0x8d, 0xa4, 0x11, 0x2f, 0x9e, 0xe3, 0x54, 0xc5, 0x8b, 0xe7,
    0x38, 0x55, 0xf1, 0xe2, 0x5d, 0x91, 0xaa, 0x38, 0xb6, 0x2b, 0xb2, 0xc1,
    0xdb, 0x7b, 0xfd, 0x9e, 0xb2, 0xf8, 0x93, 0x3f, 0xc4, 0x0f, 0x2f, 0x80,
    0x18, 0x0c, 0xf3, 0x14, 0xee, 0xcb, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image5_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x03,
    0x7f, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0x8d, 0x94, 0xb1, 0x6f, 0x5c,
    0x45, 0x10, 0xc6, 0x7f, 0x67, 0xbd, 0x62, 0x4e, 0x72, 0xb1, 0x2b, 0x51,
    0xbc, 0xd7, 0xd9, 0x1d, 0x17, 0xd1, 0x38, 0x4e, 0x81, 0x12, 0x28, 0x30,
    0x55, 0x4e, 0xa9, 0x8e, 0x34, 0x24, 0x95, 0xed, 0x54, 0x49, 0xf8, 0x0f,
    0x92, 0x8e, 0x8e, 0x96, 0x12, 0x5a, 0xaa, 0xe0, 0x34, 0x51, 0xe8, 0x8c,
    0x14, 0xc9, 0x17, 0x0a, 0x8e, 0x43, 0x82, 0xc4, 0x45, 0xa4, 0xbc, 0x2b,
    0x50, 0x76, 0x25, 0x2c, 0x76, 0x8a, 0x93, 0xde, 0x14, 0x4f, 0x5a, 0x8a,
    0x77, 0xbe, 0xd8, 0xf8, 0x2c, 0xb1, 0xcd, 0xbc, 0xd1, 0x6a, 0xbf, 0xf7,
    0xcd, 0xf7, 0xcd, 0x4c, 0x8f, 0x4b, 0xce, 0xd1, 0xe1, 0x51, 0x36, 0x8c,
    0xcd, 0x72, 0x13, 0x0c, 0x10, 0x68, 0xac, 0xe9, 0x2e, 0x0d, 0x0c, 0x43,
    0x4d, 0xc1, 0xc0, 0x89, 0x63, 0x7b, 0x67, 0xbb, 0x77, 0xf6, 0x7d, 0x71,
    0x19, 0xb0, 0x61, 0xec, 0x7c, 0xbc, 0x43, 0xd2, 0x84, 0x88, 0xa0, 0xa6,
    0x94, 0x52, 0x62, 0x66, 0x78, 0xe7, 0x49, 0x96, 0xf0, 0xe2, 0xa9, 0x63,
    0x4d, 0xfa, 0x2b, 0x5d, 0x78, 0xdf, 0x5b, 0x81, 0x09, 0xc0, 0x64, 0x3c,
    0xc9, 0x82, 0x80, 0xb0, 0x04, 0xc3, 0xa0, 0xa1, 0xc1, 0xcc, 0x10, 0x91,
    0x25, 0x73, 0x80, 0xed, 0xeb, 0xe7, 0x19, 0xaf, 0x5d, 0x06, 0x1c, 0x34,
    0x60, 0xad, 0x11, 0x35, 0x42, 0x0b, 0x75, 0xac, 0x09, 0x27, 0xe1, 0x5c,
    0xae, 0x73, 0xa5, 0x8e, 0x35, 0xb4, 0x17, 0xdf, 0xaf, 0x64, 0xfc, 0x6a,
    0xfc, 0x2a, 0x47, 0x8d, 0x38, 0xe7, 0x00, 0x38, 0xcb, 0x7c, 0xa1, 0x13,
    0xc8, 0xe2, 0xd3, 0x8c, 0x1b, 0x3b, 0x37, 0x2e, 0xe0, 0xac, 0x64, 0x6c,
    0xad, 0x75, 0x40, 0xf3, 0xce, 0xa0, 0xb3, 0xcc, 0xd5, 0x14, 0x8a, 0x4e,
    0x02, 0x11, 0x21, 0xd9, 0x45, 0x7d, 0xe1, 0x12, 0xf3, 0xb4, 0x55, 0x04,
    0xc1, 0xad, 0xbb, 0xa5, 0xfb, 0x4e, 0xdc, 0xb2, 0x03, 0x96, 0x55, 0x18,
    0xf4, 0xe9, 0xaf, 0x04, 0x5e, 0xc9, 0xb8, 0x74, 0x25, 0x86, 0x61, 0xed,
    0x02, 0xb4, 0x70, 0x4b, 0xa6, 0x6a, 0xba, 0x64, 0x6e, 0xad, 0xd1, 0xd0,
    0xac, 0x04, 0x5e, 0xa9, 0xf1, 0xb3, 0x83, 0x67, 0xf9, 0xca, 0x87, 0x57,
    0x28, 0x3f, 0x28, 0x3b, 0x59, 0x4e, 0x35, 0x3d, 0x13, 0x0d, 0x43, 0x10,
    0xc6, 0xbf, 0x8f, 0xb1, 0xc6, 0xf0, 0xbe, 0xe2, 0xf3, 0xe1, 0x7b, 0xad,
    0xcf, 0x49, 0x31, 0x79, 0x31, 0xc9, 0xd3, 0x37, 0x53, 0xa6, 0x3f, 0x4f,
    0x41, 0x40, 0x44, 0x08, 0x1a, 0xa8, 0x5c, 0x45, 0x88, 0xe7, 0xe3, 0xa9,
    0xc6, 0xb3, 0x7a, 0xc6, 0xf8, 0x97, 0x31, 0xe1, 0xef, 0xc0, 0xfd, 0xaf,
    0xee, 0xe7, 0xe1, 0xcd, 0x21, 0xb7, 0x86, 0xb7, 0x7a, 0xcb, 0x3f, 0x4c,
    0xc6, 0x93, 0xfc, 0x72, 0x3a, 0x85, 0x13, 0x23, 0xcc, 0x13, 0xae, 0x10,
    0xb4, 0x5d, 0xc8, 0xf0, 0x9f, 0x68, 0x05, 0x54, 0x45, 0x45, 0x2a, 0x14,
    0x3d, 0x49, 0x1c, 0xbe, 0x3c, 0xc4, 0x4e, 0x8c, 0x30, 0x0f, 0x5c, 0xff,
    0xe8, 0x2a, 0xa3, 0xdd, 0x51, 0xc7, 0x78, 0xf2, 0x62, 0x92, 0xa7, 0xc7,
    0x53, 0xee, 0xdc, 0x1c, 0x61, 0xd2, 0x19, 0xa3, 0x1a, 0x11, 0x71, 0x24,
    0x53, 0xfc, 0x69, 0x44, 0x48, 0x66, 0x1c, 0xcb, 0x6b, 0x0e, 0xec, 0x07,
    0x86, 0x0c, 0xa9, 0xa6, 0x8e, 0xf1, 0x1f, 0x10, 0x66, 0x01, 0x01, 0xc6,
    0xbf, 0x8e, 0x09, 0xf3, 0xd4, 0x31, 0xfd, 0xee, 0xfb, 0x27, 0x39, 0xff,
    0x93, 0xff, 0xd7, 0x79, 0x97, 0xdf, 0xe5, 0x41, 0x1e, 0x64, 0xc9, 0x92,
    0x07, 0x79, 0x90, 0xdf, 0xbe, 0x7e, 0x9b, 0xbf, 0x79, 0xfc, 0x6d, 0xde,
    0xba, 0xb6, 0x95, 0xcb, 0x72, 0x23, 0x3b, 0xe7, 0xf2, 0x46, 0xb9, 0x91,
    0xd7, 0xea, 0xba, 0x66, 0x7f, 0xef, 0x0b, 0xf0, 0x2b, 0xcd, 0xbd, 0x70,
    0x04, 0x61, 0xc4, 0x67, 0x0c, 0x18, 0x30, 0x62, 0xc4, 0xe6, 0x60, 0x93,
    0xfd, 0xdd, 0x3d, 0x86, 0x9f, 0x0e, 0x01, 0x50, 0xd5, 0xce, 0x3c, 0xe9,
    0x7b, 0xc2, 0x71, 0xea, 0x66, 0x5e, 0x0d, 0x9c, 0x74, 0x43, 0xb5, 0xd8,
    0x05, 0xe2, 0x40, 0x4c, 0x30, 0x61, 0x79, 0xff, 0x40, 0xbf, 0x66, 0x24,
    0x35, 0x95, 0x56, 0xd4, 0x12, 0x68, 0x34, 0xb1, 0xb9, 0x31, 0x80, 0xc5,
    0x64, 0x1a, 0x50, 0xcc, 0xea, 0x63, 0x7c, 0x21, 0x84, 0x36, 0xe1, 0x9b,
    0x3e, 0x81, 0x84, 0x33, 0x8f, 0x4a, 0xc2, 0x99, 0xa0, 0x18, 0x0e, 0x8f,
    0x4a, 0xc0, 0x59, 0x85, 0x12, 0x70, 0x54, 0x18, 0x89, 0xda, 0x02, 0x91,
    0x88, 0x98, 0xf0, 0xdb, 0x9f, 0xe3, 0xc5, 0x3a, 0xea, 0x3a, 0x72, 0xcd,
    0x15, 0x8e, 0xd4, 0x82, 0x17, 0x4f, 0xb0, 0x84, 0xeb, 0x7b, 0x8c, 0x84,
    0x6b, 0x05, 0x45, 0x91, 0x42, 0x30, 0x12, 0xd2, 0x3a, 0x22, 0x01, 0x29,
    0x1c, 0x91, 0x04, 0xad, 0xa0, 0x44, 0xca, 0xe2, 0xd4, 0xe0, 0xf7, 0x5a,
    0x1a, 0xc6, 0x9a, 0x16, 0x0a, 0xc9, 0x08, 0x21, 0xe0, 0x10, 0x62, 0x0a,
    0x08, 0x42, 0x6c, 0x15, 0xa1, 0x6b, 0x31, 0x4c, 0xd0, 0x36, 0xe2, 0xcc,
    0xa1, 0xf3, 0x88, 0x33, 0xba, 0x9c, 0x92, 0x38, 0x8f, 0xf4, 0xe9, 0x76,
    0xc6, 0x62, 0x2f, 0x71, 0xf5, 0xda, 0x75, 0xd6, 0x5c, 0xeb, 0xba, 0xb2,
    0x0b, 0xc1, 0x30, 0x5c, 0xeb, 0x88, 0x44, 0x5c, 0xeb, 0x50, 0x5b, 0x44,
    0x89, 0xb8, 0xb6, 0x44, 0x89, 0xb8, 0xa2, 0x5c, 0x54, 0xe2, 0x88, 0x36,
    0x43, 0x0a, 0x47, 0x43, 0xc2, 0xbb, 0x0a, 0x03, 0xca, 0xb2, 0x64, 0x6f,
    0xef, 0x0e, 0x85, 0x16, 0x4a, 0x33, 0x13, 0x02, 0x09, 0x4f, 0x45, 0x92,
    0x1a, 0x6f, 0x15, 0x35, 0xc7, 0x78, 0xa9, 0xa8, 0x6d, 0x86, 0x47, 0xa8,
    0x65, 0x86, 0xc7, 0x11, 0x99, 0x75, 0x95, 0xd0, 0x31, 0x0e, 0x6d, 0xc4,
    0x53, 0x51, 0xeb, 0x18, 0x11, 0x78, 0xf4, 0xf8, 0x11, 0xb7, 0x47, 0xb7,
    0x7b, 0xbd, 0xa3, 0xc3, 0xa3, 0x7c, 0xf0, 0xf4, 0x80, 0x1f, 0x7f, 0x7a,
    0x8e, 0xcd, 0x1b, 0xaa, 0x75, 0x4f, 0x6a, 0x8d, 0x7e, 0x01, 0xb4, 0x74,
    0x13, 0x57, 0x18, 0x72, 0x9a, 0xaf, 0x57, 0x68, 0x9b, 0x96, 0xb9, 0x14,
    0x1e, 0x9d, 0x07, 0x58, 0x17, 0xf6, 0x77, 0xef, 0xf2, 0xf0, 0xc1, 0xc3,
    0xde, 0xb9, 0x25, 0xf4, 0xe4, 0xe0, 0x49, 0x3e, 0x78, 0xfa, 0x9c, 0xf0,
    0xa6, 0x5e, 0x1a, 0xd0, 0x75, 0x8f, 0xbd, 0x37, 0x65, 0xd1, 0x4e, 0x8d,
    0x81, 0x2d, 0xf6, 0xb0, 0xa9, 0xb1, 0xf5, 0xc9, 0x16, 0x77, 0xbf, 0xbc,
    0xcb, 0xbd, 0xfd, 0x7b, 0x4b, 0xbc, 0x7f, 0x01, 0x26, 0xf8, 0x38, 0x36,
    0x3c, 0x0c, 0x4e, 0x50, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image6_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xad, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xed, 0x93, 0xb1, 0x0e, 0x84,
    0x20, 0x10, 0x44, 0x9f, 0x57, 0xd9, 0xf2, 0x9f, 0xb4, 0x57, 0x5d, 0xae,
    0x30, 0xc4, 0xca, 0xd6, 0xff, 0xb4, 0xb5, 0xe3, 0x8a, 0xcd, 0x86, 0x15,
    0x51, 0xa1, 0x34, 0xe7, 0x34, 0x84, 0xb0, 0x3b, 0xfb, 0xc8, 0x00, 0x3c,
    0xba, 0xad, 0xba, 0xb6, 0xf2, 0x29, 0xc2, 0x0a, 0x38, 0x60, 0x01, 0x7a,
    0x60, 0x65, 0x1e, 0x1d, 0xfe, 0xe3, 0x37, 0x5e, 0x66, 0x33, 0x47, 0xf0,
    0x07, 0x83, 0x42, 0x4c, 0x66, 0x67, 0x6b, 0xea, 0xef, 0xf6, 0xcd, 0x3e,
    0x1b, 0x96, 0xc8, 0xe0, 0x7b, 0x30, 0x58, 0x6e, 0x62, 0xc9, 0x0b, 0x26,
    0x6a, 0x9e, 0xef, 0xaf, 0xa4, 0xf5, 0x32, 0xfc, 0x95, 0x0e, 0x94, 0x6c,
    0xc9, 0x48, 0x6b, 0x4c, 0x6d, 0xbf, 0xc8, 0x18, 0x3b, 0xb6, 0xc1, 0x38,
    0xe0, 0xdd, 0x10, 0xae, 0xf6, 0xef, 0x8c, 0x6d, 0x10, 0x96, 0xbc, 0x56,
    0xda, 0x5f, 0x24, 0xd6, 0x43, 0x4b, 0x5e, 0x6b, 0xae, 0xf5, 0x45, 0x62,
    0x25, 0x6d, 0x25, 0x97, 0x4c, 0xa6, 0xb1, 0x3f, 0x2f, 0xab, 0x55, 0x18,
    0x42, 0x14, 0x53, 0x5d, 0x93, 0x1a, 0x7f, 0x5e, 0x2e, 0xfb, 0x7a, 0x5a,
    0x82, 0x7e, 0xf4, 0x9f, 0xfa, 0x01, 0xf6, 0x5b, 0x4a, 0xca, 0x93, 0xb0,
    0x4d, 0x39, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42,
    0x60, 0x82
};

    static const unsigned char image7_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xa7, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xed, 0x52, 0x31, 0x0e, 0xc2,
    0x30, 0x0c, 0xbc, 0x30, 0xb1, 0xe6, 0x9f, 0xfd, 0x82, 0x85, 0x2a, 0x4f,
    0xec, 0xf9, 0x67, 0x57, 0x36, 0x33, 0x80, 0x6b, 0x3b, 0x6d, 0x43, 0x28,
    0x13, 0x52, 0x6e, 0x89, 0x12, 0xdd, 0x9d, 0xcf, 0xb1, 0x81, 0x81, 0xbf,
    0x45, 0xaa, 0x1f, 0x78, 0x66, 0xa1, 0x5b, 0x06, 0xb0, 0x00, 0xb8, 0x02,
    0x78, 0x00, 0xd0, 0x3b, 0x6d, 0xf8, 0x9d, 0xc6, 0x2c, 0x66, 0xd2, 0x3a,
    0xa7, 0x83, 0x02, 0x2c, 0x5a, 0x7c, 0x25, 0xc4, 0xa4, 0x47, 0xc9, 0x8a,
    0xc4, 0x4e, 0x3c, 0x4f, 0x43, 0x4d, 0xd1, 0xd8, 0x44, 0x3d, 0xed, 0xfa,
    0xce, 0x28, 0x6d, 0xef, 0xc0, 0xc5, 0xc8, 0x9a, 0xa4, 0x07, 0x94, 0x8c,
    0xef, 0x4d, 0x4d, 0xef, 0x8c, 0xf3, 0xbb, 0xbd, 0x5e, 0x50, 0x8a, 0x83,
    0x8d, 0xfa, 0x2a, 0x71, 0xfe, 0xc2, 0xb8, 0x4e, 0x1a, 0xf5, 0x55, 0xe2,
    0xe5, 0x84, 0xa9, 0x26, 0x8d, 0xfa, 0xd5, 0x98, 0x67, 0xff, 0x67, 0x67,
    0x92, 0x36, 0x67, 0xc4, 0xf2, 0xda, 0x0e, 0x96, 0x72, 0x2f, 0x1f, 0x0a,
    0xb4, 0xb1, 0xb3, 0x5a, 0x2c, 0xfb, 0x7b, 0x3a, 0x30, 0xf0, 0x2b, 0x9e,
    0xd4, 0x5b, 0x48, 0xe8, 0x82, 0xad, 0x42, 0x86, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image8_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xab, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xcd, 0x92, 0x3b, 0x0e, 0x84,
    0x30, 0x0c, 0x44, 0x27, 0x88, 0x62, 0x5b, 0xee, 0xc9, 0x15, 0xb6, 0x72,
    0x49, 0x9b, 0x7b, 0xd2, 0xd2, 0x79, 0x8b, 0x90, 0x15, 0xf2, 0x8e, 0x15,
    0x07, 0xf6, 0x37, 0x0d, 0x4a, 0x70, 0x5e, 0x9e, 0x1c, 0x03, 0x3f, 0x8a,
    0x76, 0xee, 0x3f, 0x33, 0xbc, 0x59, 0xe4, 0x3f, 0xc1, 0xcd, 0x36, 0x44,
    0xc1, 0x0c, 0x94, 0xae, 0x82, 0x9b, 0x80, 0xb3, 0xe0, 0x1a, 0x35, 0xdf,
    0x66, 0xa2, 0x46, 0x47, 0x60, 0xe8, 0xcc, 0xc7, 0xa6, 0xa2, 0x27, 0xe1,
    0x36, 0x00, 0x61, 0x63, 0xd9, 0xa1, 0x4b, 0x18, 0x1e, 0x00, 0xe7, 0x0a,
    0x4b, 0xc0, 0xb6, 0xaf, 0xa5, 0xcb, 0x9e, 0xc4, 0x9a, 0x1e, 0x81, 0xa7,
    0xe1, 0x99, 0x40, 0x2c, 0xcc, 0x87, 0x3b, 0xad, 0x10, 0x05, 0x66, 0x32,
    0x56, 0x37, 0xb3, 0xbe, 0x27, 0x0f, 0x3e, 0xbe, 0x6e, 0x65, 0x07, 0x0a,
    0x94, 0x1e, 0xdb, 0x4c, 0xb4, 0x92, 0x18, 0xaf, 0xa4, 0xac, 0x9a, 0x32,
    0x43, 0x56, 0x4f, 0xc1, 0xcc, 0x60, 0x73, 0xfe, 0x8b, 0x96, 0xcb, 0x42,
    0x89, 0x3c, 0x90, 0x34, 0x47, 0xce, 0xb9, 0x4d, 0xb4, 0x98, 0xad, 0xe8,
    0x30, 0xfa, 0x4e, 0x1e, 0x70, 0xbb, 0x33, 0xf8, 0xc6, 0x07, 0x16, 0xcb,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image9_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x00,
    0xde, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xd5, 0x94, 0x3d, 0x0e, 0x82,
    0x40, 0x10, 0x46, 0xdf, 0x10, 0x0a, 0x2d, 0xb9, 0x8f, 0x8d, 0xad, 0x89,
    0xb7, 0x21, 0x56, 0x50, 0x61, 0x67, 0xab, 0x57, 0xb1, 0xb5, 0xf1, 0x3e,
    0x94, 0xda, 0x8d, 0xc5, 0x06, 0x84, 0x30, 0xc3, 0xaf, 0x14, 0x4e, 0xb2,
    0x99, 0xc0, 0x7c, 0xfb, 0xcd, 0xdb, 0xc9, 0x02, 0xfc, 0x5b, 0x08, 0xa0,
    0x23, 0x34, 0xf3, 0x8c, 0x55, 0x6d, 0x6f, 0x11, 0x69, 0xea, 0x26, 0x45,
    0x6c, 0x98, 0x40, 0xe8, 0x56, 0x67, 0x11, 0xd1, 0xa9, 0xe6, 0xb5, 0xb1,
    0x47, 0x3d, 0xd7, 0xbc, 0x1e, 0x85, 0x45, 0x1c, 0xde, 0x5d, 0x81, 0x12,
    0x48, 0x9c, 0x9c, 0xbb, 0xcd, 0xd4, 0x0b, 0x40, 0xa1, 0xe8, 0xa9, 0x5f,
    0x14, 0x0a, 0xf3, 0xa8, 0x51, 0x8d, 0x2e, 0xd2, 0x59, 0xc1, 0x3b, 0x31,
    0xeb, 0x21, 0xde, 0x1c, 0x76, 0x19, 0x96, 0xf9, 0x88, 0x19, 0x97, 0xbd,
    0xf5, 0xfd, 0x11, 0x5e, 0x64, 0x3c, 0x9e, 0x68, 0x73, 0x2c, 0xee, 0xad,
    0xf8, 0x9a, 0x25, 0x66, 0xbd, 0x6a, 0x94, 0x9e, 0x82, 0xcb, 0x96, 0x8c,
    0x7b, 0xc3, 0x7c, 0x21, 0xf1, 0x06, 0x91, 0x73, 0xeb, 0x39, 0x8c, 0x25,
    0x97, 0x45, 0xc4, 0xaa, 0x69, 0x67, 0x8f, 0xc8, 0x8d, 0x1f, 0x10, 0xfb,
    0xfa, 0xd9, 0xc4, 0x56, 0x34, 0xf5, 0xab, 0x11, 0x47, 0x15, 0x8d, 0xb7,
    0xbc, 0x7b, 0x6c, 0x6b, 0x69, 0x11, 0x0f, 0x9c, 0xb1, 0xd4, 0x39, 0xc4,
    0xf1, 0x80, 0x8a, 0xc5, 0x33, 0x1e, 0x22, 0x58, 0x85, 0x38, 0x7c, 0x04,
    0xde, 0xdf, 0xcd, 0xca, 0x2b, 0xc6, 0x07, 0x4d, 0x68, 0xae, 0xdc, 0x7a,
    0x2a, 0x95, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
    0x42, 0x60, 0x82
};

    static const unsigned char image10_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x01,
    0x47, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xb5, 0x95, 0xbd, 0x6e, 0xc3,
    0x20, 0x1c, 0xc4, 0x7f, 0x44, 0x1e, 0x18, 0xcd, 0x73, 0x64, 0x8c, 0xd4,
    0xa1, 0xaf, 0x54, 0x56, 0x4f, 0x89, 0x55, 0x45, 0x49, 0x86, 0xc8, 0x1d,
    0xe9, 0x4b, 0x75, 0xcf, 0x73, 0x38, 0x23, 0x83, 0x25, 0x3a, 0x60, 0xbb,
    0x0e, 0x1f, 0xb5, 0xd3, 0x34, 0x27, 0xa1, 0xf3, 0x1f, 0xf0, 0x71, 0x1c,
    0x58, 0x86, 0x27, 0x41, 0xcc, 0x8c, 0xbb, 0x3f, 0xbc, 0x13, 0x4d, 0x4a,
    0x89, 0xe0, 0x26, 0xbd, 0x22, 0x2f, 0x19, 0x8d, 0x0c, 0x1d, 0xb3, 0xa2,
    0xf7, 0x8a, 0xaf, 0x06, 0x51, 0x73, 0x36, 0x1c, 0xf6, 0x9e, 0x43, 0xa1,
    0xa1, 0x4d, 0x91, 0x98, 0x7f, 0x63, 0x43, 0x00, 0xce, 0x9c, 0x0c, 0x6d,
    0xa7, 0x91, 0x45, 0x43, 0xb5, 0xad, 0xb8, 0x7c, 0x5d, 0xb8, 0xda, 0x2b,
    0xa5, 0x2c, 0xb3, 0xbc, 0x7e, 0x59, 0xd3, 0x9c, 0x1a, 0x6c, 0x57, 0xa1,
    0x0a, 0x83, 0xde, 0xea, 0x1b, 0xd7, 0x2b, 0x80, 0xb6, 0xd3, 0x28, 0x69,
    0xb0, 0x5d, 0x05, 0x80, 0xed, 0x2c, 0xa5, 0x2c, 0xa1, 0x83, 0x52, 0x96,
    0xc9, 0xda, 0xcf, 0xab, 0x50, 0xd2, 0x9b, 0x0a, 0x51, 0x00, 0xc8, 0xa2,
    0xa1, 0xb5, 0x1a, 0x55, 0x18, 0x40, 0xb3, 0x79, 0xdd, 0x64, 0x83, 0x9c,
    0x42, 0x15, 0x86, 0xd6, 0xfa, 0x9d, 0x42, 0x15, 0x0b, 0x8f, 0x2b, 0x5b,
    0x8d, 0x39, 0x7b, 0x1e, 0xea, 0xa5, 0x1c, 0x62, 0x15, 0xae, 0x3c, 0x38,
    0xbf, 0xb7, 0x0e, 0x21, 0x00, 0x77, 0xd8, 0x93, 0x75, 0x52, 0xbf, 0x1b,
    0xa0, 0x05, 0xd4, 0x0c, 0xd7, 0x37, 0x87, 0x17, 0x65, 0x1c, 0x3b, 0x69,
    0x71, 0x6e, 0x97, 0xcd, 0x59, 0x88, 0x23, 0xce, 0xed, 0xfa, 0xeb, 0x58,
    0xe7, 0x33, 0x8e, 0xb3, 0x33, 0xbd, 0xc0, 0x67, 0xe4, 0xd4, 0xb9, 0xb7,
    0xbe, 0x66, 0x10, 0x77, 0x50, 0x8b, 0x51, 0xf8, 0xb7, 0x0c, 0xbd, 0x08,
    0xbd, 0x48, 0x0a, 0xed, 0xf8, 0x34, 0x15, 0x2f, 0xe0, 0xe7, 0x1e, 0x3f,
    0xe2, 0x38, 0x14, 0x5f, 0x94, 0xf1, 0x9c, 0x63, 0x21, 0x3e, 0x00, 0x3b,
    0x59, 0x54, 0x3e, 0x9e, 0x71, 0xea, 0x60, 0x85, 0x38, 0xfe, 0x47, 0xc6,
    0x29, 0x28, 0x1e, 0xca, 0x38, 0xc5, 0x7e, 0x07, 0x93, 0x03, 0xcd, 0xb7,
    0x83, 0xbb, 0x17, 0xd0, 0xb8, 0x05, 0xbf, 0x19, 0xe3, 0x96, 0x7d, 0x79,
    0x21, 0x3f, 0x09, 0xdf, 0x6b, 0x6e, 0x66, 0x95, 0xec, 0x2e, 0x71, 0x6c,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

    static const unsigned char image11_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x16,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xc4, 0xb4, 0x6c, 0x3b, 0x00, 0x00, 0x02,
    0xd7, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xd5, 0x94, 0xb1, 0x8b, 0x1b,
    0x47, 0x14, 0x87, 0x3f, 0x1d, 0x2a, 0x9e, 0xe0, 0x8a, 0x59, 0xb8, 0x42,
    0x0b, 0x29, 0x76, 0xc0, 0x01, 0xcb, 0x26, 0x85, 0xe4, 0x46, 0x92, 0x49,
    0x71, 0x77, 0x21, 0x60, 0xa5, 0xca, 0xe1, 0x14, 0xb9, 0xf6, 0x5c, 0x19,
    0x52, 0x85, 0xfb, 0x0b, 0x5c, 0xbb, 0x73, 0x48, 0x65, 0x42, 0xc0, 0xb1,
    0x8b, 0x08, 0xb9, 0xf0, 0x45, 0x86, 0x04, 0xe9, 0x20, 0x89, 0xee, 0x2a,
    0x4b, 0x07, 0xc1, 0x59, 0x43, 0x02, 0x73, 0x70, 0x82, 0xdd, 0x22, 0xb0,
    0x03, 0x59, 0xd8, 0x57, 0x2c, 0x6c, 0x8a, 0x38, 0x26, 0xe0, 0x1c, 0xba,
    0x90, 0x14, 0xc9, 0xd7, 0xcd, 0xf0, 0xf8, 0xe6, 0xc7, 0xe3, 0xbd, 0x81,
    0xff, 0x1b, 0xb5, 0x3f, 0x1f, 0x66, 0xe3, 0x59, 0xe5, 0xce, 0x1c, 0xf1,
    0x32, 0x86, 0x52, 0xc8, 0x4a, 0x8f, 0x5d, 0x6f, 0x12, 0xbc, 0x11, 0xd0,
    0x6e, 0xb5, 0xb9, 0xd6, 0xbf, 0x56, 0x3b, 0x4f, 0x74, 0xae, 0x78, 0x38,
    0x1a, 0x56, 0xf1, 0xc2, 0x41, 0xd4, 0xa0, 0xdf, 0xdb, 0x24, 0x13, 0x25,
    0x90, 0x80, 0xf8, 0xc5, 0x9c, 0x1f, 0x0f, 0x17, 0x04, 0x25, 0x0c, 0x36,
    0x07, 0x5c, 0x7f, 0xf7, 0xfa, 0x85, 0xe4, 0x35, 0x80, 0x83, 0xd1, 0x41,
    0x35, 0x7e, 0x32, 0xe6, 0xca, 0x3b, 0x1d, 0x06, 0xbb, 0x7b, 0x00, 0x14,
    0x14, 0xa4, 0xbf, 0x2a, 0x52, 0x57, 0x92, 0x3c, 0x63, 0xf1, 0x68, 0x8c,
    0x2e, 0x53, 0x76, 0xdf, 0xdf, 0xbd, 0x50, 0xf2, 0xda, 0x64, 0x3a, 0xa9,
    0x8e, 0x0e, 0x8f, 0x08, 0x37, 0xdb, 0xb4, 0xbb, 0x7d, 0x14, 0x30, 0x3e,
    0x20, 0xf5, 0x60, 0x14, 0xe2, 0xd4, 0x41, 0x94, 0xe0, 0x51, 0xdc, 0xd3,
    0x39, 0x4d, 0x60, 0xff, 0xe3, 0xfd, 0x95, 0xe2, 0xb5, 0xec, 0x2c, 0x83,
    0x28, 0x60, 0xb0, 0xf9, 0x1e, 0x94, 0x10, 0xfa, 0x00, 0xf7, 0x22, 0xa3,
    0xc0, 0xe1, 0x70, 0x84, 0x58, 0xf4, 0xd0, 0x22, 0x0a, 0xb6, 0xd7, 0x22,
    0x5d, 0x7a, 0x0e, 0xc6, 0x07, 0xd5, 0x2a, 0x71, 0x7d, 0xb1, 0x8c, 0xd9,
    0xfa, 0x70, 0x87, 0x84, 0x02, 0xcd, 0x61, 0x7e, 0xe2, 0xd0, 0x28, 0x41,
    0x8d, 0x82, 0x07, 0x05, 0x9a, 0x18, 0x16, 0xa7, 0x09, 0xf6, 0xb2, 0xa5,
    0xd8, 0x00, 0x4d, 0x74, 0x95, 0x97, 0x35, 0x29, 0x05, 0xb3, 0x1e, 0x00,
    0x4a, 0x5c, 0x77, 0xd0, 0x2d, 0x50, 0x11, 0x44, 0x05, 0xb4, 0x89, 0x11,
    0xcb, 0xc2, 0xc7, 0x48, 0x53, 0xf0, 0x3e, 0x23, 0xdc, 0x08, 0x89, 0xd3,
    0x78, 0xa5, 0xb8, 0xee, 0xd5, 0xe3, 0x4b, 0xa5, 0xc8, 0x32, 0x4c, 0x09,
    0x69, 0x9e, 0x62, 0x10, 0xf4, 0xd4, 0xd0, 0x2c, 0xaf, 0x10, 0xff, 0xf4,
    0x15, 0xf2, 0x96, 0xc7, 0x2b, 0x04, 0xa5, 0xe0, 0xcb, 0x02, 0x4b, 0xb0,
    0x3a, 0xb1, 0x11, 0x21, 0xf3, 0x19, 0x0d, 0x1a, 0xa4, 0x5e, 0x31, 0x08,
    0x89, 0xcf, 0x68, 0x12, 0x12, 0xff, 0x30, 0x87, 0xb6, 0xe2, 0x37, 0x5e,
    0xde, 0xd7, 0x15, 0xc9, 0xa1, 0x90, 0xe2, 0x02, 0xad, 0x58, 0x17, 0xfc,
    0x32, 0x21, 0xf1, 0x09, 0x02, 0x24, 0x69, 0x02, 0xd2, 0x20, 0xf6, 0x0b,
    0x1a, 0x4d, 0x4f, 0xaa, 0x1e, 0xcd, 0x85, 0x54, 0x53, 0x24, 0x07, 0x97,
    0x3a, 0x3a, 0x6f, 0x76, 0x56, 0x8b, 0x3b, 0xad, 0x0e, 0xf1, 0x77, 0xc7,
    0x88, 0x0a, 0x89, 0x77, 0x34, 0x44, 0xf0, 0x69, 0x46, 0x88, 0x25, 0x3b,
    0x2d, 0x40, 0xa1, 0xc8, 0x3d, 0x82, 0xc1, 0x9d, 0xc5, 0xb4, 0x24, 0xc4,
    0xac, 0x9b, 0xd5, 0xe2, 0xed, 0xc1, 0x76, 0xad, 0x65, 0x2c, 0xd3, 0xa7,
    0x23, 0x54, 0x0c, 0x89, 0xf7, 0x98, 0x3a, 0x2c, 0x8a, 0x29, 0xde, 0xa4,
    0x80, 0xe2, 0x33, 0x70, 0x89, 0x43, 0x4f, 0x1c, 0xe1, 0xa5, 0x90, 0x20,
    0x0a, 0x18, 0x8e, 0x86, 0xd5, 0xb3, 0x6f, 0x9f, 0x9d, 0x3b, 0x76, 0x6b,
    0x00, 0xb6, 0x6d, 0xb1, 0xb9, 0x21, 0x7b, 0x7c, 0x48, 0x88, 0xa0, 0xb9,
    0x62, 0x6d, 0x13, 0xb1, 0xbf, 0x8f, 0x5a, 0xa8, 0x19, 0xe6, 0xfb, 0x84,
    0x7e, 0xb7, 0xcf, 0xad, 0xbd, 0x5b, 0xb5, 0xf9, 0xc9, 0x9c, 0xe3, 0x6f,
    0x8e, 0xb9, 0xfb, 0xe9, 0x5d, 0x26, 0xe3, 0xc9, 0x5f, 0xca, 0x5f, 0x6d,
    0xd0, 0x64, 0x3a, 0xa9, 0xdc, 0xc2, 0x71, 0xf4, 0xf3, 0x1c, 0xd9, 0x68,
    0x80, 0x08, 0x01, 0x86, 0xe4, 0x17, 0x87, 0x94, 0xb0, 0xb5, 0xb9, 0xc5,
    0xcd, 0x9d, 0x9b, 0x35, 0x80, 0xe7, 0xf3, 0xe7, 0xd5, 0xc3, 0x2f, 0x1f,
    0x32, 0x7a, 0x3c, 0xc2, 0x18, 0xc3, 0xde, 0x07, 0x7b, 0xd8, 0xb6, 0x65,
    0x7b, 0x6b, 0xbb, 0xf6, 0x9a, 0xf8, 0x0f, 0x66, 0xd3, 0x59, 0x15, 0xbb,
    0x18, 0x72, 0xd0, 0xba, 0x12, 0x85, 0x11, 0xb6, 0x69, 0xb9, 0xda, 0xbf,
    0xfa, 0x5a, 0xed, 0x83, 0x2f, 0x1e, 0x54, 0xa3, 0xcf, 0x46, 0xc4, 0xcb,
    0x98, 0x9d, 0x1b, 0x3b, 0xb4, 0xdf, 0x6e, 0xbf, 0x7a, 0xfc, 0x1f, 0x33,
    0x99, 0x4e, 0xaa, 0x3b, 0xfb, 0x77, 0xaa, 0x28, 0x8a, 0xaa, 0x5e, 0xb7,
    0x57, 0xdd, 0xfb, 0xe4, 0xde, 0xca, 0x75, 0xff, 0x5b, 0x0c, 0x3f, 0x1f,
    0x56, 0xbd, 0x6e, 0xaf, 0x8a, 0x2e, 0x45, 0xd5, 0xed, 0x8f, 0x6e, 0x57,
    0xff, 0x4e, 0xec, 0x97, 0xcc, 0xbe, 0x9e, 0x55, 0xf7, 0x1f, 0xdd, 0x47,
    0xcb, 0xd5, 0x7f, 0xc9, 0x7f, 0x8f, 0xdf, 0x00, 0x46, 0xee, 0x57, 0x82,
    0x91, 0x6c, 0xc2, 0x84, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

    switch (id) {
        case image0_ID: return QPixmap((const char**)image0_data);
        case image1_ID:  { QImage img; img.loadFromData(image1_data, sizeof(image1_data), "XBM.GZ"); return QPixmap::fromImage(img); }
        case image2_ID:  { QImage img; img.loadFromData(image2_data, sizeof(image2_data), "PNG"); return QPixmap::fromImage(img); }
        case image3_ID:  { QImage img; img.loadFromData(image3_data, sizeof(image3_data), "PNG"); return QPixmap::fromImage(img); }
        case image4_ID:  { QImage img; img.loadFromData(image4_data, sizeof(image4_data), "PNG"); return QPixmap::fromImage(img); }
        case image5_ID:  { QImage img; img.loadFromData(image5_data, sizeof(image5_data), "PNG"); return QPixmap::fromImage(img); }
        case image6_ID:  { QImage img; img.loadFromData(image6_data, sizeof(image6_data), "PNG"); return QPixmap::fromImage(img); }
        case image7_ID:  { QImage img; img.loadFromData(image7_data, sizeof(image7_data), "PNG"); return QPixmap::fromImage(img); }
        case image8_ID:  { QImage img; img.loadFromData(image8_data, sizeof(image8_data), "PNG"); return QPixmap::fromImage(img); }
        case image9_ID:  { QImage img; img.loadFromData(image9_data, sizeof(image9_data), "PNG"); return QPixmap::fromImage(img); }
        case image10_ID:  { QImage img; img.loadFromData(image10_data, sizeof(image10_data), "PNG"); return QPixmap::fromImage(img); }
        case image11_ID:  { QImage img; img.loadFromData(image11_data, sizeof(image11_data), "PNG"); return QPixmap::fromImage(img); }
        default: return QPixmap();
    } // switch
    } // icon

};

namespace Ui {
    class GUI: public Ui_GUI {};
} // namespace Ui

class GUI : public Q3MainWindow, public Ui::GUI
{
    Q_OBJECT

public:
    GUI(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel);
    ~GUI();

public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileReload();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void filePrint();
    virtual void fileExit();
    virtual void editUndo();
    virtual void editRedo();
    virtual void editCut();
    virtual void editCopy();
    virtual void editPaste();
    virtual void editFind();
    virtual void helpIndex();
    virtual void helpContents();
    virtual void helpAbout();
    virtual void saveXML();

	//Nan Zhang added slots
	virtual void showEntireScene(void);
	virtual void runTetCSG(void);
	virtual void setViewerWinSize(void);
    virtual void playpause(bool);
	virtual void stepAnimation(void);
	virtual void saveSnapshot(void);
	virtual void saveCamera(void);
	virtual void loadCamera(void);
    virtual void exportGraph(void);


protected slots:
    virtual void languageChange();

};

#endif // GUI_H
