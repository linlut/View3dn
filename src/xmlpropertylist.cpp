// --------------------------------------------------------------

/*!
  \class CXmlPropertyList propertyeditor.h
  \brief CXmlPropertyList is a QListView derived class which is used for editing widget properties

  This class is used for widget properties. It has to be child of a
  PropertyEditor.

  To initialize it for editing a widget call setupProperties() which
  iterates through the properties of the current widget (see
  PropertyEditor::widget()) and builds the list.

  To update the item values, refetchData() can be called.

  If the value of an item has been changed by the user, and this
  change should be applied to the widget's property, valueChanged()
  has to be called.

  To set the value of an item, setPropertyValue() has to be called.
*/

#include <qpainter.h>
#include <qpalette.h>
#include <qapplication.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qstrlist.h>
#include <qmetaobject.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qfontdialog.h>
#include <qspinbox.h>
#include <qevent.h>
#include <qobjectlist.h>
#include <qlistbox.h>
#include <qfontdatabase.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsizepolicy.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaccel.h>
#include <qworkspace.h>
#include <qtimer.h>
#include <qdragobject.h>
#include <qdom.h>
#include <qprocess.h>
#include <qstyle.h>
#include <qdatetimeedit.h>
#include <qassistantclient.h>
#include <qdrawutil.h>
#include <qmultilineedit.h> // FIXME: obsolete
#include <qsplitter.h>
#include <qdatatable.h>
#include <qtextview.h>
#include <limits.h>

#include "propertyeditor.h"
#include "formwindow.h"
#include "command.h"
#include "metadatabase.h"
#include "propertyobject.h"
//#include <widgetdatabase.h>
#include "widgetfactory.h"
//#include "globaldefs.h"
#include "defs.h"
#include "asciivalidator.h"
#include "paletteeditorimpl.h"
#include "multilineeditorimpl.h"
#include "mainwindow.h"
#include "project.h"
#include "hierarchyview.h"
#include "database.h"
#include "menubareditor.h"
#include "xmlpropertylist.h"

class CXmlPropertyList;


#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H
#include <qcolor.h>
#include <qapplication.h>
#define BOXLAYOUT_DEFAULT_MARGIN 11
#define BOXLAYOUT_DEFAULT_SPACING 6
#ifndef NO_STATIC_COLORS
static QColor *backColor1 = 0;
static QColor *backColor2 = 0;
static QColor *selectedBack = 0;

static void init_colors()
{
    if ( backColor1 )
	return;

#if 0 // a calculated alternative for backColor1
    QColorGroup myCg = qApp->palette().active();
    int h1, s1, v1;
    int h2, s2, v2;
    myCg.color( QColorGroup::Base ).hsv( &h1, &s1, &v1 );
    myCg.color( QColorGroup::Background ).hsv( &h2, &s2, &v2 );
    QColor c( h1, s1, ( v1 + v2 ) / 2, QColor::Hsv );
#endif

    backColor1 = new QColor(  250, 248, 235 );
    backColor2 = new QColor( 255, 255, 255 );
    selectedBack = new QColor( 230, 230, 230 );
}
#endif
#endif


class PropertyWhatsThis : public QWhatsThis
{
private:
    PropertyList *propertyList;

public:
	PropertyWhatsThis( CXmlPropertyList *l ): QWhatsThis( l->viewport() ), propertyList( l )
	{
	}
/*
	QString PropertyWhatsThis::text( const QPoint &pos )
	{
		return propertyList->whatsThisAt( pos );
	}
	
	bool PropertyWhatsThis::clicked( const QString& href )
	{
		if ( !href.isEmpty() ) {
		QAssistantClient *ac = MainWindow::self->assistantClient();
		ac->showPage( QString( qInstallPathDocs() ) + "/html/" + href );
		}
		return FALSE; // do not hide window
	}
*/
};



CXmlPropertyList::CXmlPropertyList( CXmlPropertyEditor *e )
    : PropertyList( e ), editor( e )
{
    init_colors();

    whatsThis = new PropertyWhatsThis( this );
    showSorted = FALSE;
    header()->setMovingEnabled( FALSE );
    header()->setStretchEnabled( TRUE );
    setResizePolicy( QScrollView::Manual );
    viewport()->setAcceptDrops( false );
    viewport()->installEventFilter( this );
    addColumn( tr( "Property" ) );
    addColumn( tr( "Value" ) );

    connect( header(), SIGNAL( sizeChange( int, int, int ) ), this, SLOT( updateEditorSize() ) );
    connect( header(), SIGNAL( sectionClicked( int ) ), this, SLOT( toggleSort() ) );
    connect( this, SIGNAL( pressed( QListViewItem *, const QPoint &, int ) ), this, SLOT( itemPressed( QListViewItem *, const QPoint &, int ) ) );
    connect( this, SIGNAL( doubleClicked( QListViewItem * ) ), this, SLOT( toggleOpen( QListViewItem * ) ) );
    disconnect( header(), SIGNAL( sectionClicked( int ) ), this, SLOT( changeSortColumn( int ) ) );

    setSorting( -1 );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    setColumnWidthMode( 1, Manual );
    mousePressed = FALSE;
    pressItem = 0;
    theLastEvent = MouseEvent;
    header()->installEventFilter( this );
}

void CXmlPropertyList::toggleSort()
{
    showSorted = !showSorted;
    editor->clear();
    editor->setup();
}

void CXmlPropertyList::resizeEvent( QResizeEvent *e )
{
    QListView::resizeEvent( e );
    if ( currentItem() )
	( ( PropertyItem* )currentItem() )->showEditor();
}

static QVariant::Type type_to_variant( const QString &s )
{
    if ( s == "Invalid " )
	return QVariant::Invalid;
    if ( s == "Map" )
	return QVariant::Map;
    if ( s == "List" )
	return QVariant::List;
    if ( s == "String" )
	return QVariant::String;
    if ( s == "StringList" )
	return QVariant::StringList;
    if ( s == "Font" )
	return QVariant::Font;
    if ( s == "Pixmap" )
	return QVariant::Pixmap;
    if ( s == "Brush" )
	return QVariant::Brush;
    if ( s == "Rect" )
	return QVariant::Rect;
    if ( s == "Size" )
	return QVariant::Size;
    if ( s == "Color" )
	return QVariant::Color;
    if ( s == "Palette" )
	return QVariant::Palette;
    if ( s == "ColorGroup" )
	return QVariant::ColorGroup;
    if ( s == "IconSet" )
	return QVariant::IconSet;
    if ( s == "Point" )
	return QVariant::Point;
    if ( s == "Image" )
	return QVariant::Image;
    if ( s == "Int" )
	return QVariant::Int;
    if ( s == "UInt" )
	return QVariant::UInt;
    if ( s == "Bool" )
	return QVariant::Bool;
    if ( s == "Double" )
	return QVariant::Double;
    if ( s == "CString" )
	return QVariant::CString;
    if ( s == "PointArray" )
	return QVariant::PointArray;
    if ( s == "Region" )
	return QVariant::Region;
    if ( s == "Bitmap" )
	return QVariant::Bitmap;
    if ( s == "Cursor" )
	return QVariant::Cursor;
    if ( s == "SizePolicy" )
	return QVariant::SizePolicy;
    if ( s == "Date" )
	return QVariant::Date;
    if ( s == "Time" )
	return QVariant::Time;
    if ( s == "DateTime" )
	return QVariant::DateTime;
    return QVariant::Invalid;
}

#ifndef QT_NO_SQL
static bool parent_is_data_aware( QWidget *w )
{
    QWidget *p = w ? w->parentWidget() : 0;
    while ( p && !p->isTopLevel() ) {
	if ( ::qt_cast<QDesignerDataBrowser*>(p) || ::qt_cast<QDesignerDataView*>(p) )
	    return TRUE;
	p = p->parentWidget();
    }
    return FALSE;
}
#endif

/*!  Sets up the property list by adding an item for each designable
property of the widget which is just edited.
*/

void CXmlPropertyList::setupProperties()
{/*
    if ( !editor->widget() )
	return;
    bool allProperties = !::qt_cast<Spacer*>(editor->widget());
    QStrList lst = editor->widget()->metaObject()->propertyNames( allProperties );
    PropertyItem *item = 0;
    QMap<QString, bool> unique;
    QObject *w = editor->widget();
    QStringList valueSet;
    bool parentHasLayout =
	w->isWidgetType() &&
	!editor->formWindow()->isMainContainer( (QWidget*)w ) && ( (QWidget*)w )->parentWidget() &&
	WidgetFactory::layoutType( ( (QWidget*)w )->parentWidget() ) != WidgetFactory::NoLayout;
    for ( QPtrListIterator<char> it( lst ); it.current(); ++it ) {
	const QMetaProperty* p =
	    editor->widget()->metaObject()->
	    property( editor->widget()->metaObject()->findProperty( it.current(), allProperties), allProperties );
	if ( !p )
	    continue;
	if ( unique.contains( QString::fromLatin1( it.current() ) ) )
	    continue;
	if ( ::qt_cast<QDesignerToolBar*>(editor->widget()) ||
	     ::qt_cast<MenuBarEditor*>(editor->widget()) ) {
	    if ( qstrcmp( p->name(), "minimumHeight" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "minimumWidth" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "maximumHeight" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "maximumWidth" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "geometry" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "sizePolicy" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "minimumSize" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "maximumSize" ) == 0 )
		continue;
	}
	unique.insert( QString::fromLatin1( it.current() ), TRUE );
	if ( editor->widget()->isWidgetType() &&
	     editor->formWindow()->isMainContainer( (QWidget*)editor->widget() ) ) {
	    if ( qstrcmp( p->name(), "geometry" ) == 0 )
		continue;
	} else { // hide some toplevel-only stuff
	    if ( qstrcmp( p->name(), "icon" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "iconText" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "caption" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "sizeIncrement" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "baseSize" ) == 0 )
		continue;
	    if ( parentHasLayout && qstrcmp( p->name(), "geometry" ) == 0 )
		continue;
	    if ( ::qt_cast<QLayoutWidget*>(w) || ::qt_cast<Spacer*>(w) ) {
		if ( qstrcmp( p->name(), "sizePolicy" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "minimumHeight" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "minimumWidth" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "maximumHeight" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "maximumWidth" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "geometry" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "minimumSize" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "maximumSize" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "enabled" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "paletteForegroundColor" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "paletteBackgroundColor" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "paletteBackgroundPixmap" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "palette" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "font" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "cursor" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "mouseTracking" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "focusPolicy" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "acceptDrops" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "autoMask" ) == 0 )
		    continue;
		if ( qstrcmp( p->name(), "backgroundOrigin" ) == 0 )
		    continue;
	    }
	}
	if ( ::qt_cast<QActionGroup*>(w) ) {
	    if ( qstrcmp( p->name(), "usesDropDown" ) == 0 )
		continue;
	    if ( qstrcmp( p->name(), "toggleAction" ) == 0 )
		continue;
	}
	if ( qstrcmp( p->name(), "minimumHeight" ) == 0 )
	    continue;
	if ( qstrcmp( p->name(), "minimumWidth" ) == 0 )
	    continue;
	if ( qstrcmp( p->name(), "maximumHeight" ) == 0 )
	    continue;
	if ( qstrcmp( p->name(), "maximumWidth" ) == 0 )
	    continue;
	if ( qstrcmp( p->name(), "buttonGroupId" ) == 0 ) { // #### remove this when designable in Q_PROPERTY can take a function (isInButtonGroup() in this case)
	    if ( !editor->widget()->isWidgetType() ||
		 !editor->widget()->parent() ||
		 !::qt_cast<QButtonGroup*>(editor->widget()->parent()) )
		continue;
	}

	bool isPropertyObject = w->isA( "PropertyObject" );

	if ( ( p->designable(w) ||
	       isPropertyObject && p->designable( ( (PropertyObject*)w )->widgetList().first() ) ) &&
	     ( !isPropertyObject || qstrcmp( p->name(), "name" ) != 0 ) ) {
	    if ( p->isSetType() ) {
		if ( QString( p->name() ) == "alignment" ) {
		    QStringList lst;
		    lst << p->valueToKey( AlignAuto )
			<< p->valueToKey( AlignLeft )
			<< p->valueToKey( AlignHCenter )
			<< p->valueToKey( AlignRight )
			<< p->valueToKey( AlignJustify );
		    item = new PropertyListItem( this, item, 0, "hAlign", FALSE );
		    item->setValue( lst );
		    setPropertyValue( item );
		    if ( MetaDataBase::isPropertyChanged( editor->widget(), "hAlign" ) )
			item->setChanged( TRUE, FALSE );
		    if ( !::qt_cast<QMultiLineEdit*>(editor->widget()) ) {
			lst.clear();
			lst << p->valueToKey( AlignTop )
			    << p->valueToKey( AlignVCenter )
			    << p->valueToKey( AlignBottom );
			item = new CXmlPropertyListItem( this, item, 0, "vAlign", FALSE );
			item->setValue( lst );
			setPropertyValue( item );
			if ( MetaDataBase::isPropertyChanged( editor->widget(), "vAlign" ) )
			    item->setChanged( TRUE, FALSE );
			item = new PropertyBoolItem( this, item, 0, "wordwrap" );
			if ( ::qt_cast<QGroupBox*>(w) )
			    item->setVisible( FALSE );
			setPropertyValue( item );
			if ( MetaDataBase::isPropertyChanged( editor->widget(), "wordwrap" ) )
			    item->setChanged( TRUE, FALSE );
		    }
		} else {
		    QStrList lst( p->enumKeys() );
		    QStringList l;
		    QPtrListIterator<char> it( lst );
		    while ( it.current() != 0 ) {
			l << QString(*it);
			++it;
		    }
		    item = new PropertyEnumItem( this, item, 0, p->name() );
		    item->setValue( l );
		    setPropertyValue( item );
		    if ( MetaDataBase::isPropertyChanged( editor->widget(), p->name() ) )
			item->setChanged( TRUE, FALSE );
		}
	    } else if ( p->isEnumType() ) {
		QStrList l = p->enumKeys();
		QStringList lst;
		for ( uint i = 0; i < l.count(); ++i ) {
		    QString k = l.at( i );
		    // filter out enum-masks
		    if ( k[0] == 'M' && k[1].category() == QChar::Letter_Uppercase )
			continue;
		    lst << k;
		}
		item = new CXmlPropertyListItem( this, item, 0, p->name(), FALSE );
		item->setValue( lst );
	    } else {
		QVariant::Type t = QVariant::nameToType( p->type() );
		if ( !addPropertyItem( item, p->name(), t ) )
		    continue;
	    }
	}
	if ( item && !p->isSetType() ) {
	    if ( valueSet.findIndex( item->name() ) == -1 ) {
		setPropertyValue( item );
		valueSet << item->name();
	    }
	    if ( MetaDataBase::isPropertyChanged( editor->widget(), p->name() ) )
		item->setChanged( TRUE, FALSE );
	}
    }

    if ( !::qt_cast<QSplitter*>(w) && !::qt_cast<MenuBarEditor*>(w) && !::qt_cast<QDesignerToolBar*>(w) &&
	 w->isWidgetType() && WidgetFactory::layoutType( (QWidget*)w ) != WidgetFactory::NoLayout ) {
	item = new PropertyLayoutItem( this, item, 0, "layoutSpacing" );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "layoutSpacing" )
	     || MetaDataBase::spacing( editor->widget() ) != -1 )
	    layoutInitValue( item, TRUE );
	else
	    layoutInitValue( item );
	item = new PropertyLayoutItem( this, item, 0, "layoutMargin" );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "layoutMargin" )
	     || MetaDataBase::margin( editor->widget() ) != -1 )
	    layoutInitValue( item, TRUE );
	else
	    layoutInitValue( item );
	QWidget *widget = (QWidget*)w;
	if ( ( !::qt_cast<QLayoutWidget*>(widget) &&
	       widget->parentWidget() && ::qt_cast<FormWindow*>(widget->parentWidget()) ) ) {
	    item = new CXmlPropertyListItem( this, item, 0, "resizeMode", FALSE );
	    QStringList lst;
	    lst << "Auto" << "FreeResize" << "Minimum" << "Fixed";
	    item->setValue( lst );
	    setPropertyValue( item );
	    QString resizeMod = MetaDataBase::resizeMode( editor->widget() );
	    if ( !resizeMod.isEmpty() &&
		 resizeMod != WidgetFactory::defaultCurrentItem( editor->widget(), "resizeMode" ) ) {
		item->setChanged( TRUE, FALSE );
		MetaDataBase::setPropertyChanged( editor->widget(), "resizeMode", TRUE );
	    }
	}
    }

    if ( !::qt_cast<Spacer*>(w) && !::qt_cast<QLayoutWidget*>(w) &&
	 !::qt_cast<MenuBarEditor*>(w) && !::qt_cast<QDesignerToolBar*>(w) ) {
	item = new PropertyTextItem( this, item, 0, "toolTip", TRUE, FALSE );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "toolTip" ) )
	    item->setChanged( TRUE, FALSE );
	item = new PropertyTextItem( this, item, 0, "whatsThis", TRUE, TRUE );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "whatsThis" ) )
	    item->setChanged( TRUE, FALSE );
    }

#ifndef QT_NO_SQL
    if ( !::qt_cast<QDataTable*>(editor->widget()) && !::qt_cast<QDataBrowser*>(editor->widget()) &&
	 !::qt_cast<QDataView*>(editor->widget()) && parent_is_data_aware( ::qt_cast<QWidget*>(editor->widget()) ) ) {
	item = new PropertyDatabaseItem( this, item, 0, "database", editor->formWindow()->mainContainer() != w );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "database" ) )
	    item->setChanged( TRUE, FALSE );
    }

    if ( ::qt_cast<QDataTable*>(editor->widget()) || ::qt_cast<QDataBrowser*>(editor->widget()) ||
	 ::qt_cast<QDataView*>(editor->widget()) ) {
	item = new PropertyDatabaseItem( this, item, 0, "database", FALSE );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "database" ) )
	    item->setChanged( TRUE, FALSE );
	item = new PropertyBoolItem( this, item, 0, "frameworkCode" );
	setPropertyValue( item );
	if ( MetaDataBase::isPropertyChanged( editor->widget(), "frameworkCode" ) )
	    item->setChanged( TRUE, FALSE );
    }
#endif

    if ( w->isA("PropertyObject") ) {
	const QWidgetList wl = ( (PropertyObject*)w )->widgetList();
	QPtrListIterator<QWidget> wIt( wl );
	while ( *wIt ) {
	    if ( (*wIt)->inherits("CustomWidget") ) {
		MetaDataBase::CustomWidget *cw = ( (CustomWidget*)*wIt )->customWidget();
		setupCusWidgetProperties( cw, unique, item );
	    }
	    ++wIt;
	}
    } else if ( w->inherits( "CustomWidget" ) ) {
	MetaDataBase::CustomWidget *cw = ( (CustomWidget*)w )->customWidget();
	setupCusWidgetProperties( cw, unique, item );
    }

    setCurrentItem( firstChild() );

    if ( showSorted ) {
	setSorting( 0 );
	sort();
	setSorting( -1 );
	setCurrentItem( firstChild() );
	qApp->processEvents();
    }
*/
    updateEditorSize();
}


bool CXmlPropertyList::addPropertyItem( PropertyItem *&item, const QCString &name, QVariant::Type t )
{
	/*
    if ( name == "buddy" ) {
	CXmlPropertyListItem *itm = new PropertyListItem( this, item, 0, name, TRUE );
	QPtrDict<QWidget> *widgets = editor->formWindow()->widgets();
	QPtrDictIterator<QWidget> it( *widgets );
	QStringList l;
	l << "";
	while ( it.current() ) {
	    if ( editor->formWindow()->canBeBuddy( it.current() ) ) {
		if ( l.find( it.current()->name() ) == l.end() )
		    l << it.current()->name();
	    }
	    ++it;
	}
	l.sort();
	itm->setValue( l );
	item = itm;
	return TRUE;
    }

    switch ( t ) {
    case QVariant::String:
	item = new PropertyTextItem( this, item, 0, name, TRUE,
				     ::qt_cast<QLabel*>(editor->widget()) ||
				     ::qt_cast<QTextView*>(editor->widget()) ||
				     ::qt_cast<QButton*>(editor->widget()) );
	break;
    case QVariant::CString:
	item = new PropertyTextItem( this, item, 0,
				     name, name == "name" &&
				     editor->widget() == editor->formWindow()->mainContainer(),
				     FALSE, TRUE );
	break;
    case QVariant::Bool:
	item = new PropertyBoolItem( this, item, 0, name );
	break;
    case QVariant::Font:
	item = new PropertyFontItem( this, item, 0, name );
	break;
    case QVariant::Int:
	if ( name == "accel" )
	    item = new PropertyTextItem( this, item, 0, name, FALSE, FALSE, FALSE, TRUE );
	else if ( name == "layoutSpacing" || name == "layoutMargin" )
	    item = new PropertyLayoutItem( this, item, 0, name );
	else if ( name == "resizeMode" )
	    item = new CXmlPropertyListItem( this, item, 0, name, TRUE );
	else
	    item = new PropertyIntItem( this, item, 0, name, TRUE );
	break;
    case QVariant::Double:
	item = new PropertyDoubleItem( this, item, 0, name );
	break;
    case QVariant::KeySequence:
	item = new PropertyKeysequenceItem( this, item, 0, name );
	break;
    case QVariant::UInt:
	item = new PropertyIntItem( this, item, 0, name, FALSE );
	break;
    case QVariant::StringList:
	item = new CXmlPropertyListItem( this, item, 0, name, TRUE );
	break;
    case QVariant::Rect:
	item = new PropertyCoordItem( this, item, 0, name, PropertyCoordItem::Rect );
	break;
    case QVariant::Point:
	item = new PropertyCoordItem( this, item, 0, name, PropertyCoordItem::Point );
	break;
    case QVariant::Size:
	item = new PropertyCoordItem( this, item, 0, name, PropertyCoordItem::Size );
	break;
    case QVariant::Color:
	item = new PropertyColorItem( this, item, 0, name, TRUE );
	break;
    case QVariant::Pixmap:
	item = new PropertyPixmapItem( this, item, 0, name, PropertyPixmapItem::Pixmap );
	break;
    case QVariant::IconSet:
	item = new PropertyPixmapItem( this, item, 0, name, PropertyPixmapItem::IconSet );
	break;
    case QVariant::Image:
	item = new PropertyPixmapItem( this, item, 0, name, PropertyPixmapItem::Image );
	break;
    case QVariant::SizePolicy:
	item = new PropertySizePolicyItem( this, item, 0, name );
	break;
    case QVariant::Palette:
	item = new PropertyPaletteItem( this, item, 0, name );
	break;
    case QVariant::Cursor:
	item = new PropertyCursorItem( this, item, 0, name );
	break;
    case QVariant::Date:
	item = new PropertyDateItem( this, item, 0, name );
	break;
    case QVariant::Time:
	item = new PropertyTimeItem( this, item, 0, name );
	break;
    case QVariant::DateTime:
	item = new PropertyDateTimeItem( this, item, 0, name );
	break;
    default:
	return FALSE;
    }
	*/
    return TRUE;
}

void CXmlPropertyList::setCurrentItem( QListViewItem *i )
{
    if ( !i )
	return;

    if ( currentItem() )
	( (PropertyItem*)currentItem() )->hideEditor();
    QListView::setCurrentItem( i );
    ( (PropertyItem*)currentItem() )->showEditor();
}

void CXmlPropertyList::updateEditorSize()
{
    if ( currentItem() )
	( ( PropertyItem* )currentItem() )->showEditor();
}

/*!  This has to be called if the value if \a i should be set as
  property to the currently edited widget.
*/

void CXmlPropertyList::valueChanged( PropertyItem *i )
{
	/*
    if ( !editor->widget() )
	return;
    QString pn( tr( "Set '%1' of '%2'" ).arg( i->name() ).arg( editor->widget()->name() ) );
    SetPropertyCommand *cmd = new SetPropertyCommand( pn, editor->formWindow(),
						      editor->widget(), editor,
						      i->name(), WidgetFactory::property( editor->widget(), i->name() ),
						      i->value(), i->currentItem(), i->currentItemFromObject() );

    cmd->execute();
    editor->formWindow()->commandHistory()->addCommand( cmd, TRUE );
	*/
}

void CXmlPropertyList::layoutInitValue( PropertyItem *i, bool changed )
{
	/*
    if ( !editor->widget() )
	return;
    QString pn( tr( "Set '%1' of '%2'" ).arg( i->name() ).arg( editor->widget()->name() ) );
    SetPropertyCommand *cmd = new SetPropertyCommand( pn, editor->formWindow(),
						      editor->widget(), editor,
						      i->name(), WidgetFactory::property( editor->widget(), i->name() ),
						      i->value(), i->currentItem(), i->currentItemFromObject() );
    cmd->execute();
    if ( i->value().toString() != "-1" )
	changed = TRUE;
    i->setChanged( changed );
	*/
}

void CXmlPropertyList::itemPressed( QListViewItem *i, const QPoint &p, int c )
{
    if ( !i )
	return;
    PropertyItem *pi = (PropertyItem*)i;
    if ( !pi->hasSubItems() )
	return;

    if ( c == 0 && viewport()->mapFromGlobal( p ).x() < 20 )
	toggleOpen( i );
}

void CXmlPropertyList::toggleOpen( QListViewItem *i )
{
    if ( !i )
	return;
    PropertyItem *pi = (PropertyItem*)i;
    if ( pi->hasSubItems() ) {
	pi->setOpen( !pi->isOpen() );
    } else {
	pi->toggle();
    }
}

bool CXmlPropertyList::eventFilter( QObject *o, QEvent *e )
{
	/*
    if ( !o || !e )
	return TRUE;

    PropertyItem *i = (PropertyItem*)currentItem();
    if ( e->type() == QEvent::KeyPress )
	theLastEvent = KeyEvent;
    else if ( e->type() == QEvent::MouseButtonPress )
	theLastEvent = MouseEvent;

    if ( o != this &&e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ( ke->key() == Key_Up || ke->key() == Key_Down ) &&
	     ( o != this || o != viewport() ) &&
	     !( ke->state() & ControlButton ) ) {
	    QApplication::sendEvent( this, (QKeyEvent*)e );
	    return TRUE;
	} else if ( ( !::qt_cast<QLineEdit*>(o) ||
		      ( ::qt_cast<QLineEdit*>(o) && ( (QLineEdit*)o )->isReadOnly() ) ) &&
		    i && i->hasSubItems() ) {
	    if ( !i->isOpen() &&
		 ( ke->key() == Key_Plus ||
		   ke->key() == Key_Right ))
		i->setOpen( TRUE );
	    else if ( i->isOpen() &&
		      ( ke->key() == Key_Minus ||
			ke->key() == Key_Left ) )
		i->setOpen( FALSE );
	} else if ( ( ke->key() == Key_Return || ke->key() == Key_Enter ) && ::qt_cast<QComboBox*>(o) ) {
	    QKeyEvent ke2( QEvent::KeyPress, Key_Space, 0, 0 );
	    QApplication::sendEvent( o, &ke2 );
	    return TRUE;
	}
    } else if ( e->type() == QEvent::FocusOut && ::qt_cast<QLineEdit*>(o) && editor->formWindow() ) {
	QTimer::singleShot( 100, editor->formWindow()->commandHistory(), SLOT( checkCompressedCommand() ) );
    } else if ( o == viewport() ) {
	QMouseEvent *me;
	CXmlPropertyListItem* i;
	switch ( e->type() ) {
	case QEvent::MouseButtonPress:
	    me = (QMouseEvent*)e;
	    i = (CXmlPropertyListItem*) itemAt( me->pos() );
	    if ( i && ( ::qt_cast<PropertyColorItem*>(i) || ::qt_cast<PropertyPixmapItem*>(i) ) ) {
		pressItem = i;
		pressPos = me->pos();
		mousePressed = TRUE;
	    }
	    break;
	case QEvent::MouseMove:
	    me = (QMouseEvent*)e;
	    if ( me && me->state() & LeftButton && mousePressed) {

		i = (CXmlPropertyListItem*) itemAt( me->pos() );
		if ( i  && i == pressItem ) {

		    if (( pressPos - me->pos() ).manhattanLength() > QApplication::startDragDistance() ){
			if ( ::qt_cast<PropertyColorItem*>(i) ) {
			    QColor col = i->value().asColor();
			    QColorDrag *drg = new QColorDrag( col, this );
			    QPixmap pix( 25, 25 );
			    pix.fill( col );
			    QPainter p( &pix );
			    p.drawRect( 0, 0, pix.width(), pix.height() );
			    p.end();
			    drg->setPixmap( pix );
			    mousePressed = FALSE;
			    drg->dragCopy();
			}
			else if ( ::qt_cast<PropertyPixmapItem*>(i) ) {
			    QPixmap pix = i->value().asPixmap();
			    if( !pix.isNull() ) {
				QImage img = pix.convertToImage();
				QImageDrag *drg = new QImageDrag( img, this );
				drg->setPixmap( pix );
				mousePressed = FALSE;
				drg->dragCopy();
			    }
			}
		    }
		}
	    }
	    break;
	default:
	    break;
	}
    } else if ( o == header() ) {
	if ( e->type() == QEvent::ContextMenu ) {
	    ((QContextMenuEvent *)e)->accept();
	    QPopupMenu menu( 0 );
	    menu.setCheckable( TRUE );
	    const int cat_id = 1;
	    const int alpha_id = 2;
	    menu.insertItem( tr( "Sort &Categorized" ), cat_id );
	    int alpha = menu.insertItem( tr( "Sort &Alphabetically" ), alpha_id );
	    if ( showSorted )
		menu.setItemChecked( alpha_id, TRUE );
	    else
		menu.setItemChecked( cat_id, TRUE );
	    int res = menu.exec( ( (QContextMenuEvent*)e )->globalPos() );
	    if ( res != -1 ) {
		bool newShowSorted = ( res == alpha );
		if ( showSorted != newShowSorted ) {
		    showSorted = newShowSorted;
		    editor->clear();
		    editor->setup();
		}
	    }
	    return TRUE;
	}
    }
*/
    return QListView::eventFilter( o, e );

}

/*!  This method re-initializes each item of the property list.
*/

void CXmlPropertyList::refetchData()
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	PropertyItem *i = (PropertyItem*)it.current();
	if ( !i->propertyParent() )
	    setPropertyValue( i );
	if ( i->hasSubItems() )
	    i->initChildren();
	bool changed = MetaDataBase::isPropertyChanged( editor->widget(), i->name() );
	if ( ( i->name() == "layoutSpacing" || i->name() == "layoutMargin" ) ) {
	    if ( i->value().toString() != "-1" )
		i->setChanged( TRUE, FALSE );
	    else
		i->setChanged( FALSE, FALSE );
	}
	else if ( changed != i->isChanged() )
	    i->setChanged( changed, FALSE );
    }
    updateEditorSize();
}

static void clearAlignList( QStrList &l )
{
    if ( l.count() == 1 )
	return;
    if ( l.find( "AlignAuto" ) != -1 )
	l.remove( "AlignAuto" );
    if ( l.find( "WordBreak" ) != -1 )
	l.remove( "WordBreak" );
}

/*!  This method initializes the value of the item \a i to the value
  of the corresponding property.
*/

void CXmlPropertyList::setPropertyValue( PropertyItem *i )
{
	/*
    const QMetaProperty *p =
	editor->widget()->metaObject()->
	property( editor->widget()->metaObject()->findProperty( i->name(), TRUE), TRUE );
    if ( !p ) {
	if ( i->name() == "hAlign" ) {
	    int align = editor->widget()->property( "alignment" ).toInt();
	    p = editor->widget()->metaObject()->
		property( editor->widget()->metaObject()->findProperty( "alignment", TRUE ), TRUE );
	    align &= ~AlignVertical_Mask;
	    QStrList l = p->valueToKeys( align );
	    clearAlignList( l );
	    ( (CXmlPropertyListItem*)i )->setCurrentItem( l.last() );
	} else if ( i->name() == "vAlign" ) {
	    int align = editor->widget()->property( "alignment" ).toInt();
	    p = editor->widget()->metaObject()->
		property( editor->widget()->metaObject()->findProperty( "alignment", TRUE ), TRUE );
	    align &= ~AlignHorizontal_Mask;
	    ( (CXmlPropertyListItem*)i )->setCurrentItem( p->valueToKeys( align ).last() );
	} else if ( i->name() == "wordwrap" ) {
	    int align = editor->widget()->property( "alignment" ).toInt();
	    if ( align & WordBreak )
		i->setValue( QVariant( TRUE, 0 ) );
	    else
		i->setValue( QVariant( FALSE, 0 ) );
	} else if ( i->name() == "layoutSpacing" ) {
	    ( (PropertyLayoutItem*)i )->setValue( MetaDataBase::spacing( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ) ) );
	} else if ( i->name() == "layoutMargin" ) {
	    ( (PropertyLayoutItem*)i )->setValue( MetaDataBase::margin( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ) ) );
	} else if ( i->name() == "resizeMode" ) {
	    ( (CXmlPropertyListItem*)i )->setCurrentItem( MetaDataBase::resizeMode( WidgetFactory::containerOfWidget( (QWidget*)editor->widget() ) ) );
	} else if ( i->name() == "toolTip" || i->name() == "whatsThis" || i->name() == "database" || i->name() == "frameworkCode" ) {
	    i->setValue( MetaDataBase::fakeProperty( editor->widget(), i->name() ) );
	} else if ( editor->widget()->inherits( "CustomWidget" ) ) {
	    MetaDataBase::CustomWidget *cw = ( (CustomWidget*)editor->widget() )->customWidget();
	    if ( !cw )
		return;
	    i->setValue( MetaDataBase::fakeProperty( editor->widget(), i->name() ) );
	}
	return;
    }
    if ( p->isSetType() )
	( (PropertyEnumItem*)i )->setCurrentValues( p->valueToKeys( editor->widget()->property( i->name() ).toInt() ) );
    else if ( p->isEnumType() )
	( (CXmlPropertyListItem*)i )->setCurrentItem( p->valueToKey( editor->widget()->property( i->name() ).toInt() ) );
    else if ( qstrcmp( p->name(), "buddy" ) == 0 )
	( (CXmlPropertyListItem*)i )->setCurrentItem( editor->widget()->property( i->name() ).toString() );
    else
	i->setValue( editor->widget()->property( i->name() ) );
	*/
}

void CXmlPropertyList::setCurrentProperty( const QString &n )
{
    if ( currentItem() && currentItem()->text( 0 ) == n ||
	 currentItem() && ( (PropertyItem*)currentItem() )->propertyParent() &&
	 ( (PropertyItem*)currentItem() )->propertyParent()->text( 0 ) == n )
	return;

    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( it.current()->text( 0 ) == n ) {
	    setCurrentItem( it.current() );
	    break;
	}
    }
}

PropertyEditor *CXmlPropertyList::propertyEditor() const
{
    return editor;
}

void CXmlPropertyList::resetProperty()
{
    if ( !currentItem() )
	return;
    PropertyItem *i = (PropertyItem*)currentItem();
    if ( !MetaDataBase::isPropertyChanged( editor->widget(), i->PropertyItem::name() ) )
	return;
    QString pn( tr( "Reset '%1' of '%2'" ).arg( i->name() ).arg( editor->widget()->name() ) );
    SetPropertyCommand *cmd = new SetPropertyCommand( pn, editor->formWindow(),
						      editor->widget(), editor,
						      i->name(), i->value(),
						      WidgetFactory::defaultValue( editor->widget(), i->name() ),
						      WidgetFactory::defaultCurrentItem( editor->widget(), i->name() ),
						      i->currentItem(), TRUE );
    cmd->execute();
    editor->formWindow()->commandHistory()->addCommand( cmd, FALSE );
    if ( i->hasSubItems() )
	i->initChildren();
}

void CXmlPropertyList::viewportDragEnterEvent( QDragEnterEvent *e )
{
	/*
    CXmlPropertyListItem *i = (CXmlPropertyListItem*) itemAt( e->pos() );
    if( !i ) {
	e->ignore();
	return;
    }

    if ( ::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode( e ) )
	e->accept();
    else if ( ::qt_cast<PropertyPixmapItem*>(i) && QImageDrag::canDecode( e ) )
	e->accept();
    else
	e->ignore();
	*/
}

void CXmlPropertyList::viewportDragMoveEvent ( QDragMoveEvent *e )
{
    PropertyListItem *i = (PropertyListItem*) itemAt( e->pos() );
    if( !i ) {
	e->ignore();
	return;
    }

    if ( ::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode( e ) )
	e->accept();
    else if ( ::qt_cast<PropertyPixmapItem*>(i) && QImageDrag::canDecode( e ) )
	e->accept();
    else
	e->ignore();
}

void CXmlPropertyList::viewportDropEvent ( QDropEvent *e )
{
	/*
    CXmlPropertyListItem *i = (CXmlPropertyListItem*) itemAt( e->pos() );
    if( !i ) {
	e->ignore();
	return;
    }

    if ( ::qt_cast<PropertyColorItem*>(i) && QColorDrag::canDecode( e ) ) {
	QColor color;
	QColorDrag::decode( e, color );
	i->setValue( QVariant( color ) );
	valueChanged( i );
	e->accept();
    }
    else if ( ::qt_cast<PropertyPixmapItem*>(i)  && QImageDrag::canDecode( e ) ) {
	QImage img;
	QImageDrag::decode( e, img );
	QPixmap pm;
	pm.convertFromImage( img );
	i->setValue( QVariant( pm ) );
	valueChanged( i );
	e->accept();
    }
    else
	e->ignore();
	*/
}

QString CXmlPropertyList::whatsThisAt( const QPoint &p )
{
    return whatsThisText( itemAt( p ) );
}

void CXmlPropertyList::showCurrentWhatsThis()
{
    if ( !currentItem() )
	return;
    QPoint p( 0, currentItem()->itemPos() );
    p = viewport()->mapToGlobal( contentsToViewport( p ) );
    QWhatsThis::display( whatsThisText( currentItem() ), p, viewport() );
}

QString CXmlPropertyList::whatsThisText( QListViewItem *i )
{
    if ( !i || !editor->widget() )
	return QString::null;
    readPropertyDocs();
    if ( ( (PropertyItem*)i )->propertyParent() )
	i = ( (PropertyItem*)i )->propertyParent();

    const QMetaObject *mo = editor->widget()->metaObject();
    QString prop = ( (PropertyItem*)i )->name();
    while ( mo ) {
	QString s;
	s = QString( mo->className() ) + "::" + prop;
	QMap<QString, QString>::Iterator it;
	if ( ( it = propertyDocs.find( s ) ) != propertyDocs.end() ) {
	    return *it;
	}
	mo = mo->superClass();
    }

    return tr( "<p><b>QWidget::%1</b></p><p>There is no documentation available for this property.</p>" ).arg( prop );
}

void CXmlPropertyList::readPropertyDocs()
{
    if ( !propertyDocs.isEmpty() )
	return;

    QString docFile = MainWindow::self->documentationPath() + "/propertydocs";
    QFile f( docFile );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QDomDocument doc;
    QString errMsg;
    int errLine;
    if ( !doc.setContent( &f, &errMsg, &errLine ) )
	return;
    QDomElement e = doc.firstChild().toElement().firstChild().toElement();

    for ( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	QDomElement n = e.firstChild().toElement();
	QString name;
	QString doc;
	for ( ; !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "name" )
		name = n.firstChild().toText().data();
	    else if ( n.tagName() == "doc" )
		doc = n.firstChild().toText().data();
	}
	doc.insert( 0, "<p><b>" + name + "</b></p>" );
	propertyDocs.insert( name, doc );
    }
}

CXmlPropertyList::LastEventType CXmlPropertyList::lastEvent()
{
    return theLastEvent;
}


// --------------------------------------------------------------

/*!
  \class CXmlPropertyEditor CXmlPropertyEditor.h
  \brief PropertyEdior toplevel window

  This is the toplevel window of the property editor which contains a
  listview for editing properties.
*/

CXmlPropertyEditor::CXmlPropertyEditor( QWidget *parent )
    : PropertyEditor( parent )
{
    setCaption( tr( "Property Editor" ) );
    xmllistview = new CXmlPropertyList( this );
    //addTab( listview, tr( "P&roperties" ) );
}

void CXmlPropertyEditor::setWidget( QObject *w, FormWindow *fw )
{
	/*
    if ( fw && fw->isFake() )
	w = fw->project()->objectForFakeForm( fw );
    eList->setFormWindow( fw );
    if ( w && w == wid ) {
	// if there is no properties displayed then we really should show them.
	// to do this check the number of properties in the list.
	bool ret = (listview->childCount() > 0) ? TRUE : FALSE;
	if ( wid->isWidgetType() && WidgetFactory::layoutType( (QWidget*)wid ) != WidgetFactory::NoLayout ) {
	    QListViewItemIterator it( listview );
	    ret = FALSE;
	    while ( it.current() ) {
		if ( it.current()->text( 0 ) == "layoutSpacing" || it.current()->text( 0 ) == "layoutMargin" ) {
		    ret = TRUE;
		    break;
		}
		++it;
	    }
	}
	if ( ret )
	    return;
    }

    if ( !w || !fw ) {
	setCaption( tr( "Property Editor" ) );
	clear();
	wid = 0;
	formwindow = 0;
	return;
    }

    wid = w;
    formwindow = fw;
    setCaption( tr( "Property Editor (%1)" ).arg( formwindow->name() ) );
    listview->viewport()->setUpdatesEnabled( FALSE );
    listview->setUpdatesEnabled( FALSE );
    clear();
    listview->viewport()->setUpdatesEnabled( TRUE );
    listview->setUpdatesEnabled( TRUE );
    setup();
	*/
}

void CXmlPropertyEditor::clear()
{
	/*
    listview->setContentsPos( 0, 0 );
    listview->clear();
	*/
}

void CXmlPropertyEditor::setup()
{
	/*
    if ( !formwindow || !wid )
	return;
    listview->viewport()->setUpdatesEnabled( FALSE );
    listview->setupProperties();
    listview->viewport()->setUpdatesEnabled( TRUE );
    listview->updateEditorSize();

    // the forced processing of events in setupProperties() can set wid to 0, so we have to check
    if ( wid && !wid->isA( "PropertyObject" ) ) {
	eList->viewport()->setUpdatesEnabled( FALSE );
	eList->setup();
	eList->viewport()->setUpdatesEnabled( TRUE );
    }
	*/
}

void CXmlPropertyEditor::refetchData()
{
//    listview->refetchData();
}

void CXmlPropertyEditor::emitWidgetChanged()
{
    //if ( formwindow && wid )
	//formwindow->widgetChanged( wid );
}

void CXmlPropertyEditor::closed( FormWindow *w )
{
	/*
    if ( w == formwindow ) {
	formwindow = 0;
	wid = 0;
	clear();
    }
	*/
}

void CXmlPropertyEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

CXmlPropertyList *CXmlPropertyEditor::xmlPropertyList() const
{
    return xmllistview;
}

QString CXmlPropertyEditor::currentProperty() const
{/*
    if ( !wid )
	return QString::null;
    if ( ( (PropertyItem*)listview->currentItem() )->propertyParent() )
	return ( (PropertyItem*)listview->currentItem() )->propertyParent()->name();
	*/
    return ( (PropertyItem*)xmllistview->currentItem() )->name();
}

QString CXmlPropertyEditor::classOfCurrentProperty() const
{
	/*
    if ( !wid )
	return QString::null;
    QObject *o = wid;
    QString curr = currentProperty();
    QMetaObject *mo = o->metaObject();
    while ( mo ) {
	QStrList props = mo->propertyNames( FALSE );
	if ( props.find( curr.latin1() ) != -1 )
	    return mo->className();
	mo = mo->superClass();
    }
	*/
    return QString::null;
}

QMetaObject* CXmlPropertyEditor::metaObjectOfCurrentProperty() const
{
	/*
    if ( !wid )
	return 0;
    return wid->metaObject();
	*/
	return 0;
}

void CXmlPropertyEditor::resetFocus()
{
	/*
    if ( listview->currentItem() )
	( (PropertyItem*)listview->currentItem() )->showEditor();
	*/
}

void CXmlPropertyEditor::setXmlPropertyEditorEnabled( bool b )
{
	/*
    if ( !b )
	removePage( listview );
    else
	insertTab( listview, tr( "Property Editor" ), 0 );
    updateWindow();
	*/
}

void CXmlPropertyEditor::updateWindow()
{
    if ( isHidden() && count() ) {
	parentWidget()->show();
	MainWindow::self->setAppropriate( (QDockWindow*)parentWidget(), TRUE );
    } else if ( isShown() && !count() ) {
	parentWidget()->hide();
	MainWindow::self->setAppropriate( (QDockWindow*)parentWidget(), FALSE );
    }
}

