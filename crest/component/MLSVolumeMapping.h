//File: MLSVolumeMapping.h--Moving Least Square based mapping

#ifndef CIA3D_COMPONENT_MAPPING_MLSVOLUME_MAPPING_H
#define CIA3D_COMPONENT_MAPPING_MLSVOLUME_MAPPING_H

#include <crest/component/MLSMapping.h>

namespace cia3d{
namespace component{


class MLSVolumeMapping : public MLSMapping
{	
private:
	void init(CPolyObj *src, CPolyObj *dst);

public:
	MLSVolumeMapping(void): MLSMapping()
	{
		m_pQuat = NULL;
	}
	
	virtual ~MLSVolumeMapping()
	{
		SafeDeleteArray(m_pQuat);
		m_pQuat = NULL;
	}
	
	virtual void parse(ZBaseObjectDescription* arg)
	{
		MLSMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}

protected:
	virtual EdgeTable *buildEdgeTable(CPolyObj *poly);

	virtual CPointOrgnizer3D* buildPointOrgnizer3D(CPolyObj *poly);

};


} // namespace component
} // namespace cia3d

#endif
