//FILE: mappedmodel.h

#ifndef _CIA3D_INC_MAPPEDMODEL_H
#define _CIA3D_INC_MAPPEDMODEL_H

#include <crest/core/baseobject.h>

namespace cia3d{
namespace core{

template<class TDataTypes>
class MappedModel : public virtual ZBaseObject
{
public:
	typedef TDataTypes DataTypes;
	typedef typename DataTypes::Coord Coord;
	typedef typename DataTypes::Deriv Deriv;
	typedef typename DataTypes::VecCoord VecCoord;
	typedef typename DataTypes::VecDeriv VecDeriv;
	
	virtual ~MappedModel() { }
	
	virtual VecCoord* getX() = 0;
	virtual VecDeriv* getV() = 0;
	
	virtual const VecCoord* getX()  const = 0;
	virtual const VecDeriv* getV()  const = 0;
};


} // namespace zcore
} // namespace cia3d

#endif
