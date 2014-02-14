//FILE: simu_thinshell.inl
#include "time_integration.h"
#include "zsparse_matrix.h"


class CSimuThinshellT: public CSimuShell
{

protected:
	bool m_bIsSpringShell;

	T *m_pThinshellElement;	//thinshell element array
							//# of thinshell elm. can be found in m_nElementCount of CSimuEntity
							//NOTE that it is not the input triangle count!!!

	//compute elastic forces for shell elements made from the triangle mesh
	inline void _computeElasticForcesTri(const bool needjacobian, double3x3 *ppjac[6])
	{
		Vector3d force[4];
		for (int i=0; i<m_nElementCount; i++){
			T *E = &m_pThinshellElement[i];
			const int* pNodeBuffer = E->getElementNodeBuffer();
			const int i0 = pNodeBuffer[0]; 
			const int i1 = pNodeBuffer[1];
			const int i2 = pNodeBuffer[2]; 
			const int i3 = pNodeBuffer[3];
			const Vector3d& p0 = m_pVertInfo[i0].m_pos;
			const Vector3d& p1 = m_pVertInfo[i1].m_pos;
			const Vector3d& p2 = m_pVertInfo[i2].m_pos;
			const Vector3d& p3 = m_pVertInfo[i3].m_pos;
			E->computeForce(p0, p1, p2, p3, m_static, m_mtl, force, ppjac);
			m_pVertInfo[i0].m_force += force[0];
			m_pVertInfo[i1].m_force += force[1];
			m_pVertInfo[i2].m_force += force[2];
			m_pVertInfo[i3].m_force += force[3];
			if (needjacobian){
				if (m_bIsSpringShell){
					saveVertSitffMatrixIntoSparseMatrix(i0, i2, *(ppjac[0]));
					saveVertSitffMatrixIntoSparseMatrix(i1, i2, *(ppjac[1]));
					saveVertSitffMatrixIntoSparseMatrix(i0, i3, *(ppjac[2]));
					saveVertSitffMatrixIntoSparseMatrix(i1, i3, *(ppjac[3]));
				}
				else{
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i2, ppjac[0], true);
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i3, ppjac[3], true);
					saveVertSitffMatrixIntoSparseMatrix(i1, i0, *(ppjac[6]), true);
				}
			}	
		}
	}

	//compute elastic forces for shell elements made from the quad mesh
	inline void _computeElasticForcesQuad(const bool needjacobian, double3x3 *ppjac[6])
	{
		Vector3d force[6];
		for (int i=0; i<m_nElementCount; i++){
			T *E = &m_pThinshellElement[i];
			const int* pNodeBuffer = E->getElementNodeBuffer();
			const int i0 = pNodeBuffer[0]; const int i1 = pNodeBuffer[1];
			const int i2 = pNodeBuffer[2]; const int i3 = pNodeBuffer[3];
			const int i4 = pNodeBuffer[4]; const int i5 = pNodeBuffer[5];
			const Vector3d& p0 = m_pVertInfo[i0].m_pos;
			const Vector3d& p1 = m_pVertInfo[i1].m_pos;
			const Vector3d& p2 = m_pVertInfo[i2].m_pos;
			const Vector3d& p3 = m_pVertInfo[i3].m_pos;
			const Vector3d& p4 = m_pVertInfo[i4].m_pos;
			const Vector3d& p5 = m_pVertInfo[i5].m_pos;	
			E->computeForce(p0, p1, p2, p3, p4, p5, m_static, m_mtl, force, ppjac);
			m_pVertInfo[i0].m_force += force[0];
			m_pVertInfo[i1].m_force += force[1];
			m_pVertInfo[i2].m_force += force[2];
			m_pVertInfo[i3].m_force += force[3];
			m_pVertInfo[i4].m_force += force[4];
			m_pVertInfo[i5].m_force += force[5];
			if (needjacobian){
				if (m_bIsSpringShell){
					saveVertSitffMatrixIntoSparseMatrix(i0, i3, *(ppjac[0]));
					saveVertSitffMatrixIntoSparseMatrix(i1, i2, *(ppjac[1]));
					saveVertSitffMatrixIntoSparseMatrix(i0, i4, *(ppjac[2]));
					saveVertSitffMatrixIntoSparseMatrix(i1, i5, *(ppjac[3]));
				}
				else{
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i2, ppjac[0], true);
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i3, ppjac[3], true);
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i4, ppjac[6], true);
					saveVertSitffMatrixIntoSparseMatrix(i0, i1, i5, ppjac[9], true);
					saveVertSitffMatrixIntoSparseMatrix(i1, i0, *(ppjac[12]), true);
				}
			}
		}
	}


public:
	//compute elastic forces
	inline void computeElasticForces(const bool needjacobian)
	{
		const int N=16;
		double3x3 jacobian[N], *ppjac[N];
		if (!needjacobian)
			for (int i=0; i<N; i++) ppjac[i] = NULL;
		else
			for (int i=0; i<N; i++) ppjac[i] = &jacobian[i];
		if (m_nSurfaceType==3){
			_computeElasticForcesTri(needjacobian, ppjac);
		}
		else{
			ASSERT0(m_nSurfaceType==4);
			_computeElasticForcesQuad(needjacobian, ppjac);
		}
	}

public:
	CSimuThinshellT(
		const Vector3d *pVertex, const int nv,		//vertex array and length
		const Vector3d &init_velocity,				//vertex velocity
		const int *pElement, const int nv_per_elm, const int nelm,	//element information
		const double &shell_thickness,
		const CMeMaterialProperty& mtl,				//material property
		const int isStatic=0						//static or dynamic simulation
		):
		CSimuShell(pVertex, nv, init_velocity, pElement, nv_per_elm, nelm, shell_thickness, mtl, isStatic)
	{
		//first, need to decide mass
		const double rho = mtl.getDensity();
		computeNodeMass(pVertex, rho);

		//then, check how many thin shell elements are needed
		const int isSolid = 0;
		EdgeTable et(isSolid, NULL, nv, (int*)pElement, nelm, nv_per_elm);
		et.buildEdgeTable();
		m_nElementCount = et.m_nEdge;		
		//this number includes other edges, the true value is smaller
		int nRealElmSize = m_nElementCount;
		if (nRealElmSize<16) nRealElmSize=16;
		m_pThinshellElement = new T[nRealElmSize];
		assert(m_pThinshellElement!=NULL);
		m_bIsSpringShell = T::isSpringElement();
		m_bHasConstantStiffness = T::hasConstantStiffness();
		m_bStiffnessIsMatrix33 = T::isStiffnessMatrix3X3();

		//traverse all the edges to init the shell elements
		int cc = 0;
		for (int i=0; i<nv; i++){
			_VertexEdgeTablePointer *pedgeptr = et.m_ppVETP[i];
			while (pedgeptr!=NULL){
				_EdgeTableElementStruct *pedge = pedgeptr->m_pEdgeTableElement;
				Vector8i quad;
				quad.x = pedge->m_nV0;		
				quad.y = pedge->m_nV1;
				const int tid0 = pedge->m_nT0;
				const int tid1 = pedge->m_nT1;
				if ((tid0!=-1) && (tid1!=-1) && (i==quad.x)){
					if (nv_per_elm==3)
						_decideTriangleRemainingNodes(pElement, tid0, tid1, quad);
					else
						_decideQuadRemainingNodes(pElement, tid0, tid1, quad);
					const Vector3d& p0 = pVertex[quad.x];
					const Vector3d& p1 = pVertex[quad.y];
					const Vector3d& p2 = pVertex[quad.z];
					const Vector3d& p3 = pVertex[quad.w];
					if (nv_per_elm==3)
						m_pThinshellElement[cc].init(mtl, m_static, &quad.x, p0, p1, p2, p3, shell_thickness);
					else{
						const Vector3d& p4 = pVertex[quad.x1];
						const Vector3d& p5 = pVertex[quad.y1];
						m_pThinshellElement[cc].init(mtl, m_static, &quad.x, p0, p1, p2, p3, p4, p5, shell_thickness);
					}
					cc++;
				}
				pedgeptr = pedgeptr->m_pNext;
			}
		}
		assert(cc <= m_nElementCount);
		m_nElementCount = cc;
		printf("Vertex#: %d, polygon#: %d, shell element#:%d.\n", nv, nelm, cc);
	}

	virtual ~CSimuThinshellT(void)
	{
		SafeDeleteArray(m_pThinshellElement);
	}

	void appendElement(const T& elm)
	{
		m_pThinshellElement[m_nElementCount] = elm;
		m_nElementCount++;
	}

	//compute the acceleration vector for the current location 
	virtual void computeAcceleration(const unsigned int timeid, const bool _needJacobian=false)
	{
		//clear zero for next comp. when stiff is not constant
		//if (!this->hasConstantStiffness()){
		//	CSparseMatrix33 *pmat = this->getOdeIntegrator()->getSparseMatrix();
		//	if (pmat) pmat->clearMatrix();	
		//}

		//init forces as zero and add gravity load
		initForceAndApplyGravity();

		//add elastic forces
		bool needJacobian = _needJacobian;
		if (this->hasConstantStiffness()) needJacobian = false;
		computeElasticForces(needJacobian);

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
		//for (int i=0; i<m_nElementCount; i++){
			//m_pThinshellElement[i].updateState();
		//}
	}

	virtual void plastifyItself(const double& threshold)
	{

		for (int i=0; i<m_nElementCount; i++){
			T & e = m_pThinshellElement[i];
			e.plastifyItself(threshold);
		}
	}

	static CSimuThinshellT *genShellFromFile(const char *fname, const CMeMaterialProperty &mtl, const Vector3d *pVecTrans=NULL, const int isstatic=0)
	{
		extern bool loadQuadMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad, Vector2f *&tex);
		extern bool loadTriangleMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector3i*& pQuad, int &nquad, Vector2f *&tex);
		Vector3d *pVertex;		
		Vector3i *element;
		Vector4i *qelement;
		Vector2f *tex;
		Vector3d init_velocity(0,0,0);
		int nv_per_elm, nv, nelm, *pelm;
		const double shell_thickness=0.001;
		const int len = strlen(fname);
		bool istrimesh = false;
		if (strstr(fname+(len-4), ".txt")!=NULL){
			istrimesh=true;
		}
		if (istrimesh){
			nv_per_elm=3;
			bool r =loadTriangleMeshFile(fname, pVertex, nv, element, nelm, tex);
			pelm = (int*)element;
			if (!r) return NULL;
		}
		else{
			nv_per_elm=4;
			bool r =loadQuadMeshFile(fname, pVertex, nv, qelement, nelm, tex);
			pelm = (int*)qelement;
			if (!r) return NULL;
		}

		if (pVecTrans!=NULL){
			for (int i=0; i<nv; i++){
				pVertex[i]+=*pVecTrans;
			}
		}

		CSimuThinshellT *pshell = new CSimuThinshellT(
			pVertex, nv,					//vertex array and length
			init_velocity,					//vertex velocity
			pelm, nv_per_elm, nelm,			//element information
			shell_thickness,
			mtl,							//material property
			isstatic
			);
		assert(pshell!=NULL);
		if (tex){
			const int stride = 2;
			pshell->setTextureCoord(&tex[0].x, stride);
		}

		return pshell;
	}

	virtual void computeStiffnessMatrix(void)
	{
		//check sparse matrix
		CSparseMatrix33 *pmat = this->getOdeIntegrator()->getSparseMatrix();
		if (pmat==NULL) return;
		
		//init buffer
		const int N=20;
		double3x3 jacobian[N], *ppjac[N];
		for (int i=0; i<N; i++){ jacobian[i].setZeroMatrix(), ppjac[i] = &jacobian[i]; }

		//call main func
		if (m_nSurfaceType==3){
			_computeElasticForcesTri(true, ppjac);
		}
		else{
			ASSERT0(m_nSurfaceType==4);
			_computeElasticForcesQuad(true, ppjac);
		}
	}

};
