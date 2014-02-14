//File: IdentityMapping.cpp

#include <crest/core/Config.h>
#include <crest/component/IdentityMapping.h>
#include <crest/core/ObjectFactory.h>

namespace cia3d{
namespace component{

//using namespace cia3d::component;

CIA3D_DECL_CLASS(IdentityMapping);

//typedef  ObjectCreator< IdentityMapping > ObjectCreator4IdentityMapping;

int IdentityMappingClass = 
cia3d::core::RegisterObject("Mapping where child points are the same as parent points")
.add< IdentityMapping >()
.addAlias("IdentityMapping")
;


} // namespace component
} // namespace sofa

