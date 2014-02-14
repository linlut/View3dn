//FILE: simu_staticrod.h
#ifndef _INC_SIMUSTATICROD_ZN2008_H
#define _INC_SIMUSTATICROD_ZN2008_H

#include <assert.h>
#include <vectorall.h>
#include "thinshell_element.h"
#include "simu_entity.h"
#include "element_gyrod.h"


void ComputeNodeMassForRods(const Vector3d *pVertex, const int nv,
	const Vector2i *pElement, const int nelm, 
	const double &crossarea, const double &rho, 
	CSimuEntity::VertexInfo *m_pVertInfo);


template <class T>
class CSimuStaticRodT: public CSimuEntity
{
protected:
	T *m_pShearElement;			//shear element array
	Vector2i *m_pRodVertexID;	//vertex ids for each rod

private:
	//compute elastic forces
	void _computeElasticForces(const bool needjacobian)
	{
		Vector3d force;
		double3x3 jacobian;
		double3x3 *ppjac = &jacobian;
		if (!needjacobian) ppjac = NULL;

		//for shell elements made from triangle mesh
		for (int i=0; i<m_nElementCount; i++){
			T &E = m_pShearElement[i];
			const Vector2i ROD = m_pRodVertexID[i];
			const int i0 = ROD.x;
			const int i1 = ROD.y;
			const Vector3d& p0 = m_pVertInfo[i0].m_pos;
			const Vector3d& p1 = m_pVertInfo[i1].m_pos;
			E.computeNodalForce(p0, p1, m_mtl, force, ppjac);
			m_pVertInfo[i0].m_force += force;
			m_pVertInfo[i1].m_force += -force;
			if (needjacobian)
				saveVertSitffMatrixIntoSparseMatrix(i0, i1, *ppjac);
		}
	}

	//compute per node mass in initialization
	void _computeNodeMass(const Vector3d *pVertex, const Vector2i *pElement, const int nelm, const double &crossarea, const double &rho)
	{
		ASSERT0(m_pVertInfo!=NULL);
		ComputeNodeMassForRods(pVertex, m_nVertexCount, pElement, nelm, crossarea, rho, m_pVertInfo);
	}

	bool _exportMeshPLT(FILE *fp)
	{
		int i;
		char PLYTYPESTR[16]={"LINE"};

		//vertex array
		const double rad = 1e-3;
		const int nv = m_nVertexCount, npoly = m_nElementCount;
		fprintf(fp, "TITLE = SHELLSIMULATION\n");
		fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\",\"TR3\"\n");
		fprintf(fp, "ZONE N = %d  E = %d  F = FEPOINT  ET = %s  RAD =%g\n", nv, npoly, PLYTYPESTR, rad);
		//compute TR3
		double *pVar = new double[nv];
		assert(pVar!=NULL);
		for (i=0; i<nv; i++) pVar[i] = 0;
		for (i=0; i<npoly; i++){
			const double3x3 *F = m_pShearElement[i].getDeformationGradient();
			double I1 = 0;
			if (F) I1 = F->computeI1();
			const Vector2i& ll = m_pRodVertexID[i];
			const int x = ll.x; const int y = ll.y;
			if (fabs(pVar[x])<I1) pVar[x]=I1;
			if (fabs(pVar[y])<I1) pVar[y]=I1;			
		}

		for (i=0; i<nv; i++){
			const Vector3d& p= m_pVertInfo[i].m_pos;
			fprintf(fp, "%.10lG %.10lG %.10lG\n", p.x, p.y, p.z);
			fprintf(fp, "%.6lG\n", pVar[i]);
		}
		//mesh connectivity
		for (i=0; i<npoly; i++){
			const Vector2i& ll = m_pRodVertexID[i];
			int x = ll.x; int y = ll.y;
			x++; y++; 
			fprintf(fp, "%d %d\n", x, y);
		}
		delete [] pVar;
		return 1;
	}

public:
	CSimuStaticRodT(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int nelm,		//element information
		const double &rod_crossarea,				//cross section area
		const CMeMaterialProperty& mtl				//material property
	 )
	 :CSimuEntity(pVertex, nv, init_velocity, &mtl)
	{
		//first, need to decide mass
		const double rho = mtl.getDensity();
		_computeNodeMass(pVertex, (const Vector2i*)pElement, nelm, rod_crossarea, rho);

		//the shear element count is the same as the input element count
		m_nElementCount = nelm;
		m_pRodVertexID = new Vector2i[m_nElementCount];
		assert(m_pRodVertexID!=NULL);
		memcpy(m_pRodVertexID, pElement, sizeof(Vector2i)*m_nElementCount);
		m_pShearElement = new T[m_nElementCount];
		assert(m_pShearElement!=0);

		//init the shear elements
		double3x3 rot; 
		rot.setIdentityMatrix();
		for (int i=0; i<m_nElementCount; i++){
			const Vector2i ll = m_pRodVertexID[i];
			T &e = m_pShearElement[i];
			const Vector3d& p0 = pVertex[ll.x];
			const Vector3d& p1 = pVertex[ll.y];
			const double vol = Distance(p0, p1)*rod_crossarea;
			e.init(mtl, p0, p1, rot, vol);
		}
		printf("Vertex#: %d, polygon#: %d, shell element#:%d.\n", nv, nelm, nelm);
	}

	virtual ~CSimuStaticRodT(void)
	{
		SafeDeleteArray(m_pShearElement);
		SafeDeleteArray(m_pRodVertexID);
	}

	void exportElementState(const int i, const CMeMaterialProperty &material, FILE *fp) const
	{
		if (!m_pShearElement) return;
		m_pShearElement[i].exportState(material, fp);		
	}

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool needJacobian=false)
	{
		//init forces as zero and add gravity load
		initForceAndApplyGravity();

		//add elastic forces
		_computeElasticForces(needJacobian);

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
			//m_pShearElement[i].updateState();
		}
	}

	virtual void exportMesh(const int count, const char *name, const char *ext)
	{
		char fname[200];
		sprintf(fname, "ROD%d.plt", count);
		FILE *fp = fopen(fname, _WA_);
		if (fp==NULL) return;
		_exportMeshPLT(fp);
		fclose(fp);
	}

	virtual bool exportMeshPlt(FILE *fp)
	{
		bool r = _exportMeshPLT(fp);
		return r;
	}

	virtual void computeStiffnessMatrix(void)
	{
		assert(0);
	}

};

typedef CSimuStaticRodT <CShearElement> CSimuStaticRod;
typedef CSimuStaticRodT <CGspringElement> CSimuStaticSpring;
typedef CSimuStaticRodT <CGyrodElement> CSimuStaticGyrod;


#endif