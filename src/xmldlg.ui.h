/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <assert.h>
#include "qinputdialog.h"
#include "qpopupmenu.h"



void CXmlIODialog::buttonLoadXMLClicked(void)
{
}


void CXmlIODialog::buttonSaveXMLClicked(void)
{

}


void CXmlIODialog::slotListViewDoubleClick(QListViewItem * pitem, const QPoint & pt, int xx)
{
	if (pitem==NULL) return;
	QString typestr = pitem->text(1);
	if (typestr.isNull()) return;

	const QString propname = pitem->text(0);
	const QString existingStr = pitem->text(2);
	QString text;
	bool ok=false;

	if (typestr=="multi"){
		QDomNode* pnode = _searchItemNodeByName(m_pDomTree->documentElement(), propname);
		if (pnode==NULL)
			return;
		QString options = pnode->toElement().attribute("option");
		QStringList oplist = QStringList::split(",", options);

		//create a menu from which to choose
	    QPopupMenu * file = new QPopupMenu(this);
		int menuid = 0;
		for (QStringList::Iterator it = oplist.begin(); it != oplist.end(); ++it, menuid++){
			QString opStr = *it;
			file->insertItem(opStr, menuid, menuid);
		    connect(file, SIGNAL(activated(int)), this, SLOT(slotSetOptionFromMenu(int)));
		}
		file->popup(pt);
		return;
	}
	else if (typestr=="bool"){
		const char * pch = existingStr.ascii();
		if (pch[0]=='y')
			text = "n";
		else
			text = 'y';
		pitem->setText(2, text);
		ok = true;
	}
	else{		
		//modify the value in the listview table
		QString promptStr = "Input " + propname + ":";
		text = QInputDialog::getText(
				promptStr, "Change the value:", QLineEdit::Normal,
				existingStr, &ok, this );
		if (!ok || text.isEmpty())
			return;
		pitem->setText(2, text);
	}

	//modify the value in the XML tree node;
	if (ok && existingStr!=text){
		QDomNode* pnode = _searchItemNodeByName(m_pDomTree->documentElement(), propname);
		if (pnode!=NULL)
			pnode->toElement().setAttribute("value", text);
	}
}


QDomNode* CXmlIODialog::_searchItemNodeByName( const QDomNode & anode, const QString &id )
{
	static QDomNode staticnode;
	QDomNode node = anode.firstChild();
	while (!node.isNull()) {
		const QString nodename = node.nodeName();
		if (nodename == "xmlitem"){
			QString propname = node.toElement().attribute( "name");
			if (propname==id){
				staticnode = node;
				return &staticnode;
			}
		}
		else{
			QDomNode * r = _searchItemNodeByName(node, id);
			if (r) return r;
		}
		node = node.nextSibling();
	}
	return NULL;
}


void CXmlIODialog::slotSetOptionFromMenu(int id)
{
	QListViewItem * pitem = m_xmlListView->currentItem();
	if (pitem==NULL) return;

	char sbuff[64];
	const int mid = id + 1;
	sprintf(sbuff, "%d", mid);
	const QString val(sbuff);
	const QString propname(pitem->text(0));
	pitem->setText(2, val);

	QDomNode* pnode = _searchItemNodeByName(m_pDomTree->documentElement(), propname);
	if (pnode!=NULL)
		pnode->toElement().setAttribute("value", val);
}
