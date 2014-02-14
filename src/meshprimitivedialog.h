/****************************************************************************
** Form interface generated from reading ui file 'meshprimitivedialog.ui'
**
** Created: Tue Nov 28 12:10:07 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CMESHPRIMITIVEDIALOG_H
#define CMESHPRIMITIVEDIALOG_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>
#include "nametableitem.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class CGLPreviewWin;
class QPushButton;
class QSplitter;
class QListView;
class QListViewItem;

class CMeshPrimitiveDialog : public QDialog
{
    Q_OBJECT

public:
    CMeshPrimitiveDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CMeshPrimitiveDialog();

    QPushButton* buttonApply;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QSplitter* m_pSplitter;
    QListView* m_pListView;
    CGLPreviewWin* m_pGLPreviewWin;

    virtual void prepareNameTable( void );
    virtual void appendToListview();

public slots:
    virtual void onListViewDoubleClicked( QListViewItem * plist, const QPoint & pt, int ipos );
    virtual void onApply();

protected:
    CNameTableItem m_table[30];
    int m_nLen;

    QGridLayout* CMeshPrimitiveDialogLayout;
    QHBoxLayout* m_pLayout1;
    QSpacerItem* m_pHSpacing1;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // CMESHPRIMITIVEDIALOG_H
