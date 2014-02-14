/****************************************************************************
** Form interface generated from reading ui file 'xmldlg.ui'
**
** Created: Tue Nov 28 11:22:18 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CXMLIODIALOG_H
#define CXMLIODIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include <qdom.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QListView;
class QListViewItem;

class CXmlIODialog : public QDialog
{
    Q_OBJECT

public:
    CXmlIODialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CXmlIODialog();

    QPushButton* m_buttonLoadXML;
    QPushButton* m_buttonSaveXML;
    QPushButton* m_buttonOk;
    QPushButton* m_buttonCancel;
    QListView* m_xmlListView;

    QDomDocument *m_pDomTree;

public slots:
    virtual void buttonLoadXMLClicked( void );
    virtual void buttonSaveXMLClicked( void );
    virtual void slotListViewDoubleClick( QListViewItem * pitem, const QPoint & pt, int xx );
    virtual void slotSetOptionFromMenu( int id );

protected:
    QGridLayout* CXmlIODialogLayout;
    QVBoxLayout* layout3;
    QSpacerItem* spacer3;

protected slots:
    virtual void languageChange();

private:
    QDomNode * _searchItemNodeByName( const QDomNode & anode, const QString & id );

};

#endif // CXMLIODIALOG_H
