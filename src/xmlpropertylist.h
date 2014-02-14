#ifndef __INC_XmlXmlPropertyList_H__
#define __INC_XmlXmlPropertyList_H__

#include <qfeatures.h>
#include <qvariant.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qguardedptr.h>
#include <qtabwidget.h>
#include <qmodules.h>
#include <qptrlist.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
//#include "hierarchyview.h"
#include "metadatabase.h"
#include "propertyeditor.h"


class CXmlPropertyList;
class CXmlPropertyEditor;
class QPainter;
class QColorGroup;
class QLineEdit;
class QPushButton;
class QHBox;
class QSpinBox;
class QLabel;
class QFrame;
class FormWindow;
class QCloseEvent;
class QResizeEvent;
class PropertyWhatsThis;
class QDateEdit;
class QTimeEdit;
class QDateTimeEdit;


class CXmlPropertyList : public PropertyList
{
    Q_OBJECT

public:
    CXmlPropertyList( CXmlPropertyEditor *e );

    virtual void setupProperties();

    virtual void setCurrentItem( QListViewItem *i );
    virtual void valueChanged( PropertyItem *i );
    virtual void refetchData();
    virtual void setPropertyValue( PropertyItem *i );
    virtual void setCurrentProperty( const QString &n );

    void layoutInitValue( PropertyItem *i, bool changed = FALSE );
    PropertyEditor *propertyEditor() const;
    QString whatsThisAt( const QPoint &p );
    void showCurrentWhatsThis();

    enum LastEventType { KeyEvent, MouseEvent };
    LastEventType lastEvent();

public slots:
    void updateEditorSize();
    void resetProperty();
    void toggleSort();

private slots:
    void itemPressed( QListViewItem *i, const QPoint &p, int c );
    void toggleOpen( QListViewItem *i );
    bool eventFilter( QObject *o, QEvent *e );

protected:
    void resizeEvent( QResizeEvent *e );
    bool addPropertyItem( PropertyItem *&item, const QCString &name, QVariant::Type t );

    void viewportDragEnterEvent( QDragEnterEvent * );
    void viewportDragMoveEvent ( QDragMoveEvent * );
    void viewportDropEvent ( QDropEvent * );

protected:
    CXmlPropertyEditor *editor;

private:
    void readPropertyDocs();
    QString whatsThisText( QListViewItem *i );

private:
    PropertyListItem* pressItem;
    QPoint pressPos;
    bool mousePressed;
    bool showSorted;
    QMap<QString, QString> propertyDocs;
    PropertyWhatsThis *whatsThis;
    LastEventType theLastEvent;
};



class CXmlPropertyEditor : public PropertyEditor
{
    Q_OBJECT

public:
    CXmlPropertyEditor( QWidget *parent );

    void clear();
    void setup();

    void emitWidgetChanged();
    void refetchData();

    void closed( FormWindow *w );

    CXmlPropertyList *xmlPropertyList() const;
    //FormWindow *formWindow() const;
    //EventList *eventList() const;

    QString currentProperty() const;
    QString classOfCurrentProperty() const;
    QMetaObject* metaObjectOfCurrentProperty() const;

    void resetFocus();

    void setXmlPropertyEditorEnabled( bool b );

signals:
    void hidden();

public slots:
    void setWidget( QObject *w, FormWindow *fw );

protected:
    void closeEvent( QCloseEvent *e );

private:
    void updateWindow();

private:
    CXmlPropertyList *xmllistview;

};



#endif