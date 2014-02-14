//File: BaseRendering.h

#ifndef _INC_BASE_RENDERING_H_
#define _INC_BASE_RENDERING_H_

#include <view3dn/scenegraph.h>
#include <crest/core/BaseObjectDescription.h>


namespace cia3d{
namespace ogl{

class BaseRendering: public CBaseRendering
{
private:
	void _init(CPolyObj *pobj, CBaseMapping *pMapping)
	{
		m_pVisualObj = pobj;
		m_pMapping = pMapping;
	}

public:

	BaseRendering(void): CBaseRendering(){}

	virtual ~BaseRendering(){}

	virtual void parse(ZBaseObjectDescription* arg);
	
};

}//ogl
}//cia3d

#endif
