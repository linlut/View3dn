//FILE: simu_entity.h

#ifndef _INC_SIMU_ENTITY_ZN2008_H
#define _INC_SIMU_ENTITY_ZN2008_H


#include <stdio.h>
#include <assert.h>
#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "constraints.h"

class CSimuEngine;
class ODEIntegrator;
class CSparseMatrix33;

#ifndef INT64KEY
#define INT64KEY __int64
#endif

class CEdgeAuxStruct
{
public:
	int m_vertID[2];
	double m_mass;
	double m_crossarea;
	double m_stiff;

	CEdgeAuxStruct(void)
	{
		m_mass = 0;
		m_crossarea = 0;
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


//base class for all simulation entities
#define CONSTRAINT_BUFF_LENGTH  12
class CSimuEntity
{
	friend class CSimuEngine;
	friend class CConstraint;

public:
	struct VertexInfo
	{
	public:
		Vector3d m_pos;					//vertex position at time step t^n
		Vector3d m_pos0;				//vertex position for time step t^(n-1)
		Vector3d m_velocity;			//velocity for time step t^n
		Vector3d m_velocity0;			//velocity for time step t^(n-1)
		Vector3d m_acc;					//acceleration
		Vector3d m_force;				//sumed force
		double m_mass;					//mass
		CVertexConstraints m_constraints;//constraints for each vertex

	public:
		VertexInfo(){}
		~VertexInfo(){}

		//for time integration
		inline Vector3d getLatestPosition(void)
		{
			return m_pos;
		}

		inline void updateCurrentPosition(const Vector3d & pos)
		{
			m_pos = pos;
		}

		inline void updateCurrentAndPreviousPosition(const Vector3d & pos)
		{
			m_pos = pos;
			m_pos0 = pos;
		}

	};

protected:
	int m_nIntegrationMethod;	//set type of integration method
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

	//===================================
	bool m_bHasConstantStiffness;	//stiffness const or not?
	bool m_bStiffnessIsMatrix33;	//in implicit integration, sparse matrix elements' type, 0: double, 1: double3x3
	bool _padding2[2];

	//===================================
	ODEIntegrator *m_pOdeIntegrator;

protected:
	//save stiffness tensor into sparse matrix
	void saveVertSitffMatrixIntoSparseMatrix(const int vi, const int vj, const double3x3& stiffmat, const bool FAST=false);

	//save stiffness for a vertex fan structure
	void saveVertSitffMatrixIntoSparseMatrix(
		CSparseMatrix33 *pStiffMatrix,	//stiffness matrix
		const int vi,					//center vertex ID
		const int vj_array[],			//1-ring neighbour vertex list
		const double3x3 stiffmatj[],	//stiffness matrix list
		const int vjlen,				//length of 1-ring vertex list
		const bool FASTSTIFF);			//stiffness type, true:double, false:double3x3
	void saveVertSitffMatrixIntoSparseMatrix(const int vi, const int vj[], const double3x3 stiffmatj[], const int jlen, const bool FASTSTIFF=false);

	//save stiffness for thin shell/triangle pair structure
	void saveVertSitffMatrixIntoSparseMatrix(const int vi, const int vj, const int vk, const double3x3 stiffmat[], const bool FASTSTIFF=false);

	//save stiffness for thin shell/ quad pair structure
	void saveVertSitffMatrixIntoSparseMatrix(const int vi, const int vj, const int vk, const int vl, const double3x3 stiffmat[], const bool FASTSTIFF=false);

	void applyAllPositionConstraints(const double &current_tm, const double &dt);

	//return value: number of vertices
	virtual int exportOBJFileObject(const int ID, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;

private:
	//assign mass to avoid problems for zero-mass nodes
	void _checkMass(void);

public:
	CSimuEntity(
		const Vector3d *pVertex,			//vertex array
		const int nv,						//# vertices
		const Vector3d &init_velocity,		//initial velocity
		const CMeMaterialProperty* pMtl		//material pointer of the entity
		);

	virtual ~CSimuEntity(void);

	inline bool hasConstantStiffness(void)
	{
		return m_bHasConstantStiffness;
	}

	inline bool isStiffnessMatrix3X3(void)
	{
		return m_bStiffnessIsMatrix33;
	}

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

	inline ODEIntegrator * getOdeIntegrator(void)
	{
		return m_pOdeIntegrator;
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
		m_mtl.init();
		m_mtl.computeEffectiveYoungDamp();
	}

	//output a specified vertex position to a stream
	void dumpVertexPosition(const int vertID, const int sequenceID, FILE *fp) const;

	//init the vertex force in each iteration
	void initForceAndApplyGravity(void);

	//set boundary conditions
	void setFixedVerticesByZValue(const double minz, const double maxz);

	void setIntegrationMethod(const int tid);

	//==========some demo func=============================
	inline void flattenX(const double x0)
	{
		double delta;
		for (int i=0; i<m_nVertexCount; i++){
			delta = 2e-5 * (i%17);
			m_pVertInfo[i].m_pos.x = x0 + delta;
		}
	}

	//=====================================================

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

	inline virtual void applyZeroForceVelo(const double &current_tm, const double &dt)
	{
		//other constraints, user specified
		for (int i=0; i<CONSTRAINT_BUFF_LENGTH; i++){
			CConstraint *p = m_pConstraints[i];
			if (p){
				if (p->isEnabled() && (!p->isExpired(current_tm)))
					p->applyZeroForceVelo(this, current_tm, dt);
			}
		}
	}

	//one time step
	virtual void advanceOneTimeStep(const unsigned int timeid, const double& dt);

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

	virtual void plastifyItself(const double& threshold){}

	virtual void computeStiffnessMatrix(void)=NULL;

};


void ComputeNodeMassTet(const Vector3d *pVertex, const int m_nVertexCount, 
	const int *pElement, const int nelm, 
	const double &rho, CSimuEntity::VertexInfo *m_pVertInfo);

void ComputeNodeMassHex(const Vector3d *pVertex, const int m_nVertexCount, 
	const int *pElement, const int nelm, 
	const double &rho, CSimuEntity::VertexInfo *m_pVertInfo);

bool ExportMeshPLTSurface(const CSimuEntity::VertexInfo *m_pVertInfo, const int nv, 
	const int *m_pSurfacePoly, const int m_nSurfaceType, const int m_nSurfacePoly, FILE *fp);

bool ExportMeshOBJSurface(const int ID, const int objid, const CSimuEntity::VertexInfo *m_pVertInfo, const int nv, 
	const int *m_pSurfacePoly, const int m_nSurfaceType, const int m_nSurfacePoly, const int vertexbaseid, 
	const float *m_pTextureCoord, const int m_nTextureCoordStride, const int texbaseid, 
	FILE *fp);



#endif