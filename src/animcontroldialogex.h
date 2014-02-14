#include "animcontroldialog.h"

class CAnimControlDialogEx: public CAnimControlDialog
{
public:

	CAnimControlDialogEx(QWidget* parent, const char* name, char *startfilename, char *endfilename, int *inc, char *outputfilename):
		CAnimControlDialog(parent, name)
	{
		m_pStartFileName = startfilename;
		m_pEndFileName = endfilename;
		m_pOutputFileName = outputfilename;
		m_pIncreaseCounter = inc;

		strcpy(m_strStartFileName, m_pStartFileName);
		strcpy(m_strEndFileName, m_pEndFileName);
		strcpy(m_strOutputFileName, m_pOutputFileName);
		m_nInc = *m_pIncreaseCounter;

		lineEditOutFileName->setText(m_strOutputFileName);
		lineEditStartFileName->setText(m_strStartFileName);
		lineEditEndFileName->setText(m_strEndFileName);

		char buff[100];
		sprintf(buff, "%d", m_nInc);
		lineEditInc->setText(buff);
		
	}
};