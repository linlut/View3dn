/****************************************************************************
*
*    MaterialTree.h
*
*****************************************************************************/

#ifndef __CCIA3dDLG_32111_H__
#define __CCIA3dDLG_32111_H__


class QListView;
class QLabel;
class QPainter;
class QColorGroup;
class QObjectList;
class QPopupMenu;

#include <assert.h>
#include <qlistview.h>
#include <qdom.h>
#include "materialtree.h"


class CMaterialTree : public QListView
{
    Q_OBJECT

public:

    CMaterialTree(const QString& appPathDir, SET_OBJECT_MATERIAL_FUNC_T* pfunc, QWidget *parent=0, const char *name=0);

    ~CMaterialTree();

private:

	bool m_bModified;

    QDomDocument m_libDomTree;

	QDomDocument m_usrDomTree;

	QString m_appPathDir;			///directory of the application;

	SET_OBJECT_MATERIAL_FUNC_T* m_pSetObjectMaterial;

private:

	///============BUILD TREE=================================
    void getHeaderInformation( const QDomElement &header);

    void buildTree( QListViewItem *parentItem, const QDomElement &parentElement );

	void addItemNodeContent(const QDomNode &inode, QListViewItem *pParentItem);

	QListViewItem* addMatItemNodeContent(QDomNode &inode, QListViewItem *pParentItem, QListViewItem *preItem);

	///load material file
	bool _loadMaterialFile(const QString &fileName, const int dst);

	bool _saveMaterialFile(void);

	///============SEARCH TREE=================================
	QListViewItem* _getRootListViewItem(QListViewItem *pListViewItem)
	{
		QListViewItem * p = pListViewItem, *root=NULL;
		while (p!=NULL){
			root = p;
			p=p->parent();
		}
		return root;
	}

	bool _isMatlibListViewItem(QListViewItem *pa)
	{
		QListViewItem *proot = _getRootListViewItem(pa);
		if (proot==NULL) 
			assert(0);
		QString n0 = proot->text(0);
		if (n0=="Predefined Materials"){
			return true;
		}
		return false;
	}

	bool _isItem(QListViewItem *pitem)
	{
		QListViewItem *pp = pitem->firstChild();
		if (pp == NULL) return true;
		return false;
	}

	bool _isMatitem(QListViewItem *pitem)
	{
		QListViewItem *pp = pitem->firstChild();
		if (pp == NULL) return false;
		if (pp->firstChild()) return false;
		if (pp->text(1)!="" && pp->text(2)!="") 
			return true;
		return false;
	}

	bool _isLowestLevelFolder(QListViewItem *pitem)
	{
		if (pitem->text(1)!="") 
			return false;
		QString p = pitem->text(2);
		if (p=="") return false;
		const char *pchar = p.ascii();
		if (pchar[0]=='C' && pchar[1]=='L')
			return true;
		return false;
	}

	bool _isMatclass(QListViewItem *pitem)
	{
		bool f=false;
		if ((!_isItem(pitem)) && (!_isMatitem(pitem)))
			f = true;
		return f;
	}

	QDomNode* CMaterialTree::_searchMaterialByID(const QDomNode& anode, const QString &id);


	///============EDITING================================

	void _modifyMaterialMatitem(QListViewItem *pitem);

	void _modifyMaterialItem(QListViewItem *pitem);

	void _modifyMaterialNode(const QDomNode& n, const QString &attrname, const QString &attrval)
	{
		QDomNode a = n.firstChild();
		while (!a.isNull()) {
			QString name = a.toElement().attribute("name");
			if (name==attrname){
				a.toElement().setAttribute("value", attrval);
				break;
			}
			a = a.nextSibling();
		}
	}


public:

	QDomNode* CMaterialTree::searchMaterialByID(const QString &id)
	{
		QDomElement root1 = m_libDomTree.documentElement();
		QDomNode * p = _searchMaterialByID(root1, id);
		if (!p){
			QDomElement root2 = m_usrDomTree.documentElement();
			p = _searchMaterialByID(root2, id);
		}
		return p;
	}


protected slots:

	void slotReturnPressed(QListViewItem * pListViewItem);

	void slotDoubleClicked(QListViewItem *pListViewItem, const QPoint & pt, int col);

	void slotRightButtonPressed(QListViewItem *pitem, const QPoint & pt, int col);

	void slotNewMatItem(void);

	void slotDelMatItem(void);

	void slotModifyMaterialItem()
	{
		QListViewItem *pitem = currentItem();
		if (pitem)
			_modifyMaterialItem(pitem);
	}

	void slotModifyMaterialMatItem()
	{
		QListViewItem *pitem = currentItem();
		if (!pitem) return;

		_modifyMaterialMatitem(pitem);
	}

	void slotSaveMaterialFile(void)
	{
		_saveMaterialFile();
	}

	void slotSaveMaterialFileAs(void);

	void slotSpecifyObjectMaterial(void);

};

#endif // CMaterialTree_H
