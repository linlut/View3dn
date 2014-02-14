
#include <assert.h>
#include "listviews.h"

#include <qinputdialog.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qregexp.h>


const int _BUFFLENGTH = 3;
static char *nodalSettingStr[_BUFFLENGTH]={
		"Velocity", 
		"Displacement", 
		"Shell Thickness"
};
static char *typeStr[_BUFFLENGTH]={
		"VEC3  ", 
		"VEC3", 
		"VEC3"
};


QListViewItem* CFemAttribListView::_setAttribData(const CFemObjAttrib *pAttrib, const char*name)
{
	int i = 0;
    static char *typestr=" ";

	//Option for analysis package
	m_table[i++]=CNameTableItem("Object Load", 'V', &pAttrib->m_vLoad);
	m_table[i++]=CNameTableItem("Velocity", 'V', &pAttrib->m_vVelocity);
	m_table[i++]=CNameTableItem("Displacement", 'V', &pAttrib->m_vDisplacement);
    m_table[i++]=CNameTableItem("Layer Count", 'i', &pAttrib->m_nLayer);
    m_table[i++]=CNameTableItem("Shell Thickness", 'd', &pAttrib->m_lfShellThickness);
    m_nTableLen = i;

	QListViewItem  * globalSetting = new QListViewItem ( this, name );
	for (i=0; i<m_nTableLen; i++){
        CNameTableItem *ptab = &m_table[i];
        char *sname, *stype, *sval, *soption, *scomm;
	    ptab->toString(sname, stype, sval, soption, scomm);
		QListViewItem  *f = new QListViewItem(globalSetting, sname, stype, sval);
	}
	return globalSetting;
}


void CFemAttribListView::setAttribData(CSceneNode *pnode)
{
	if (m_pActiveSceneNode == pnode) return;
    this->clear();

	m_pActiveSceneNode = pnode;
	m_pPickAttribItem = NULL;
	if (pnode==NULL) return;
	m_pPickAttribItem=_setAttribData(&pnode->m_FemAttrib, "Nodal settings");
}


static inline void TXT2VEC(const char * s, Vector3d & v)
{
	double x, y, z;
	sscanf(s, "%lf,%lf,%lf", &x, &y, &z);
	v.x = x, v.y=y, v.z=z;
}

void CFemAttribListView::_restoreFromItem(const QListViewItem *pItem, CFemObjAttrib *pAttrib)
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



CFemAttribListView::CFemAttribListView(CSceneNode * pActiveNode, QWidget *parent, const char *name):
	QListView(parent, name)
{		
	m_pAttribItem = NULL;
	m_pPickAttribItem = NULL;

    header()->setClickEnabled( TRUE );
    addColumn( "Attributes " );
    addColumn( " Type " );
    addColumn( " Values " );

    setRootIsDecorated( TRUE );
	setSorting(-1);

	m_pActiveSceneNode = pActiveNode;
	setAttribData(pActiveNode);

	connect(this, SIGNAL( doubleClicked(QListViewItem *, const QPoint &, int)), 
			this, SLOT(slotDoubleClicked(QListViewItem *, const QPoint &, int)));

    resize(200, 100);
}


void CFemAttribListView::initFolders()
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


void CFemAttribListView::setupFolders()
{
}



void CFemAttribListView::slotDoubleClicked ( QListViewItem *pListViewItem, const QPoint & pt, int x)
{
    bool ok;
	if (pListViewItem==NULL) return;
	QString existingStr = pListViewItem->text(2);
	if (existingStr.isEmpty()) return;
    QString text = QInputDialog::getText(
            "Input/change value", "Change the value:", QLineEdit::Normal,
            existingStr, &ok, this );
    
    if ( ok && !text.isEmpty() ) {
		pListViewItem->setText(2, text);
		QString attribname = pListViewItem->text(0);
        if (m_pActiveSceneNode){ 
	        for (int i=0; i<m_nTableLen; i++){
                CNameTableItem *ptab = &m_table[i];
                if (strcmp(attribname, ptab->m_sName)==0){
		            ptab->setValue(text);
                    break;
                }
	        }
        }
    } 
}
