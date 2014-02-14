//
// C++ Interface: Field.h
//
// Description:
//
//
#ifndef CIA3D_CORE_OBJECTMODEL_FIELD_H
#define CIA3D_CORE_OBJECTMODEL_FIELD_H

#if !defined(__GNUC__) || (__GNUC__ > 3 || (_GNUC__ == 3 && __GNUC_MINOR__ > 3))
#pragma once
#endif

#include <crest/core/FieldBase.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

namespace cia3d{
namespace core{

typedef std::istringstream istrstream;
typedef std::ostringstream ostrstream;

/**
Pointer to data, readable and writable from/to a string.*/
template < class T = void* >
class Field : public FieldBase
{
public:
    /** Constructor
    \param t a pointer to the value
    \param h help on the field
    */
    Field( T* t, const char* h )
            : FieldBase(h)
            , ptr(t)
    {}

    virtual ~Field()
    {}

    inline void printValue(std::ostream& out) const ;
    inline std::string getValueString() const ;
    inline std::string getValueTypeString() const ; //{ return std::string(typeid(*ptr).name()); }
    inline T* beginEdit()
    {
        m_isSet = true;
        return ptr;
    }
    inline void endEdit()
    {}
    inline void setValue(const T& value )
    {
        *beginEdit()=value;
        endEdit();
    }
    inline const T& getValue() const
    {
        return *ptr;
    }

protected:
    /// Pointer to the parameter
    T* ptr;


    /** Try to read argument value from an input stream.
        Return false if failed
    */
    inline bool read( std::string& s )
    {
        if (s.empty())
            return false;
        //std::cerr<<"Field::read "<<s.c_str()<<std::endl;
        istrstream istr( s.c_str() );
        istr >> *ptr;
        if( istr.fail() )
        {
            //std::cerr<<"field "<<getName<<" could not read value: "<<s<<std::endl;
            return false;
        }
        else
        {
            m_isSet = true;
            return true;
        }
    }


}
;

/// Specialization for reading strings
template<>
inline
bool Field<std::string>::read( std::string& str )
{
    *ptr = str;
    m_isSet = true;
    return true;
}

/// General case for printing default value
template<class T>
inline
void Field<T>::printValue( std::ostream& out=std::cout ) const
{
    out << *ptr << " ";
}

/// General case for printing default value
template<class T>
inline
std::string Field<T>::getValueString() const
{
    ostrstream out;
    out << *ptr;
    return out.str();
}

template<class T>
inline
std::string Field<T>::getValueTypeString() const
{
    return FieldBase::typeName(ptr);
}


} // namespace core
} // namespace cia3d

#endif
