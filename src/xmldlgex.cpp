
#include "xmldlgex.h"


/*
class CXmlIODialogEx : public CXmlIODialog
{
public:

//    QPushButton* m_buttonCancel;
    //QPushButton* buttonOk;
    //QPushButton* buttonSaveXML;
    //QPushButton* buttonLoadXML;
    //QListView* m_xmlListView;
    //QDomDocument *m_pDomTree;
};
*/

CXmlIODialogEx::CXmlIODialogEx(QDomDocument *pDomTree, QWidget* parent, const char* name, bool modal, WFlags fl):
	CXmlIODialog(parent, name, modal, fl)	
{
	setCaption(name);

	//init the listview
	m_xmlListView->removeColumn(0);
	m_xmlListView->removeColumn(0);
	m_xmlListView->clear();
    m_xmlListView->addColumn( "Name       " );
    m_xmlListView->addColumn( "Type   " );
    m_xmlListView->addColumn( "Value      " );
    m_xmlListView->addColumn( "Comments     " );
    m_xmlListView->setSorting( -1 );
    m_xmlListView->setRootIsDecorated( TRUE );

	//build the listview from the DomDocument
	m_pDomTree = pDomTree;

    // get the node information from the DOM
    QDomElement root = pDomTree->documentElement();
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
		//puts(node.nodeName());
		if ( node.isElement() && node.nodeName() == "xmldlg" ) {
			QDomElement body = node.toElement();
			_buildTree(NULL, body);
			break;
		}
		node = node.nextSibling();
    }
}


void CXmlIODialogEx::_buildTree(QListViewItem *parentItem, const QDomElement &parentElement )
{
	bool isClassNode, isItemNode;
    QListViewItem *thisItem = NULL;
    QDomNode node = parentElement.firstChild();
    while ( !node.isNull() ) {
		isClassNode = node.nodeName() == "xmlclass";
		isItemNode = node.nodeName() == "xmlitem";
		if (node.isElement() && (isClassNode || isItemNode)){
			// add a new list view item for the outline in column 0
			if ( parentItem == NULL )
				thisItem = new QListViewItem(m_xmlListView, thisItem );
			else
				thisItem = new QListViewItem(parentItem, thisItem);
			thisItem->setText(0, node.toElement().attribute("name"));
			
			// add data entry for column 1,2,3
			if (isItemNode)
				_addItemNodeContent(node, thisItem);

			// recursive build of the tree
			if (isClassNode){
				QString id = node.toElement().attribute("value");
				if (id!="") thisItem->setText(2, id);
				_buildTree( thisItem, node.toElement() );
			}
		}
		node = node.nextSibling();
    }
}


void CXmlIODialogEx::_addItemNodeContent(const QDomNode &inode, QListViewItem *thisItem)
{
	QDomNode node = inode;
	QDomElement e = node.toElement();
	thisItem->setText( 1, e.attribute( "type" ));
	thisItem->setText( 2, e.attribute( "value" ));
	QString str_comment = e.attribute( "comment" );
	if (!str_comment.isNull())
		thisItem->setText( 3, str_comment);			
}
