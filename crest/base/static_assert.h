//  (C) Copyright John Maddock 2000.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/static_assert for documentation.
#ifndef _ZSTATIC_ASSERT_H
#define _ZSTATIC_ASSERT_H
#include <crest/base/config_assert.h>


#ifndef BOOST_STATIC_ASSERT

template <bool x> struct STATIC_ASSERTION_FAILURE{};
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct static_assert_test{};

#if defined(_MSC_VER)
#define BOOST_STATIC_ASSERT( B ) \
	typedef static_assert_test<\
      sizeof(STATIC_ASSERTION_FAILURE< (bool)( B ) >)>\
         sofa_concat(boost_static_assert_typedef_,__LINE__)

#elif (defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)) || (defined(__GNUC__) && (__GNUC__ == 3) && ((__GNUC_MINOR__ == 3) || (__GNUC_MINOR__ == 4)))
// agurt 15/sep/02: a special care is needed to force Intel C++ and GCC 3.3/3.4 to issue an error 
// instead of warning in case of failure
# define BOOST_STATIC_ASSERT( B ) \
    typedef char sofa_concat(boost_static_assert_typedef_, __LINE__) \
        [ STATIC_ASSERTION_FAILURE< (bool)( B ) >::value ]

#elif defined(__sgi)
// special version for SGI MIPSpro compiler
#define BOOST_STATIC_ASSERT( B ) \
   BOOST_STATIC_CONSTANT(bool, \
     sofa_concat(boost_static_assert_test_, __LINE__) = ( B )); \
   typedef static_assert_test<\
     sizeof(STATIC_ASSERTION_FAILURE< \
       sofa_concat(boost_static_assert_test_, __LINE__) >)>\
         sofa_concat(boost_static_assert_typedef_, __LINE__)

#else
// generic version
#define BOOST_STATIC_ASSERT( B ) \
   typedef static_assert_test<\
      sizeof(STATIC_ASSERTION_FAILURE< (bool)( B ) >)>\
         sofa_concat(boost_static_assert_typedef_, __LINE__)
#endif

#endif

#endif
