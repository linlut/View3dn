
/*! \file etkQuartic.hpp
    \brief  Various quartic routines.
 */

#ifndef etkQuadric_hpp
#define etkQuadric_hpp 

/// Solve quartic equation of form x**4 + a*x**3 + b*x**2 + c*x + d = 0 
// the modification here is to make it compatible with the Cg language, which
// is a C-like language running on NVIDIA graphics hardware
#include "etkTypes.h"

struct QuarticSolverResult
{
	int num;			//number of roots
	QFLOAT roots[4];	//the four roots
};

QuarticSolverResult etkQuartic (const QFLOAT a, const QFLOAT b, const QFLOAT c, const QFLOAT d);

#endif
