
#include <assert.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qregexp.h>
#include "objectlistview.h"


QListViewItem* CObjectListView::_AddOneEntry(const CSceneNode *pnode)
{
	if (pnode==NULL) return NULL;

	const char *objname = pnode->GetObjectName();
	const char *objtypename = pnode->m_pObject->Description();
	char str[100];
	sprintf(str, "%d", pnode->m_pObject->m_nVertexCount);

	QListViewItem *pitem = new QListViewItem(this, objname, objtypename, str);
	
	return pitem;
}


void CObjectListView::update(CSceneGraph &sg)
{
    this->clear();
	const int nobj = sg.size();
	if (nobj == 0) return;
	
	for (int i=nobj-1; i>=0; i--){
		const CSceneNode *pnode = sg.GetSceneNode(i);
		QListViewItem* t = _AddOneEntry(pnode);
	}
	/*
	m_pActiveSceneNode = pnode;
	m_pPickAttribItem = NULL;
	if (pnode==NULL) return;
	m_pPickAttribItem=_setAttribData(&pnode->m_FemAttrib, "Nodal settings");
	*/
}


void CObjectListView::_restoreFromItem(const QListViewItem *pItem, CFemObjAttrib *pAttrib)
{

    ASSERT(0);
/*
    int i;
	assert(pItem!=NULL);
	assert(pAttrib!=NULL);
	QListViewItem* p[100];
	
	int nchild = pItem->childCount();
	p[0]=pItem->firstChild();
	for (i=1; i<nchild; i++)
		p[i]=p[i-1]->nextSibling();
	for (i=0; i<nchild; i++){
		QString name=p[i]->text(0);
		QString value=p[i]->text(2);
		const char *sname = name.ascii();
		const char *tname = value.ascii();
		if (strcmp(sname, nodalSettingStr[0])==0)
			TXT2VEC(tname, pAttrib->m_vVelocity);
		else if (strcmp(sname, nodalSettingStr[1])==0)
			TXT2VEC(tname, pAttrib->m_vDisplacement);
		else if (strcmp(sname, nodalSettingStr[2])==0)
			TXT2VEC(tname, pAttrib->m_vLoad);
		else 
			assert(0);
	}
   */
}


CObjectListView::CObjectListView(CSceneGraph &sg, QWidget *parent, const char *name):
	QListView(parent, name), 
	m_pSG(&sg)
{		
    header()->setClickEnabled( TRUE );
    addColumn( "       Name      " );
    addColumn( "  Type " );
	addColumn( " Vertex#" );

    setRootIsDecorated(true);
	setSorting(-1);

	connect(this, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)), 
			this, SLOT(slotDoubleClicked(QListViewItem *, const QPoint &, int)));

	//--------------------------------------
	update(sg);
	//--------------------------------------

    resize(180, 100);
}


void CObjectListView::initFolders()
{
    ASSERT0(0);
/*
    unsigned int mcount = 1;
	char *nodalSettingStr[3]={
		"Load", 
		"Velocity", 
		"Displacement"
	};

	Folder * globalSetting = new Folder( 0, "Global settings");
	Folder * nodalSetting = new Folder( 0, "Nodal settings");

	for (i=0; i<3; i++){
		Folder *f2 = new Folder(globalSetting, nodalSettingStr[i]);
	}

	for (i=0; i<3; i++){
		Folder *f2 = new Folder(nodalSetting, nodalSettingStr[i]);
	}


	lstFolders.append( globalSetting );
	lstFolders.append( nodalSetting );
*/
}


void CObjectListView::setupFolders()
{
}



void CObjectListView::slotDoubleClicked( QListViewItem *pListViewItem, const QPoint & pt, int x)
{
	if (pListViewItem==NULL) return;
	QString selectedName = pListViewItem->text(0);
	if (selectedName.isEmpty()) return;

	const int nobj = m_pSG->size();
	int objid=-1;
	for (int i=0; i<nobj; i++){
		CSceneNode *pnode = m_pSG->GetSceneNode(i);
		const char *objname = pnode->GetObjectName();
		if (strcmp(objname, selectedName)==0){
			objid = i;
			break;
		}
	}
	if (objid<0) return;

	CSceneNode *pnode = m_pSG->GetSceneNode(objid);
	m_pSG->setActiveSceneNode(pnode);
	m_pSG->updateGLWindows();

}
