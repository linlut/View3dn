/****************************************************************************
** $Id: qt/tabdialog.h   3.3.1   edited May 27 2003 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <qtabdialog.h>
#include <qstring.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

///My drawing configuration
#include <drawparms.h>		


class TabDialog : public QTabDialog
{
    Q_OBJECT

private:
	CGLDrawParms _DrawParms;
	CGLDrawParms * _pDrawParms;

    QRadioButton * _drawline;
    QRadioButton * _drawhline;
    QRadioButton * _drawfill;

	QCheckBox * _enableLighting;
	QCheckBox * _enableTexture;
	QCheckBox * _enableDepth;
	QCheckBox * _enableSmoothing;
	QCheckBox * _enableAxis;

public:
    TabDialog(QWidget *parent, const char *diagname, CGLDrawParms* pdrawparms);



protected:
    //QString filename;
    //QFileInfo fileinfo;

    void setupTab1();
    void setupTab2();
    void setupTab3();

private slots:
	void ToggleDrawline(void);
	void ToggleLighting(void);
	void ProcApplyEvent(void);
	void ProcCancelEvent(void);
	void ProcHelpEvent(void);

};

#endif
