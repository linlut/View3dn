//File: IdentityMapping.h
#ifndef _INC_IDENTITY_MAPPING_H_
#define _INC_IDENTITY_MAPPING_H_

#include <iostream>
#include <crest/component/BaseMapping.h>
#include <vectorall.h>
#include <crest/core/BaseObjectDescription.h>

namespace cia3d{
namespace component{

class IdentityMapping : public BaseMapping
{
private:
	void _init(void)
	{
		assert(m_pSrcObj != NULL);
		assert(m_pDstObj != NULL);
		if (m_pSrcObj->m_nVertexCount != m_pDstObj->m_nVertexCount){
			std::cout<<"Vertex array size not equal!\n";
			assert(0);
		}
	}

public:
	
	IdentityMapping(void): BaseMapping(){}
	
	virtual ~IdentityMapping(){}	
		
	//Update the mapping whenever the source object changes
	virtual void updateMapping(const Vector3d* in, Vector3d* out)
	{
		if (m_pSrcObj == NULL || m_pDstObj == NULL){
			std::cout << "Source or destination object not initilized!\n";
			return;
		}
		const int nv = m_pSrcObj->m_nVertexCount;
		for (int i=0; i<nv; i++) out[i]=in[i];
	}

	virtual void parse(ZBaseObjectDescription* arg)
	{
		BaseMapping::parse(arg);
		_init();
	}
};

} // namespace component
} // namespace cia3d


#endif
