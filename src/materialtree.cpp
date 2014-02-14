/****************************************************************************
** $Id: qt/MaterialTree.cpp   3.3.1   edited May 27 2003 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "materialtree.h"
#include <qfile.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>


void DomNode2Cia3dMaterial(const int orderid, const QDomNode& inode, FILE *fp)
{
	char symbols[1024], values[1024];
	int sptr=0, vptr=0;

    QDomNode node = inode.firstChild();
	while (!node.isNull()){
		QString nodename = node.toElement().attribute("name");
		QString nodevalue = node.toElement().attribute("value");
		if (nodename=="SEPARATOR"){
			fprintf(fp, "%s:\n", symbols);
			fprintf(fp, "%s\n", values);
			sptr = vptr = 0;
			symbols[0] = values[0] = 0;
		}
		else{
			const char * pstr = nodename.ascii();
			const int slen = strlen(pstr);
			char formatstr [50];
			sprintf(&symbols[sptr], "%s ", pstr);
			if (nodename=="ID_MAT"){
				sprintf(formatstr, "%c%dd ", 0x25, slen);
				sprintf(&values[vptr], formatstr, orderid);
			}
			else{
				sprintf(formatstr, "%c%ds ", 0x25, slen);
				sprintf(&values[vptr], formatstr, nodevalue.ascii());
			}
			sptr = strlen(symbols);
			vptr = strlen(values);
		}
		node = node.nextSibling();
	}

	if (sptr>0 && vptr>0){
		fprintf(fp, "%s:\n", symbols);
		fprintf(fp, "%s\n", values);
	}
	fprintf(fp, "\n\n");
}


static inline 
void ClearDomMaterialNode(QDomNode& node)
{
	QDomElement e = node.toElement();
	e.setAttribute("name", "NONAME");

	QDomNode p = e.firstChild();
	while (!p.isNull()){
		const QString nname = p.nodeName();
		if (nname=="item"){
			p.toElement().setAttribute("value", "X");
		}
		p = p.nextSibling();
	}
}


static inline
void GenerateNewNodeID(const QString& id, QDomNode& parent, QDomNode& newnode) 
{
	const int BUFFLEN = 2048;
	int i;
	char buff[200];
	strcpy(buff, id.ascii());
	buff[0]='0', buff[1]='0';
	const int base = atoi(buff);

	unsigned char *signs= new unsigned char [BUFFLEN];
	assert(signs!=NULL);
	for (i=0; i<BUFFLEN; i++) signs[i]=0;

	QDomNode node = parent.firstChild();
	while (!node.isNull()){
		QString val = node.toElement().attribute("value");
		QString name = node.toElement().attribute("name");
		puts(name.ascii());
		const char *pch = val.ascii();
		assert(pch!=NULL);
		int n = atoi(pch) -base;
		assert(n>=0 && n<BUFFLEN);
		signs[n]=1;
		node = node.nextSibling();
	}

	for (i=1; i<BUFFLEN; i++)
		if (signs[i]==0) break;
	delete []signs;

	const int foundid = i + base;
	sprintf(buff, "%6d", foundid);
	newnode.toElement().setAttribute("value", buff);
}


//====================================================================

#define MATLIB_FILENAME "/matlib.xml"
#define MATUSR_FILENAME "/matusr.xml"

CMaterialTree::CMaterialTree( const QString& appdir, CSceneNode *pnode, QWidget *parent, const char *listviewname)
    : QListView( parent, listviewname )
{
	m_pActiveSceneNode = pnode;

    // div. configuration of the list view
    addColumn( "CLASS" );
    addColumn( "ID                               " );
    addColumn( "VALUE    " );
    setSorting( -1 );
    setRootIsDecorated( TRUE );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOn );
    setColumnWidthMode( 1, Manual );

	m_appPathDir = appdir; 
	QString libfile = appdir + MATLIB_FILENAME;
	QString usrfile = appdir + MATUSR_FILENAME;

	_loadMaterialFile(usrfile, 1);
	_loadMaterialFile(libfile, 0);
	m_bModified = false;

	connect(this, SIGNAL(returnPressed(QListViewItem *)), 
			this, SLOT(slotReturnPressed(QListViewItem *)));
	connect(this, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)), 
			this, SLOT(slotDoubleClicked(QListViewItem *, const QPoint &, int)));
	connect(this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), 
			this, SLOT(slotRightButtonPressed(QListViewItem *, const QPoint &, int)));
	
}


CMaterialTree::~CMaterialTree()
{
	if (m_bModified){
		_saveMaterialFile();
	}
}


QDomNode* CMaterialTree::_searchMaterialByID(const QDomNode& anode, const QString &id)
{
	static QDomNode staticnode;
	QDomNode node = anode.firstChild();
	while (!node.isNull()) {
		const QString nname = node.nodeName();
		if (nname == "matclass" || nname == "matitem"){
			QString val = node.toElement().attribute( "value");
//			printf("%s\n", val.ascii());
			if (val==id){
				staticnode = node;
				return &staticnode;
			}
		}
		else if (nname=="item" || nname=="hditem")
			return NULL;

		if (nname != "matitem"){
			QDomNode * r = _searchMaterialByID(node, id);
			if (r) return r;
		}
		node = node.nextSibling();
	}
	return NULL;
}


bool CMaterialTree::_loadMaterialFile(const QString &fileName, const int dst)
{
	// load which file?
	QDomDocument *pDomTree = NULL;
	if (dst==0) 
		pDomTree = &m_libDomTree;
	else
		pDomTree = &m_usrDomTree;

    // read the XML file and create DOM tree
    QFile opmlFile( fileName );
    if (!opmlFile.open(IO_ReadOnly)){
		QMessageBox::critical(0, tr("Critical Error"), tr("Cannot open file %1").arg(fileName));
		return false;
    }

    if (!pDomTree->setContent(&opmlFile)){
		QMessageBox::critical(0, tr("Critical Error"), tr("Parsing error for file %1").arg(fileName));
		opmlFile.close();
		return false;
    }
    opmlFile.close();

    // get the header information from the DOM
    QDomElement root = pDomTree->documentElement();
    QDomNode node = root.firstChild();
    while ( !node.isNull() ) {
		if ( node.isElement() && node.nodeName() == "head" ) {
			QDomElement header = node.toElement();
			getHeaderInformation( header );
			break;
		}
		node = node.nextSibling();
    }

    // create the tree view out of the DOM
    node = root.firstChild();
    while ( !node.isNull() ) {
		if ( node.isElement() && node.nodeName() == "body" ) {
			QDomElement body = node.toElement();
			buildTree( 0, body );
			break;
		}
		node = node.nextSibling();
    }
	return true;
}


void CMaterialTree::getHeaderInformation( const QDomElement &header )
{
    // visit all children of the header element and look if you can make
    // something with it
    QDomNode node = header.firstChild();
    while ( !node.isNull() ) {
		if ( node.isElement() ) {
			// case for the different header entries
			if ( node.nodeName() == "title" ) {
				QDomText textChild = node.firstChild().toText();
				if ( !textChild.isNull() ) {
					setColumnText( 0, textChild.nodeValue() );
				}
			}
		}
		node = node.nextSibling();
    }
	setColumnText( 1, "ID");
	setColumnText( 2, "VALUE");

}


void CMaterialTree::buildTree(QListViewItem *parentItem, const QDomElement &parentElement )
{
	bool isClassNode, isItemNode;
    QListViewItem *thisItem = NULL;
    QDomNode node = parentElement.firstChild();
    while ( !node.isNull() ) {
		isClassNode = node.nodeName() == "matclass";
		isItemNode = node.nodeName() == "matitem";
		if (node.isElement() && (isClassNode || isItemNode)){
			// add a new list view item for the outline in column 0
			if ( parentItem == 0 )
				thisItem = new QListViewItem( this, thisItem );
			else
				thisItem = new QListViewItem( parentItem, thisItem);
			thisItem->setText( 0, node.toElement().attribute("name"));
			const QString id = node.toElement().attribute("value");
			if (id!="") thisItem->setText(2, id);
			
			// add data entry for column 1 and 2
			if (isItemNode)
				addItemNodeContent(node, thisItem);

			// recursive build of the tree
			if (isClassNode){
				buildTree( thisItem, node.toElement());
			}
		}
		node = node.nextSibling();
    }
}


void CMaterialTree::addItemNodeContent(const QDomNode &inode, QListViewItem *pParentItem)
{
    QDomNode node = inode.firstChild();
	QListViewItem *thisItem = NULL;
	while (!node.isNull()){
		if (node.nodeName() == "item"){
		    thisItem = thisItem = new QListViewItem(pParentItem, thisItem);
			thisItem->setText( 1, node.toElement().attribute( "name" ));
			thisItem->setText( 2, node.toElement().attribute( "value" ));			
		}
		node = node.nextSibling();
	}	
}


QListViewItem * CMaterialTree::addMatItemNodeContent(QDomNode &node, QListViewItem *parentItem, QListViewItem *preItem)
{
	QListViewItem * thisItem=preItem;

	if (parentItem == NULL)
		thisItem = new QListViewItem(this, thisItem);
	else
		thisItem = new QListViewItem(parentItem, thisItem);

	const QDomElement& e = node.toElement();
	thisItem->setText(0, e.attribute("name"));
	QString id = e.attribute("value");
	if (id!="") thisItem->setText(2, id);			
	addItemNodeContent(node, thisItem);
	return thisItem;
}


bool CMaterialTree::_saveMaterialFile(void)
{
	QString usrfile = m_appPathDir + MATUSR_FILENAME;
	QDomDocument *pDomTree = &m_usrDomTree;

	const QString& str = pDomTree->toString(4);
	FILE *fp =fopen(usrfile.ascii(), "w");
	fprintf(fp, "%s\n", str.ascii());
	fclose(fp);
	return true;
}


void CMaterialTree::_modifyMaterialMatitem(QListViewItem *pitem)
{
	bool ok;

	//modify the value in the listview table
	QString existingStr = pitem->text(0);
	QString promptStr = "New material name";
    QString text = QInputDialog::getText(
            promptStr, "Change the value:", QLineEdit::Normal,
            existingStr, &ok, this );
    if (!ok || text.isEmpty())
		return;

	//set the value on the GUI
	pitem->setText(0, text);

	//modify the value in the XML tree node;
	QString id = pitem->text(2);
	assert(id!="");
	QDomNode* pnode = _searchMaterialByID(m_usrDomTree.documentElement(), id);
	assert(pnode!=NULL);
	pnode->toElement().setAttribute("name", text);

	m_bModified = true;
}


void CMaterialTree::_modifyMaterialItem(QListViewItem *pitem)
{
	bool ok;

	//modify the value in the listview table
	QString idString = pitem->text(1);
	QString existingStr = pitem->text(2);
	QString promptStr = "Input " + idString + ":";
    QString text = QInputDialog::getText(
            promptStr, "Change the value:", QLineEdit::Normal,
            existingStr, &ok, this );
    if (!ok || text.isEmpty())
		return;

	//set the value on the GUI
	pitem->setText(2, text);

	//modify the value in the XML tree node;
	QListViewItem *pp = pitem->parent();
	assert(pp!=NULL);
	QString parentID = pp->text(2);
	assert(parentID!="");
	QDomNode* pnode = _searchMaterialByID(m_usrDomTree.documentElement(), parentID);
	assert(pnode!=NULL);
	_modifyMaterialNode(*pnode, idString, text);

	m_bModified = true;
}



void CMaterialTree::slotDoubleClicked(QListViewItem *pListViewItem, const QPoint & pt, int xx)
{
	//if not the lowest level view items, no change
	if (pListViewItem==NULL) return;
	const bool isfolder = _isLowestLevelFolder(pListViewItem);
	if (isfolder) return;

	const bool isitem = _isItem(pListViewItem);
	const bool ismatitem = _isMatitem(pListViewItem);
	if ((!isitem) && (!ismatitem)) return;
	
	//if item is in the matlib, no change;
	if (_isMatlibListViewItem(pListViewItem))
		return;

	//modify the value in the listview table
	if (isitem)
		_modifyMaterialItem(pListViewItem);
	else
		_modifyMaterialMatitem(pListViewItem);
}


void CMaterialTree::slotReturnPressed(QListViewItem * pListViewItem)
{
	/*
	QDomNode* p = searchMaterialByID("800001");
	if (p){
		FILE *fp = fopen("c:/users/nanzhang/mat.txt", "w");
		DomNode2Cia3dMaterial(0, *p, fp);
		fclose(fp);
	}
	*/
}


void CMaterialTree::slotRightButtonPressed(QListViewItem *pitem, const QPoint & pt, int)
{
	if (pitem==NULL) return;
    QPopupMenu * file = NULL;
	file= new QPopupMenu(this);

	if (_isMatlibListViewItem(pitem)){
		if (_isLowestLevelFolder(pitem)){
			file->insertItem( "Sort", this,  SLOT(NULL), NULL);
		}
		else if (_isMatitem(pitem)){
			file->insertItem("Specify Object Material", this, SLOT(slotSpecifyObjectMaterial()));
		}
		else{ 
			delete file;
			return;
		}
	}
	else{
		if (_isLowestLevelFolder(pitem)){
			file->insertItem( "New Material", this, SLOT(slotNewMatItem()));
			file->insertSeparator();
			file->insertItem( "Sort", this,  SLOT(NULL), NULL);
			file->insertSeparator();
		}
		else if (_isMatitem(pitem)){
			file->insertItem("Rename Material", this, SLOT(slotModifyMaterialMatItem()));
			file->insertItem("Delete Material", this, SLOT(slotDelMatItem()));
			file->insertSeparator();
			file->insertItem("Specify Object Material", this, SLOT(slotSpecifyObjectMaterial()));
			file->insertSeparator();
		}
		else if (_isItem(pitem)){
			file->insertItem( "Modify", this,  SLOT(slotModifyMaterialItem()));
			file->insertSeparator();
		}
		file->insertItem("Save", this,  SLOT(slotSaveMaterialFile(void)));
		file->insertItem("Save As", this,  SLOT(slotSaveMaterialFileAs(void)));
	}
	file->popup(pt);
}



void CMaterialTree::slotNewMatItem(void)
{
	QListViewItem * pitem = currentItem();
	if (pitem==NULL) return;
	if (!_isLowestLevelFolder(pitem))
		return;

	//find the same folder in material lib
	QString id = pitem->text(2);
	QDomElement root = m_libDomTree.documentElement();
	QDomNode* p = _searchMaterialByID(root, id);
	if (p==NULL) return;
	
	//create a copy of its child;
	QDomNode child = p->firstChild();
	if (child.isNull()) return;
	QDomNode copynode = child.cloneNode(true);

	//map the node to the user dom and the tree view;
	QDomElement root2 = m_usrDomTree.documentElement();
	QDomNode* p2 = _searchMaterialByID(root2, id);
	assert(p2!=NULL);

	GenerateNewNodeID(id, *p2, copynode);	//set new id number
	ClearDomMaterialNode(copynode);			//clear the input area
	addMatItemNodeContent(copynode, pitem, NULL);	//add to the listview
	p2->appendChild(copynode);						//add to the node tree;
	
	m_bModified = true;
}



void CMaterialTree::slotDelMatItem()
{
	//if the node is not a material item node;
	QListViewItem * pitem = currentItem();
	if (pitem==NULL || !_isMatitem(pitem))
		return;

	int r = QMessageBox::question(this, 
			"Delete a meterial entry", 
			"Do you want to delete this meterial?", 
			QMessageBox::Yes, QMessageBox::No);
	if (r==QMessageBox::No)
		return;

	//delete the item from the ListViewTree class;
	QString id = pitem->text(2);
	QListViewItem *pparent = pitem->parent();
	if (pparent) 
		pparent->takeItem(pitem);
	else
		takeItem(pitem);

	//also delete it from the XML structure;
	QDomElement root = m_usrDomTree.documentElement();
	QDomNode* p = _searchMaterialByID(root, id);
	assert(p!=NULL);
	QDomNode ppNode = p->parentNode();
	assert(!ppNode.isNull());
	ppNode.removeChild(*p);

	//clear 
	m_bModified = true;
}


void CMaterialTree::slotSaveMaterialFileAs(void)
{
	//get file name
	static int count = 0;
	static QString path;	
	if (count==0){
		path = m_appPathDir; 
		count++;
	}
    QString fname = QFileDialog::getSaveFileName(
						path,
						"User material file (*.xml)",
						this,
						"Save material file dialog",
						"Choose a material file to save");
    if (fname.isEmpty()) return;

	//save the file
	QDomDocument *pDomTree = &m_usrDomTree;
	const QString& str = pDomTree->toString(4);
	FILE *fp =fopen(fname.ascii(), "w");
	fprintf(fp, "%s\n", str.ascii());
	fclose(fp);
}



void CMaterialTree::slotSpecifyObjectMaterial(void)
{
	QListViewItem * pitem = currentItem();
	if (pitem==NULL) return;
	if (!_isMatitem(pitem))
		return;

	QString id = pitem->text(2);
	assert(id!="");

	if (m_pActiveSceneNode)
		m_pActiveSceneNode->setMaterialAttrib(id.ascii());
}
