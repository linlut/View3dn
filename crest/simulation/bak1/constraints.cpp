//FILE: constraints.cpp
#include <geomath.h>
#include "simu_entity.h"
#include "constraints.h"

typedef	CSimuEntity::VertexInfo VertexInfo;


bool CForceConstraint::_applyForce(const int vid)
{
	return true;

}

CForceConstraint::CForceConstraint(
	const int prior, const double &tm_start, const double& tm_end, const int nv, const int *vertbuffer, const Vector3d& force):
	CConstraint(prior, tm_start, tm_end, nv, vertbuffer)
{
	m_force = force;
}


void CForceConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	VertexInfo *pVertex = pobj->getVertexInfoPtr();
	if (m_nVertexCount>0){ 
		for (int i=0; i<m_nVertexCount; i++){
			const int vid = m_pVertexIDArray[i];
			VertexInfo *p = &pVertex[vid];
			//_applyForce();
		}
		return;
	}

	if (m_nVertexCount<0){//all the vertices are involved
		const int nv = pobj->getVertexCount();
		for (int i=0; i<nv; i++){
			VertexInfo *p = &pVertex[i];
			//_applyForce(pos0, pos1);
		}
	}
}


CFixedPositionConstraint::CFixedPositionConstraint(
	const int prior, const double &tm_start, const double& tm_end, const int nv, const int * vertbuffer):
	CConstraint(prior, tm_start, tm_end, nv, vertbuffer)
{
	m_pVertexPos = NULL;
}

void CFixedPositionConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	const int nVert = pobj->getVertexCount();
	CSimuEntity::VertexInfo* pVertInfo = pobj->getVertexInfoPtr();
	if (m_pVertexPos==NULL){//first call
		if (m_nVertexCount>0){
			m_pVertexPos = new Vector3d[m_nVertexCount+2];
			for (int i=0; i<m_nVertexCount; i++){
				const int vid = m_pVertexIDArray[i];
				m_pVertexPos[i] = pVertInfo[vid].m_pos;
			}
		}
		else{
			m_nVertexCount = nVert;
			m_pVertexPos = new Vector3d[nVert];
			for (int i=0; i<m_nVertexCount; i++)
				m_pVertexPos[i] = pVertInfo[i].m_pos;
		}
		assert(m_pVertexPos!=NULL);
	}

	for (int i=0; i<m_nVertexCount; i++){
		const Vector3d pos = m_pVertexPos[i];
		int vid = i;
		if (m_nVertexCount != nVert) 
			vid = m_pVertexIDArray[i];
		//pVertInfo[vid].updateCurrentAndPreviousPosition(pos);
		pVertInfo[vid].updateCurrentPosition(pos);
		//pobj->fixOneVertexPositionFromBackup(vid);
	}
}

//=====================================================================

CPlaneConstraint::CPlaneConstraint(
	const int prior, const double &tm_start, const double& tm_end, 
	const int nv, const int *vertbuffer, const double plane[4]):
	CConstraint(prior, tm_start, tm_end, nv, vertbuffer)
{
	m_plane[0] = plane[0];
	m_plane[1] = plane[1];
	m_plane[2] = plane[2];
	m_plane[3] = plane[3];
}

void CPlaneConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	VertexInfo *pVertex = pobj->getVertexInfoPtr();
	Vector3d pos0, pos1;
	if (m_nVertexCount>0){ 
		for (int i=0; i<m_nVertexCount; i++){
			const int vid = m_pVertexIDArray[i];
			VertexInfo *p = &pVertex[vid];
			pos0 = p->getLatestPosition();
			bool r = _projectVertex(pos0, pos1);
			if (r) p->updateCurrentAndPreviousPosition(pos1);
		}
		return;
	}

	if (m_nVertexCount<0){//all the vertices are involved
		const int nv = pobj->getVertexCount();
		for (int i=0; i<nv; i++){
			VertexInfo *p = &pVertex[i];
			pos0 = p->getLatestPosition();
			const bool r = _projectVertex(pos0, pos1);
			if (r) p->updateCurrentAndPreviousPosition(pos1);
		}
	}
}
//=================================================================

CSphereConstraint::CSphereConstraint(
	const int prior, const double &tm_start, const double& tm_end, 
	const int nv, const int *vertbuffer, const double& radius, const Vector3d &center):
	CConstraint(prior, tm_start, tm_end, nv, vertbuffer)
{
	m_center = center;
	m_radius = radius;
}


void CSphereConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	Vector3d pos0, pos1;
	VertexInfo *pVertex = pobj->getVertexInfoPtr();
	ASSERT0(pVertex!=NULL);
	if (m_nVertexCount>0){ 
		for (int i=0; i<m_nVertexCount; i++){
			const int vid = m_pVertexIDArray[i];
			VertexInfo *p = &pVertex[vid];
			pos0 = p->getLatestPosition();
			bool r = _projectVertex(pos0, pos1);
			if (r) p->updateCurrentAndPreviousPosition(pos1);
		}
		return;
	}
	if (m_nVertexCount<0){//all the vertices are involved
		const int nv = pobj->getVertexCount();
		for (int i=0; i<nv; i++){
			VertexInfo *p = &pVertex[i];
			pos0 = p->getLatestPosition();
			const bool r = _projectVertex(pos0, pos1);
			if (r) p->updateCurrentAndPreviousPosition(pos1);
		}
	}
}

//=================================================================
CPlanarRotationConstraint::CPlanarRotationConstraint(
	const int prior, const double &tm_start, const double& tm_end, 
	const int nv, const int *vertbuffer, 
	const Vector3d &center, const Vector3d &norm, const double &av
	):  
	CConstraint(prior, tm_start, tm_end, nv, vertbuffer)
{
	m_center = center;
	m_norm = norm;
	m_av = av;
}

inline Vector3d _roateAPoint(const Vector3d &P, const Vector3d& P0, const Matrix2d& mat)
{
	Vector3d d = P - P0;
	Vector3d rotd = d*mat;
	Vector3d q = P0+rotd;
	return q;
}

void CPlanarRotationConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	const Vector3d zerovec(0, 0, 0);
	double angle=m_av * dt;
	Matrix2d mat;
	RotationAxisMatrix(m_norm, angle, mat);
	VertexInfo *pvert = pobj->getVertexInfoPtr();

	if (m_nVertexCount>0){ 
		for (int i=0; i<m_nVertexCount; i++){
			const int vid = m_pVertexIDArray[i];
			Vector3d & x0 = pvert[vid].m_tmp0;
			Vector3d & x = pvert[vid].m_pos;
			x = _roateAPoint(x0, m_center, mat);
			x0 = x;
		}
		return;
	}

	if (m_nVertexCount<0){//all the vertices are involved
		const int nv = pobj->getVertexCount();
		for (int i=0; i<nv; i++){
			const int vid = i;
			Vector3d & x0 = pvert[vid].m_tmp0;
			Vector3d & x = pvert[vid].m_pos;
			x = _roateAPoint(x0, m_center, mat);
			x0 = x;
		}
	}
}
 

//=================================================================
void CVelocityConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	VertexInfo *pvert = pobj->getVertexInfoPtr();
	const Vector3d dx = m_velocity*dt;
	if (m_nVertexCount>0){ 
		for (int i=0; i<m_nVertexCount; i++){
			const int vid = m_pVertexIDArray[i];
			Vector3d & x0 = pvert[vid].m_tmp0;
			Vector3d & x = pvert[vid].m_pos;
			x = x0+dx;
		}
	}
}


//=================================================================
void CMouseConstraint::applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	double t = dt/(fabs(m_endTime- current_tm) + 1E-10);
	if (t>1) t=1;
	else if (t<0) t=0;
	
	VertexInfo &vert = pobj->getVertexInfoPtr()[m_nVertexID];
	const Vector3d pos0 = vert.m_tmp0;		//old position
	const Vector3d dist = m_targetPos - pos0;
	const Vector3d apos = dist*t + pos0;
	vert.updateCurrentAndPreviousPosition(apos);
}


//=================================================================
void CCollisionConstraint::updatePosition(
	const Vector3d *pVertexPos, const int *pVertexID, const int nv, const double &cur_time)
{
	m_nVertexCount = nv;
	if (nv>m_nVertexBufferLen){
		m_nVertexBufferLen = nv;
		m_nVertexCount = nv;
		SafeDeleteArray(m_pVertexIDArray);
		SafeDeleteArray(m_pVertexPos);	
		m_pVertexIDArray = new int [nv];
		m_pVertexPos = new Vector3d [nv];
		ASSERT0(m_pVertexIDArray!=NULL && m_pVertexPos!=NULL);
	}
	for (int i=0; i<nv; i++){
		m_pVertexIDArray[i] = pVertexID[i];
		m_pVertexPos[i] = pVertexPos[i];
	}
	m_startTime = cur_time;
	m_endTime = cur_time + m_inputTimeInterval;
	m_bEnabled = true;
}


void CCollisionConstraint::applyConstraint(
	CSimuEntity *pobj, const double &current_tm, const double &dt)
{
	//printf("Collision constriant count=%d\n", m_nVertexCount);
	if (m_nVertexCount==0) return;
	double t = dt/(fabs(m_endTime- current_tm) + 1E-10);
	if (t>1) t=1;
	else if (t<0) t=0;
	
	VertexInfo* pVertInfo = pobj->getVertexInfoPtr();
	for (int i=0; i<m_nVertexCount; i++){
		const int vid = m_pVertexIDArray[i];
		VertexInfo *pCurVert = &pVertInfo[vid];
		const Vector3d& pos0 = pCurVert->m_tmp0;
		const Vector3d dist = m_pVertexPos[i] - pos0;
		const Vector3d pos = dist*t+pos0;
		//Overwrite the previous step position, this will implicitly set the velocity as ZERO
		pCurVert->updateCurrentAndPreviousPosition(pos);
	}
}
