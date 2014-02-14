/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <assert.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include "glpreviewwin.h"


void CMeshPrimitiveDialog::onListViewDoubleClicked( QListViewItem *plist, const QPoint &pt, int ipos )
{
    if (plist==NULL) return;
    bool ok;
    QString text, existingStr = plist->text(2);

    text = QInputDialog::getText(
           "Input/change value", "Change the value:", QLineEdit::Normal,
           existingStr, &ok, this);

    if ( ok && !text.isEmpty() ){
        plist->setText(2, text);
        QString varname = plist->text(0);
        for (int i=0; i<m_nLen; i++){
            CNameTableItem *pt = &m_table[i];
            if (strcmp(varname.ascii(), pt->m_sName)==0){
                pt->setValue(text);
                break;
            }
        }
    } 
}


void CMeshPrimitiveDialog::onApply()
{

}


void CMeshPrimitiveDialog::prepareNameTable( void )
{
    return;
}


void CMeshPrimitiveDialog::appendToListview()
{
    m_pListView->removeColumn(0);
    m_pListView->removeColumn(0);
    m_pListView->clear();
    m_pListView->addColumn( "Var Name " );
    m_pListView->addColumn( "Type  " );
    m_pListView->addColumn( "Value    " );
    m_pListView->setSorting( -1 );
    m_pListView->setRootIsDecorated( false );

    for (int i=m_nLen-1; i>=0; i--){
        char *name, *type, *val, *option, *comm;
        m_table[i].toString(name, type, val, option, comm);
        QListViewItem  *f = new QListViewItem(m_pListView, name, type, val);
    }    
}







