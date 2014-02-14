//File: RigidMapping.cpp

#include <crest/core/Config.h>
#include <crest/component/RigidMapping.h>
#include <crest/core/ObjectFactory.h>
#include <crest/extlibs/horn_registration/etkRegistration.hpp>
#include <crest/core/ObjectLib.h>

namespace cia3d{
namespace component{

using namespace cia3d::core;

CIA3D_DECL_CLASS(RigidMapping);

int RigidMappingClass = 
cia3d::core::RegisterObject("RigidMapping")
.add<RigidMapping>()
.addAlias("RigidMapping")
;


void RigidMapping::init(CPolyObj *src, CPolyObj *dst)
{
	m_pSrcObj = src;
	m_pDstObj = dst;

	//now, class based init
	int i;
	const Vector3d *p = src->m_pVertex;
	const int nv = src->m_nVertexCount;

	//init, use the first vertex and last vertex to init
	Vector3d center(0,0,0);		//geo center of the src obj
	for (i=0; i<nv; i++) center += p[i];
	center *= (1.0/nv);
	for (i=0; i<NRIGID_VERT; i++){
		const int j = NRIGID_VERT-1;
		m_refVertex[i] = p[j];
		m_refVertexID[i] = j;
	}

	//if there are not enough vertices
	if (nv < NRIGID_VERT){
		for (i=0; i<nv; i++){
			m_refVertex[i] = p[i];
			m_refVertexID[i] = i;
		}
	}
	else{
	//there are enough vertices;
		float step = 1.0f*nv/NRIGID_VERT - 0.010f;
		for (i=0; i<NRIGID_VERT; i++){
			const int j = (int(i*step + 0.5f))%nv;
			m_refVertex[i] = p[j];
			m_refVertexID[i] = j;
		}
	}

	//compute weights
	const double damp = 1e-2;
	computeSquareDistanceWeights(center, m_refVertex, NRIGID_VERT, damp, m_weight);
}

void RigidMapping::updateMapping(const Vector3d* in, Vector3d* out)
{
	matrix3x3 rot;
	float3 trans;
	if (m_pSrcObj == NULL || m_pDstObj == NULL) return;

	{//get the transformation R & T
		Vector3d q[NRIGID_VERT];
		for (int i=0; i<NRIGID_VERT; i++)
			q[i]= in[m_refVertexID[i]];
		float4 Quat;
		etkRegistration reg;
		reg.hornRegistration(m_refVertex, q, m_weight, NRIGID_VERT, Quat, rot, trans);
	}
	{//use R & T
		if (out==NULL) return;
		const int nv = m_pDstObj->m_nVertexCount;
		Vector3d *pVertex = m_pDstObj->m_pVertex;
		for (int i=0; i<nv; i++){
			float3 v(pVertex[i].x, pVertex[i].y, pVertex[i].z);
			float3 r = rot*v + trans;
			out[i] = Vector3d(r.x, r.y, r.z);
		}
	}
}

} // namespace component
} // namespace cia3d
