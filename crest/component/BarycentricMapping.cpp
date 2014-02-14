//File: BarycentricMapping.cpp

//#include <crest/base/matrix2d.h>
#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/component/BarycentricMapping.h>

namespace cia3d{
namespace component{

CIA3D_DECL_CLASS(BarycentricMapping);

int BarycentricMappingClass = 
cia3d::core::RegisterObject("Barycentric Mapping")
.add<BarycentricMapping>()
.addAlias("BarycentricMapping")
;


//find nearest tetrahedron
static inline double _VertexTetDistance2(const Vector3d&p, const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d )
{
	const Vector3d center = (a+b+c+d)*0.250;  //find the center of tet
	const double dist2 = Distance2(center, p);
	return dist2;
}

static inline double _VertexTetDistance2(const Vector3d&p, const Vector3d *pVertex, const Vector4i& tet)
{
	const Vector3d &a= pVertex[tet.x];
	const Vector3d &b= pVertex[tet.y];
	const Vector3d &c= pVertex[tet.z];
	const Vector3d &d= pVertex[tet.w];
	const double dist2 = _VertexTetDistance2(p, a, b, c, d);
	return dist2;
}

static inline int 
_findNearestTet(const Vector3d &p, const Vector3d *pVertex, const Vector4i *pTet, const int ntet)
{
	double dist2 = 1e80;
	int tetid = -1;
	for (int i=0; i<ntet; i++){
		const double _dist2 = _VertexTetDistance2(p, pVertex, pTet[i]);
		if (dist2>_dist2){
			dist2 = _dist2;
			tetid = i;
		}
	}
	return tetid;
}

static inline void 
_findNearestTet(const Vector3d &p, const Vector3d *pVertex, const int *pConnectivity, const int nelm, const int type, Vector4i& out)
{
	assert(type==4 || type==8);
	double dist2 = 1e80;

	if (type==4){
		const Vector4i *pTet = (Vector4i*)pConnectivity;
		int tetid = 0;
		for (int i=0; i<nelm; i++){
			const double _dist2 = _VertexTetDistance2(p, pVertex, pTet[i]);
			if (dist2>_dist2){
				dist2 = _dist2;
				tetid = i;
			}
		}
		out = pTet[tetid];
	}
	else{//type==8
		const Vector8i *pHex = (Vector8i*)pConnectivity;
		Vector4i hextet[6];
		for (int i=0; i<nelm; i++){
			hexConnectivityToTetConnectivity(pHex[i], hextet);
			for (int j=0; j<6; j++){
				const double _dist2 = _VertexTetDistance2(p, pVertex, hextet[j]);
				if (dist2>_dist2){
					dist2 = _dist2;
					out = hextet[j];
				}
			}
		}
	}
}

static double tet_volume(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d)
{
	Matrix2d m;
	m.x[0][0]=a.x, 	m.x[0][1]=a.y, 	m.x[0][2]=a.z,    m.x[0][3]=1, 
	m.x[1][0]=b.x, 	m.x[1][1]=b.y, 	m.x[1][2]=b.z,    m.x[1][3]=1, 
	m.x[2][0]=c.x, 	m.x[2][1]=c.y, 	m.x[2][2]=c.z,    m.x[2][3]=1, 
	m.x[3][0]=d.x, 	m.x[3][1]=d.y, 	m.x[3][2]=d.z,    m.x[3][3]=1;
	const double vol = m.det4x4();
	return vol;
}

//source obj: the simulation object
//Destination obj: the visual object
void BarycentricMapping::init(CPolyObj *src, CPolyObj *dst)
{
	//copy first
	m_pSrcObj = src; m_pDstObj = dst;
	assert(src!=NULL);
	assert(dst!=NULL);	
	if (src->m_pElement==NULL || src->m_nElementCount==0){
		fprintf(stderr, "Error: The source object is not a solid obj!\n");
		m_pSrcObj = NULL; 
		m_pDstObj = NULL;
		return;
	}

	//init some buffers
	const int nv = dst->m_nVertexCount;
	m_RefVertex.resize(nv);
	m_RefVertex.clear();

	//find the nearest tetrahedra 
	const Vector3d *pv = src->m_pVertex;
	const int nelm = src->m_nElementCount;
	int *pelm = src->m_pElement;
	Vector4i tet;
	for (int i=0; i<nv; i++){
		const Vector3d p = dst->m_pVertex[i];
		_findNearestTet(p, pv, pelm, nelm, src->m_nElmVertexCount, tet);
		const int v0=tet.x, v1=tet.y, v2=tet.z, v3=tet.w;
		const Vector4i t0(v1, v3, v2, v0);
		const Vector4i t1(v0, v2, v3, v1); 
		const Vector4i t2(v3, v1, v0, v2); 
		const Vector4i t3(v0, v1, v2, v3);
		const double tvol = tet_volume(pv[v0], pv[v1], pv[v2], pv[v3]);
		ASSERT0(tvol>0);
		const double vol0 = tet_volume(pv[t0.x], pv[t0.y], pv[t0.z], p);
		const double vol1 = tet_volume(pv[t1.x], pv[t1.y], pv[t1.z], p);
		const double vol2 = tet_volume(pv[t2.x], pv[t2.y], pv[t2.z], p);
		const double vol3 = tvol - (vol0+vol1+vol2); // - tet_volume(pv[t3.x], pv[t3.y], pv[t3.z], p);
		const Vector4f w = Vector4f(vol0,vol1,vol2,vol3)/tvol; 
		const DeformationRecord rec(tet, w);
		m_RefVertex.push_back(rec);
	}
}

inline void 
_barycentricInterpolation(
	const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d, 
	const double &wa, const double &wb, const double &wc, const double &wd, 
	Vector3d &r)
{
	r = (a*wa) + (b*wb) + (c*wc) + (d*wd);
}

void BarycentricMapping::updateMapping(const Vector3d* in, Vector3d* out)
{
	if (m_pSrcObj == NULL || m_pDstObj == NULL) 
		return;
	if (out==NULL) 
		return;

	const int nsize = m_pDstObj->m_nVertexCount;
	for (int i=0; i<nsize; i++){
		const int *refVertexID = m_RefVertex[i].refVertexID;
		const float *refVertexWeight = m_RefVertex[i].refVertexWeight;
		const Vector3d *a = &in[refVertexID[0]];
		const Vector3d *b = &in[refVertexID[1]];
		const Vector3d *c = &in[refVertexID[2]];
		const Vector3d *d = &in[refVertexID[3]];
		const double wa = (double)refVertexWeight[0];
		const double wb = (double)refVertexWeight[1];
		const double wc = (double)refVertexWeight[2];
		const double wd = (double)refVertexWeight[3];
		_barycentricInterpolation(*a, *b, *c, *d, wa, wb, wc, wd, out[i]);
	}
}


void BarycentricMapping::updateMapping(const Vector3d* vin, Vector3f* vout)
{
	if (m_pSrcObj == NULL || m_pDstObj == NULL) 
		return;
	if (vout==NULL) 
		return;

	const int nsize = m_pDstObj->m_nVertexCount;
	for (int i=0; i<nsize; i++){
		const int *refVertexID = m_RefVertex[i].refVertexID;
		const float *refVertexWeight = m_RefVertex[i].refVertexWeight;
		const Vector3d *a = &vin[refVertexID[0]];
		const Vector3d *b = &vin[refVertexID[1]];
		const Vector3d *c = &vin[refVertexID[2]];
		const Vector3d *d = &vin[refVertexID[3]];
		const double wa = (double)refVertexWeight[0];
		const double wb = (double)refVertexWeight[1];
		const double wc = (double)refVertexWeight[2];
		const double wd = (double)refVertexWeight[3];
		Vector3d r;
		_barycentricInterpolation(*a, *b, *c, *d, wa, wb, wc, wd, r);
		vout[i] = Vector3f(r.x, r.y, r.z);
	}
}


} // namespace component
} // namespace cia3d
