//FILE: objlistview.h

#ifndef __OBJLISTVIEW_ZN2011_H__
#define __OBJLISTVIEW_ZN2011_H__

#include <qsplitter.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
//#include <qptrlist.h>
#include <qlistview.h>

#include "scenegraph.h"
#include "nametableitem.h"

#if QT_VERSION >= 0x040000
  #include <QListWidgetItem>
  #include <QListWidget>
  #define QListViewItem QListWidgetItem 
#else
	class QListView;
#endif

class QLabel;
class QPainter;
class QColorGroup;
class QPopupMenu;

// -----------------------------------------------------------------
class CObjectListView : public QListView
{
    Q_OBJECT

private:
	CSceneGraph *m_pSG;

private:
	QListViewItem* _AddOneEntry(const CSceneNode *pnode);

	void _restoreFromItem(const QListViewItem *pItem, CFemObjAttrib *pAttrib);

public:
    CObjectListView(CSceneGraph &sg, QWidget *parent = 0, const char *name = 0 );

    ~CObjectListView()
    {}

	void update(CSceneGraph &sg);

protected:
    void initFolders();

    //void initFolder( Folder *folder, unsigned int &count );
    void setupFolders();

public slots:
	//This signal is emitted whenever an item is double-clicked. It's emitted on the 
	//second button press, not the second button release. The arguments are the relevant 
	//QListViewItem (may be 0), the point in global coordinates and the relevant column 
	//(or -1 if the click was outside the list). 
	void slotDoubleClicked ( QListViewItem *pListViewItem, const QPoint & pt, int x);

	void activeSceneNodeChanged(CSceneGraph& sg)
	{
		this->update(sg);
	}

	void pickingCursorChanged(QCursor *qcur)
	{
	}

};



#endif
