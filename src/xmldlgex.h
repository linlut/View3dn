/****************************************************************************
** File: xmldlgex.h
** Created: Fri Feb 24 , 2006
**      by: Nan Zhang
**
****************************************************************************/

#ifndef __CXMLIODIALOG_EX_H
#define __CXMLIODIALOG_EX_H

#include <qlistview.h>
#include "xmldlg.h"


class CXmlIODialogEx : public CXmlIODialog
{

private:

	void _addItemNodeContent(const QDomNode &inode, QListViewItem *pParentItem);

	void _buildTree(QListViewItem *parentItem, const QDomElement &parentElement);


public:
    CXmlIODialogEx(QDomDocument *pDomTree, QWidget* parent=0, const char* name=0, bool modal=FALSE, WFlags fl=0);

    ~CXmlIODialogEx()
	{

	}
};

#endif 
