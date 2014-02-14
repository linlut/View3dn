//FILE: simu_engine.h

#ifndef _INC_SIMU_ENGINE_ZN2008_H
#define _INC_SIMU_ENGINE_ZN2008_H


#include <stdio.h>
#include <assert.h>
#include <vector>
#include <vectorall.h>
#include "mechanical_property.h"


class CSimuEntity;
class CSimuEngine;

typedef void SIMU_DISPLAY_CALLBACK_FUNC(CSimuEngine *pengine);

class CSimuEngine
{
protected:
	#define SIMUENTITY_BUFFER_SIZE 20
	static CSimuEngine staticEngine;
	SIMU_DISPLAY_CALLBACK_FUNC *m_pFuncDisplay;
	CSimuEntity* m_pObjects[SIMUENTITY_BUFFER_SIZE];
	int m_nObjects;			//# of objects in the scene
	double m_timestep;		//simulation time step;

private:
	void _advanceOneTimeStep(const int stepID);

	//export the entire scene to an OBJ file 
	//input: id=-1: all objects, otherwise the specified object
	bool _exportSceneToObj(const char *filename, const int stepid, const int id=-1) const;
	bool _exportSceneToPlt(const char *filename, const int stepid, const int id=-1) const;

public:
	CSimuEngine(void)
	{
		m_nObjects = 0;
		m_timestep = 1e-3;
		for (int i=0; i<SIMUENTITY_BUFFER_SIZE; i++) m_pObjects[i]=NULL; 
		m_pFuncDisplay = NULL;
	}

	void clear(void);

	virtual ~CSimuEngine(void){ clear(); }

	static CSimuEngine* getInstance(void)
	{
		return &staticEngine;
	}

	double getTimestep(void) const
	{
		return m_timestep;
	}

	int addSimuObject(CSimuEntity* pobj)
	{
		if (m_nObjects>=SIMUENTITY_BUFFER_SIZE)
			return -1;
		m_pObjects[m_nObjects] = pobj;
		m_nObjects++;
		return m_nObjects-1;
	}

	void setDisplayCallback(SIMU_DISPLAY_CALLBACK_FUNC*p)
	{
		m_pFuncDisplay = p;
	}

	void setTimestep(const double & t)
	{
		m_timestep = t;
	}

	void preStartSimulation(void);

	inline void advanceOneTimeStep(const int nStepID)
	{
		_advanceOneTimeStep(nStepID);
	}

	void startSimulation(const int nsteps, const int outputstep=1);

	void startSimulationInSteps(const int nsteps, const int outputstep=1)
	{
		startSimulation(nsteps, outputstep);
	}

	void startSimulationInStepsWithCollisionInput(
		const int nsteps, const int outputstep, const char *colfilename);

	//runtime in milisec., outputtimeinterval in  milisec.
	void startSimulationInMiliSeconds(const double& runtime, const double outputTimeInterval)
	{
		const int nsteps = runtime*0.001/m_timestep;  //m_timestep is in sec.
		int outputstep = outputTimeInterval*0.001/m_timestep;
		if (outputstep<1) outputstep=1;
		startSimulationInSteps(nsteps, outputstep);
	}

	//interface with the rendering thread in a multi-threading env.
	void copyVertexPositions(vector<Vector3d> & vtbuf) const;

	bool exportMesh(const int SID, const char *filename, const int oid=-1) const;

	//collision handling for the simulation objects
	//where the input are from instrucments/soft tissue collision detection
	void updateCollisionConstraint(
		const Vector3d *pVertexPos, const int *pVertexID, const int nv, 
		const double &cur_time, const double& inputTimeInterval);

};


#endif