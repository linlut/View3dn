//File: BarycentricPolarMapping.cpp

#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/component/BarycentricPolarMapping.h>
#include <crest/base/PolarDecompose.h>
#include <crest/extlibs/horn_registration/etkQuaternion.hpp>
#include <crest/system/ztime.h>

namespace cia3d{
namespace component{

CIA3D_DECL_CLASS(BarycentricPolarMapping);

int BarycentricPolarMappingClass = 
cia3d::core::RegisterObject("BarycentricPolarMapping")
.add<BarycentricPolarMapping>()
.addAlias("BarycentricPolarMapping")
;

//source obj: the simulation object
//Destination obj: the visual object
void BarycentricPolarMapping::init(CPolyObj *src, CPolyObj *dst)
{
	if (m_pSrcObj == NULL || m_pDstObj == NULL) return;
	const int nv = m_pSrcObj->m_nVertexCount;
	const int nelm = m_pSrcObj->m_nElementCount;
	m_elmQuat.resize(nelm);
	m_vertQuat.resize(nv);
}

inline void 
setFromRotationMatrix(const Mat3x3d& m, float q[4])
{
  // Compute one plus the trace of the matrix
	const double onePlusTrace = 1.0 + m[0][0] + m[1][1] + m[2][2];
	if (onePlusTrace > 1E-6){
      // Direct computation
		const double s = sqrt(onePlusTrace) * 2.0;
		q[0] = (m[2][1] - m[1][2]) / s;
		q[1] = (m[0][2] - m[2][0]) / s;
		q[2] = (m[1][0] - m[0][1]) / s;
		q[3] = 0.25 * s;
	}
	else{
      // Computation depends on major diagonal term
		if ((m[0][0] > m[1][1])&(m[0][0] > m[2][2])){ 
		const double s = sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2.0; 
		q[0] = 0.25 * s;
		q[1] = (m[0][1] + m[1][0]) / s; 
		q[2] = (m[0][2] + m[2][0]) / s; 
		q[3] = (m[1][2] - m[2][1]) / s;
		}
		else if (m[1][1] > m[2][2]){ 
			const double s = sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2.0; 
			q[0] = (m[0][1] + m[1][0]) / s; 
			q[1] = 0.25 * s;
			q[2] = (m[1][2] + m[2][1]) / s; 
			q[3] = (m[0][2] - m[2][0]) / s;
		}
		else{ 
			const double s = sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2.0; 
			q[0] = (m[0][2] + m[2][0]) / s; 
			q[1] = (m[1][2] + m[2][1]) / s; 
			q[2] = 0.25 * s;
			q[3] = (m[0][1] - m[1][0]) / s;
		}
    }
	Vector4f *quat = (Vector4f*)&q[0];
	quat->Normalize();
}

void BarycentricPolarMapping::_computeElementQuats(const CPolyObj *poly, const Vector3d *deformv, vector<Vector4f>& quat)
{
	const int nelm = poly->m_nElementCount;
	const Vector4i *ptet = (const Vector4i *)poly->m_pElement;
	const Vector8i *phex = (const Vector8i *)poly->m_pElement;
	const Vector4f halfvec = Vector4f(0.5,0.5,0.5,0.5);
	Vector3d *pvert = poly->m_pVertex;
	for (int i=0; i<nelm; i++){
		Vector3d p0, p1, p2, p3, q0, q1, q2, q3;
		if (poly->m_nElmVertexCount==4){
			const Vector4i& tet = ptet[i];
			p0 = pvert[tet.x]; p1 = pvert[tet.y];
			p2 = pvert[tet.z]; p3 = pvert[tet.w];
			q0 = deformv[tet.x]; q1 = deformv[tet.y];
			q2 = deformv[tet.z]; q3 = deformv[tet.w];
		}
		else{
			ASSERT0(poly->m_nElmVertexCount==8);
			const Vector8i& tet = phex[i];
			p0 = pvert[tet.x]; p1 = pvert[tet.y];
			p2 = pvert[tet.w]; p3 = pvert[tet.x1];
			q0 = deformv[tet.x]; q1 = deformv[tet.y];
			q2 = deformv[tet.w]; q3 = deformv[tet.x1];
		}
		Vector3d e0 = p1-p0;
		Vector3d e1 = p2-p0;
		Vector3d e2 = p3-p0;
		Vector3d ee0 = q1-q0;
		Vector3d ee1 = q2-q0;
		Vector3d ee2 = q3-q0;

		Mat3x3d::Line r1(e0.x, e0.y, e0.z);
		Mat3x3d::Line r2(e1.x, e1.y, e1.z);
		Mat3x3d::Line r3(e2.x, e2.y, e2.z);
		Mat3x3d X(r1, r2, r3), X_inv;
		//X.transpose();
		invertMatrix(X_inv, X);

		Mat3x3d::Line s1(ee0.x, ee0.y, ee0.z);
		Mat3x3d::Line s2(ee1.x, ee1.y, ee1.z);
		Mat3x3d::Line s3(ee2.x, ee2.y, ee2.z);
		Mat3x3d Y(s1, s2, s3);
		//Y.transpose();
		Mat3x3d M=Y*X_inv;
		Mat3x3d Q, S;
		//polar decomp, where Q is the roation matrix, S is the scaling matrix
		cia3d::base::polar_decomp<double>(M, Q, S);
		Vector4f q;
		Q.transpose();
		setFromRotationMatrix(Q, &q.x);
		quat[i] = Vector4f(q[0], q[1], q[2], q[3])*0.5f + halfvec;
	}
}

inline void 
_barycentricInterpolation(
	const Vector4f& a, const Vector4f& b, const Vector4f& c, const Vector4f& d, 
	const float wa, const float wb, const float wc, const float wd, 
	Vector4f &r)
{
	r = (a*wa) + (b*wb) + (c*wc) + (d*wd);
}

static void TetSumQuat(const Vector4i *ptet, const int ntet, const Vector4f *pElmQuat, const int nv, Vector4f *pVertQuat)
{
	int i;
	const Vector4f zero(0,0,0,0);
	for (i=0; i<nv; i++) pVertQuat[i] = zero;
	for (i=0; i<ntet; i++){
		const Vector4i tet = ptet[i];
		const Vector4f quat = pElmQuat[i];
		pVertQuat[tet.x]+= quat;
		pVertQuat[tet.y]+= quat;
		pVertQuat[tet.z]+= quat;
		pVertQuat[tet.w]+= quat;
	}
	for (i=0; i<nv; i++) 
		pVertQuat[i].Normalize();
}

static void HexSumQuat(const Vector8i *phex, const int nhex, const Vector4f *pElmQuat, const int nv, Vector4f *pVertQuat)
{
	int i;
	const Vector4f zero(0,0,0,0);
	for (i=0; i<nv; i++) pVertQuat[i] = zero;
	for (i=0; i<nhex; i++){
		const Vector8i tet = phex[i];
		const Vector4f quat = pElmQuat[i];
		pVertQuat[tet.x]+= quat;
		pVertQuat[tet.y]+= quat;
		pVertQuat[tet.z]+= quat;
		pVertQuat[tet.w]+= quat;
		pVertQuat[tet.x1]+= quat;
		pVertQuat[tet.y1]+= quat;
		pVertQuat[tet.z1]+= quat;
		pVertQuat[tet.w1]+= quat;
	}
	for (i=0; i<nv; i++) pVertQuat[i].Normalize();
}


void BarycentricPolarMapping::_computeRotation(const Vector3d *invert)
{
	//init the vertex normals
	if (m_pNormal){
		if (m_pDstObj->m_pVertexNorm==NULL){
			m_pDstObj->ComputeVertexNormals();
			printf("Vertex normal is computed!\n");
		}
	}
	Vector3f *pRefNorm = m_pDstObj->m_pVertexNorm;

	//per elment quaternion computation
	_computeElementQuats(m_pSrcObj, invert, m_elmQuat);

	//compute the avergage;
	const int nsizeV = m_pSrcObj->m_nVertexCount;
	const int nsizeE = m_pSrcObj->m_nElementCount;
	const Vector4i *ptet = (const Vector4i *)m_pSrcObj->m_pElement;
	const Vector8i *phex = (const Vector8i *)m_pSrcObj->m_pElement;
	if (m_pSrcObj->m_nElmVertexCount==4)
		TetSumQuat(ptet, nsizeE, &m_elmQuat[0], nsizeV, &m_vertQuat[0]);
	else if (m_pSrcObj->m_nElmVertexCount==8)
		HexSumQuat(phex, nsizeE, &m_elmQuat[0], nsizeV, &m_vertQuat[0]);
	else
		assert(0);

	//then perform the Barycentric interpolation 
	for (int i=0; i<m_pDstObj->m_nVertexCount; i++){
		const int *refVertexID = m_RefVertex[i].refVertexID;
		const float *refVertexWeight = m_RefVertex[i].refVertexWeight;
		const Vector4f *a = &m_vertQuat[refVertexID[0]];
		const Vector4f *b = &m_vertQuat[refVertexID[1]];
		const Vector4f *c = &m_vertQuat[refVertexID[2]];
		const Vector4f *d = &m_vertQuat[refVertexID[3]];
		const float wa = refVertexWeight[0];
		const float wb = refVertexWeight[1];
		const float wc = refVertexWeight[2];
		const float wd = refVertexWeight[3];
		Vector4f quat;
		_barycentricInterpolation(*a, *b, *c, *d, wa, wb, wc, wd, quat);
		quat.Normalize();
		//save the rotation quaternion if required
		if (m_pQuat) m_pQuat[i] = quat;
		//rotate the normal
		if (m_pNormal){
			float4 quat4(quat.x, quat.y, quat.z, quat.w);
			matrix3x3 rot = quaterionToMatrix33(quat4);
			m_pNormal[i] = rot*(&pRefNorm[i].x); 
		}
	}
}


void BarycentricPolarMapping::updateMapping(const Vector3d* invert, Vector3d* outvert)
{
	const int timerid=0;
	startFastTimer(timerid);
	//parent function to deform all vertices;
	BarycentricMapping::updateMapping(invert, outvert);
	//comp rotation
	_computeRotation(invert);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Software BarycentricPolar comp time is");
}


void BarycentricPolarMapping::updateMapping(const Vector3d* invert, Vector3f* outvert)
{
	const int timerid=0;
	startFastTimer(timerid);
	//parent function to deform all vertices;
	BarycentricMapping::updateMapping(invert, outvert);
	//comp rotation
	_computeRotation(invert);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Software BarycentricPolar comp time is");
}



} // namespace component
} // namespace cia3d

