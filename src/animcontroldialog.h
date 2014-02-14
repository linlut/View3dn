/****************************************************************************
** Form interface generated from reading ui file 'animcontroldialog.ui'
**
** Created: Fri Jan 12 18:41:49 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CANIMCONTROLDIALOG_H
#define CANIMCONTROLDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QGroupBox;
class QLineEdit;
class QLabel;

class CAnimControlDialog : public QDialog
{
    Q_OBJECT

public:
    CAnimControlDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CAnimControlDialog();

    QPushButton* buttonHelp;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QGroupBox* groupBox2;
    QLineEdit* lineEditOutFileName;
    QPushButton* pushButtonOutputFileName;
    QGroupBox* groupBox1;
    QPushButton* pushButtonStartFileName;
    QLabel* textLabel2;
    QLineEdit* lineEditStartFileName;
    QPushButton* pushButtonEndFileName;
    QLabel* textLabel2_2;
    QLineEdit* lineEditEndFileName;
    QLabel* textLabel3;
    QLineEdit* lineEditInc;

    char *m_pOutputFileName;
    int * m_pIncreaseCounter;
    char* m_pEndFileName;
    char* m_pStartFileName;
    char m_strStartFileName[256];
    char m_strEndFileName[256];
    char m_strOutputFileName[256];
    int m_nInc;

public slots:
    virtual void onButtonOK();
    virtual void onButtonCancel();
    virtual void onLineEditOutputFileName();
    virtual void onLineEditInc();
    virtual void onLineEditEndFileName();
    virtual void onLineEditStartFileName();
    virtual void onPushButtonStartFileName();
    virtual void onPushButtonEndFileName();
    virtual void onPushButtonOutputFileName();

protected:
    QGridLayout* CAnimControlDialogLayout;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;
    QHBoxLayout* groupBox2Layout;
    QGridLayout* groupBox1Layout;
    QHBoxLayout* layout47;
    QSpacerItem* spacer35;

protected slots:
    virtual void languageChange();

};

#endif // CANIMCONTROLDIALOG_H
