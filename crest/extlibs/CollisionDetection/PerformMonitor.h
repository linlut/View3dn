//////////////////////////////////////////////////////////////////////////////
/// MedVR Simulation
/// S.Y.H.
/// Virtual Environment Lab
/// Biomedical Engineering
/// Electrical Engineering
/// UTSW, UTA
/////////////////////////////////////////////////////////////////////////////
// PerformMonitor.h : main header file for the PerformMonitor
//

#ifndef __PERFORMMONITOR_H__
#define __PERFORMMONITOR_H__

class CPerformMonitor
{
public:
	CPerformMonitor::CPerformMonitor();
	virtual ~CPerformMonitor();

// Attributes
public:
	bool m_bActive;

private:

protected:
	CString m_sentence;
	CFile m_file_Perform;
	LARGE_INTEGER m_khz;

// Operations
public:
	void MarkTime(LARGE_INTEGER* pOldCounter);
	float CheckElapseTime(LARGE_INTEGER* pOldCounter, BOOL update);
	float RecordTime(LARGE_INTEGER* pOldCounter, BOOL update, char id);
	void Record(CString s);

// Implementation

};



#endif
