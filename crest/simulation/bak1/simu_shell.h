//FILE: simu_shell.h
#ifndef _INC_SIMU_SHELL_ZN20080912_H
#define _INC_SIMU_SHELL_ZN20080912_H

#include <assert.h>
#include <vectorall.h>
#include "simu_entity.h"


class CSimuShell: public CSimuEntity
{
protected:
	//===============================================================================
	class CRelaxationEdge
	{
	public:
		int x, y;
		double restlength2; 
	};

	//=================================================================================
	int m_static;							//static/dynamic simulation, def. 0: dynamic simu
	int m_nSurfaceType;						//=3 or 4, number of nodes per surface element
	int m_nSurfacePoly;						//# of surface polygon
	int *m_pSurfacePoly;					//surface polygon array
	Vector3d *m_pSurfaceNorm;				//surface polygon normal array
	double m_thickness;						//Shell thickness
	//=================================================================================
	//relaxation is a method for handling the super-elasticity problem
	//also, it is very helpful to stablize the simulation. 
	//this technique is widely used in cloth simualation, where cloth should be 
	//inextensible (should not be stretched too much).
	CRelaxationEdge *m_pRelaxationEdge;		//array of edges for relaxation
	int m_nRelaxationEdge;					//# of edges for relaxation

private:
	bool _exportMeshPLT(FILE *fp);

	bool _exportMeshTXT(const char *fname);

	void _performEdgeRelaxationOneIterationUp(const double &DLEN_TOL, const double &KK);
	void _performEdgeRelaxationOneIterationDown(const double &DLEN_TOL, const double &KK);
	int _exportOBJFileObjectQuad(const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;
	int _exportOBJFileObjectTri(const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;

public:
	CSimuShell(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int nv_per_elm, const int nelm,	//element information
		const double &shell_thickness,
		const CMeMaterialProperty& mtl,				//material property
		const int isStatic=0						//static or dynamic simulation
		);

	virtual ~CSimuShell(void)
	{
		SafeDeleteArray(m_pSurfacePoly);
		SafeDeleteArray(m_pSurfaceNorm);
		SafeDeleteArray(m_pRelaxationEdge);
	}

	virtual void exportMesh(const int count, const char *name, const char *ext);

	virtual bool exportMeshPlt(FILE *fp);

	//current_tm: current physical time (in ms.)
	//dt: the time step length
	virtual void performEdgeRelaxation(const double &current_tm, const double &dt);

protected:
	//compute per node mass in initialization
	void computeNodeMass(const Vector3d *pVertex, const double &rho);

	void computeSurfaceNormal(const Vector3d *pVertex, const int stride);

	void computeSurfaceAreaWeightedNormal(const Vector3d *pVertex, const int stride);

	void buildRelaxationEdges(const Vector3d *pVertex, const int rigidlevel);

	virtual int exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const;

};


void ComputeNodeMassForSurface(const Vector3d *pVertex, const int nv,
	const int *pElement, const int nv_per_elm, const int nelm, 
	const double &thickness, const double &rho, 
	CSimuEntity::VertexInfo *m_pVertInfo);


#endif