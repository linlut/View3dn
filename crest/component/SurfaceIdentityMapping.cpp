//File: IdentityMapping.cpp

#include <crest/core/Config.h>
#include <crest/component/SurfaceIdentityMapping.h>
#include <crest/core/ObjectFactory.h>

namespace cia3d{
namespace component{

using namespace cia3d::core;

CIA3D_DECL_CLASS(SurfaceIdentityMapping);


int SurfaceIdentityMappingClass = 
cia3d::core::RegisterObject("Mapping surface points are the same as the other object")
.add< SurfaceIdentityMapping >()
;


SurfaceIdentityMapping::~SurfaceIdentityMapping()
{
	correspondingIndex.clear();
}
	
//Mapping interface, brief apply the transformation from a model to an other 
//model (like apply displacement from BehaviorModel to VisualModel
void SurfaceIdentityMapping::init(CPolyObj *src, CPolyObj *dst)
{
	//set pointers
	m_pSrcObj = src; m_pDstObj = dst;
	if (m_pSrcObj->m_nVertexCount < m_pDstObj->m_nVertexCount){
		std::cout<<"Vertex array size not enough!\n";
		assert(0);
	}

	//setup the correspondence, first compute the error tolerance
	Vector3d lowleft, upright;
	src->ComputeBoundingBox(lowleft, upright);
	Vector3d dist = upright - lowleft;
	const double MAXDIM = _MAX3_(dist.x, dist.y, dist.z);
	const double TOLR = (dist.x+dist.y+dist.z - MAXDIM)*0.5 * 1e-6;
	const double TOLR2 = TOLR*TOLR;

	correspondingIndex.clear();
	const int nsize = dst->m_nVertexCount;
	correspondingIndex.resize(nsize);
	const int srccount = src->m_nVertexCount;
	const Vector3d *pSrcVert = src->m_pVertex;
	const Vector3d *pDstVert = dst->m_pVertex;

	for (int i = 0; i < nsize; ++i){
		bool found = false;
		for (int j = 0;  j < srccount; ++j ){
			if (Distance2(pDstVert[i], pSrcVert[j]) < TOLR2 ){
				correspondingIndex[i] = j;
				found = true;
				break;
			}
		}
		if (!found){
			std::cerr<<"ERROR(SurfaceIdentityMapping): point "<<i<<" not found.\n";
			correspondingIndex[i] = 0;
		}
	}

}
		
//Update the mapping whenever the source object changes
void SurfaceIdentityMapping::updateMapping(const Vector3d* in, Vector3d* out)
{
	if (m_pSrcObj == NULL || m_pDstObj == NULL){
		std::cout << "Source or destination object not initilized!\n";
		return;
	}
	const int nv = m_pDstObj->m_nVertexCount;
	for (int i=0; i<nv; i++){
		const int j = correspondingIndex[i];
		out[i]=in[j];
	}
}

} // namespace component
} // namespace sofa

