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
	#define SIMUENTITY_BUFFER_SIZE 32
	static CSimuEngine staticEngine;
	SIMU_DISPLAY_CALLBACK_FUNC *m_pFuncDisplay;
	CSimuEntity* m_pObjects[SIMUENTITY_BUFFER_SIZE];
	int m_nObjects;			//# of objects in the scene
	double m_timestep;		//simulation time step;

private:
	unsigned int m_nMinOutputStep;
	bool m_bOutputPltFile;
	bool m_bOutputObjFile;
	
	//advance one time step
	void _advanceOneTimeStep(const unsigned int stepID);

	//export the entire scene to an OBJ file 
	//input: id=-1: all objects, otherwise the specified object
	bool _exportSceneToObj(const char *filename, const int stepid, const int id=-1) const;
	bool _exportSceneToPlt(const char *filename, const int stepid, const int id=-1) const;

public:
	CSimuEngine(void)
	{
		m_nMinOutputStep = 0;
		m_nObjects = 0;
		m_timestep = 1e-3;
		for (int i=0; i<SIMUENTITY_BUFFER_SIZE; i++) m_pObjects[i]=NULL; 
		m_pFuncDisplay = NULL;

		m_bOutputPltFile = true;
		m_bOutputObjFile = false;
	}

	void clear(void);

	virtual ~CSimuEngine(void){ clear(); }

	inline static CSimuEngine* getInstance(void)
	{
		return &staticEngine;
	}

	inline double getTimestep(void) const
	{
		return m_timestep;
	}

	inline int addSimuObject(CSimuEntity* pobj)
	{
		if (m_nObjects>=SIMUENTITY_BUFFER_SIZE)
			return -1;
		m_pObjects[m_nObjects] = pobj;
		m_nObjects++;
		return m_nObjects-1;
	}

	inline void setDisplayCallback(SIMU_DISPLAY_CALLBACK_FUNC*p)
	{
		m_pFuncDisplay = p;
	}

	inline void setTimestep(const double & t)
	{
		m_timestep = t;
	}

	inline void setOutputPltFile(const bool f)
	{
		m_bOutputPltFile = f;
	}

	inline void setOutputObjFile(const bool f)
	{
		m_bOutputObjFile = f;
	}

	inline void setMinOutputStep(const unsigned int i)
	{
		m_nMinOutputStep = i;
	}

	void preStartSimulation(void);

	inline void advanceOneTimeStep(const int nStepID)
	{
		_advanceOneTimeStep(nStepID);
	}

	void startSimulation(const unsigned int nsteps, const unsigned int outputstep=1);

	inline void startSimulationInSteps(const unsigned int nsteps, const unsigned int outputstep=1)
	{
		startSimulation(nsteps, outputstep);
	}

	void startSimulationInStepsWithCollisionInput(
		const unsigned int nsteps, const unsigned int outputstep, const char *colfilename);

	//runtime in milisec., outputtimeinterval in  milisec.
	inline void startSimulationInMiliSeconds(const double& runtime, const double outputTimeInterval)
	{
		const unsigned int nsteps = runtime*0.001/m_timestep;			//m_timestep is in sec.
		unsigned int outputstep = outputTimeInterval*0.001/m_timestep;
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