//File: IdentityMapping.h
#ifndef _INC_IDENTITY_MAPPING_H_
#define _INC_IDENTITY_MAPPING_H_

#include <iostream>
#include <crest/component/BaseMapping.h>
#include <vectorall.h>
#include <crest/core/BaseObjectDescription.h>

namespace cia3d{
namespace component{

class SurfaceIdentityMapping : public BaseMapping
{
private:
	std::vector<unsigned int> correspondingIndex;

	void init(CPolyObj *src, CPolyObj *dst);

public:
	
	SurfaceIdentityMapping(void): BaseMapping(){}
	
	virtual ~SurfaceIdentityMapping();
			
	//Update the mapping whenever the source object changes
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
