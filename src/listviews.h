
#ifndef __FEMATTR_LISTVIEWS311_H__
#define __FEMATTR_LISTVIEWS311_H__

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

class CFemAttribListView : public QListView
{
    Q_OBJECT

private:
	QListViewItem *m_pAttribItem;
	QListViewItem *m_pPickAttribItem;

	CSceneNode    *m_pActiveSceneNode;
    CNameTableItem m_table[8];
    int m_nTableLen;

private:

	QListViewItem* _setAttribData(const CFemObjAttrib * pAttrib, const char* name);

	void _restoreFromItem(const QListViewItem *pItem, CFemObjAttrib *pAttrib);

public:
    CFemAttribListView(CSceneNode * pActiveNode, QWidget *parent = 0, const char *name = 0 );

    ~CFemAttribListView()
    {}

	void setAttribData(CSceneNode *pnode);

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

	void activeSceneNodeChanged(CSceneNode* p)
	{
		this->setAttribData(p);
	}

	void pickingCursorChanged(QCursor *qcur)
	{
	}
};



#endif
