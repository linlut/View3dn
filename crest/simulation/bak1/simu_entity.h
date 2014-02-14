//FILE: simu_entity.h

#ifndef _INC_SIMU_ENTITY_ZN2008_H
#define _INC_SIMU_ENTITY_ZN2008_H


#include <stdio.h>
#include <assert.h>
#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "time_integration.h"
#include "constraints.h"

class CSparseMatrix33;
class CSimuEngine;

#ifndef INT64KEY
#define INT64KEY __int64
#endif

class CEdgeAuxStruct
{
public:
	int m_vertID[2];
	double m_mass;
	double m_stiff;

	CEdgeAuxStruct(void)
	{
		m_mass = 0;
		m_stiff = 0;
		m_vertID[0] = m_vertID[1]=0;
	}

	~CEdgeAuxStruct(void){}

	static INT64KEY INT2KEY(const int key0, const int key1)
	{
		int a, b;
		if (key0<key1)
			a = key0, b=key1;
		else
			a = key1, b=key0;

		INT64KEY x = b;
		x<<=32;
		x+=a;
		return x;		
	}

	INT64KEY getKey(void) const
	{
		return INT2KEY(m_vertID[0], m_vertID[1]);
	}
};


class CImplicitSolverData
{
public:
	CSparseMatrix33 *m_pSparseMat33;
	Vector3d * m_pVecBuffer0;
	Vector3d * m_pVecBuffer1;

private:
	int m_nv;

public:
	CImplicitSolverData(void)
	{
		m_pVecBuffer0 = NULL;
		m_pVecBuffer1 = NULL;
		m_pSparseMat33 = NULL;
		m_nv = 0;
	}

	~CImplicitSolverData(void);
	
	void update(const int nv);

	static void VectorArrayProduct(const Vector3d *p0, const Vector3d *p1, const int n, Vector3d *p)
	{
		for (int i=0; i<n; i++){
			p[i].x = p0[i].x*p1[i].x;
			p[i].y = p0[i].y*p1[i].y;
			p[i].z = p0[i].z*p1[i].z;
		}
	}
};


//base class for all simulation entities
#define CONSTRAINT_BUFF_LENGTH  8
class CSimuEntity
{
	friend class CSimuEngine;

public:
	class VertexInfo
	{
	public:
		Vector3d m_pos;					//vertex position at time t
		Vector3d m_velocity;			//velocity of the vertex
		Vector3d m_acc;					//acceleration
		Vector3d m_force;				//sumed force
		double m_mass;					//mass
		double m_var;					//no use yet
		CVertexConstraints m_constraints;//constraints for each vertex
		//----------4 TMP vars, for time integration usage-----------
		Vector3d m_tmp0;				//usually used as the position temp bufffer
		Vector3d m_tmp1;				//vertex velocity buffer
		Vector3d m_tmp2;				//vertex acceleration buffer
		Vector3d m_tmp3;				//vertex force buffer

	public:
		VertexInfo(){}
		~VertexInfo(){}

		//for time integration
		inline Vector3d getLatestPosition(void)
		{
			return m_tmp0;
		}

		inline void updateCurrentPosition(const Vector3d & pos)
		{
			m_pos = pos;
		}

		inline void updateCurrentAndPreviousPosition(const Vector3d & pos)
		{
			m_pos = pos;
			m_tmp0 = pos;
		}
	};

	CImplicitSolverData	*m_pImplicitSolverData;		//Sparse mat data used for implicit integration

protected:
	float * m_pTextureCoord;	//texture coordinate
	int m_nTextureCoordStride;	//=2, tx, tx
								//=3, solid texture tx, ty, tz
	VertexInfo *m_pVertInfo;	//buffer of vertex info
	int m_nVertexCount;			//number of vertices
	int m_nElementCount;		//number of elements used for simmulation

	Vector3d m_gravity_acc;		//gravity acceleration
	CMeMaterialProperty m_mtl;	//Computational mech. based material property
	CConstraint *m_pConstraints[CONSTRAINT_BUFF_LENGTH];
	CCollisionConstraint *m_pCollisionConstraint;

	//save stiffness tensor into sparse matrix
	void saveVertSitffMatrixIntoSparseMatrix(const int vi, const int vj, const double3x3& stiffmat);

	void applyAllPositionConstraints(const double &current_tm, const double &dt);

private:

	//velocity verlet integration method
	inline void _velocityVerletIntegration(const int timeid, const double& dt)
	{
		VelocityVerletIntegration(*this, timeid, dt);
	}

	inline void _velocityBasedIntegration(const int timeid, const double& dt)
	{
		VelocityBasedIntegration(*this, timeid, dt);
	}

	//According to Matthias Teschner et al. CGI 2004 
	//"A Versatile and Robust Modelfor Geometrically Complex Deformable Solids"
	//Verlet is slightly better than velocity verlet
	inline void _verletIntegration(const int timeid, const double& dt)
	{
		VerletIntegration(*this, timeid, dt);
	}

	inline void _implicitRectangularIntegration(const int timeid, const double& dt)
	{
		CMeMaterialProperty mtlbak = m_mtl;
		m_mtl.setDamping(0,0);			//disable dampling
		ImplicitRectangularIntegration(*this, timeid, dt);
		m_mtl = mtlbak;
	}

	void _implicitEulerIntegration(const int timeid, const double& dt);

	//assign mass to avoid problems for zero-mass nodes
	void _checkMass(void);


public:
	CSimuEntity(
		const Vector3d *pVertex, const int nv, const Vector3d &init_velocity, 
		const CMeMaterialProperty* pMtl=NULL //material property				
		);

	virtual ~CSimuEntity(void);

	inline int getVertexCount(void) const
	{
		return m_nVertexCount;
	}

	inline int getElementCount(void) const
	{
		return m_nElementCount;
	}

	inline VertexInfo *getVertexInfoPtr(void)
	{
		return m_pVertInfo;	
	}

	inline const CMeMaterialProperty* getMaterial(void) const
	{
		return &m_mtl;	
	}

	//set gravity acceleration
	inline void setGravityAccelertion(const Vector3d &g)
	{
		m_gravity_acc = g;
	}

	inline void setTextureCoord(const float *ptex, const int stride)
	{
		m_pTextureCoord = (float*)ptex;	
		m_nTextureCoordStride = stride;
	}

	//set constraints for a node
	inline void setConstraints(const int vertexID, const CVertexConstraints & constraints)
	{
		if (!m_pVertInfo) return;
		if (vertexID>=0 && vertexID<m_nVertexCount){
			m_pVertInfo[vertexID].m_constraints = constraints;
		}
	}

	inline void setTimeStepForMaterialRelatedIssues(const double &dt)
	{
		m_mtl.setTimeStepLength(dt);
	}

	//output a specified vertex position to a stream
	void dumpVertexPosition(const int vertID, const int sequenceID, FILE *fp) const;

	//init the vertex force in each iteration
	void initForceAndApplyGravity(void);

	//=====================================================
	//a pack of functions for the purpose of constraints
	inline void fixOneVertexPositionFromBackup(const int vid)
	{
		ASSERT0(vid>=0 && vid<m_nVertexCount);
		m_pVertInfo[vid].m_pos = m_pVertInfo[vid].m_tmp0;
	}

	void addConstraint(const CConstraint * pconstraint);

	inline virtual void updateCollisionConstraint(
			const Vector3d *pVertexPos, const int *pVertexID, const int nv, 
			const double &cur_time, const double& inputTimeInterval)
	{
		CCollisionConstraint *&p = m_pCollisionConstraint;
		if (p==NULL)
			p = new CCollisionConstraint(inputTimeInterval);
		ASSERT0(p!=NULL);
		p->updatePosition(pVertexPos, pVertexID, nv, cur_time);
	}

	//=====================================================
	inline virtual void performEdgeRelaxation(const double &current_tm, const double &dt)
	{
		applyAllPositionConstraints(current_tm, dt);
	}

	//one time step
	virtual void advanceOneTimeStep(const int timeid, const double& dt);

	//compute the acceleration vector for the current location 
	//input: true-- also compute jacobian of pF/pX
	//       false-- no need to compute
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false)=0;

	//update necessary state variables after time integration
	virtual void updateState(void)=0;

	//export mesh
	virtual void exportMesh(const int count, const char *name, const char *ext)=0;

	virtual bool exportMeshPlt(FILE *fp)=0;

	//======================debug functions===================================
	//nodeid = -1, export all, otherwise, a specific node
	virtual void exportNodeInfo(const int nodeid=-1);

protected:
	//return value: number of vertices
	virtual int exportOBJFileObject(const int ID, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;
};


void ComputeNodeMassTet(const Vector3d *pVertex, const int m_nVertexCount, 
	const int *pElement, const int nelm, 
	const double &rho, CSimuEntity::VertexInfo *m_pVertInfo);

void ComputeNodeMassHex(const Vector3d *pVertex, const int m_nVertexCount, 
	const int *pElement, const int nelm, 
	const double &rho, CSimuEntity::VertexInfo *m_pVertInfo);

#endif