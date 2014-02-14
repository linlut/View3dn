//FILE: simu_engine.cpp
#include <memory.h>
#include <sysconf.h>
#include <ztime.h>
#include "simu_engine.h"
#include "simu_entity.h"


//static class var definition
CSimuEngine CSimuEngine::staticEngine;

void CSimuEngine::clear(void)
{
	for (int i=0; i<m_nObjects; i++){
		SafeDelete(m_pObjects[i]);
		m_pObjects[i] = NULL;
	}
	m_nObjects = 0;
}

void CSimuEngine::_advanceOneTimeStep(const int stepID)
{
	for (int i=0; i<m_nObjects; i++){
		CSimuEntity* pobj = m_pObjects[i];
		ASSERT0(pobj!=NULL);
		pobj->advanceOneTimeStep(stepID, m_timestep);
	}
}


void CSimuEngine::preStartSimulation(void)
{
	for (int i=0; i<m_nObjects; i++)
		m_pObjects[i]->setTimeStepForMaterialRelatedIssues(m_timestep);
}


void CSimuEngine::startSimulation(const int nsteps, const int outputstep)
{
	char objfilename[200];
	//set up materials, in case problem
	//esp. to strain rate, we pre-mult the 1/dt/dt on the lame cofficients of damping
	preStartSimulation();
	int N = outputstep;
	if (nsteps<0) return;

	for (int i=0; i<nsteps; i++){
		int &m_nStepID = i;
#ifdef _DEBUG
		if (i==nsteps-1){
			int agag=1;
			int yyy=2;
		}
#endif			
		_advanceOneTimeStep(m_nStepID);
		if (m_nStepID%N==0){
			const int istep = m_nStepID/N;
			//call the display callback function
			if (m_pFuncDisplay) m_pFuncDisplay(this);
			sprintf(objfilename, "output%d.obj", istep);
			this->exportMesh(istep, objfilename); 
			sprintf(objfilename, "output%d.plt", istep);
			this->exportMesh(istep, objfilename);
			printf("Total %d steps, now %d\r", nsteps, m_nStepID);
		}
	}
	printf("\n");
}


class CollisionFileInput
{
private:
	FILE *fp;

public:
	//
	~CollisionFileInput(void)
	{
		if (fp) fclose(fp);
	}

	CollisionFileInput(const char *fname)
	{
		fp = fopen(fname, _RA_);
		assert(fp!=NULL);
	}

	//end of file
	bool eof(void)
	{
		bool r= feof(fp);
		return r;
	}

	//advanceOneLine
	bool advanceOneLine(Vector3d pos[], int id[], int &n)
	{
		const int BUFFERLEN = 1024;
		char sbuffer[BUFFERLEN];
		fgets(sbuffer, BUFFERLEN-1, fp);
		const char *spos;
		n=0;

		spos = strstr(sbuffer, "Collision buffer size");
		if (spos!=NULL){
			sscanf(spos+22, "%d", &n);
			for (int i=0; i<n; i++){
				fgets(sbuffer, BUFFERLEN-1, fp);
				double x, y, z;
				sscanf(sbuffer, "VID %d, %lg %lg %lg", &id[i], &x, &y, &z);
				pos[i] = Vector3d(x, y, z);
			}
		}
		return true;
	}
};

void CSimuEngine::startSimulationInStepsWithCollisionInput(
	const int nsteps, const int outputstep, const char *colfilename)
{
	preStartSimulation();
	CollisionFileInput col(colfilename);
	double phytime = 10000;

	const int N = outputstep;
	const double TIMEOUT=1.0/30;
	Vector3d colpos[100];
	int vertexid[100];
	int ncollision;

	for (int i=0; i<nsteps; i++, phytime+=m_timestep){
		if (phytime>=TIMEOUT){
			phytime=0;
			if (!col.eof()){
				col.advanceOneLine(colpos, vertexid, ncollision);
				const double cur_time = i*m_timestep*1000.0;
				const double timeoutMS = TIMEOUT*1000;
				m_pObjects[0]->updateCollisionConstraint(colpos, vertexid, ncollision, cur_time, timeoutMS);
			}
		}
		int &m_nStepID = i;
		_advanceOneTimeStep(m_nStepID);
		if (m_nStepID%N==0){
			m_pObjects[0]->exportMesh(m_nStepID/N, "", "");
			printf("Total %d steps, current %d step\r", nsteps, m_nStepID);
		}
	}
	printf("\n");
}


void CSimuEngine::copyVertexPositions(vector<Vector3d>& vtbuf) const
{
	Vector3d zerovec(0,0,0);
	vtbuf.clear();
	vtbuf.push_back(zerovec);

	for (int i=0; i<m_nObjects; i++){
		const CSimuEntity* pobj = m_pObjects[i];
		CSimuEntity& a = (CSimuEntity&)(*pobj);
		ASSERT0(pobj!=NULL);
		const CSimuEntity::VertexInfo* pvertinfo = a.getVertexInfoPtr();
		const int nv = a.getVertexCount();
		for (int j=0; j<nv; j++)
			vtbuf.push_back(pvertinfo[j].m_pos);		
	}
}

bool CSimuEngine::_exportSceneToObj(const char *filename, const int stepid, const int objid) const
{
	bool rflag = true;
	//parameter check
	if (m_nObjects==0 || objid>=m_nObjects) 
		return false;
	FILE *fp = fopen(filename, _WA_);
	if (fp==NULL) return false;

	//export comments
	int nobj = 1;
	if (objid<0) nobj = m_nObjects;
	fprintf(fp, "#Simu engine OBJ export, %d objects\n", nobj);
	fprintf(fp, "mtllib material.mtl\n");

	//export only a specified object
	if (objid>=0){
		const CSimuEntity* pobj = m_pObjects[objid];
		pobj->exportOBJFileObject(stepid, objid, 0, 0, fp);
		goto RET_POINT;
	}

	//export all the simulation objects in the scene
	int vertexbaseid=1, texbaseid=1;
	for (int i=0; i<m_nObjects; i++){
		const CSimuEntity* pobj = m_pObjects[i];
		pobj->exportOBJFileObject(stepid, i, vertexbaseid, texbaseid, fp);
		const int nv = pobj->m_nVertexCount;
		vertexbaseid += nv;

	}

RET_POINT:
	if (fp!=NULL) fclose(fp);
	return rflag;
}


bool CSimuEngine::_exportSceneToPlt(const char *filename, const int stepid, const int id) const
{
	bool rflag = true;
	//parameter check
	if (m_nObjects==0 || id>=m_nObjects) 
		return false;
	FILE *fp = fopen(filename, _WA_);
	if (fp==NULL) return false;

	//export comments
	int nobj = 1;
	if (id<0) nobj = m_nObjects;

	//export only a specified object
	if (id>=0){
		CSimuEntity* pobj = m_pObjects[id];
		assert(pobj!=NULL);
		pobj->exportMeshPlt(fp);
		goto RET_POINT;
	}

	//export all the simulation objects in the scene
	for (int i=0; i<m_nObjects; i++){
		CSimuEntity* pobj = m_pObjects[i];
		assert(pobj!=NULL);
		pobj->exportMeshPlt(fp);
	}

RET_POINT:
	if (fp!=NULL) fclose(fp);
	return rflag;
}


bool CSimuEngine::exportMesh(const int SID, const char *filename, const int oid) const
{
	bool r=false;
	const int n = strlen(filename);
	if (strcmp(&filename[n-4], ".obj")==0 || strcmp(&filename[n-4], ".OBJ")==0){
		r = _exportSceneToObj(filename, SID, oid);
	}
	else{
		if (strcmp(&filename[n-4], ".plt")==0 || strcmp(&filename[n-4], ".PLT")==0){
			r = _exportSceneToPlt(filename, SID, oid);
		}
	}
	return r;
}


void CSimuEngine::updateCollisionConstraint(
	const Vector3d *pVertexPos, const int *pVertexID, const int nv, 
	const double &cur_time, const double& inputTimeInterval)
{
	if (m_nObjects<1) return;

	//first, decide the object vertex id
	Vector2i vrange[100];
	vrange[0].x = 0; 
	vrange[0].y = m_pObjects[0]->getVertexCount();
	for (int i=1; i<m_nObjects; i++){
		vrange[i].x = vrange[i-1].y;
		const int totalvert = m_pObjects[i]->getVertexCount();
		vrange[i].y = vrange[i].x + totalvert;
	}

	//a naive method to distribute all of the constraints into different objects
	const int BUFFLENGTH = 2000;
	int vidbuff[BUFFLENGTH];
	Vector3d vposbuff[BUFFLENGTH];
	for (int i=0; i<m_nObjects; i++){
		int cc = 0;
		const int vlow = vrange[i].x;
		const int vhigh = vrange[i].y;
		for (int j=0; j<nv; j++){
			const int vid = pVertexID[j];
			if (vid>=vlow && vid<vhigh){
				vidbuff[cc] = vid-vlow;
				vposbuff[cc] = pVertexPos[j];
				cc++;
			}
		}
		if (cc>BUFFLENGTH){
			printf("Buffer overflow, size is %d, limit is %d.\n", cc, BUFFLENGTH);
			exit(0);
		}
		else if (cc>=0){
			m_pObjects[i]->updateCollisionConstraint(
				pVertexPos, pVertexID, nv, cur_time, inputTimeInterval);
		}
	}
}



//=========================================================
static int test_func(void)
{
	CSimuEngine e;
	return 1;
}

static int test = test_func();

