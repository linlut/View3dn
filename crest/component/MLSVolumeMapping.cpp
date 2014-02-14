//File: MLSVolumeMapping.cpp

#include <crest/core/Config.h>
#include <crest/core/ObjectFactory.h>
#include <crest/component/MLSVolumeMapping.h>
#include <view3dn/edgetable.h>
#include <view3dn/pointorgnizer.h>

namespace cia3d{
namespace component{

//CIA3D_DECL_CLASS(MLSVolumeMapping);

static int MLSVolumeMappingClass = 
cia3d::core::RegisterObject("MLSVolumeMapping")
.add<MLSVolumeMapping>()
.addAlias("MLSVolumeMapping")
;

//source obj: the simulation object
//Destination obj: the visual object
void MLSVolumeMapping::init(CPolyObj *src, CPolyObj *dst)
{
	MLSMapping::init(src, dst);
	if (dst){
		const int nv = dst->m_nVertexCount;
		m_pQuat = new Vector4f[nv];
		assert(m_pQuat!=NULL);
	}
}

EdgeTable* MLSVolumeMapping::buildEdgeTable(CPolyObj *poly)
{
	EdgeTable *et = NULL;
	Vector3f *pVertex = NULL; 
	const int nv = poly->m_nVertexCount;
	int* ptri = poly->m_pElement; 
	const int ntri = poly->m_nElementCount;
	if (ptri==NULL || ntri==0){
		//fprintf(stderr, "Error (in buildEdgeTable): the mesh is not a solid mesh!\n");
		//return NULL;
		return MLSMapping::buildEdgeTable(poly);
	}

	const int nvpt = poly->m_nElmVertexCount;
	const bool refinemesh=false;
	const int isSolid =true;
	et = new EdgeTable(isSolid, pVertex, nv, ptri, ntri, nvpt, refinemesh);
	assert(et!=NULL);
	et->buildEdgeTable();
	return et;
}


CPointOrgnizer3D* MLSVolumeMapping::buildPointOrgnizer3D(CPolyObj *poly)
{
	CPointOrgnizer3D *grid=NULL;
	const int nv = poly->m_nVertexCount;
	int *pconnectivity = poly->m_pPolygon;
	grid = new CPointOrgnizer3D(poly->m_pVertex, poly->m_nVertexCount);
	assert(grid!=NULL);
	return grid;
}


} // namespace component
} // namespace cia3d

