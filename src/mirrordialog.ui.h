/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


#include "dparmdialogex.h"



void MirrorObjectDialog::handleCheckBoxM1()
{
	CGLDrawParms * p = _ppDrawParmsM3[0];
    p->m_bValidDrawParms = checkBoxM1->isChecked();
}



void MirrorObjectDialog::handleCheckBoxM2()
{
	CGLDrawParms * p = _ppDrawParmsM3[1];
    p->m_bValidDrawParms = checkBoxM2->isChecked();
}



void MirrorObjectDialog::handleCheckBoxM3()
{
	CGLDrawParms * p = _ppDrawParmsM3[2];
    p->m_bValidDrawParms = checkBoxM3->isChecked();
}


void MirrorObjectDialog::handlePushButtonM1()
{
	CGLDrawParms * &p = _ppDrawParmsM3[0];
	if (!p->m_bValidDrawParms) return;
    CDGDrawParms* tabdialog = new CDGDrawParms(p, this, "Rendering Options");
    tabdialog->exec();
}


void MirrorObjectDialog::handlePushButtonM2()
{
	CGLDrawParms * &p = _ppDrawParmsM3[1];
	if (!p->m_bValidDrawParms) return;
    CDGDrawParms* tabdialog = new CDGDrawParms(p, this, "Rendering Options");
    tabdialog->exec();
}


void MirrorObjectDialog::handlePushButtonM3()
{
	CGLDrawParms * &p = _ppDrawParmsM3[2];
	if (!p->m_bValidDrawParms) return;
    CDGDrawParms* tabdialog = new CDGDrawParms(p, this, "Rendering Options");
    tabdialog->exec();
}


void MirrorObjectDialog::init()
{
}


void MirrorObjectDialog::handleButtonOK()
{
    accept();
}


void MirrorObjectDialog::handleButtonCancel()
{
    reject();
}


void MirrorObjectDialog::onLineEditP0()
{
    QString s = lineEditP0->text();
	float x, y, z;
	sscanf(s.ascii(), "%f %f %f", &x, &y, &z);
	_pDrawParms->m_vPickedVertex0 = Vector3d(x, y, z);
}


void MirrorObjectDialog::onLineEditP1()
{
    QString s = lineEditP1->text();
	float x, y, z;
	sscanf(s.ascii(), "%f %f %f", &x, &y, &z);
	_pDrawParms->m_vPickedVertex1 = Vector3d(x, y, z);
}
