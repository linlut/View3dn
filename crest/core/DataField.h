//
// C++ Interface: Field
//
// Description:
//
#ifndef SOFA_CORE_OBJECTMODEL_DATAFIELD_H
#define SOFA_CORE_OBJECTMODEL_DATAFIELD_H

#if !defined(__GNUC__) || (__GNUC__ > 3 || (_GNUC__ == 3 && __GNUC_MINOR__ > 3))
#pragma once
#endif

#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <crest/core/FieldBase.h>

namespace cia3d{
namespace core{

typedef std::istringstream istrstream;
typedef std::ostringstream ostrstream;

/**
Pointer to data, readable and writable from/to a string.*/
template < class T = void* >
class DataField : public cia3d::core::FieldBase
{
public:

    /** Constructor
    \param helpMsg help on the field
     */
    DataField( const char* helpMsg=0 )
    : FieldBase(helpMsg)
    {}

    /** Constructor
    \param value default value
    \param helpMsg help on the field
     */
    DataField( const T& value, const char* helpMsg=0 )
    : FieldBase(helpMsg), m_value(value)
    {}

    virtual ~DataField(){}

    inline void setHelpMsg( const char* msg ){ this->help = msg; }
    inline void printValue(std::ostream& out) const ;
    inline std::string getValueString() const ;
    inline std::string getValueTypeString() const; // { return std::string(typeid(m_value).name()); }
    inline T* beginEdit()
    {
        m_isSet = true;
        return &m_value;
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
        return m_value;
    }

protected:
    /// Value
    T m_value;


    /** Try to read argument value from an input stream.
        Return false if failed
    */
    inline bool read( std::string& s )
    {
        if (s.empty())
            return false;
        //std::cerr<<"Field::read "<<s.c_str()<<std::endl;
        istrstream istr( s.c_str() );
        istr >> m_value;
        if( istr.fail() )
        {
            return false;
        }
        else
        {
            m_isSet = true;
            return true;
        }
    }
};


/// Specialization for reading strings
template<>
inline bool DataField<std::string>::read( std::string& str )
{
    m_value = str;
    m_isSet = true;
    return true;
}

/// General case for printing default value
template<class T>
inline void DataField<T>::printValue( std::ostream& out=std::cout ) const
{
    out << m_value << " ";
}

/// General case for printing default value
template<class T>
inline std::string DataField<T>::getValueString() const
{
    ostrstream out;
    out << m_value;
    return out.str();
}

template<class T>
inline std::string DataField<T>::getValueTypeString() const
{
    return FieldBase::typeName(&m_value);
}


} // namespace core
} // namespace cia3d

#endif
