//FILE: truss_thread.cpp

#include <math.h>
#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <crest/system/setdirectory.h>
#include <crest/simulation/simu_engine.h>
#include <crest/system/ztime.h>
#include "truss_thread.h"

extern void REAL_IMPACT2(int *ID_GET_TYPE, double *DT_REAL, int *INC_ISTEP, int *ID_TIME, int*ID_SEARCH_C);
extern int USE_INTERNAL_SIMULATOR;
extern void displayCallback(CSimuEngine *pe, const double& phytime);


namespace cia3d{
namespace simulation{

//static memeber data for the class
CDeformationThread* CDeformationThread::pInstance=NULL;

//deformation functions defined in FORTRAN
extern "C" {
//FORTRAN CODE
void REAL_IMPACT(int *ID_GET_TYPE, double *DT_REAL, int *INC_ISTEP, int *ID_TIME, int *ID_SEARCH_C);
/*    ID_GET_TYPE=1: request element data 
      ID_GET_TYPE=2: request only boundary surface data 
      DT_REAL: output interval (in sec.)
      INC_ISTEP: iteration number for output
      ID_TIME=1: out data in phisical time
      ID_TIME=2: out data in computing time
      ID_TIME=3: out data in iteration number
      ID_SEARCH_C=1 m-s search and contact; otherwise not
*/
}

void CDeformationThread::changeWorkingDirectory(void)
{
	QString& s=m_sceneFileName;
	if (s.isEmpty()) return;
	QString path = QFileInfo(s).absolutePath();
	QDir::setCurrent(path);
}


inline void _initTimeVars(double& phytime, double &lastdisptime, ctime_t& zero_tm)
{
	phytime = 0, lastdisptime=0;
	zero_tm = CTime::getTime();
}

void CDeformationThread::_runInternalDeformationCode(
	const int NSTEP,				//
	const double &DISPINTERVAL,		//milisec, display interval in CPU time
	const double &OUTPUTINTERVAL	//milisec, output interval in physical time
	)
{
	const int N=8;
	const int N1000=2000;
	double cputime=0, phytime=0, lastdisptime;
	ctime_t zero_tm;
	CSimuEngine *pEngine = CSimuEngine::getInstance();
	pEngine->preStartSimulation();
	const double timesteplen = pEngine->getTimestep()*1000;
	int i;

	//run the simulation
	const int NN=60;
	for (i=0; i<NSTEP; i++){
		int KK = i%NN;
		if (KK==0){
			KK = i/NN;
			char filenameb[200];
			sprintf(filenameb, "OUTPUT%d.obj", KK);
			//pEngine->exportMesh(KK, filenameb);
			//if (KK>500) exit(0);
		}
		//re-init after a long run
		if (i%N1000==0){
			const double ratio = cputime/(phytime+1e-10)*100.0;
			printf("Computation time CPU %lg, PHY %lg, ratio %lg percent.\n", cputime, phytime, ratio);
			_initTimeVars(phytime, lastdisptime, zero_tm);
		}

		//the code part deals with the sleeping/step/animation options
		volatile int simflag = *pSimThreadState();
		if (simflag==0){
			_sleep(500); 
			_initTimeVars(phytime, lastdisptime, zero_tm);
			continue;
		}
		else if (simflag == 1){
			*pSimThreadState() = 0;
		}
		//this part is the main deformation loop
		pEngine->advanceOneTimeStep(i);
		phytime+=timesteplen;
		
		//check display or other events
		if (i%N==0){
			ctime_t current_tm = CTime::getTime();
			cputime = getTimeDiffMS(zero_tm, current_tm);
			const double diftime = phytime- cputime;
			if (diftime>1.0){
				_sleep(diftime);
				//printf("Sleep %lg ms to wait!\n", diftime);
				current_tm = CTime::getTime();
				cputime = getTimeDiffMS(zero_tm, current_tm);
			}
			//printf("Step: %d, comp tm=%lg ms\n", i, (current_tm-previous_tm)/N);
			if (cputime >= lastdisptime+DISPINTERVAL){
				const double simulationtime = timesteplen * i;
				displayCallback(pEngine, simulationtime);
				lastdisptime = cputime;
			}
		}
	}

	//idle after finishing the required steps;
	if (i>=NSTEP){
		while (1){
			_sleep(1000);
		};
	}
}


void CDeformationThread::run(void)
{
	//msleep(1500);
	//std::cout<< "Deformation thread has started!\n";
	//while (m_nCareerGoal == 0){ //do nothing
	//	msleep(100);
	//};
	continueRunning();			//The thread will start running

	//change to the current direcotry
	std::string strCurrentDir = cia3d::system::SetDirectory::GetCurrentDir();
	std::cout << strCurrentDir <<std::endl;

	if (USE_INTERNAL_SIMULATOR){
		//CSimuEngine *pEngine = CSimuEngine::getInstance();
		const double runtimeCount=10000000;
		const double disptime = 33.3;		//33ms for an output
		//const double outputtime = 33;		//33ms for an output
		//pEngine->setDisplayCallback(displayCallback);
		//pEngine->startSimulationInMiliSeconds(runtime, outputtime);
		_runInternalDeformationCode(runtimeCount, disptime, disptime);
	}
	else{//PROF SHA's FORTRAN code to run the simulation
		int ID_GET_TYPE = 2;		//1: elment, 2: boundary surface
		int INC_ISTEP = 8;			//loop number
		double DT_REAL = 1.0/20;	//every 0.05 sec. for an output
		int ID_TIME=2;				//1: output in phisical time
									//2: output in computing time
									//3: output in iteration number
		int ID_SEARCH_C=1;			//1: search, otherwise no search
		REAL_IMPACT2(&ID_GET_TYPE, &DT_REAL, &INC_ISTEP, &ID_TIME, &ID_SEARCH_C);
	}
}

}//simulation
}//cia3d


//==================================================

//cia3d::simulation::CDeformationThread defThread;

extern "C" {

//int* pSimThreadState = &defThread.m_nCareerGoal;

void lockDataExchangeBuffer(void)
{
	cia3d::simulation::CDeformationThread *p = cia3d::simulation::CDeformationThread::getInstance();
	ASSERT0(p!=NULL);
	QMutex &m = p->mutex;
	m.lock();
}

void LOCKDATAEXCHANGEBUFFER(void)
{
	lockDataExchangeBuffer();
}

void unlockDataExchangeBuffer(void)
{
	cia3d::simulation::CDeformationThread *p = cia3d::simulation::CDeformationThread::getInstance();
	ASSERT0(p!=NULL);
	QMutex &m = p->mutex;
	m.unlock();
}

void UNLOCKDATAEXCHANGEBUFFER(void)
{
	unlockDataExchangeBuffer();
}

}

