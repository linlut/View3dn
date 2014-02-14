//FILE: simu_entity.cpp
#include <memory.h>
#include <sysconf.h>
#include <geomath.h>
#include "simu_entity.h"
#include "zsparse_matrix.h"
#include "constraints.h"


void CImplicitSolverData::update(const int nv)
{
	if (m_nv<nv){
		SafeDeleteArray(m_pVecBuffer0);
		m_pVecBuffer0 = new Vector3d[nv*2];
		m_pVecBuffer1 = &m_pVecBuffer0[nv];
	}
	//allocate buffer
	if (m_pSparseMat33==NULL){
		m_pSparseMat33 = new CSparseMatrix33(nv);
		assert(m_pSparseMat33!=NULL);
	}
	else
		m_pSparseMat33->init(nv);

	//copy
	m_nv = nv;
}

CImplicitSolverData::~CImplicitSolverData(void)
{
	SafeDeleteArray(m_pVecBuffer0);
	SafeDelete(m_pSparseMat33);
}


//================================================================

void ComputeNodeMassTet(const Vector3d *pVertex, const int m_nVertexCount, 
		const int *pElement, const int nelm, 
		const double &rho, CSimuEntity::VertexInfo *m_pVertInfo)
{
	int i;
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	//First, init node mass as zero
	for (i=0; i<m_nVertexCount; i++) m_pVertInfo[i].m_mass = 0;
	//loop the elements;
	const Vector4i *pquad = (const Vector4i *)pElement;
	const double K = rho * 0.25;
	for (i=0; i<nelm; i++){
		const Vector4i quad = pquad[i];
		const Vector3d& p0 = pVertex[quad.x];
		const Vector3d& p1 = pVertex[quad.y];
		const Vector3d& p2 = pVertex[quad.z];
		const Vector3d& p3 = pVertex[quad.w];
		const double vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double mass = vol * K;
		m_pVertInfo[quad.x].m_mass += mass;
		m_pVertInfo[quad.y].m_mass += mass;
		m_pVertInfo[quad.z].m_mass += mass;
		m_pVertInfo[quad.w].m_mass += mass;
	}
}

void ComputeNodeMassHex(const Vector3d *pVertex, const int m_nVertexCount, 
		const int *pElement, const int nelm, 
		const double &rho, CSimuEntity::VertexInfo *m_pVertInfo)
{
	int i;
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	//First, init node mass as zero
	for (i=0; i<m_nVertexCount; i++) m_pVertInfo[i].m_mass = 0;
	//loop the elements;
	const Vector8i *pquad = (const Vector8i *)pElement;
	const double K = rho * 0.125;
	for (i=0; i<nelm; i++){
		const Vector8i quad = pquad[i];
		const double vol = computeHexhedronVolume(pVertex, quad);
		const double mass = vol * K;
		m_pVertInfo[quad.x].m_mass += mass;
		m_pVertInfo[quad.y].m_mass += mass;
		m_pVertInfo[quad.z].m_mass += mass;
		m_pVertInfo[quad.w].m_mass += mass;
		m_pVertInfo[quad.x1].m_mass += mass;
		m_pVertInfo[quad.y1].m_mass += mass;
		m_pVertInfo[quad.z1].m_mass += mass;
		m_pVertInfo[quad.w1].m_mass += mass;
	}
}


CSimuEntity::~CSimuEntity(void)
{
	SafeDeleteArray(m_pVertInfo);
	SafeDelete(m_pImplicitSolverData);

	for (int i=0; i<CONSTRAINT_BUFF_LENGTH; i++) 
		SafeDelete(m_pConstraints[i]);
	SafeDelete(m_pCollisionConstraint);
}


void CSimuEntity::initForceAndApplyGravity(void)
{
	int i;
	Vector3d zero = Vector3d(0,0,0);

	//init with gravity force
	if (m_gravity_acc==zero){
		for (i=0; i<m_nVertexCount; i++){
			m_pVertInfo[i].m_force = zero;
			m_pVertInfo[i].m_acc = zero;
		}
	}
	else{
		for (i=0; i<m_nVertexCount; i++){
			VertexInfo *pnode = &m_pVertInfo[i];
			pnode->m_force = m_gravity_acc * pnode->m_mass;
			pnode->m_acc = m_gravity_acc;
		}
	}

	//velocity damping
	/*
	double dampv = m_mtl.getVelocityDamping();
	if (dampv>1e-12){
		dampv = -dampv;
		for (i=0; i<m_nVertexCount; i++){
			VertexInfo *pnode = &m_pVertInfo[i];
			const Vector3d v = pnode->m_velocity;
			const double mass = pnode->m_mass;
			const Vector3d acc = v*dampv;
			const Vector3d force = acc*mass;
			pnode->m_acc += acc;
			pnode->m_force += force;
		}
	}
	*/
}


void CSimuEntity::_checkMass(void)
{
	for (int i=0; i<m_nVertexCount; i++){
		VertexInfo *pnode = &m_pVertInfo[i];
		if (pnode->m_mass==0)	//unused nodes, assign mass to avoid problems
			pnode->m_mass =1;
	}
}


CSimuEntity::CSimuEntity(
		const Vector3d *pVertex, const int nv, 
		const Vector3d &init_velocity, 
		const CMeMaterialProperty* pmtl				//material property	
	)
{
	int i;
	if (pmtl) m_mtl = *pmtl;
	m_mtl.init();
	CVertexConstraints constraints;
	Vector3d zero = Vector3d(0,0,0);
	m_gravity_acc = zero;

	//set texture coord as NULL
	m_pTextureCoord = NULL;	
	m_nTextureCoordStride = 0;

	//init all the constraints
	for (i=0; i<CONSTRAINT_BUFF_LENGTH; i++) m_pConstraints[i]=NULL;
	m_pCollisionConstraint = NULL;

	m_nElementCount = 0;
	m_pVertInfo = new VertexInfo[nv];
	assert(m_pVertInfo!=0);
	memset(m_pVertInfo, 0, sizeof(VertexInfo)*nv);
	m_nVertexCount = nv;

	//Sparse mat data used for implicit integration
	m_pImplicitSolverData=NULL;

	for (i=0; i<nv; i++){
		VertexInfo *node = &m_pVertInfo[i];
		node->m_pos = pVertex[i];
		node->m_velocity = init_velocity;
		node->m_acc = zero;
		node->m_constraints = constraints;
	}
}


void CSimuEntity::addConstraint(const CConstraint *pconstraint)
{
	int i, j;
	//find the right position using linear search
	const int nprior = pconstraint->getPrior();
	for (i=0; i<CONSTRAINT_BUFF_LENGTH-1; i++){
		CConstraint *p = m_pConstraints[i];	
		if (p){
			const int n1 = p->getPrior();
			if (n1>nprior) break;
		}
		else{
			m_pConstraints[i] = (CConstraint *)pconstraint;
			return;
		}
	}
	//insert and move 
	for (j=CONSTRAINT_BUFF_LENGTH-1; j>i; j--)
		m_pConstraints[j] = m_pConstraints[j-1];	
	m_pConstraints[i] = (CConstraint *)pconstraint;
}


//output a specified vertex position to a stream
void CSimuEntity::dumpVertexPosition(const int vertID, const int sequenceID, FILE * fp) const
{
	if (m_pVertInfo==NULL) return;
	if (vertID<0 || vertID>=m_nVertexCount) return;
	if (fp==NULL) return;

	Vector3d x = m_pVertInfo[vertID].m_pos; 
	fprintf(fp, "%d %lg %lg %lg\n", sequenceID, x.x, x.y, x.z);
}


void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(const int i, const int j, const double3x3& stiffmat)
{
	ASSERT0(i!=j);
	double3x3 stiffmat2 = -stiffmat;
	CSparseMatrix33 *p = m_pImplicitSolverData->m_pSparseMat33;
	p->addWithMatrixElement(i, i, stiffmat);
	p->addWithMatrixElement(i, j, stiffmat2);
	p->addWithMatrixElement(j, j, stiffmat);
	p->addWithMatrixElement(j, i, stiffmat2);
}


void CSimuEntity::_implicitEulerIntegration(const int timeid, const double& dt)
{
	//disable dampling in implicit mode
	CMeMaterialProperty mtlbak = m_mtl;
	m_mtl.setDamping(0,0);			

	//call the function
	ImplicitEulerIntegration(*this, timeid, dt);

	//restore material prop.
	m_mtl = mtlbak;
}


void CSimuEntity::advanceOneTimeStep(const int timeid, const double& dt)
{
	//if (timeid>50000) this->setGravityAccelertion(Vector3d(0,1.75, 0)*0.5);
	if (timeid==0){
		initForceAndApplyGravity();
		_checkMass();
	}

	//-----velocity verlet integration method
	//_velocityVerletIntegration(timeid, dt); return;

//============================================
	_verletIntegration(timeid, dt); 
	//static int nodeid=25;
	//exportNodeInfo(nodeid);
	return;
//============================================

	//_velocityBasedIntegration(timeid, dt); return;
	//_implicitRectangularIntegration(timeid, dt);

	_implicitEulerIntegration(timeid, dt);

}


void CSimuEntity::applyAllPositionConstraints(const double &current_tm, const double &dt)
{
	{//apply collision constraints, which has the lowest priority
		CCollisionConstraint *p = m_pCollisionConstraint;
		if (p && p->isEnabled() && (!p->isExpired(current_tm))){
			p->applyConstraint(this, current_tm, dt);
		}
	}

	//other constraints, user specified
	for (int i=0; i<CONSTRAINT_BUFF_LENGTH; i++){
		CConstraint *p = m_pConstraints[i];
		if (p){
			if (p->isEnabled() && (!p->isExpired(current_tm)))
				p->applyConstraint(this, current_tm, dt);
		}
	}
}


//nodeid = -1, export all, otherwise, a specific node
void CSimuEntity::exportNodeInfo(const int nodeid)
{
	int i0, i1;
	if (nodeid==-1)
		i0=0, i1=m_nVertexCount; 
	else
		i0=nodeid, i1=i0+1;
	for (int i=i0; i<i1; i++){
		Vector3d f = m_pVertInfo[i].m_force;
		printf("Node %d: force %lg %lg %lg\n", i, f.x, f.y, f.z);
	}
}


int CSimuEntity::exportOBJFileObject(const int ID, const int objid, const int vertexbaseid, const int texbaseid, FILE *fp) const
{
	int i;
	fprintf(fp, "g S%04d_OBJ%d_FACES\n", ID, objid);

	//export vertex coord
	for (i=0; i<m_nVertexCount; i++){
		const Vector3d& v = m_pVertInfo[i].m_pos;
		const double x = v.x;
		const double y = v.y;
		const double z = v.z;
		fprintf(fp, "v %lg %lg %lg\n", x, y, z);
	}
	//export vertex texture coord
	if (m_pTextureCoord){
		if (m_nTextureCoordStride==2){
			const Vector2f *ptex = (const Vector2f *)m_pTextureCoord;
			for (i=0; i<m_nVertexCount; i++){
				const float x = ptex[i].x;
				const float y = ptex[i].y;
				fprintf(fp, "vt %g %g\n", x, y);
			}
		}
		else{
			assert(m_nTextureCoordStride==3);
			const Vector3f *ptex = (const Vector3f *)m_pTextureCoord;
			for (i=0; i<m_nVertexCount; i++){
				const float x = ptex[i].x;
				const float y = ptex[i].y;
				const float z = ptex[i].z;
				fprintf(fp, "vt %g %g %g\n", x, y, z);
			}
		}
	}
	return m_nVertexCount;
}


//======================================================================

static int test_func(void)
{
	return 1;
}

static int test = test_func();
