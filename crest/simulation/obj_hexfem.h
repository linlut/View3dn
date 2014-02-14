//FILE: obj_hexfem.cpp
//Object deformation for hexahedral elements using FEM 

#ifndef _INC_OBJHEXFEM_NZ2011_H
#define _INC_OBJHEXFEM_NZ2011_H

#include <assert.h>
#include <vector3d.h>
#include <vector4i.h>
#include "simu_entity.h"
#include "element_cubefem.h"


template <class FEMELEMENT_t> 
class _CSimuHexahedron: public CSimuEntity
{
protected:
	int m_nIsStatic;				//static, small strain FEM, doesnot need to compute rotation
	FEMELEMENT_t *m_pHexElement;	//hexahedral element array represnted using shear elm
	int m_nHexElement;				//number of tet. element
	int* m_pSurfacePoly;
	int m_nSurfacePoly;
	double3x3 m_elmStiffTemplate[8][8];	//stiffness matrix is dependent on Poisson's ratio only

private:
	//compute elastic forces
	void _computeElasticForces(const unsigned int timeid, const bool needjacobian)
	{
		Vector3d force[8];
		const int NSIFF = 64;
		double3x3 jacobian[NSIFF], *ppjac[NSIFF];
		if (needjacobian){
			for (int i=0; i<NSIFF; i++)
				jacobian[i].setZeroMatrix(), ppjac[i]=&jacobian[i];
		}
		else
			for (int i=0; i<NSIFF; i++) ppjac[i]=NULL;

		CSparseMatrix33 *pSM = m_pOdeIntegrator->getSparseMatrix();
		for (int i=0; i<m_nElementCount; ++i){
			FEMELEMENT_t &E = m_pHexElement[i];
			const int* v = E.getElementNodeBuffer();
			E.computeForce(timeid, m_nIsStatic, 
				m_pVertInfo[v[0]].m_pos, 
				m_pVertInfo[v[1]].m_pos, 
				m_pVertInfo[v[2]].m_pos, 
				m_pVertInfo[v[3]].m_pos, 
				m_pVertInfo[v[4]].m_pos, 
				m_pVertInfo[v[5]].m_pos, 
				m_pVertInfo[v[6]].m_pos, 
				m_pVertInfo[v[7]].m_pos, 
				m_mtl, force, ppjac);
			for (int i=0; i<8; i++)
				m_pVertInfo[v[i]].m_force += force[i];

			//Note: the 8x8 matrix elements are all assigned with values
			if (needjacobian){ 
				E.saveStiffness(pSM, jacobian);		
			}
		}
	}

	//compute per node mass in initialization
	inline void _computeNodeMass(
		const Vector3d *pVertex,const int *pElement, const int nelm, const double &rho)
	{
		ComputeNodeMassHex(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
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
	_CSimuHexahedron(
		const int isStatic,
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int nhexelm,		//element information
		const int *pFace, const int nface,
		const CMeMaterialProperty& mtl				//material property
		):
		CSimuEntity(pVertex, nv, init_velocity, &mtl), 
		m_nIsStatic(isStatic)
	{
		m_bHasConstantStiffness = FEMELEMENT_t::hasConstantStiffness();
		m_bStiffnessIsMatrix33 = FEMELEMENT_t::isStiffnessMatrix3X3();

		//alloc buffer
		m_nElementCount = nhexelm;	
		m_pHexElement = new FEMELEMENT_t[m_nElementCount];
		assert(m_pHexElement!=0);

		//init cube element stiffness buffer
		{
		m_mtl.init();
		double poisson = m_mtl.getPoisson();
		double young = m_mtl.getYoung();
		Vector3d p0 = pVertex[pElement[0]];
		Vector3d p1 = pVertex[pElement[1]];
		const double edgelen = Distance(p0, p1);
		buildCubeElementStiffness(edgelen, young, poisson, m_elmStiffTemplate);
		}

		//
		m_pSurfacePoly = new int [nface*4];
		assert(m_pSurfacePoly!=NULL);
		memcpy(m_pSurfacePoly, pFace, sizeof(int)*nface*4);
		m_nSurfacePoly = nface;

		//first, need to decide mass
		const double rho = m_mtl.getDensity();
		_computeNodeMass(pVertex, pElement, nhexelm, rho);

		//then, init each tetra2 element
		const Vector8i *pquad = (const Vector8i*)pElement;
		for (int i=0; i<m_nElementCount; i++){
			FEMELEMENT_t *p = &m_pHexElement[i];
			const Vector8i& q = pquad[i];
			p->init(m_mtl, &q.x, 
				pVertex[q.x], pVertex[q.y], pVertex[q.z], pVertex[q.w],
				pVertex[q.x1], pVertex[q.y1], pVertex[q.z1], pVertex[q.w1], 
				m_elmStiffTemplate);
		}
		//printf("There are %d hexahedral elements generated.\n", m_nElementCount);
	}

	virtual ~_CSimuHexahedron(void)
	{
		SafeDeleteArray(m_pHexElement);
		SafeDeleteArray(m_pSurfacePoly);
	}

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJac=false)
	{
		//init forces as zero and add gravity load
		initForceAndApplyGravity();

		//add elastic forces
		_computeElasticForces(timeid, needJac);

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
			m_pHexElement[i].updateState();
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
		const int nSurfaceType = 4;
		return ExportMeshPLTSurface(m_pVertInfo, m_nVertexCount, 
			m_pSurfacePoly, nSurfaceType, m_nSurfacePoly, fp);
	}

	virtual int exportOBJFileObject(
		const int stepid, const int objid, const int vertexbaseid, 
		const int texbaseid, FILE *fp) const
	{
		const float *m_pTextureCoord = NULL;
		const int m_nTextureCoordStride = 2;
		const int nSurfaceType=4;
		const bool r = ExportMeshOBJSurface(stepid, objid, m_pVertInfo, m_nVertexCount, 
				m_pSurfacePoly, nSurfaceType, m_nSurfacePoly, vertexbaseid, 
				m_pTextureCoord, m_nTextureCoordStride, texbaseid, fp);
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
typedef _CSimuHexahedron<CCubeFemElement> CSimuCubeObject;

#endif
