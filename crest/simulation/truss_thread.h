//FILE: truss_thread.h

#ifndef _SIM_TRUSS_THREAD_H_
#define _SIM_TRUSS_THREAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <QMutex.h>
#include <QThread.h>
#include <QWaitCondition.h>
#include <QObject.h>
#include <QString.h>

extern "C" {
void lockDataExchangeBuffer(void);
void LOCKDATAEXCHANGEBUFFER(void);
void unlockDataExchangeBuffer(void);
void UNLOCKDATAEXCHANGEBUFFER(void);
};

namespace cia3d{
namespace simulation{

class CDeformationThread : public QThread
{
private:
	void changeWorkingDirectory(void);

	void _runInternalDeformationCode(
		const int NSTEP,				//
		const double &DISPINTERVAL,		//milisec, display interval in CPU time
		const double &OUTPUTINTERVAL	//milisec, output interval in physical time
		);
	
	static CDeformationThread *pInstance;

public:
	QMutex mutex;
	int m_nCareerGoal;			//0: sleep, 1: single step, -1: run
	QString m_sceneFileName;

public:
	CDeformationThread(): QThread(), mutex(), m_sceneFileName()
	{
		m_nCareerGoal = 0;	//do nothing
	}

	static CDeformationThread* getInstance(void)
	{
		if (pInstance==NULL) 
			pInstance = new CDeformationThread();
		return pInstance;
	}

	static void terminateInstance(void)
	{
		if (pInstance){
			pInstance->terminate();
			pInstance->wait();
			delete pInstance;
			pInstance = NULL;
		}
	}

	void pauseRunning(void)
	{
		m_nCareerGoal = 0;
	}

	void continueRunning(void)
	{
		m_nCareerGoal = -1;
	}

	void singleStep(void)
	{
		m_nCareerGoal = 1;
	}

	virtual void run(void);


//#if (QT_VERSION > 0x39999) 
//signals:
//void oneSimuStepFinished(void);
//#endif

};

} // namespace simulation
} // namespace cia3d


inline int *pSimThreadState(void)
{
	typedef cia3d::simulation::CDeformationThread CDeformationThread;
	CDeformationThread *pDefInst = CDeformationThread::getInstance();
	CDeformationThread &defThread = *pDefInst;
	return &defThread.m_nCareerGoal;
}


#endif