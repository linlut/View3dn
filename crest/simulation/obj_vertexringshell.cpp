//FILE: obj_vertexringsolid.cpp
#include <vector>
#include <geomath.h>
#include <view3dn/edgetable.h>
#include "simu_shell.h"
#include "obj_vertexringsolid.h"
#include "obj_vertexringshell.h"


CSimuVertexRingShell::~CSimuVertexRingShell(void)
{
	SafeDeleteArray(m_pElement);
}

void CSimuVertexRingShell::_computeNodeMass(
	const Vector3d *pVertex, 
	const int *pElement, const int nv_per_elm, const int nelm, 
	const double &thickness, const double &rho)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	ComputeNodeMassForSurface(pVertex, m_nVertexCount, pElement, nv_per_elm, nelm, thickness, rho, m_pVertInfo);
}

void CSimuVertexRingShell::_init(
	const Vector3d *pVertex,				//vertex buffer
	const int springtype,					//methods: 0: Van Galder, 1: Lloyd 2008 TVCG, default=1
	const int *pElement,					//element buffer
	const int nv_per_elm,					//vertex # in each element, tet=3, hexahedron=4
	const int nelm,							//element information
	const double& thickness,				//shell thickness
	const CMeMaterialProperty &_mtl
	)
{
	int  i;

	//i/o format conversion
	m_mtl = _mtl;
	_initClassVars();

	//init element buffer
	m_nSurfaceElement = nelm;
	m_nElementType = nv_per_elm;
	const int nsize = nv_per_elm * nelm;
	m_pElement = new int[nsize];
	assert(m_pElement!=NULL);
	for (i=0; i<nsize; i++) m_pElement[i] = pElement[i];

	//determine nodal masses first
	const double rho = m_mtl.getDensity();
	_computeNodeMass(pVertex, pElement, nv_per_elm, nelm, thickness, rho);

	//create strings using different tech to assign the stiffness ratio
	CEdgeMap edgemap;
	assert(m_nElementType==3 || m_nElementType==4);
	buildEdgeMapFromSurfaceElements(springtype, pVertex, m_nElementType, pElement, nelm, thickness, rho, edgemap);

	//construct the input buffer and continue to init the vertexring obj
	vector<CSimuEdgeInput> edgeinput;
	ConvertEdgeMapToEdgeInput(pVertex, &m_mtl, edgemap, edgeinput);
	const CSimuEdgeInput *pedge = &edgeinput[0];
	const unsigned int nedge = edgeinput.size();
	CSimuVertexRingObj::init(pedge, nedge, m_mtl);

	//check nodes with which are only degree two, then fix them into degree 3
	this->setAlgorithm(0);
	assert(m_nVertexCount==m_nVRingElementCount);
	for (i=0; i<m_nVRingElementCount; i++){
		CVertexRingElement* pelm = &m_pVRingElement[i];
		if (pelm->getRodNumber() != 2) continue;
		//================================================
		int buffer[100];
		Vector2i nbrID;
		pelm->getElementNodeBuffer(&nbrID.x);
		CVertexRingElement *pNbr0 = &m_pVRingElement[nbrID.x];
		CVertexRingElement *pNbr1 = &m_pVRingElement[nbrID.y];
		const int count = pNbr0->findCommonNeighbors(*pNbr1, i, buffer);
		if (count>0){
			//add one vertex as neighbor
			pelm->addOneNeighorNodeInCaseOnlyTwoNeighbors(pVertex, buffer[0], m_mgr);
			//more importantly, we need to change the initialization of the two rods
			int v0, v1, index;
			CGyrodElementType* pgyrod = findGyrodElementByNodeID(i, nbrID.x, index, v0, v1);
			assert(pgyrod!=NULL);
			double vol = pgyrod->getVolume();
			*pgyrod = CGyrodElementType(m_mtl, pVertex[v0], pVertex[v1], vol);
			pgyrod = findGyrodElementByNodeID(i, nbrID.y, index, v0, v1);
			assert(pgyrod!=NULL);
			vol = pgyrod->getVolume();
			*pgyrod = CGyrodElementType(m_mtl, pVertex[v0], pVertex[v1], vol);
		}
		else{
			printf("Find node_%d with only two neighbors, but unable to fix!\n", i);
		}
	}
}


//export mesh
bool CSimuVertexRingShell::_exportMeshPLT(FILE *fp)
{
	return ExportMeshPLTSurface(m_pVertInfo, 
		m_nVertexCount, m_pElement, m_nElementType, m_nSurfaceElement, fp);
}


void CSimuVertexRingShell::updateRotationQuaternionForAllElements(const unsigned int tm, const bool needQuat)
{
	assert(m_nVRingElementCount==m_nVertexCount);
	Vector3d N, ZEROVEC(0,0,0);
	int i;

	for (i=0; i<m_nVertexCount; i++) m_pVRingElement[i].m_normal = ZEROVEC;
	if (m_nElementType==3){
		const Vector3i *ptri = (const Vector3i*) m_pElement;	
		for (i=0; i<m_nSurfaceElement; i++){
			const Vector3i& tri = ptri[i];
			const Vector3d p0 = m_pVertInfo[tri.x].m_pos;
			const Vector3d p1 = m_pVertInfo[tri.y].m_pos;
			const Vector3d p2 = m_pVertInfo[tri.z].m_pos;
			compute_triangle_areanormal(p0, p1, p2, N);
			N.normalize();
			m_pVRingElement[tri.x].m_normal += N;
			m_pVRingElement[tri.y].m_normal += N;
			m_pVRingElement[tri.z].m_normal += N;
		}
	}
	else{
		const Vector4i *ptri = (const Vector4i*) m_pElement;	
		for (i=0; i<m_nSurfaceElement; i++){
			const Vector4i& tri = ptri[i];
			const Vector3d p0 = m_pVertInfo[tri.x].m_pos;
			const Vector3d p1 = m_pVertInfo[tri.y].m_pos;
			const Vector3d p2 = m_pVertInfo[tri.z].m_pos;
			const Vector3d p3 = m_pVertInfo[tri.w].m_pos;
			compute_quad_areanormal(p0, p1, p2, p3, N);
			N.normalize();
			m_pVRingElement[tri.x].m_normal += N;
			m_pVRingElement[tri.y].m_normal += N;
			m_pVRingElement[tri.z].m_normal += N;
			m_pVRingElement[tri.w].m_normal += N;
		}
	}
	for (i=0; i<m_nVertexCount; i++) 
		m_pVRingElement[i].m_normal.normalize();

	//compute rotation, with possible reflection removal
	const Vector3d *pVertex = &m_pVertInfo[0].m_pos;
	const int stride = sizeof(CSimuEntity::VertexInfo);
	for (i=0; i<m_nVRingElementCount; i++){
		CVertexRingElement& elm = m_pVRingElement[i];
		elm.computeRotationQuaternionSolidOrShell(1, pVertex, stride);
	}

}

int CSimuVertexRingShell::exportOBJFileObject(const int stepid, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	const float *m_pTextureCoord = NULL;
	const int m_nTextureCoordStride = 2;
	const int m_nSurfaceType = m_nElementType;	
	const int *m_pSurfacePoly = m_pElement;
	const int m_nSurfacePoly = m_nSurfaceElement;
	const bool r = ExportMeshOBJSurface(stepid, objid, m_pVertInfo, m_nVertexCount, 
			m_pSurfacePoly, m_nSurfaceType, m_nSurfacePoly, vertexbaseid, m_pTextureCoord, m_nTextureCoordStride, texbaseid, fp);
	return m_nVertexCount;
}
