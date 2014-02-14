/****************************************************************************
** Form implementation generated from reading ui file 'xmldlg.ui'
**
** Created: Tue Nov 28 11:22:31 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "xmldlg.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "xmldlg.ui.h"

/*
 *  Constructs a CXmlIODialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CXmlIODialog::CXmlIODialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CXmlIODialog" );
    setSizeGripEnabled( TRUE );
    CXmlIODialogLayout = new QGridLayout( this, 1, 1, 6, 6, "CXmlIODialogLayout"); 

    layout3 = new QVBoxLayout( 0, 0, 6, "layout3"); 

    m_buttonLoadXML = new QPushButton( this, "m_buttonLoadXML" );
    layout3->addWidget( m_buttonLoadXML );

    m_buttonSaveXML = new QPushButton( this, "m_buttonSaveXML" );
    m_buttonSaveXML->setAutoDefault( TRUE );
    layout3->addWidget( m_buttonSaveXML );
    spacer3 = new QSpacerItem( 20, 121, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout3->addItem( spacer3 );

    m_buttonOk = new QPushButton( this, "m_buttonOk" );
    m_buttonOk->setAutoDefault( TRUE );
    m_buttonOk->setDefault( TRUE );
    layout3->addWidget( m_buttonOk );

    m_buttonCancel = new QPushButton( this, "m_buttonCancel" );
    m_buttonCancel->setAutoDefault( TRUE );
    layout3->addWidget( m_buttonCancel );

    CXmlIODialogLayout->addLayout( layout3, 0, 1 );

    m_xmlListView = new QListView( this, "m_xmlListView" );
    m_xmlListView->setFrameShape( QListView::LineEditPanel );

    CXmlIODialogLayout->addWidget( m_xmlListView, 0, 0 );
    languageChange();
    resize( QSize(618, 426).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( m_buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( m_buttonSaveXML, SIGNAL( clicked() ), this, SLOT( buttonSaveXMLClicked() ) );
    connect( m_buttonLoadXML, SIGNAL( clicked() ), this, SLOT( buttonLoadXMLClicked() ) );
    connect( m_xmlListView, SIGNAL( doubleClicked(QListViewItem*,const QPoint&,int) ), this, SLOT( slotListViewDoubleClick(QListViewItem*,const QPoint&,int) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CXmlIODialog::~CXmlIODialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CXmlIODialog::languageChange()
{
    setCaption( tr( "XML Input/Output" ) );
    m_buttonLoadXML->setText( tr( "Load XML" ) );
    m_buttonLoadXML->setAccel( QKeySequence( QString::null ) );
    m_buttonSaveXML->setText( tr( "Save XML" ) );
    m_buttonSaveXML->setAccel( QKeySequence( QString::null ) );
    m_buttonOk->setText( tr( "&OK" ) );
    m_buttonOk->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    m_buttonCancel->setText( tr( "&Cancel" ) );
    m_buttonCancel->setAccel( QKeySequence( QString::null ) );
}

