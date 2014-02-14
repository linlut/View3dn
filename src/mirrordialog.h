/****************************************************************************
** Form interface generated from reading ui file 'mirrordialog.ui'
**
** Created: Wed Sep 13 17:52:35 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MIRROROBJECTDIALOG_H
#define MIRROROBJECTDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include <drawparms.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class MirrorObjectDialog : public QDialog
{
    Q_OBJECT

public:
    MirrorObjectDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MirrorObjectDialog();

    QButtonGroup* buttonGroup1;
    QPushButton* pushButtonM1;
    QPushButton* pushButtonM2;
    QPushButton* pushButtonM3;
    QCheckBox* checkBoxM1;
    QCheckBox* checkBoxM2;
    QCheckBox* checkBoxM3;
    QGroupBox* groupBox1;
    QLabel* textLabel2;
    QLabel* textLabel2_2;
    QLineEdit* lineEditP0;
    QLineEdit* lineEditP1;
    QPushButton* buttonHelp;
    QPushButton* buttonOk;

    CGLDrawParms * _pDrawParms;
    CGLDrawParms ** _ppDrawParmsM3;

public slots:
    virtual void handleCheckBoxM1();
    virtual void handleCheckBoxM2();
    virtual void handleCheckBoxM3();
    virtual void handlePushButtonM1();
    virtual void handlePushButtonM2();
    virtual void handlePushButtonM3();
    virtual void init();
    virtual void handleButtonOK();
    virtual void handleButtonCancel();
    virtual void onLineEditP0();
    virtual void onLineEditP1();

signals:
    void applyButtonPressed();

protected:
    QVBoxLayout* layout3;
    QVBoxLayout* layout2;

protected slots:
    virtual void languageChange();

};

#endif // MIRROROBJECTDIALOG_H
