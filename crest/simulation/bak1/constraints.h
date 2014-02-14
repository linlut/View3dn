//FILE: constraints.h
#ifndef _INC_CONSTRAINTS_ZN2008_H
#define _INC_CONSTRAINTS_ZN2008_H
#include <sysconf.h>
#include <vector3d.h>

class CSimuEntity;

//abstract base class for various types of constraints on the objects
//e.g. fixed position, mouse dragging, collision introduced, et al.
class CConstraint
{
protected:
	double m_startTime;
	double m_endTime;
	//================================================
	int m_nVertexBufferLen;		//the max size of the buffer
	int m_nVertexCount;			//however, the current buffer size
	int *m_pVertexIDArray;		//vertex id array	
	Vector3d *m_pVertexPos;		//vertex position array
	//================================================
	int m_nPrior;
	bool m_bEnabled;			//constraint is enabled or not
	bool ____padding[3];		//some padding bytes

public:
	CConstraint *m_pNext;		//used to form a simle linked list

	void init(const int prior, const double &tm_start, const double& tm_end, const int nv, const int *vertbuffer)
	{	
		m_startTime = tm_start;
		m_endTime = tm_end;
		m_nVertexCount = 0;
		m_pVertexIDArray = NULL;
		m_pVertexPos = NULL;
		m_nPrior = prior;
		m_bEnabled = true;
		m_pNext = NULL;

		m_nVertexCount = nv;
		m_nVertexBufferLen = nv;
		if (m_nVertexCount>0){
			m_pVertexIDArray = new int [nv];
			assert(m_pVertexIDArray!=NULL);
			for (int i=0; i<nv; i++)
				m_pVertexIDArray[i] = vertbuffer[i];
		}
	}

	CConstraint(const int prior, const double &tm_start, const double& tm_end, const int nv, const int *vertbuffer)
	{	
		init(prior, tm_start, tm_end, nv, vertbuffer);
	}

	bool isEnabled(void) const
	{
		return m_bEnabled;
	}

	int getPrior(void) const
	{
		return m_nPrior;
	}

	virtual ~CConstraint()
	{
		SafeDeleteArray(m_pVertexIDArray);
		SafeDeleteArray(m_pVertexPos);
	}

	virtual bool isExpired(const double& currentTime) const
	{
		bool r = false;
		if (currentTime<m_startTime || currentTime>m_endTime)
			r = true;
		return r;
	}

	//current time is in Mili-sec.
	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt)=0;
};


class CFixedPositionConstraint: public CConstraint
{

private:
	Vector3d * m_pVertexPos;

public:
	CFixedPositionConstraint(const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer);
	
	virtual ~CFixedPositionConstraint(void)
	{
		SafeDeleteArray(m_pVertexPos);
	}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};


class CForceConstraint: public CConstraint
{
private:
	Vector3d m_force;

	bool _applyForce(const int vid);

public:
	CForceConstraint(const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer, const Vector3d& force);

	virtual ~CForceConstraint(void){}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};




class CBoxConstraint: public CConstraint
{

};


class CPlaneConstraint: public CConstraint
{
private:
	double m_plane[4];
	bool _projectVertex(const Vector3d &pos0, Vector3d &pos1)
	{
		Vector3d *pnorm = (Vector3d*)&m_plane[0];
		double dist = DotProd(pos0, *pnorm) + m_plane[3];
		if (dist>0)
			return false;
		pos1 = (-dist*1.001)*(*pnorm)+ pos0;
		return true;
	}

public:
	CPlaneConstraint(const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer, const double plane[4]);

	virtual ~CPlaneConstraint(void){}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};


class CSphereConstraint: public CConstraint
{
private:
	double m_radius;
	Vector3d m_center;

	bool _projectVertex(const Vector3d & pos, Vector3d &newpos)
	{
		bool r = false;
		Vector3d dir = pos - m_center;
		const double ll = Magnitude(dir);
		if (ll > m_radius){
			//outside the sphere, no need action
			return r;
		}

		r = true;
		if (ll==0) 
			dir=Vector3d(1,0,0);
		else
			dir /= ll;
		const double GAP = 1e-8;
		newpos= m_center + dir * (GAP+m_radius);
		return r;
	}

public:
	CSphereConstraint(const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer, const double& radius, const Vector3d &center);

	virtual ~CSphereConstraint(void){}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);
};


class CPlanarRotationConstraint: public CConstraint
{
private:
	Vector3d m_center;		//rotation center;
	Vector3d m_norm;		//rotation plane normal direction
	double m_av;			//angular velocity per ms

public:
	CPlanarRotationConstraint(
		const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer, 
		const Vector3d &center, const Vector3d &norm, const double &av
		);

	virtual ~CPlanarRotationConstraint(void){}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};

class CVelocityConstraint: public CConstraint
{
private:
	Vector3d m_velocity;				//velocity

public:
	CVelocityConstraint(
		const int prior, const double &tm_start, const double& tm_end, 
		const int nv, const int *vertbuffer, 
		const Vector3d &velocity):
		CConstraint(prior, tm_start, tm_end, nv, vertbuffer)	
	{
		m_velocity = velocity;
	}

	virtual ~CVelocityConstraint(void){}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};


class CMouseConstraint: public CConstraint
{
private: 
	double m_inputTimeInterval;	//the input frequency, which is used to determine the effective time of the constraint
	int m_nVertexID;			//the node to be controlled
	Vector3d m_targetPos;		//the target vertex position

public:
	//note: inputTimeInterval is in mili-second
	CMouseConstraint(const int vertid, const double inputTimeInterval): 
		CConstraint(10000, 0, -1, 0, NULL)	//priority is very high, but no vertex buffer is needed
	{
		m_nVertexID = vertid;
		m_inputTimeInterval = inputTimeInterval;
		m_targetPos = Vector3d(0,0,0);
	}

	virtual ~CMouseConstraint(void){}

	//note: current time is in mili-second
	virtual void updatePosition(const Vector3d &mspos, const double &cur_time)
	{
		m_bEnabled = true;
		m_targetPos = mspos;
		m_startTime = cur_time;
		m_endTime = cur_time + m_inputTimeInterval;
	}

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};


class CCollisionConstraint: public CConstraint
{
private:
	double m_inputTimeInterval;	//the input frequency, which is used to determine the effective time of the constraint
	
public:
	CCollisionConstraint(const double& inputTimeInterval):
		CConstraint(0, 0, -1, 0, NULL)	
		//const int prior, tm_start, tm_end, const int nv, const int *vertbuffer
	{
		m_inputTimeInterval = inputTimeInterval;
	}

	virtual ~CCollisionConstraint(void){}

	//note: current time is in mili-second
	virtual void updatePosition(const Vector3d *pVertexPos, const int *pVertexID, const int nv, const double &cur_time);

	virtual void applyConstraint(CSimuEntity *pobj, const double &current_tm, const double &dt);

};


#endif