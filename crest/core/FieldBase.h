//
// C++ Interface: FieldBase
//
// Description: 
//
//
#ifndef CIA3D_CORE_OBJECTMODEL_FIELDBASE_H
#define CIA3D_CORE_OBJECTMODEL_FIELDBASE_H

#if !defined(__GNUC__) || (__GNUC__ > 3 || (_GNUC__ == 3 && __GNUC_MINOR__ > 3))
#pragma once
#endif

#include <list>
#include <iostream>

namespace cia3d{
namespace core{

/**
Abstract base class for all fields, independently of their type.*/
class FieldBase{

public:
	/** Constructor
	\param l long name
	\param h help
	\param m true iff the argument is mandatory
	*/
	FieldBase( const char* h)
		: help(h)
		, m_isSet(false)
	{}
	
	/// Base destructor: does nothing.
	virtual ~FieldBase(){}

	/// Read the command line
	virtual bool read( std::string& str ) = 0;
        
	/// Print the value of the associated variable
    virtual void printValue( std::ostream& ) const =0;

 	/// Print the value of the associated variable
    virtual std::string getValueString() const=0;

	/// Print the value type of the associated variable
    virtual std::string getValueTypeString() const=0;

	/// Help message
    const char* help;
              
    /// True if the value has been modified
    inline bool isSet() const { return m_isSet; }
        
protected:
	/// True iff a value has bee read on the command line
	bool m_isSet;

	static std::string decodeTypeName(const std::type_info& t);

    template<class T>
    static std::string typeName(const T* = NULL)
    {
        return decodeTypeName(typeid(T));
    }

};


} // namespace core
} // namespace sofa

#endif
