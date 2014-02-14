
/*! \file etkQuartic.hpp
    \brief  Various quartic routines.
    (Original author: Don Herbison-Evans)
 */

#ifndef etkQuadric_hpp
#define etkQuadric_hpp 1

/// Solve quartic equation of form x**4 + a*x**3 + b*x**2 + c*x + d = 0 
int etkQuartic (const double& a, const double& b, const double& c, const double& d, double rts[4]);

#endif
