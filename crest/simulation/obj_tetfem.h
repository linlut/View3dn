//FILE: obj_tetfem.cpp
//Object deformation for tet elements using FEM 

#ifndef _INC_ELMTETFEM_NZ2009_H
#define _INC_ELMTETFEM_NZ2009_H

#include <assert.h>
#include <vector3d.h>
#include <vector4i.h>
#include "simu_entity.h"
#include "element_tetfem.h"


template <class FEMELEMENT_t> 
class _CSimuTetra: public CSimuEntity
{
protected:
	int m_nIsStatic;						//static, small strain FEM, doesnot need to compute rotation
	FEMELEMENT_t *m_pTetra2Element;			//tetrahedral element array represnted using shear elm
	int m_nTetra2Element;					//number of tet. element
	int* m_pSurfacePoly;
	int m_nSurfacePoly;

private:
	//compute elastic forces
	void _computeElasticForces(const unsigned int timeid, const bool needjacobian)
	{
		Vector3d force[4];
		const int NSIFF = 9;
		double3x3 jacobian[NSIFF], *ppjac[NSIFF];
		for (int i=0; i<NSIFF; i++) ppjac[i]=NULL;
		if (needjacobian){
			for (int i=0; i<NSIFF; i++)
				jacobian[i].setZeroMatrix(), ppjac[i]=&jacobian[i];
		}

		CSparseMatrix33 *pSM = m_pOdeIntegrator->getSparseMatrix();
		for (int i=0; i<m_nElementCount; i++){
			FEMELEMENT_t &E = m_pTetra2Element[i];
			const int* pNodeBuffer = E.getElementNodeBuffer();
			const int ia = pNodeBuffer[0];
			const int ib = pNodeBuffer[1];
			const int ic = pNodeBuffer[2];
			const int id = pNodeBuffer[3];
			E.computeForce(timeid, m_nIsStatic, 
				m_pVertInfo[ia].m_pos, 
				m_pVertInfo[ib].m_pos, 
				m_pVertInfo[ic].m_pos, 
				m_pVertInfo[id].m_pos, 
				m_mtl, force, ppjac);
			m_pVertInfo[ia].m_force += force[0];
			m_pVertInfo[ib].m_force += force[1];
			m_pVertInfo[ic].m_force += force[2];
			m_pVertInfo[id].m_force += force[3];
			if (needjacobian) 
				E.saveStiffness(pSM, jacobian);		
		}
	}

	//compute per node mass in initialization
	inline void _computeNodeMass(const Vector3d *pVertex,const int *pTetElement, const int ntetelm, const double &rho)
	{
		ComputeNodeMassTet(pVertex, m_nVertexCount, pTetElement, ntetelm, rho, m_pVertInfo);
	}

	inline bool _exportMeshPLT(const char *fname)
	{
		FILE *fp = fopen(fname, _WA_);
		if (fp==NULL) return false;
		const int nv = m_nVertexCount;
		const bool r = ExportMeshPLTSurface(m_pVertInfo, nv, m_pSurfacePoly, 3, m_nSurfacePoly, fp);
		fclose(fp);
		return r;
	}

public:
	_CSimuTetra(
		const int isStatic,
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int ntetelm,		//element information
		const int *pFace, const int nface,
		const CMeMaterialProperty& mtl				//material property
		):
		CSimuEntity(pVertex, nv, init_velocity, &mtl), 
		m_nIsStatic(0)
	{
		m_nIsStatic = isStatic;
		m_bHasConstantStiffness = FEMELEMENT_t::hasConstantStiffness();
		m_bStiffnessIsMatrix33 = FEMELEMENT_t::isStiffnessMatrix3X3();

		//alloc buffer
		m_nElementCount = ntetelm;	
		m_pTetra2Element = new FEMELEMENT_t[m_nElementCount];
		assert(m_pTetra2Element!=0);

		//
		m_pSurfacePoly = new int [nface*3];
		assert(m_pSurfacePoly!=NULL);
		memcpy(m_pSurfacePoly, pFace, sizeof(int)*nface*3);
		m_nSurfacePoly = nface;

		//first, need to decide mass
		m_mtl.init();
		double rho = m_mtl.getDensity();
		_computeNodeMass(pVertex, pElement, ntetelm, rho);

		//then, init each tetra2 element
		const Vector4i *pquad = (const Vector4i*)pElement;
		for (int i=0; i<m_nElementCount; i++){
			FEMELEMENT_t *p = &m_pTetra2Element[i];
			const Vector4i& q = pquad[i];
			p->init(m_mtl, &q.x, pVertex[q.x], pVertex[q.y], pVertex[q.z], pVertex[q.w]);
		}
		printf("There are %d tetrahedral elements generated.\n", m_nElementCount);
	}

	virtual ~_CSimuTetra(void)
	{
		SafeDeleteArray(m_pTetra2Element);
		SafeDeleteArray(m_pSurfacePoly);
	}

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false)
	{
		//init forces as zero and add gravity load
		initForceAndApplyGravity();

		//add elastic forces
		_computeElasticForces(timeid, needJacobian);

		//add collison forces

		//compute acc
		for (int i=0; i<m_nVertexCount; i++){
			VertexInfo *node = &m_pVertInfo[i];
			Vector3d &a = node->m_acc;
			Vector3d &f = node->m_force;
			double invmass = 1.0/node->m_mass;
			a = f * invmass;
		}
	}

	//update necessary state variables after time integration
	virtual void updateState(void)
	{
		for (int i=0; i<m_nElementCount; i++){
			m_pTetra2Element[i].updateState();
		}
	}

	virtual void exportMesh(const int count, const char *name, const char *ext)
	{
		char ffname[200];
		sprintf(ffname, "shell%d.plt", count);
		_exportMeshPLT(ffname);
	}

	virtual bool exportMeshPlt(FILE *fp)
	{
		const int nSurfaceType = 3;
		return ExportMeshPLTSurface(m_pVertInfo, m_nVertexCount, 
			m_pSurfacePoly, nSurfaceType, m_nSurfacePoly, fp);
	}

	virtual int exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
	{
		const float *m_pTextureCoord = NULL;
		const int m_nTextureCoordStride = 2;
		const int nSurfaceType=3;
		const bool r = ExportMeshOBJSurface(stepid, objid, m_pVertInfo, m_nVertexCount, 
				m_pSurfacePoly, nSurfaceType, m_nSurfacePoly, vertexbaseid, m_pTextureCoord, m_nTextureCoordStride, texbaseid, fp);
		return m_nVertexCount;
	}

	virtual void computeStiffnessMatrix(void)
	{
		//check sparse matrix
		CSparseMatrix33 *pmat = this->getOdeIntegrator()->getSparseMatrix();
		if (pmat==NULL) return;
		pmat->clear();	

		const bool needjacobian = true;
		computeAcceleration(1, needjacobian);
	}

};


//Tet mesh simulation using corotational method
typedef _CSimuTetra<CTetFemElement> CSimuTetra;
typedef CSimuTetra CSimuTetraCorotation;


#include "element_tetfem2.h"
typedef _CSimuTetra<CTetFem2Element> CSimuTetra2;
typedef CSimuTetra2 CSimuTetraGreen;


#include "element_tetfem3.h"
typedef _CSimuTetra<CTetFem3Element> CSimuTetra3;
typedef CSimuTetra3 CSimuTetraShell;

#endif
