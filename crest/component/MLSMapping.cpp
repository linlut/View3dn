//File: MLSMapping.cpp

#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/component/MLSMapping.h>
#include <crest/extlibs/horn_registration/etkRegistration.hpp>
#include <crest/GPU/CUDA/etkQuartic.cu>
#include <crest/GPU/CUDA/etkRegistration.cu>
#include <crest/system/ztime.h>
#include <view3dn/pointorgnizer.h>
#include <view3dn/edgetable.h>

namespace cia3d{
namespace component{

CIA3D_DECL_CLASS(MLSMapping);

static unsigned char MLSMappingClass = 
	cia3d::core::RegisterObject("MLS Mapping")
	.add<MLSMapping>()
	.addAlias("MLSMapping");

static int* _getSurfaceVertices(const int nv, const int *pConnectivity, const int len, int &nSurfacePoints)
{
	int i, *flags = new int[nv];
	assert(flags!=NULL);
	for (i=0; i<nv; i++) flags[i]=0;
	for (i=0; i<len; i++){
		const int j = pConnectivity[i];
		flags[j] = 1;
	}
	for (nSurfacePoints=i=0; i<nv; i++){
		if (flags[i])
			flags[nSurfacePoints++]=i;
	}
	return flags;
}

//Get the nearest three vertices
static inline void 
_getNearestN3(const Vector3d& p, Vector3d v[], set<int>& nbr, int mylist[])
{
	const double MAXDOUBLE = 1e100;
	const int N3 = 3;
	const int nsize = nbr.size();
	if (nsize<N3){
		assert(0);
	}
	//if only 3 of them, there is no choice
	if (nsize==N3){
		set<int>::iterator itr=nbr.begin();
		for (int i=0; i<N3; i++)
			mylist[i]= *itr, itr++;
	}
	//otherwise, choose the nearest 3
	else{
		double dist[800];
		int vertexid[800];
		set<int>::iterator itr=nbr.begin();
		for (int i=0; i<nsize; i++, itr++){
			const int x = *itr;
			dist[i] = Distance2(p, v[x]);
			vertexid[i] = x;
		}
		for (int i=0; i<N3; i++){
			double MINDIST = dist[i];
			int id = vertexid[i], ipos=i;
			for (int j=0; j<nsize; j++){
				if (MINDIST>dist[j]){
					MINDIST = dist[j];
					id = vertexid[j];
					ipos = j;
				}
			}
			mylist[i]=id;
			dist[ipos]= MAXDOUBLE;
		}
	}
}

//Build the vertex connectivity for all vertices;
//Here we use the boundary surface to build it.
EdgeTable* MLSMapping::buildEdgeTable(CPolyObj *poly)
{
	EdgeTable *et = NULL;
	Vector3f *pVertex = NULL; 
	const int nv = poly->m_nVertexCount;
	int* ptri = poly->m_pPolygon; 
	const int ntri = poly->m_nPolygonCount;
	const int nvpt = poly->m_nPlyVertexCount;
	const bool refinemesh=false;
	const int isSolid =false;
	et = new EdgeTable(isSolid, pVertex, nv, ptri, ntri, nvpt, refinemesh);
	assert(et!=NULL);
	et->buildEdgeTable();
	return et;
}

//Build a 3d uniform partition of all the vertices.
CPointOrgnizer3D* MLSMapping::buildPointOrgnizer3D(CPolyObj *poly)
{
	CPointOrgnizer3D *grid=NULL;
	const int nv = poly->m_nVertexCount;
	int *pconnectivity = poly->m_pPolygon;
	const int conlen = poly->m_nPlyVertexCount * poly->m_nPolygonCount;
	int nSurfacePoints;
	int *indexbuff = _getSurfaceVertices(nv, pconnectivity, conlen, nSurfacePoints);
	assert(indexbuff!=NULL);
	grid = new CPointOrgnizer3D(poly->m_pVertex, indexbuff, nSurfacePoints);
	assert(grid!=NULL);
	SafeDeleteArray(indexbuff);
	return grid;
}

//source obj: the simulation object
//Destination obj: the visual object
void MLSMapping::init(CPolyObj *src, CPolyObj *dst)
{
	//copy first
	m_pSrcObj = src; 
	m_pDstObj = dst;
	assert(src!=NULL);
	assert(dst!=NULL);

	//init some buffers to its max size
	m_RefVertex.resize(dst->m_nVertexCount);
	m_RefVertex.clear();

	//Use a point orgnizer (3D grid0 to speed up nearest-point finding
	CPointOrgnizer3D *grid= buildPointOrgnizer3D(src);
	assert(grid!=NULL);

	//Build an edge table on the mesh for 1-ring neighbours of the nearest point
	EdgeTable *et = buildEdgeTable(m_pSrcObj);
	assert(et!=NULL);

	{//find the ref vertices on the deformation model for all visual points
		MLSDeformationRecord rec;
		int neighbours[800], neighbours2[800];
		set<int> nbset;
		const double DAMPSCALE=0.038/3;
		//const double DAMPSCALE=0.10/3;
		for (int ii=0; ii<dst->m_nVertexCount; ii++){
			nbset.clear();
			const Vector3d v = dst->m_pVertex[ii];
			const int refID = grid->searchNearestPoint(v);
			ASSERT0(refID!=-1);
			const int n1 = et->getNeighbourVerices(refID, neighbours);
			for (int i=0; i<n1; i++){
				const int vv = neighbours[i];
				nbset.insert(vv);
				const int n2 = et->getNeighbourVerices(vv, neighbours2);
				for (int j=0; j<n2; j++) 
					nbset.insert(neighbours2[j]);
			}
			nbset.erase(refID);						

			//find the nearest 3 in the neighbour
			int* const pair = &rec.refVertexID[0];
			pair[0] = refID;
			//printf("Vertid: %d\n", ii);
			_getNearestN3(v, src->m_pVertex, nbset, &pair[1]);
			
			//also compute the weights and store weights
			{	const int NV = 4;
				Vector3d refVertex[NV];
    			refVertex[0] = src->m_pVertex[pair[0]];
				refVertex[1] = src->m_pVertex[pair[1]];
				refVertex[2] = src->m_pVertex[pair[2]];
				refVertex[3] = src->m_pVertex[pair[3]];
				double weightd[4];
				double dist01 = Distance2(refVertex[0], refVertex[1]);
				double dist02 = Distance2(refVertex[0], refVertex[2]);
				double dist03 = Distance2(refVertex[0], refVertex[3]);
				double damp = DAMPSCALE*(dist01+dist02+dist03);
				computeSquareDistanceWeights(v, refVertex, NV, damp, weightd);
				float* weight = &rec.refVertexWeight[0];
				weight[0] = weightd[0];
				weight[1] = weightd[1];
				weight[2] = weightd[2];
				weight[3] = weightd[3];
			}

			m_RefVertex.push_back(rec);
		}
	}

	SafeDelete(grid);
	SafeDelete(et);
}


template <class T>
void template_updateMapping(CPolyObj *m_pSrcObj, CPolyObj *m_pDstObj, 
							Vector3f *m_pNormal, 
							Vector4f *m_pQuat,
							std::vector<MLSDeformationRecord>& m_RefVertex,
							const Vector3d* in, T* vout)
{
	const Vector4f halfvec(0.5, 0.5, 0.5, 0.5);
	const int NRIGID_VERT = 4;
	matrix3x3 rot;
	float3 trans(0,0,0);
	Vector3d p[12], q[12]; 
	double weightd[12];
	float4 Quat;
	etkRegistration reg;

	if (vout==NULL) return;
	if (m_pSrcObj == NULL || m_pDstObj == NULL) return;

	//if normal deformation is also required
	const int timerid=0;
	startFastTimer(timerid);
	if (m_pNormal){
		if (m_pDstObj->m_pVertexNorm==NULL){
			m_pDstObj->ComputeVertexNormals();
			printf("Vertex normal is computed!\n");
		}
	}
	Vector3f *pRefNorm= m_pDstObj->m_pVertexNorm;
	const int nsize = m_pDstObj->m_nVertexCount;
	Vector3d* pVertex = m_pDstObj->m_pVertex;

	//deform each vertex and norm
	for (int i=0; i<nsize; i++){
		const int *refVertexID = m_RefVertex[i].refVertexID;
		const float *refVertexWeight = m_RefVertex[i].refVertexWeight;
		//get the transformation R & T
		for (int j=0; j<NRIGID_VERT; j++){
			const int id = refVertexID[j];
			p[j]= m_pSrcObj->m_pVertex[id];
			q[j]= in[id];
			weightd[j] = refVertexWeight[j];
		}
		reg.hornRegistration(p, q, weightd, NRIGID_VERT, Quat, rot, trans);	
		//use R & T
		const float3 v(pVertex[i].x, pVertex[i].y, pVertex[i].z);
		const float3 r = rot*v + trans;
		vout[i] = T(r.x, r.y, r.z);
		//rotate the normal
		if (m_pNormal)
			m_pNormal[i] = rot*(&pRefNorm[i].x);
		//save the rotation quaternion if required
		if (m_pQuat){
			m_pQuat[i] = Vector4f(Quat.x, Quat.y, Quat.z, Quat.w)*0.5f + halfvec;
		}
	}

	m_pNormal = NULL;

	stopFastTimer(timerid);
	printf("Vertex buffer size is %d\n", nsize);
	reportTimeDifference(timerid, "Software MLS comp time is");
}



void MLSMapping::updateMapping(const Vector3d* vin, Vector3d* vout)
{
	template_updateMapping<Vector3d>(m_pSrcObj, m_pDstObj, m_pNormal, m_pQuat, m_RefVertex, vin, vout);
}

void MLSMapping::updateMapping(const Vector3d* vin, Vector3f* vout)
{
	template_updateMapping<Vector3f>(m_pSrcObj, m_pDstObj, m_pNormal, m_pQuat, m_RefVertex, vin, vout);
}

} // namespace component
} // namespace cia3d


//==========OUTPUT for paper to be submitted to TVCG MLS volume deformation===============
int printMLSRotationInfo(void)
{
	Vector3d p[]={
		Vector3d(0,0,0), 
		Vector3d(1,0,0),
		Vector3d(0,0.75,0),
		Vector3d(1,0.75,0)
	};
	Vector3d q[]={
		Vector3d(0.1,-0.4,0),
		Vector3d(1, 0, 0),
		p[2],
		p[3]
	};
	int i;
	const int NV=4;
	const int nsize = 100;
	Vector3d defvert1[nsize+20], defvert2[nsize+20];
	double weightd[NV];
	float3 trans(0,0,0);
	float4 Quat;
	matrix3x3 rot;
	etkRegistration reg;
	const float3 normY(0,1,0);
	float3 defnorms[nsize+20];
	float3 errnorms[nsize+20];

	//deform each vertex and norm
	for (i=0; i<=nsize; i++){
		double t=1.0*i/nsize;
		Vector3d u=p[0]*(1-t)+p[1]*t;
		{//compute the deformed vertices without the damping
			computeSquareDistanceWeightsNoDamp(u, p, NV, weightd);
			reg.hornRegistration(p, q, weightd, NV, Quat, rot, trans);	
			const float3 v(u.x, u.y, u.z);
			const float3 r = rot*v + trans;
			defvert1[i] = Vector3d(r.x, r.y, r.z);
			float3 n2 = rot*normY;
			n2.Normalize();
			errnorms[i]=n2;
		}
		{//compute the deformed vertices with the damping
			const double damp = 0.1*0.1;
			computeSquareDistanceWeights(u, p, NV, damp, weightd);
			reg.hornRegistration(p, q, weightd, NV, Quat, rot, trans);	
			const float3 v(u.x, u.y, u.z);
			const float3 r = rot*v + trans;
			defvert2[i] = Vector3d(r.x, r.y, r.z);
			defnorms[i] = rot*normY;
			defnorms[i].Normalize();
		}
	}

    Vector3f truenorms[nsize+20];
	for (i=1; i<nsize; i++){
		Vector3d e0=defvert1[i]-defvert1[i-1];
		Vector3d e1=defvert1[i+1]-defvert1[i];
		Vector3f n0(-e0.y, e0.x, 0);
		Vector3f n1(-e1.y, e1.x, 0);
		n0.Normalize();
		n1.Normalize();
		float len0= Magnitude(e0);
		float len1= Magnitude(e1);
		float tlen = len0+len1;
		float t0 = len0/tlen;
		float t1 = len1/tlen;
		truenorms[i]= (n0*t0) + (n1*t1);
		truenorms[i].Normalize();
	}
	truenorms[0]=truenorms[1];
	truenorms[nsize]=truenorms[nsize-1];

	for (i=0; i<=nsize; i++){
		double t=1.0*i/nsize;
		printf("%lg\t%lg\t%lg\t%lg\t%lg\n", t, defvert1[i].x, defvert1[i].y ,  defvert2[i].x, defvert2[i].y);
	}
	printf("\n\n\n");

	printf("ID\t\tDistance\t\tNormdist0\t\tNormdist\n");
	for (i=0; i<=nsize; i++){
		if (i==nsize/2){
			int xxxx=1;
		}
		double t=1.0*i/nsize;
		const double dis = Distance(defvert1[i], defvert2[i]);
		//const double angdist0 = 1.0 - DotProd(truenorms[i], errnorms[i]);
		//const double angdist = 1.0 - DotProd(truenorms[i], defnorms[i]);

		double theta0 = atan2(truenorms[i].y, truenorms[i].x);
		double theta1 = atan2(errnorms[i].y, errnorms[i].x);
		double theta2 = atan2(defnorms[i].y, defnorms[i].x);
		printf("%lg\t\t%lg\t\t%lg\t\t%lg\n", t, dis, theta1/theta0, theta2/theta0);
	}

	return 1;
}

//static int _xagagasd = printMLSRotationInfo();

