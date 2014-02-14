//FILE: simu_entity.cpp
#include <memory.h>
#include <sysconf.h>
#include <geomath.h>
#include "simu_entity.h"
#include "zsparse_matrix.h"
#include "constraints.h"
#include "time_integration.h"
#include "save_stiffness.h"

void ComputeNodeMassTet(const Vector3d *pVertex, const int m_nVertexCount, 
		const int *pElement, const int nelm, 
		const double &rho, CSimuEntity::VertexInfo *m_pVertInfo)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	//First, init node mass as zero
	for (int i=0; i<m_nVertexCount; i++) 
		m_pVertInfo[i].m_mass = 0;
	//loop the elements;
	const Vector4i *pquad = (const Vector4i *)pElement;
	const double K = rho * 0.250;
	for (int i=0; i<nelm; i++){
		const Vector4i quad = pquad[i];
		const Vector3d& p0 = pVertex[quad.x];
		const Vector3d& p1 = pVertex[quad.y];
		const Vector3d& p2 = pVertex[quad.z];
		const Vector3d& p3 = pVertex[quad.w];
		const double elm_vol = computeTetrahedronVolume(p0, p1, p2, p3);
		const double node_mass = elm_vol * K;
		m_pVertInfo[quad.x].m_mass += node_mass;
		m_pVertInfo[quad.y].m_mass += node_mass;
		m_pVertInfo[quad.z].m_mass += node_mass;
		m_pVertInfo[quad.w].m_mass += node_mass;
	}
}

void ComputeNodeMassHex(const Vector3d *pVertex, const int m_nVertexCount, 
		const int *pElement, const int nelm, 
		const double &rho, CSimuEntity::VertexInfo *m_pVertInfo)
{
	if ((m_pVertInfo==NULL) || (m_nVertexCount <=0)){
		printf("Error: object not initialized!\n");
		return;
	}
	//First, init node mass as zero
	for (int i=0; i<m_nVertexCount; i++) 
		m_pVertInfo[i].m_mass = 0;
	//loop the elements;
	const Vector8i *pquad = (const Vector8i *)pElement;
	const double K = rho * 0.1250;
	for (int i=0; i<nelm; i++){
		const Vector8i q = pquad[i];
		const double elm_vol = computeHexahedronVolume(pVertex, q);
		const double node_mass = elm_vol * K;
		m_pVertInfo[q.x].m_mass += node_mass;
		m_pVertInfo[q.y].m_mass += node_mass;
		m_pVertInfo[q.z].m_mass += node_mass;
		m_pVertInfo[q.w].m_mass += node_mass;
		m_pVertInfo[q.x1].m_mass += node_mass;
		m_pVertInfo[q.y1].m_mass += node_mass;
		m_pVertInfo[q.z1].m_mass += node_mass;
		m_pVertInfo[q.w1].m_mass += node_mass;
	}
}


CSimuEntity::~CSimuEntity(void)
{
	if (m_pVertInfo)
		delete [] m_pVertInfo, m_pVertInfo=NULL;
	if (m_pOdeIntegrator) 
		delete m_pOdeIntegrator, m_pOdeIntegrator=NULL;

	for (int i=0; i<CONSTRAINT_BUFF_LENGTH; i++) 
		SafeDelete(m_pConstraints[i]);
	SafeDelete(m_pCollisionConstraint);
}


void CSimuEntity::initForceAndApplyGravity(void)
{
	//init with gravity force
	Vector3d zero(0,0,0);
	if (m_gravity_acc==zero){
		for (int i=0; i<m_nVertexCount; i++){
			m_pVertInfo[i].m_force = zero;
			m_pVertInfo[i].m_acc = zero;
		}
	}
	else{
		for (int i=0; i<m_nVertexCount; i++){
			VertexInfo *pnode = &m_pVertInfo[i];
			pnode->m_force = m_gravity_acc * pnode->m_mass;
			pnode->m_acc = m_gravity_acc;
		}
	}
}


void CSimuEntity::_checkMass(void)
{
	for (int i=0; i<m_nVertexCount; i++){
		VertexInfo *pnode = &m_pVertInfo[i];
		if (pnode->m_mass==0)	//unused nodes, assign mass to avoid problems
			pnode->m_mass = 1e-20;
	}
}


CSimuEntity::CSimuEntity(
	const Vector3d *pVertex, const int nv, 
	const Vector3d &init_velocity, 
	const CMeMaterialProperty* pmtl		//material property	
	):
	m_mtl(), 
	//set texture coord as NULL
	m_pTextureCoord(NULL),	
	m_nTextureCoordStride(0),
	//time integration
	m_nIntegrationMethod(0),
	m_pOdeIntegrator(NULL),
	//elm
	m_nElementCount(0),
	m_nVertexCount(nv), 
	m_bHasConstantStiffness(false),	//non-constant stiffness
	m_bStiffnessIsMatrix33(true)	//default stiffness element: double3x3
{
	const Vector3d zero(0,0,0);
	m_gravity_acc = zero;
	if (pmtl) m_mtl = *pmtl;
	m_mtl.init();

	try{
		//init all the constraints
		CVertexConstraints constraints;
		for (int i=0; i<CONSTRAINT_BUFF_LENGTH; i++) 
			m_pConstraints[i]=NULL;
		m_pCollisionConstraint = NULL;

		m_pVertInfo = new VertexInfo[nv + 4];
		assert(m_pVertInfo!=0);
		memset(m_pVertInfo, 0, sizeof(VertexInfo)*nv);

		for (int i=0; i<nv; i++){
			VertexInfo *node = &m_pVertInfo[i];
			node->m_pos = node->m_pos0 = pVertex[i];
			node->m_velocity = node->m_velocity0 = init_velocity;
			node->m_acc = zero;
			node->m_constraints = constraints;
		}
	}catch(...){
		std::cout<<"Construction function error in CSimuEntity.\n";
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


void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(
	const int i, const int j, const double3x3& stiffmat, const bool FASTSTIFF)
{
	CSparseMatrix33 *p = m_pOdeIntegrator->getSparseMatrix();
	if (FASTSTIFF){
		const double stiffval = stiffmat.x[0];
		const double stiffval2 = -stiffval;
		p->addWithMatrixElement(i, i, stiffval);
		p->addWithMatrixElement(i, j, stiffval2);
		p->addWithMatrixElement(j, j, stiffval);
		p->addWithMatrixElement(j, i, stiffval2);
	}
	else{
		const double3x3 stiffmat2 = -stiffmat;
		p->addWithMatrixElement(i, i, stiffmat);
		p->addWithMatrixElement(i, j, stiffmat2);
		p->addWithMatrixElement(j, j, stiffmat);
		p->addWithMatrixElement(j, i, stiffmat2);
	}
}


void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(
	CSparseMatrix33 *pStiffMatrix,	//stiffness matrix
	const int vi,					//center vertex ID
	const int vj_array[],			//1-ring neighbour vertex list
	const double3x3 stiffmatj[],	//stiffness matrix list
	const int vjlen,				//length of 1-ring vertex list
	const bool FASTSTIFF)			//stiffness type, true:double, false:double3x3
{
	CSparseMatrix33 *p = pStiffMatrix;
	const int elmtype = p->getMatrixElementType();
	if (FASTSTIFF){
		ASSERT0(elmtype==0);
		double stiffi(0.0);
		for (int j=0; j<vjlen; ++j){
			const int vj = vj_array[j];
			const double& stiffval = stiffmatj[j].x[0];
			const double stiffval2 = -stiffval;
			stiffi += stiffval;
			p->addWithMatrixElement(vj, vj, stiffval);
			p->addWithMatrixElement(vi, vj, stiffval2);
			p->addWithMatrixElement(vj, vi, stiffval2);
		}
		p->addWithMatrixElement(vi, vi, stiffi);
	}
	else{
		ASSERT0(elmtype==1);
		double3x3 stiffi(0.0);
		for (int j=0; j<vjlen; ++j){
			const int vj = vj_array[j];
			const double3x3& stiffval = stiffmatj[j];
			const double3x3 stiffval2 = -stiffval;
			stiffi += stiffval;
			p->addWithMatrixElement(vj, vj, stiffval);
			p->addWithMatrixElement(vi, vj, stiffval2);
			p->addWithMatrixElement(vj, vi, stiffval2);
		}
		p->addWithMatrixElement(vi, vi, stiffi);
	}
}


void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(
	const int vi,					//center vertex ID
	const int vj_array[],			//1-ring neighbour vertex list
	const double3x3 stiffmatj[],	//stiffness matrix list
	const int vjlen,				//length of 1-ring vertex list
	const bool FASTSTIFF)			//stiffness type, true:double, false:double3x3
{
	CSparseMatrix33 *p = m_pOdeIntegrator->getSparseMatrix();
	saveVertSitffMatrixIntoSparseMatrix(p, vi, vj_array, stiffmatj, vjlen, FASTSTIFF);
}


//save stiffness for thin shell triangle pair structure
void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(
	const int vi, const int vj, const int vk, const double3x3 stiffmat[3], const bool FASTSTIFF)
{
	CSparseMatrix33 *p = m_pOdeIntegrator->getSparseMatrix();
	SaveVertSitffMatrixIntoSparseMatrix(p, FASTSTIFF, vi, vj, vk, stiffmat);
}


//save stiffness for thin shell/ quad pair structure
void CSimuEntity::saveVertSitffMatrixIntoSparseMatrix(
	const int vi, const int vj, const int vk, const int vl, 
	const double3x3 stiffmat[12], const bool FASTSTIFF)
{
	CSparseMatrix33 *p = m_pOdeIntegrator->getSparseMatrix();
	if (FASTSTIFF){
		double s11, s12, s13, s14;
		double s21, s22, s23, s24; 
		double s31, s32, s33, s34;
		double s41, s42, s43, s44;
		s22 = stiffmat[0].x[0];
		s23 = stiffmat[1].x[0];
		s24 = stiffmat[2].x[0];
		s33 = stiffmat[3].x[0];
		s34 = stiffmat[4].x[0];
		s44 = stiffmat[5].x[0];
		s32 = s23;
		s42 = s24;
		s43 = s34;
		s12 = s21 = -(s22+s32+s42);
		s13 = s31 = -(s23+s33+s43);
		s14 = s41 = -(s24+s34+s44);
		s11 = -(s12+s13+s14);
		p->addWithMatrixElement(vi, vi, s11);
		p->addWithMatrixElement(vi, vj, s12);
		p->addWithMatrixElement(vi, vk, s13);
		p->addWithMatrixElement(vi, vl, s14);
		p->addWithMatrixElement(vj, vi, s21);
		p->addWithMatrixElement(vj, vj, s22);
		p->addWithMatrixElement(vj, vk, s23);
		p->addWithMatrixElement(vj, vl, s24);
		p->addWithMatrixElement(vk, vi, s31);
		p->addWithMatrixElement(vk, vj, s32);
		p->addWithMatrixElement(vk, vk, s33);
		p->addWithMatrixElement(vk, vl, s34);
		p->addWithMatrixElement(vl, vi, s41);
		p->addWithMatrixElement(vl, vj, s42);
		p->addWithMatrixElement(vl, vk, s43);
		p->addWithMatrixElement(vl, vl, s44);
	}
	else{
		double3x3 s11, s12, s13, s14;
		double3x3 s21, s22, s23, s24; 
		double3x3 s31, s32, s33, s34;
		double3x3 s41, s42, s43, s44;
		s22 = stiffmat[0];
		s23 = stiffmat[1];
		s24 = stiffmat[2];
		s33 = stiffmat[3];
		s34 = stiffmat[4];
		s44 = stiffmat[5];
		s32 = s23;
		s42 = s24;
		s43 = s34;
		s12 = s21 = -(s22+s32+s42);
		s13 = s31 = -(s23+s33+s43);
		s14 = s41 = -(s24+s34+s44);
		s11 = -(s12+s13+s14);
		p->addWithMatrixElement(vi, vi, s11);
		p->addWithMatrixElement(vi, vj, s12);
		p->addWithMatrixElement(vi, vk, s13);
		p->addWithMatrixElement(vi, vl, s14);
		p->addWithMatrixElement(vj, vi, s21);
		p->addWithMatrixElement(vj, vj, s22);
		p->addWithMatrixElement(vj, vk, s23);
		p->addWithMatrixElement(vj, vl, s24);
		p->addWithMatrixElement(vk, vi, s31);
		p->addWithMatrixElement(vk, vj, s32);
		p->addWithMatrixElement(vk, vk, s33);
		p->addWithMatrixElement(vk, vl, s34);
		p->addWithMatrixElement(vl, vi, s41);
		p->addWithMatrixElement(vl, vj, s42);
		p->addWithMatrixElement(vl, vk, s43);
		p->addWithMatrixElement(vl, vl, s44);
	}
}


static ODEIntegrator* _allocODESolver(CSimuEntity *pEntity, int ode_type, double *pmass, const int stride)
{
	ODEIntegrator* pOdeIntegrator = NULL;
	try{
		switch (ode_type){
		case 0:
			pOdeIntegrator = new VerletIntegrator(pEntity);
			break;
		case 10:
			pOdeIntegrator = new ImplicitEulerIntegrator(pEntity, pmass, stride);
			break;
		case 11:
			pOdeIntegrator = new ImplicitTrapezoidalIntegrator(pEntity, pmass, stride);
			break;
		case 12:
			pOdeIntegrator = new BDF2Integrator(pEntity, pmass, stride);
			break;
		case 13:
			pOdeIntegrator = new NewmarkIntegrator(pEntity, pmass, stride);
			break;
		case 14:
			pOdeIntegrator = new ImplicitGMPIntegrator(pEntity, pmass, stride);
			break;
		default:
			pOdeIntegrator = new VerletIntegrator(pEntity);
			break;
		};
	}catch(...){
		pOdeIntegrator = NULL;
		std::cout<<"Alloc ODE integrator error!\n";
	}
	return pOdeIntegrator;
}


void CSimuEntity::setIntegrationMethod(const int tid)
{
	if (m_pOdeIntegrator) delete m_pOdeIntegrator;
	m_pOdeIntegrator = NULL;
	m_nIntegrationMethod = tid;

	double *pmass = &m_pVertInfo[0].m_mass;
	assert(sizeof(VertexInfo)%sizeof(double)==0);
	const int stride = sizeof(VertexInfo)/sizeof(double);
	m_pOdeIntegrator = _allocODESolver(this, tid, pmass, stride);
}


void CSimuEntity::advanceOneTimeStep(const unsigned int timeid, const double& dt)
{
	if (timeid==0){
		if (m_pOdeIntegrator==NULL){
			double *pmass = &m_pVertInfo[0].m_mass;
			assert(sizeof(VertexInfo)%sizeof(double)==0);
			const int stride = sizeof(VertexInfo)/sizeof(double);
			m_pOdeIntegrator = _allocODESolver(this, m_nIntegrationMethod, pmass, stride);
			assert(m_pOdeIntegrator!=NULL);
		}

		const double dt1000 = dt*1000.0;
		const double current_tm = dt1000 * timeid;
		this->performEdgeRelaxation(current_tm, dt1000);

		initForceAndApplyGravity();
		_checkMass();

		//pre-compute stiffness matrix
		const bool bConstStiff = this->hasConstantStiffness();
		CSparseMatrix33* pSparseMat = m_pOdeIntegrator->getSparseMatrix();
		if ( bConstStiff && pSparseMat != NULL )
			this->computeStiffnessMatrix();
	}

	//perform integration
	m_pOdeIntegrator->doIntegration(timeid, dt);
}


void CSimuEntity::setFixedVerticesByZValue(const double minz, const double maxz)
{
	

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
bool ExportMeshPLTSurface(const CSimuEntity::VertexInfo *m_pVertInfo, const int nv, 
	const int *m_pSurfacePoly, const int m_nSurfaceType, const int m_nSurfacePoly, FILE *fp)
{
	if (m_pSurfacePoly==NULL) return false;
	int i, shapeindex;
	char* PLYTYPESTR[2]={"TRIANGLE", "QUADRILATERAL"};
	if (m_nSurfaceType==3) shapeindex=0;
	else shapeindex=1;
	//vertex array
	fprintf(fp, "TITLE = SIMULATION_RESULT\n");
	fprintf(fp, "VARIABLES = \"X\",\"Y\",\"Z\"\n");
	fprintf(fp, "ZONE N = %d  E = %d F = FEPOINT, ET = %s\n", 
		nv, m_nSurfacePoly, PLYTYPESTR[shapeindex]);
	for (i=0; i<nv; i++){
        const Vector3d p= m_pVertInfo[i].m_pos;
		fprintf(fp, "%lG %lG %lG\n", p.x, p.y, p.z);
    }
	//mesh connectivity
	if (m_nSurfaceType==3){
		const Vector3i *ptri = (const Vector3i *)m_pSurfacePoly;
		for (i=0; i<m_nSurfacePoly; i++){
			Vector3i tri = ptri[i];
			tri+=Vector3i(1,1,1);
			fprintf(fp, "%d %d %d\n", tri.x, tri.y, tri.z);
		}
	}
	else{
		const Vector4i *ptri = (const Vector4i *)m_pSurfacePoly;
		for (i=0; i<m_nSurfacePoly; i++){
			Vector4i tri = ptri[i];
			tri+=Vector4i(1,1,1,1);
			fprintf(fp, "%d %d %d %d\n", tri.x, tri.y, tri.z, tri.w);
		}
	}
	return 1;
}


bool ExportMeshOBJSurface(const int ID, const int objid, const CSimuEntity::VertexInfo *m_pVertInfo, const int nv, 
	const int *m_pSurfacePoly, const int m_nSurfaceType, const int m_nSurfacePoly, const int vertexbaseid, 
	const float *m_pTextureCoord, const int m_nTextureCoordStride, const int texbaseid, 
	FILE *fp)
{
	int i;
	if (m_pSurfacePoly==NULL) return false;
	fprintf(fp, "g S%04d_OBJ%d_FACES\n", ID, objid);

	//export vertex coord
	for (i=0; i<nv; i++){
		const Vector3d& v = m_pVertInfo[i].m_pos;
		const double x = v.x; const double y = v.y; const double z = v.z;
		fprintf(fp, "v %lg %lg %lg\n", x, y, z);
	}

	//export vertex texture coord
	if (m_pTextureCoord){
		if (m_nTextureCoordStride==2){
			const Vector2f *ptex = (const Vector2f *)m_pTextureCoord;
			for (i=0; i<nv; i++){
				const float x = ptex[i].x; const float y = ptex[i].y;
				fprintf(fp, "vt %g %g\n", x, y);
			}
		}
		else{
			assert(m_nTextureCoordStride==3);
			const Vector3f *ptex = (const Vector3f *)m_pTextureCoord;
			for (i=0; i<nv; i++){
				const float x = ptex[i].x; const float y = ptex[i].y; const float z = ptex[i].z;
				fprintf(fp, "vt %g %g %g\n", x, y, z);
			}
		}
	}

	//mesh connectivity
	if (m_nSurfaceType==3){
		Vector3i *ptri = (Vector3i *)m_pSurfacePoly; 
		Vector3i vertexbase(vertexbaseid, vertexbaseid, vertexbaseid);
		Vector3i vertextexbase(texbaseid, texbaseid, texbaseid);
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector3i tri = ptri[i] + vertexbase;
			if (m_pTextureCoord==NULL){
				fprintf(fp, "f %d %d %d\n", tri.x, tri.y, tri.z);
			}
			else{
				const Vector3i tex = ptri[i] + vertextexbase;
				fprintf(fp, "f %d/%d %d/%d %d/%d\n", 
							tri.x, tex.x,
							tri.y, tex.y,
							tri.z, tex.z);
			}
		}
	}
	else{
		Vector4i *ptri = (Vector4i *)m_pSurfacePoly; 
		Vector4i vertexbase(vertexbaseid, vertexbaseid, vertexbaseid, vertexbaseid);
		Vector4i vertextexbase(texbaseid, texbaseid, texbaseid, texbaseid);
		for (int i=0; i<m_nSurfacePoly; i++){
			const Vector4i tri = ptri[i] + vertexbase;
			if (m_pTextureCoord==NULL){
				fprintf(fp, "f %d %d %d %d\n", tri.x, tri.y, tri.z, tri.w);
			}
			else{
				const Vector4i tex = ptri[i] + vertextexbase;
				fprintf(fp, "f %d/%d %d/%d %d/%d %d/%d\n", 
							tri.x, tex.x,
							tri.y, tex.y,
							tri.z, tex.z,
							tri.w, tex.w);
			}
		}
	}
	return 1;
}

//======================================================================

static int test_func(void)
{
	printf("Size of vertinfo is %d.\n", sizeof(CSimuEntity::VertexInfo));
	return 1;
}

static int test = test_func();
