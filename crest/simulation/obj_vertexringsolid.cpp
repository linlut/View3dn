//FILE: obj_vertexringsolid.cpp
#include <vector>
#include <geomath.h>
#include <view3dn/edgetable.h>
#include "obj_vertexringsolid.h"


void CSimuVertexRingSolid::_initClassVars(void)
{
	m_nElementType = 0;
	m_pElement = NULL;

	m_nSurfaceType = 0;	
	m_nSurfacePoly = 0;
	m_pSurfacePoly = NULL;	
}

CSimuVertexRingSolid::~CSimuVertexRingSolid(void)
{
	SafeDeleteArray(m_pElement);
	SafeDeleteArray(m_pSurfacePoly);
}

void CSimuVertexRingSolid::_computeNodeMass(
	const Vector3d *pVertex, 
	const int *pElement, const int nv_per_elm, const int nelm, 
	const double &rho)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	if (nv_per_elm==4){
		//Tetrahedral mesh
		ComputeNodeMassTet(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
	}
	else{
		//Hexahedral mesh
		ComputeNodeMassHex(pVertex, m_nVertexCount, pElement, nelm, rho, m_pVertInfo);
	}
}


void ConvertEdgeMapToEdgeInput(
	const Vector3d *pVertex, const CMeMaterialProperty *pmat, CEdgeMap& edgemap, //input
	vector<CSimuEdgeInput> &edgeinput)		//output
{
	edgeinput.clear();
	CEdgeMap::iterator itr = edgemap.begin();
	while (itr!=edgemap.end()){
		const CSpringAuxStruct &e = itr->second;
		const int v0 = e.m_vertID[0];
		const int v1 = e.m_vertID[1];
		const double area = e.m_crossarea;
		const double mass = e.m_mass;
		CSimuEdgeInput x(v0, v1, area, mass, pmat);
		edgeinput.push_back(x);
		++itr;
	}
}

void CSimuVertexRingSolid::_init(
	const Vector3d *pVertex,				//vertex buffer
	const int springtype,					//methods: 0: van Galder, 1: Lloyd 08TVCG
	const int *pElement,					//element buffer
	const int nv_per_elm,					//vertex # in each element, tet=3, hexahedron=4
	const int nelm,							//element information
	const CMeMaterialProperty &_mtl
	)
{
	_initClassVars();

	//init element buffer
	m_nElementType = nv_per_elm;
	const int nsize = nv_per_elm * nelm;
	m_pElement = new int[nsize];
	assert(m_pElement!=NULL);
	for (int i=0; i<nsize; i++) m_pElement[i] = pElement[i];

	//first, need to decide mass
	m_mtl = _mtl; m_mtl.init();
	const double rho = m_mtl.getDensity();
	_computeNodeMass(pVertex, pElement, nv_per_elm, nelm, rho);

	//create strings using different tech to assign the stiffness ratio
	assert(m_nElementType==4 || m_nElementType==8);
	CEdgeMap edgemap;
	buildEdgeMapFromSolidElements(springtype, pVertex, m_nElementType, pElement, nelm, rho, edgemap);

	//construct the input buffer 
	vector<CSimuEdgeInput> edgeinput;
	ConvertEdgeMapToEdgeInput(pVertex, &m_mtl, edgemap, edgeinput);

	//continue to init the vertexring obj
	const CSimuEdgeInput *pedge = &edgeinput[0];
	const int nedge = edgeinput.size();
	CSimuVertexRingObj::init(pedge, nedge, m_mtl);
}

void CSimuVertexRingSolid::setBoundarySurface(
	const int* pSurfacePoly,					//surface polygon array
	const int nSurfaceType,						//=3 or 4, number of nodes per surface element
	const int nSurfacePoly)						//# of surface polygon
{
	SafeDeleteArray(m_pSurfacePoly);
	const int nsize = nSurfaceType * nSurfacePoly;
	m_pSurfacePoly = new int [nsize];
	assert(m_pSurfacePoly!=NULL);
	for (int i=0; i<nsize; i++) m_pSurfacePoly[i]=pSurfacePoly[i];
	m_nSurfaceType = nSurfaceType;
	m_nSurfacePoly = nSurfacePoly;
}

//export mesh
bool CSimuVertexRingSolid::_exportMeshPLT(FILE *fp)
{
	return ExportMeshPLTSurface(m_pVertInfo, m_nVertexCount, 
		m_pSurfacePoly, m_nSurfaceType, m_nSurfacePoly, fp);
}


int CSimuVertexRingSolid::exportOBJFileObject(
	const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	const float *m_pTextureCoord = NULL;
	const int m_nTextureCoordStride = 2;
	const bool r = ExportMeshOBJSurface(stepid, objid, m_pVertInfo, m_nVertexCount, 
			m_pSurfacePoly, m_nSurfaceType, m_nSurfacePoly, vertexbaseid, m_pTextureCoord, m_nTextureCoordStride, texbaseid, fp);
	return m_nVertexCount;
}
