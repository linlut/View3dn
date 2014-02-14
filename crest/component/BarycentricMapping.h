//File: BarycentricMapping.h--Moving Least Square based mapping

#ifndef CIA3D_COMPONENT_BARYCENTRIC_MAPPING_H
#define CIA3D_COMPONENT_BARYCENTRIC_MAPPING_H

#include <vector>
#include <crest/component/BaseMapping.h>

namespace cia3d{
namespace component{

const int _REFPOINT_NUM = 4;

class BarycentricMapping : public BaseMapping
{	
protected:
	class DeformationRecord{
	public:
		int refVertexID[_REFPOINT_NUM];
		float refVertexWeight[_REFPOINT_NUM];
		
		DeformationRecord(void){};
		DeformationRecord(const Vector4i& ids, const Vector4f &w)
		{
			refVertexID[0] = ids.x;
			refVertexID[1] = ids.y;
			refVertexID[2] = ids.z;
			refVertexID[3] = ids.w;
			refVertexWeight[0] = w.x;
			refVertexWeight[1] = w.y;
			refVertexWeight[2] = w.z;
			refVertexWeight[3] = w.w;
		}
	};

	vector<DeformationRecord> m_RefVertex;

private:
	void init(CPolyObj *src, CPolyObj *dst);

public:
	
	BarycentricMapping(void): BaseMapping(), m_RefVertex(){}
	
	virtual ~BarycentricMapping(){}
	
	virtual void updateMapping(const Vector3d* in, Vector3d* out);

	virtual void updateMapping(const Vector3d* in, Vector3f* out);
	
	void apply(void)
	{
		//updateMapping();
	}
	
	void applyJ(void)
	{
		//updateMapping();
	}
	
	void applyJT(void)
	{
		//updateMapping();
	}

	virtual void parse(ZBaseObjectDescription* arg)
	{
		BaseMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}


protected:

    //bool getShow(const core::objectmodel::BaseObject* m) const { return m->getContext()->getShowMappings(); }
    //bool getShow(const core::componentmodel::behavior::BaseMechanicalMapping* m) const { return m->getContext()->getShowMechanicalMappings(); }
};


} // namespace component
} // namespace cia3d

#endif
