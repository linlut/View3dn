//File: MLSMapping.h--Moving Least Square based mapping

#ifndef CIA3D_COMPONENT_MAPPING_MLS_MAPPING_H
#define CIA3D_COMPONENT_MAPPING_MLS_MAPPING_H

#include <vector>
#include <crest/component/BaseMapping.h>


class EdgeTable;
class CPointOrgnizer3D;


namespace cia3d{
namespace component{

const int MLS_REFPOINT_NUM = 4;


class MLSDeformationRecord{
public:
	int refVertexID[MLS_REFPOINT_NUM];
	float refVertexWeight[MLS_REFPOINT_NUM];
};


class MLSMapping : public BaseMapping
{	
protected:
	std::vector<MLSDeformationRecord> m_RefVertex;

	void init(CPolyObj *src, CPolyObj *dst);

public:
	
	MLSMapping(void): BaseMapping(), m_RefVertex(){}
	
	virtual ~MLSMapping(){}
	
	virtual bool canDeformNormal(void)
	{
		return true;
	}
	
	virtual void parse(ZBaseObjectDescription* arg)
	{
		BaseMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}

	virtual void updateMapping(const Vector3d* vin, Vector3d* vout);

	virtual void updateMapping(const Vector3d* vin, Vector3f* vout);

protected:

	virtual EdgeTable *buildEdgeTable(CPolyObj *poly);

	virtual CPointOrgnizer3D* buildPointOrgnizer3D(CPolyObj *poly);

    //bool getShow(const core::objectmodel::BaseObject* m) const { return m->getContext()->getShowMappings(); }
    //bool getShow(const core::componentmodel::behavior::BaseMechanicalMapping* m) const { return m->getContext()->getShowMechanicalMappings(); }
};


} // namespace component
} // namespace cia3d

#endif
