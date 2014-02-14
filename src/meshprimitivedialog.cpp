/****************************************************************************
** Form implementation generated from reading ui file 'meshprimitivedialog.ui'
**
** Created: Tue Nov 28 12:10:08 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "meshprimitivedialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "glpreviewwin.h"
#include "meshprimitivedialog.ui.h"

/*
 *  Constructs a CMeshPrimitiveDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CMeshPrimitiveDialog::CMeshPrimitiveDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CMeshPrimitiveDialog" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 100, 100, sizePolicy().hasHeightForWidth() ) );
    setPaletteForegroundColor( QColor( 0, 0, 0 ) );
    setSizeGripEnabled( TRUE );
    CMeshPrimitiveDialogLayout = new QGridLayout( this, 1, 1, 6, 4, "CMeshPrimitiveDialogLayout"); 

    m_pLayout1 = new QHBoxLayout( 0, 0, 6, "m_pLayout1"); 

    buttonApply = new QPushButton( this, "buttonApply" );
    buttonApply->setAutoDefault( TRUE );
    m_pLayout1->addWidget( buttonApply );
    m_pHSpacing1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    m_pLayout1->addItem( m_pHSpacing1 );

    buttonOk = new QPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    m_pLayout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    m_pLayout1->addWidget( buttonCancel );

    CMeshPrimitiveDialogLayout->addLayout( m_pLayout1, 1, 0 );

    m_pSplitter = new QSplitter( this, "m_pSplitter" );
    m_pSplitter->setMinimumSize( QSize( 300, 60 ) );
    m_pSplitter->setOrientation( QSplitter::Horizontal );
    m_pSplitter->setOpaqueResize( FALSE );
    m_pSplitter->setHandleWidth( 4 );

    m_pListView = new QListView( m_pSplitter, "m_pListView" );

    m_pGLPreviewWin = new CGLPreviewWin( m_pSplitter, "m_pGLPreviewWin" );

    CMeshPrimitiveDialogLayout->addWidget( m_pSplitter, 0, 0 );
    languageChange();
    resize( QSize(733, 505).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_pListView, SIGNAL( doubleClicked(QListViewItem*,const QPoint&,int) ), this, SLOT( onListViewDoubleClicked(QListViewItem*,const QPoint&,int) ) );
    connect( buttonApply, SIGNAL( clicked() ), this, SLOT( onApply() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CMeshPrimitiveDialog::~CMeshPrimitiveDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CMeshPrimitiveDialog::languageChange()
{
    setCaption( tr( "MyDialog" ) );
    buttonApply->setText( tr( "Apply" ) );
    buttonApply->setAccel( QKeySequence( QString::null ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
}

