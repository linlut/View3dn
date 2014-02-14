/****************************************************************************
** Form implementation generated from reading ui file 'animcontroldialog.ui'
**
** Created: Fri Jan 12 18:41:49 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "animcontroldialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "animcontroldialog.ui.h"

/*
 *  Constructs a CAnimControlDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CAnimControlDialog::CAnimControlDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CAnimControlDialog" );
    setMinimumSize( QSize( 540, 240 ) );
    setMaximumSize( QSize( 540, 240 ) );
    setBaseSize( QSize( 0, 0 ) );
    setSizeGripEnabled( TRUE );
    CAnimControlDialogLayout = new QGridLayout( this, 1, 1, 2, 12, "CAnimControlDialogLayout"); 
    CAnimControlDialogLayout->setResizeMode( QLayout::Minimum );

    Layout1 = new QHBoxLayout( 0, 0, 8, "Layout1"); 

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    CAnimControlDialogLayout->addLayout( Layout1, 2, 0 );

    groupBox2 = new QGroupBox( this, "groupBox2" );
    groupBox2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, groupBox2->sizePolicy().hasHeightForWidth() ) );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 8 );
    groupBox2->layout()->setMargin( 12 );
    groupBox2Layout = new QHBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    lineEditOutFileName = new QLineEdit( groupBox2, "lineEditOutFileName" );
    groupBox2Layout->addWidget( lineEditOutFileName );

    pushButtonOutputFileName = new QPushButton( groupBox2, "pushButtonOutputFileName" );
    groupBox2Layout->addWidget( pushButtonOutputFileName );

    CAnimControlDialogLayout->addWidget( groupBox2, 1, 0 );

    groupBox1 = new QGroupBox( this, "groupBox1" );
    groupBox1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, groupBox1->sizePolicy().hasHeightForWidth() ) );
    groupBox1->setMinimumSize( QSize( 400, 0 ) );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 8 );
    groupBox1->layout()->setMargin( 12 );
    groupBox1Layout = new QGridLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    pushButtonStartFileName = new QPushButton( groupBox1, "pushButtonStartFileName" );

    groupBox1Layout->addWidget( pushButtonStartFileName, 0, 2 );

    textLabel2 = new QLabel( groupBox1, "textLabel2" );

    groupBox1Layout->addWidget( textLabel2, 0, 0 );

    lineEditStartFileName = new QLineEdit( groupBox1, "lineEditStartFileName" );

    groupBox1Layout->addWidget( lineEditStartFileName, 0, 1 );

    pushButtonEndFileName = new QPushButton( groupBox1, "pushButtonEndFileName" );

    groupBox1Layout->addWidget( pushButtonEndFileName, 1, 2 );

    textLabel2_2 = new QLabel( groupBox1, "textLabel2_2" );

    groupBox1Layout->addWidget( textLabel2_2, 1, 0 );

    lineEditEndFileName = new QLineEdit( groupBox1, "lineEditEndFileName" );

    groupBox1Layout->addWidget( lineEditEndFileName, 1, 1 );

    layout47 = new QHBoxLayout( 0, 0, 6, "layout47"); 

    textLabel3 = new QLabel( groupBox1, "textLabel3" );
    layout47->addWidget( textLabel3 );

    lineEditInc = new QLineEdit( groupBox1, "lineEditInc" );
    layout47->addWidget( lineEditInc );
    spacer35 = new QSpacerItem( 340, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout47->addItem( spacer35 );

    groupBox1Layout->addMultiCellLayout( layout47, 2, 2, 0, 2 );

    CAnimControlDialogLayout->addWidget( groupBox1, 0, 0 );
    languageChange();
    resize( QSize(540, 240).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( onButtonOK() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( onButtonCancel() ) );
    connect( lineEditEndFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditEndFileName() ) );
    connect( lineEditInc, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditInc() ) );
    connect( lineEditOutFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditOutputFileName() ) );
    connect( lineEditStartFileName, SIGNAL( textChanged(const QString&) ), this, SLOT( onLineEditStartFileName() ) );
    connect( pushButtonEndFileName, SIGNAL( clicked() ), this, SLOT( onPushButtonEndFileName() ) );
    connect( pushButtonOutputFileName, SIGNAL( clicked() ), this, SLOT( onPushButtonOutputFileName() ) );
    connect( pushButtonStartFileName, SIGNAL( clicked() ), this, SLOT( onPushButtonStartFileName() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CAnimControlDialog::~CAnimControlDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CAnimControlDialog::languageChange()
{
    setCaption( tr( "Generate Animation" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    groupBox2->setTitle( tr( "Output image files" ) );
    pushButtonOutputFileName->setText( tr( "Browse" ) );
    groupBox1->setTitle( tr( "Input data file" ) );
    pushButtonStartFileName->setText( tr( "Browse" ) );
    textLabel2->setText( tr( "Starting File" ) );
    pushButtonEndFileName->setText( tr( "Browse" ) );
    textLabel2_2->setText( tr( "Ending File" ) );
    textLabel3->setText( tr( "Increment Step" ) );
}

