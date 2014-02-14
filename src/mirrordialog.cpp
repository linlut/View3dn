/****************************************************************************
** Form implementation generated from reading ui file 'mirrordialog.ui'
**
** Created: Wed Sep 13 17:52:35 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mirrordialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "mirrordialog.ui.h"

/*
 *  Constructs a MirrorObjectDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MirrorObjectDialog::MirrorObjectDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "MirrorObjectDialog" );
    setSizeGripEnabled( TRUE );

    buttonGroup1 = new QButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setGeometry( QRect( 10, 91, 390, 140 ) );

    QWidget* privateLayoutWidget = new QWidget( buttonGroup1, "layout3" );
    privateLayoutWidget->setGeometry( QRect( 200, 20, 160, 110 ) );
    layout3 = new QVBoxLayout( privateLayoutWidget, 11, 6, "layout3"); 

    pushButtonM1 = new QPushButton( privateLayoutWidget, "pushButtonM1" );
    layout3->addWidget( pushButtonM1 );

    pushButtonM2 = new QPushButton( privateLayoutWidget, "pushButtonM2" );
    layout3->addWidget( pushButtonM2 );

    pushButtonM3 = new QPushButton( privateLayoutWidget, "pushButtonM3" );
    layout3->addWidget( pushButtonM3 );

    QWidget* privateLayoutWidget_2 = new QWidget( buttonGroup1, "layout2" );
    privateLayoutWidget_2->setGeometry( QRect( 10, 20, 170, 110 ) );
    layout2 = new QVBoxLayout( privateLayoutWidget_2, 11, 6, "layout2"); 

    checkBoxM1 = new QCheckBox( privateLayoutWidget_2, "checkBoxM1" );
    layout2->addWidget( checkBoxM1 );

    checkBoxM2 = new QCheckBox( privateLayoutWidget_2, "checkBoxM2" );
    layout2->addWidget( checkBoxM2 );

    checkBoxM3 = new QCheckBox( privateLayoutWidget_2, "checkBoxM3" );
    layout2->addWidget( checkBoxM3 );

    groupBox1 = new QGroupBox( this, "groupBox1" );
    groupBox1->setGeometry( QRect( 10, 10, 391, 60 ) );

    textLabel2 = new QLabel( groupBox1, "textLabel2" );
    textLabel2->setGeometry( QRect( 8, 20, 19, 30 ) );

    textLabel2_2 = new QLabel( groupBox1, "textLabel2_2" );
    textLabel2_2->setGeometry( QRect( 197, 20, 18, 30 ) );

    lineEditP0 = new QLineEdit( groupBox1, "lineEditP0" );
    lineEditP0->setGeometry( QRect( 33, 20, 150, 30 ) );

    lineEditP1 = new QLineEdit( groupBox1, "lineEditP1" );
    lineEditP1->setGeometry( QRect( 220, 20, 160, 30 ) );
    lineEditP1->setFrameShape( QLineEdit::LineEditPanel );
    lineEditP1->setFrameShadow( QLineEdit::Sunken );

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setGeometry( QRect( 21, 243, 80, 26 ) );
    buttonHelp->setAutoDefault( TRUE );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setGeometry( QRect( 249, 243, 110, 26 ) );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    languageChange();
    resize( QSize(414, 290).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( checkBoxM1, SIGNAL( clicked() ), this, SLOT( handleCheckBoxM1() ) );
    connect( checkBoxM2, SIGNAL( clicked() ), this, SLOT( handleCheckBoxM2() ) );
    connect( checkBoxM3, SIGNAL( clicked() ), this, SLOT( handleCheckBoxM3() ) );
    connect( pushButtonM1, SIGNAL( clicked() ), this, SLOT( handlePushButtonM1() ) );
    connect( pushButtonM2, SIGNAL( clicked() ), this, SLOT( handlePushButtonM2() ) );
    connect( pushButtonM3, SIGNAL( clicked() ), this, SLOT( handlePushButtonM3() ) );
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( handleButtonOK() ) );
    connect( lineEditP0, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditP0() ) );
    connect( lineEditP1, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditP1() ) );

    // tab order
    setTabOrder( lineEditP0, lineEditP1 );
    setTabOrder( lineEditP1, checkBoxM1 );
    setTabOrder( checkBoxM1, pushButtonM1 );
    setTabOrder( pushButtonM1, checkBoxM2 );
    setTabOrder( checkBoxM2, pushButtonM2 );
    setTabOrder( pushButtonM2, checkBoxM3 );
    setTabOrder( checkBoxM3, pushButtonM3 );
    setTabOrder( pushButtonM3, buttonOk );
    setTabOrder( buttonOk, buttonHelp );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
MirrorObjectDialog::~MirrorObjectDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MirrorObjectDialog::languageChange()
{
    setCaption( tr( "Mirror dialog" ) );
    buttonGroup1->setTitle( tr( "Mirror settings" ) );
    pushButtonM1->setText( tr( "M1 Rendering options" ) );
    pushButtonM2->setText( tr( "M2 Rendering options" ) );
    pushButtonM3->setText( tr( "M3 Rendering options" ) );
    checkBoxM1->setText( tr( "Mirror 1: rotate 90 degree" ) );
    checkBoxM2->setText( tr( "Mirror 2: rotate 180 degree" ) );
    checkBoxM3->setText( tr( "Mirror 3: rotate 270 degree" ) );
    groupBox1->setTitle( tr( "Rotation axis" ) );
    textLabel2->setText( tr( "P0" ) );
    textLabel2_2->setText( tr( "P1" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
}

