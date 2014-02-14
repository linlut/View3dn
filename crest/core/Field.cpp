//
// C++ Implementation: Field
//
// Description: 
//
//
#include <crest/core/Field.h>
#include <crest/core/Base.h>

namespace cia3d{
namespace core{


std::string FieldBase::decodeTypeName(const std::type_info& t)
{
    return Base::decodeTypeName(t);
}


} // namespace core
} // namespace cia3d
