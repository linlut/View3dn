//File: RigidMapping.h

#ifndef _CIA3D_COMPONENT_MAPPING_RIGIDMAPPING_H
#define _CIA3D_COMPONENT_MAPPING_RIGIDMAPPING_H

#include <vector>
#include <crest/component/BaseMapping.h>
#include <crest/core/BaseObjectDescription.h>

namespace cia3d{
namespace component{

const int NRIGID_VERT = 5;
class RigidMapping : public BaseMapping
{	
protected:
	Vector3d m_refVertex[NRIGID_VERT];
	double m_weight[NRIGID_VERT];
	int m_refVertexID[NRIGID_VERT];

	void init(CPolyObj *src, CPolyObj *dst);

public:
	
	RigidMapping(void): BaseMapping(){}
	
	virtual ~RigidMapping(){}
	
	virtual void updateMapping(const Vector3d* in, Vector3d* out);
	
	virtual void parse(ZBaseObjectDescription* arg)
	{
		BaseMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}

};


} // namespace component
} // namespace cia3d

#endif
