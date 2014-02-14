/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <string.h>
#include <qfiledialog.h>


void CAnimControlDialog::onButtonOK()
{
    strcpy(m_pStartFileName, m_strStartFileName);
    strcpy(m_pEndFileName, m_strEndFileName);
    strcpy(m_pOutputFileName, m_strOutputFileName);
    *m_pIncreaseCounter = m_nInc;
    accept();
}


void CAnimControlDialog::onButtonCancel()
{
    reject();
}

/*
    QPushButton* pushButtonOutputFileName;
    QGroupBox* groupBox1;
    QLineEdit* lineEditEndFileName;
    QPushButton* pushButtonEndFileName;
    QLabel* textLabel2_2;
    QLabel* textLabel2;
    QLineEdit* lineEditStartFileName;
    QPushButton* pushButtonStartFileName;
    QLabel* textLabel3;
    QLineEdit* lineEditInc;
*/
void CAnimControlDialog::onLineEditOutputFileName()
{
    QString s = lineEditOutFileName->text();
    strcpy(m_strOutputFileName, s.ascii());
}


void CAnimControlDialog::onLineEditInc()
{
    QString s = lineEditInc->text();
	sscanf(s.ascii(), "%d", &m_nInc);
}


void CAnimControlDialog::onLineEditEndFileName()
{
    QString s = lineEditEndFileName->text();
    strcpy(m_strEndFileName, s.ascii());
}


void CAnimControlDialog::onLineEditStartFileName()
{
    QString s = lineEditStartFileName->text();
    strcpy(m_strStartFileName, s.ascii());
}

inline QString runOpenMeshFileDlg(QWidget *proot)
{
	static QString dirPath = "";
    QString fname = QFileDialog::getOpenFileName(
						dirPath,
						"Triangle mesh (*.txt *.ply *.plt *.obj *.stl)",
						proot,
						"Open file dialog",
						"Choose a mesh file to open");
	return fname;
}
void CAnimControlDialog::onPushButtonStartFileName()
{
	QString fname = runOpenMeshFileDlg(this);
    if (!fname.isEmpty()){
		strcpy(m_strStartFileName, fname.ascii());
		lineEditStartFileName->setText(fname);
	}
}

void CAnimControlDialog::onPushButtonEndFileName()
{
	QString fname = runOpenMeshFileDlg(this);
    if (!fname.isEmpty()){
		strcpy(m_strEndFileName, fname.ascii());
		lineEditEndFileName->setText(fname);
	}
}


void CAnimControlDialog::onPushButtonOutputFileName()
{
	static QString dirPath = "";
	QString sfilter("PNG files (*.png)");

    QString fname = QFileDialog::getSaveFileName(
						dirPath,
						"Image files (*.jpg *.bmp *.png *.ppm)",
						this,
						"Save image file dialog",
						"Choose an image file to save", 
						&sfilter
						);
    if (!fname.isEmpty()){
		strcpy(m_strOutputFileName, fname.ascii());
		lineEditOutFileName->setText(fname);
	}
}
