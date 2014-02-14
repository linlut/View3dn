//File: BaseMapping.h

#ifndef _INC_BASE_MAPPING_H_
#define _INC_BASE_MAPPING_H_

//To include the CBaseMapping class defined in scenegraph.h
#include <view3dn/scenegraph.h>
#include <crest/core/BaseObjectDescription.h>

namespace cia3d{
namespace component{

class BaseMapping: public CBaseMapping
{
private:
	//assign the pointers, plus init the mapping things
	void init(CPolyObj*psrc, CPolyObj*pdst)
	{
		m_pSrcObj = psrc; //source object
		m_pDstObj = pdst; //destination object
	}

public:

	BaseMapping(void): CBaseMapping(){}

	virtual ~BaseMapping(){}

	virtual void parse(ZBaseObjectDescription* arg);

	virtual void topologyChange(void);
	
	virtual void updateMapping(const Vector3d* vin, Vector3d* vout)
	{
		assert(0);
	}

	virtual void updateMapping(const Vector3d* vin, Vector3f* vout)
	{
		printf("This function is not implemented by BaseMapping!\n");
		assert(0);
	}

};

}//component
}//cia3d

#endif
