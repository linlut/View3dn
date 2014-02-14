//FILE: obj_vertexring.cpp
#include <quaternion.h>
#include "obj_vertexring.h"
#include "zsparse_matrix.h"
#include "time_integration.h"


const int VLIST_BUFFLEN = 60;
const int VLIST_ARRAYSIZE = 60;

void CSimuVertexRingObj::_initVars(void)
{
	m_pVRingElement = NULL;
	m_nVRingElementCount = 0;

	m_pEdge = NULL;	
	m_nEdge = 0;	
	m_pGyrod = NULL;	

	//set default alg.
	m_nAlg = 0;
	m_nRotationSkipStep = 1;
}

//find an element
CGyrodElementType* CSimuVertexRingObj::findGyrodElementByNodeID(
	const int v0, const int v1, int &index, int &nv0, int &nv1)
{	
	CGyrodElementType *p=NULL;
	if (m_pEdge==NULL || m_pGyrod==NULL) 
		return p;

	for (int i=0; i<m_nEdge; i++){
		const int vx = m_pEdge[i].v0;
		const int vy = m_pEdge[i].v1;
		const bool b1 = ((vx==v0) && (vy==v1));
		const bool b2 = ((vx==v1) && (vy==v0));
		if (b1){
			p = &m_pGyrod[i];
			index = i;
			nv0 = v0, nv1=v1;
			break;
		}
		else if (b2){
			p = &m_pGyrod[i];
			index = i;
			nv0 = v1, nv1=v0;
			break;
		}
	}
	return p;
}

CSimuVertexRingObj::~CSimuVertexRingObj(void)
{
	SafeDeleteArray(m_pVRingElement);
	SafeDeleteArray(m_pEdge);
	SafeDeleteArray(m_pGyrod);		
}

static inline void _scanEdgeList(
	const CSimuEdgeInput *pedge, const int nedge, 
	int elist[VLIST_ARRAYSIZE][VLIST_BUFFLEN], int ecount[VLIST_ARRAYSIZE], 
	const int vLow, const int vHigh)
{
	const int nsize = vHigh - vLow;
	for (int i=0; i<nsize; i++) ecount[i]=0;
	for (int i=0; i<nedge; i++){
		const CSimuEdgeInput &e = pedge[i];
		int v = e.v0;
		if (v>=vLow && v<vHigh){
			const int j = v-vLow;
			int &count = ecount[j];
			elist[j][count] = i; 
			count++;
		}
		v = e.v1;
		if (v>=vLow && v<vHigh){
			const int j = v-vLow;
			int &count = ecount[j];
			elist[j][count] = i; 
			count++;
		}
	}
}

//return total mass of the center vertex, per edge volume, material
static double _compRingNodeVolumeAndMaterial(
	const CSimuEntity::VertexInfo pVertex[], const CSimuEdgeInput pEdge[], 
	const int oneRingEdge[], const int nv,
	double edgeVol[], CMeMaterialProperty *edgeMaterial[])
{
	double mass = 0.0;
	for (int i=0; i<nv; i++){
		const int eid = oneRingEdge[i];
		const CSimuEdgeInput& e = pEdge[eid];
		const Vector3d& p0 = pVertex[e.v0].m_pos;
		const Vector3d& p1 = pVertex[e.v1].m_pos;
		const double len = Distance(p0, p1);
		edgeVol[i] = len * e.area;
		edgeMaterial[i] = e.pMaterial;
		double edgemass = e.mass;
		if (edgemass==0.0)
			edgemass = edgeVol[i]*e.pMaterial->getDensity();
		mass+=edgemass;
	}
	return mass*0.50;
}

static void _elist2vlist(
	const CSimuEntity::VertexInfo pVertex[], const CSimuEdgeInput pEdge[], 
	const int vid, const int elist[], const int nedge, 
	int vlist[], Vector3d vert[])
{
	for (int i=0; i<nedge; i++){
		const int eid = elist[i];
		const int v0 = pEdge[eid].v0;
		const int v1 = pEdge[eid].v1;
		ASSERT0(v0==vid || v1==vid);
		if (v0==vid){ 
			vlist[i]=v1;
			vert[i] = pVertex[v1].m_pos;
		}
		else{
			vlist[i]=v0;
			vert[i] = pVertex[v0].m_pos;
		}
	}
}

void CSimuVertexRingObj::init(
	const CSimuEdgeInput *pEdge,			//edge array
	const int nedge, 						//# edges
	const CMeMaterialProperty& _mtl			//material pointer
	)
{
	const CSimuEntity::VertexInfo* pVertex = m_pVertInfo;
	const int nv = m_nVertexCount;
	assert(nv>1);
	m_nAlg = 0;
	m_nRotationSkipStep = 1;
	m_mtl = _mtl; 
	m_mtl.init();
	//allocate the buffer and init
	m_pVRingElement = new CVertexRingElement[nv];
	assert(m_pVRingElement!=NULL);
	int elist[VLIST_ARRAYSIZE][VLIST_BUFFLEN], ecount[VLIST_ARRAYSIZE];
	int nloop = nv / VLIST_ARRAYSIZE; 
	if ((nv % VLIST_ARRAYSIZE) != 0) nloop++;
	if (nloop<1) nloop=1;

	m_nVRingElementCount = 0;
	int nTotalRod = 0;
	for (int n=0; n<nloop; n++){
		//build edge list for a subset of nodes;
		const int vLow = n*VLIST_ARRAYSIZE;
		int vHigh = vLow+VLIST_ARRAYSIZE;
		if (vHigh>nv) vHigh = nv;
		_scanEdgeList(pEdge, nedge, elist, ecount, vLow, vHigh);
		//use the edge list
		const int nsize = vHigh - vLow;
		for (int i=0; i<nsize; i++){
			//prepare all the information to build a vertex ring element
			const int vid = i+vLow;
			const Vector3d& pos = pVertex[vid].m_pos;
			const int nRod = ecount[i]; 
			nTotalRod += nRod;
			//edge volume and edge material
			double edgeVol[VLIST_BUFFLEN]; 
			CMeMaterialPropertyPtr edgeMaterial[VLIST_BUFFLEN];
			double mass = _compRingNodeVolumeAndMaterial(pVertex, pEdge, elist[i], nRod, edgeVol, edgeMaterial);
			//assign nodal mass
			if (mass == 0) mass=1.0E3;
			m_pVertInfo[vid].m_mass = mass;
			//build rod element
			if (nRod>=2){
				//edge node ids
				int pRingNodeID[VLIST_BUFFLEN];
				Vector3d pRingNodePos[VLIST_BUFFLEN]; 
				_elist2vlist(pVertex, pEdge, vid, elist[i], nRod, pRingNodeID, pRingNodePos);
				//init one ring neighbours
				CVertexRingElement &e = m_pVRingElement[m_nVRingElementCount];
				e.init(vid, pos, nRod, pRingNodeID, edgeMaterial, pRingNodePos, edgeVol, m_mgr);
				m_nVRingElementCount++;
			}
		}
	}
	//printf("Vertexring element count is %d.\n", m_nVRingElementCount);
	//printf("Avg 1-Ring rod# per vertex is %lg.\n", double(nTotalRod)/m_nVRingElementCount);

	//copy all the edges
	m_pEdge = new CSimuEdgeInput[nedge];
	assert(m_pEdge!=NULL);
	for (int i=0; i<nedge; i++) m_pEdge[i] = pEdge[i];
	m_nEdge = nedge;
	//------------------
	double3x3 mat; mat.setIdentityMatrix();
	m_pGyrod = new CGyrodElementType[m_nEdge];
	assert(m_pGyrod!=NULL);
	const double SS = 3.30; 
	for (int i=0; i<nedge; i++){
		const int v0 = m_pEdge[i].v0;
		const int v1 = m_pEdge[i].v1;
		const Vector3d p0 = pVertex[v0].m_pos;
		const Vector3d p1 = pVertex[v1].m_pos;
		const double vol=Distance(p0, p1)*m_pEdge[i].area * SS;
		m_pGyrod[i].init(m_mtl, p0, p1, mat, vol);
	}
}


void CSimuVertexRingObj::_computeElasticForces(
	const unsigned int timeid, const bool isStatic, const bool needjacobian)
{
	const int BUFF_LENGTH = 80;
	Vector3d force[BUFF_LENGTH], *force1= &force[1];	//force[0] is the sumed force for center node
	double3x3 jacobian[BUFF_LENGTH], *jacobian1=&jacobian[1], *pjac[BUFF_LENGTH];
	int pNodeBuffer[BUFF_LENGTH];
	if (needjacobian)
		for (int i=0; i<BUFF_LENGTH; i++) pjac[i] = &jacobian[i];
	else
		for (int i=0; i<BUFF_LENGTH; i++) pjac[i] = NULL;

	const bool FASTSTIFF = (this->m_mtl.getMaterialType()==0);
	CSparseMatrix33 *pSparseMat = this->getOdeIntegrator()->getSparseMatrix();
	if (pSparseMat){
		int datatype = 1;
		if (FASTSTIFF) datatype=0;
		pSparseMat->setMatrixElementType(datatype);
	}
	const int compRot = ((timeid%m_nRotationSkipStep) == 0);
	const int stride = sizeof(CSimuEntity::VertexInfo);
	for (int i=0; i<m_nVRingElementCount; i++){
		CVertexRingElement &e = m_pVRingElement[i];
		e.computeForce(compRot, isStatic, &m_pVertInfo[0].m_pos, stride, &m_mtl, force, pjac);
		//add center node force
		const int v0 = e.getCenterVertexID();
		m_pVertInfo[v0].m_force += force[0];
		//add neighbor nodes forces
		const int nv = e.getRodNumber();
		e.getElementNodeBuffer(pNodeBuffer);
		for (int j=0; j<nv; j++) 
			m_pVertInfo[pNodeBuffer[j]].m_force += force1[j];
		//add stiffness, note the storage specification!!
		if (needjacobian)
			saveVertSitffMatrixIntoSparseMatrix(v0, pNodeBuffer, jacobian1, nv, FASTSTIFF); 
	}
}

void CSimuVertexRingObj::updateRotationQuaternionForAllElements(const unsigned int tm, const bool needQuat)
{
	//compute the rotation for each vertex
	const int BUFFERLEN = 20000;
	int failbuffer[BUFFERLEN];
	int i, c = 0;
	assert(m_nVRingElementCount==m_nVertexCount);
	const Vector3d *pVertex = &m_pVertInfo[0].m_pos;
	const int stride = sizeof(CSimuEntity::VertexInfo);
	for (i=0; i<m_nVRingElementCount; i++){
		CVertexRingElement& elm = m_pVRingElement[i];
		if (!elm.computeRotationQuaternionSolidOrShell(0, pVertex, stride)){
			failbuffer[c++] = i;
			elm.m_rotTime = 0;
		}
		else{
			elm.m_rotTime = tm;
		}
	}
	//make up those failed vertices;
	assert(c<BUFFERLEN);
	for (i=0; i<c; i++){
		const int vid = failbuffer[i];
		CVertexRingElement& elm = m_pVRingElement[vid];
		Vector4d q(0,0,0,0);
		int count = 0;
		for (int j=0; j<elm.getRodNumber(); j++){
			const int v = elm.m_pVertexRingNode[j].m_nVertexID; 
			CVertexRingElement& elm1 = m_pVRingElement[v];
			const unsigned int tm1 = elm1.m_rotTime;
			if (tm1 == tm){
				q += elm1.m_quat;
				count ++;
			}
		}
		if (count>0){
			Quaternion qq(q[0], q[1], q[2], q[3]);
			qq.normalize();
			elm.m_quat = Vector4d(qq[0], qq[1], qq[2], qq[3]);
		}
	}
}

void CSimuVertexRingObj::_computeElasticForcesNew(
	const unsigned int timeid, const bool isStatic, const bool needjacobian)
{
	//compute the rotation for each vertex
	const bool bRecompRot = (timeid%2 == 1);
	if (timeid<=1 || bRecompRot){ 
		const bool needquat = false;
		updateRotationQuaternionForAllElements(timeid, needquat);
	}
	else{
		//use central difference to update the quaternions
		for (int i=0; i<m_nVRingElementCount; i++){
			CVertexRingElement &e = m_pVRingElement[i];
			Vector4d quat = e.m_quat + e.m_quat - e.m_quat0;
			quat.normalize();
			e.m_quat0 = e.m_quat;
			e.m_quat = quat;
			//convert quat to matrix 
			Quaternion *pquat = (Quaternion*)&e.m_quat.x;
			typedef double M33[3][3];
			pquat->getRotationMatrix(*((M33*)&e.m_R.x));
		}
	}

	//call the old alg.
	const int SKIPSTEP = m_nRotationSkipStep;
	m_nRotationSkipStep = INT_MAX-1;
	_computeElasticForces(timeid, isStatic, needjacobian);
	m_nRotationSkipStep = SKIPSTEP;

	/*
	//compute forces using the rotation
	Vector3d force;
	double3x3 jacobian, *pjac=NULL;
	const bool FASTSTIFF = (this->m_mtl.getMaterialType()==0);
	if (needjacobian) pjac = &jacobian;
	for (int i=0; i<m_nEdge; i++){
		CSimuEdgeInput& edge = m_pEdge[i];
		const int v0=edge.v0, v1=edge.v1;
		if (bRecompRot){
			//computer averaged rotation of the rod, then save it
			const Quaternion *q0 = (const Quaternion *)(&m_pVRingElement[v0].m_quat);
			const Quaternion *q1 = (const Quaternion *)(&m_pVRingElement[v1].m_quat);
			const Quaternion q = Quaternion::slerp_midpoint(*q0, *q1);
			q.getRotationMatrix(*((double(*)[3][3])(&edge.mat)));
		}
		//apply the rotation for the rod element
		const Vector3d& p0 = m_pVertInfo[v0].m_pos;
		const Vector3d& p1 = m_pVertInfo[v1].m_pos;
		m_pGyrod[i].computeNodalForce(p0, p1, edge.mat, *edge.pMaterial, force, pjac);
		//accumulate the force and stiffness
		m_pVertInfo[v0].m_force+=force;
		m_pVertInfo[v1].m_force-=force;
		if (needjacobian) 
			saveVertSitffMatrixIntoSparseMatrix(v0, v1, *pjac, FASTSTIFF);
	}
	*/
}

//compute the acceleration vector for the current location 
void CSimuVertexRingObj::computeAcceleration(const unsigned int timeid, const bool needJacobian)
{
	//init forces as zero and add gravity load
	initForceAndApplyGravity();

	//add elastic forces
	if (timeid==0) printf("Algorithm selection is %d\n", m_nAlg);
	const bool isStatic = false;
	//if (m_nAlg == 1)
	_computeElasticForcesNew(timeid, isStatic, needJacobian);
	//else
	//_computeElasticForces(timeid, isStatic, needJacobian);

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


void CSimuVertexRingObj::computeStiffnessMatrix(void)
{
	CSparseMatrix33 *pmat = this->getOdeIntegrator()->getSparseMatrix();
	if (pmat==NULL) return;
	pmat->clear();	

	const bool needJacobian = true;
	computeAcceleration(1, needJacobian);
}


void CSimuVertexRingObj::exportMesh(const int count, const char *fname, const char *ext)
{
	char ffname[200];
	sprintf(ffname, "shell%d.plt", count);
	
	FILE *fp = fopen(ffname, _WA_);
	if (fp==NULL) return;
	exportMeshPlt(fp);
	fclose(fp);
}


bool CSimuVertexRingObj::exportMeshPlt(FILE *fp)
{
	int i;
	char PLYTYPESTR[16]="LINE";
	const int nelm = this->getRodCount();
	//vertex array
	fprintf(fp, "TITLE = SHELLSIMULATION\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", m_nVertexCount, nelm, PLYTYPESTR);
	for (i=0; i<m_nVertexCount; i++){
        const Vector3d& p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%lg %lg %lg\n", p.x, p.y, p.z);
    }
	//mesh connectivity
	int nodebuffer[100];
    for (i=0; i<m_nVRingElementCount; i++){
		CVertexRingElement & elm = m_pVRingElement[i];
		const int nrod = elm.getRodNumber();
		int x = elm.getCenterVertexID(); x++;
		elm.getElementNodeBuffer(nodebuffer);
		for (int j=0; j<nrod; j++){
			int y = nodebuffer[j];
			y++;
			fprintf(fp, "%d %d\n", x, y);
		}
	}

	return true;
}


void CSimuVertexRingObj::computeStiffnessSymbolic(CSparseMatrix33 *l, CSparseMatrix33 *m)
{
	//init
	const int nv = m_nVRingElementCount; 
	l->init(nv); 
	m->init(nv);
		
	//compute and save
	const int BUFF_LENGTH=120;
	int pNodeBuffer[BUFF_LENGTH];
	double3x3 jacobianLambda[BUFF_LENGTH], jacobianMu[BUFF_LENGTH];
	for (int i=0; i<nv; i++){
		CVertexRingElement &e = m_pVRingElement[i];
		e.computeJacobianSymbolic(&m_mtl, jacobianLambda, jacobianMu);
		//add stiffness, note the storage specification!!
		const int v0 = e.getCenterVertexID();
		const int nrod = e.getRodNumber();
		e.getElementNodeBuffer(pNodeBuffer);
		saveVertSitffMatrixIntoSparseMatrix(l, v0, pNodeBuffer, jacobianLambda, nrod, false); 
		saveVertSitffMatrixIntoSparseMatrix(m, v0, pNodeBuffer, jacobianMu, nrod, false); 
	}
}
