/****************************************************************************
** $Id: qt/tabdialog.cpp   3.3.1   edited May 27 2003 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "tabdialog.h"

#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qlayout.h>


TabDialog::TabDialog(QWidget *parent, const char *diagname, CGLDrawParms* pdrawparms):
	QTabDialog(parent, diagname),
	_DrawParms()
{
	assert(pdrawparms!=NULL);
	_pDrawParms = pdrawparms;
	_DrawParms = *pdrawparms;

    setupTab1();
    setupTab2();
    setupTab3();

	setOkButton("OK"); 
	setApplyButton("Apply");
	setCancelButton("Cancel");
	setHelpButton("Help");

    connect(this, SIGNAL(applyButtonPressed()), this, SLOT( ProcApplyEvent()));
    connect(this, SIGNAL(cancelButtonPressed()), this, SLOT( ProcCancelEvent()));
    connect(this, SIGNAL(helpButtonPressed ()), this, SLOT( ProcHelpEvent()));

}



void TabDialog::setupTab1()
{
	bool f;

	///lay out;
    QVBox *tab1 = new QVBox(this);
    tab1->setMargin(5);
    //QBoxLayout *topLayout = new QHBoxLayout(tab1, 0, -1, "hbox");
    QGridLayout *topLayout = new QGridLayout(tab1, 1, 2, 0, -1, NULL);
	assert(topLayout!=NULL);

	///=============Draw MESH============
    QButtonGroup *bg = new QButtonGroup(1, QGroupBox::Horizontal, "DrawMesh", tab1);
	///drawline;
    _drawline = new QRadioButton( "Line", bg );
	if (_DrawParms.m_nDrawType == CGLDrawParms::DRAW_MESH_LINE)
		f = true;
	else
		f = false;
	_drawline->setChecked(f);
    connect(_drawline, SIGNAL(toggled(bool)), this, SLOT(ToggleDrawline()));

	///drawhiddenline;
    _drawhline = new QRadioButton( "Hiddenline", bg );
	if (_DrawParms.m_nDrawType == CGLDrawParms::DRAW_MESH_HIDDENLINE)
		f = true;
	else
		f = false;
    _drawhline->setChecked(f);
    connect(_drawhline, SIGNAL(toggled(bool)), this, SLOT(ToggleDrawline()));

	///drawfill
    _drawfill = new QRadioButton( "Fill", bg );
	if (_DrawParms.m_nDrawType == CGLDrawParms::DRAW_MESH_SHADING)
		f = true;
	else
		f = false;
    _drawfill->setChecked(f);
    connect(_drawfill, SIGNAL(toggled(bool)), this, SLOT(ToggleDrawline()));

	////=============LIGHTING and other options================
	/*
	QCheckBox * _enableLighting;
	QCheckBox * _enableTexture;
	QCheckBox * _enableDepth;
	QCheckBox * _enableSmoothing;
	QCheckBox * _enableAxis;
	float m_fAxisLineWidth;
	float m_fBoundingBoxLineWidth;
	bool m_bEnableLighting;
	bool m_bEnableDepthTest;
	bool m_bEnableTexture;
	bool m_bEnableSmoothing;

	bool m_ShowBoudingCubeFlag;
	bool m_ShowAixesFlag;
	bool m_ShowSphereControlFlag;

*/

	///==================Lighting and depth, etc===================
    QButtonGroup *bg3 = new QButtonGroup( 1, QGroupBox::Horizontal, "OpenGL", tab1);
	///lighting box;
    _enableLighting = new QCheckBox( "Lighting", bg3 );
	_enableLighting->setChecked(_DrawParms.m_bEnableLighting);
    connect(_enableLighting, SIGNAL(toggled(bool)), this, SLOT(ToggleLighting()));

	///drawhiddenline;
    _enableDepth = new QCheckBox( "Depth Test", bg3 );
    _enableDepth->setChecked(_DrawParms.m_bEnableDepthTest);
    connect(_enableDepth, SIGNAL(toggled(bool)), this, SLOT(ToggleLighting()));

	///drawhiddenline;
    _enableSmoothing = new QCheckBox( "Smoothing", bg3 );
    _enableSmoothing->setChecked(_DrawParms.m_bEnableSmoothing);
    connect(_enableSmoothing, SIGNAL(toggled(bool)), this, SLOT(ToggleLighting()));

	///drawhiddenline;
    _enableAxis = new QCheckBox( "Draw Axis", bg3 );
    _enableAxis->setChecked(_DrawParms.m_bShowAixes);
    connect(_enableAxis, SIGNAL(toggled(bool)), this, SLOT(ToggleLighting()));


	///==================================
    QButtonGroup *bg2 = new QButtonGroup( 2, QGroupBox::Horizontal, "Associated attributes", tab1 );
    (void)new QLabel( "Background color", bg2 );
    QLabel *owner = new QLabel( " sagag", bg2 );
    owner->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    (void)new QLabel( "Mesh color", bg2 );
    QLabel *group = new QLabel( "222", bg2 );
    group->setFrameStyle( QFrame::Panel | QFrame::Sunken );

	
	///=====================
    topLayout->addWidget(bg, 0, 0);
    topLayout->addWidget(bg3, 0, 1);

	addTab( tab1, "Mesh" );

}


void TabDialog::setupTab2()
{
	
    QVBox *tab2 = new QVBox( this );
    tab2->setMargin( 5 );

/*
    (void)new QLabel( "Filename:", tab1 );
    QLineEdit *fname = new QLineEdit( filename, tab1 );
    fname->setFocus();

    (void)new QLabel( "Path:", tab1 );
    QLabel *path = new QLabel( fileinfo.dirPath( TRUE ), tab1 );
    path->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    (void)new QLabel( "Size:", tab1 );
    ulong kb = (ulong)(fileinfo.size()/1024);
    QLabel *size = new QLabel( QString( "%1 KB" ).arg( kb ), tab1 );
    size->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    (void)new QLabel( "Last Read:", tab1 );
    QLabel *lread = new QLabel( fileinfo.lastRead().toString(), tab1 );
    lread->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    (void)new QLabel( "Last Modified:", tab1 );
    QLabel *lmodif = new QLabel( fileinfo.lastModified().toString(), tab1 );
    lmodif->setFrameStyle( QFrame::Panel | QFrame::Sunken );
*/


    addTab( tab2, "Texture" );

}


void TabDialog::setupTab3()
{
/*
    QVBox *tab3 = new QVBox( this );
    tab3->setMargin( 5 );
    tab3->setSpacing( 5 );
    
    (void)new QLabel( QString( "Open %1 with:" ).arg( filename ), tab3 );

    QListBox *prgs = new QListBox( tab3 );
    for ( unsigned int i = 0; i < 30; i++ ) {
        QString prg = QString( "Application %1" ).arg( i );
        prgs->insertItem( prg );
    }
    prgs->setCurrentItem( 3 );

    (void)new QCheckBox( QString( "Open files with the extension '%1' always with this application" ).arg( fileinfo.extension() ), tab3 );

    addTab( tab3, "Applications" );
*/
}




void TabDialog::ToggleDrawline(void)
{
    if (_drawline->isChecked())
		_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_LINE;
	else if (_drawhline->isChecked())
		_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_HIDDENLINE;
	else if (_drawfill->isChecked())
		_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_SHADING;
}


void TabDialog::ToggleLighting(void)
{
    if (_enableLighting->isChecked())
		_DrawParms.m_bEnableLighting = true;
	else
		_DrawParms.m_bEnableLighting = false;

    if (_enableDepth->isChecked())
		_DrawParms.m_bEnableDepthTest = true;
	else
		_DrawParms.m_bEnableDepthTest = false;

    if (_enableSmoothing->isChecked())
		_DrawParms.m_bEnableSmoothing = true;
	else
		_DrawParms.m_bEnableSmoothing = false;

    if (_enableAxis->isChecked())
		_DrawParms.m_bShowAixes = true;
	else
		_DrawParms.m_bShowAixes = false;
}


void TabDialog::ProcApplyEvent(void)
{
	*_pDrawParms = _DrawParms;
}


void TabDialog::ProcCancelEvent(void)
{
}

void TabDialog::ProcHelpEvent(void)
{

}
