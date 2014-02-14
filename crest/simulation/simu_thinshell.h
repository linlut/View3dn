//FILE: simu_thinshell.h
#ifndef _INC_SIMUTHINSHELL_ZN2008_H
#define _INC_SIMUTHINSHELL_ZN2008_H

#include <assert.h>
#include <vectorall.h>
#include "thinshell_element.h"
#include "thinshell3_element.h"
#include "thinshell4_element.h"
#include "simu_shell.h"
#include <geomath.h>
#include <view3dn/edgetable.h>


extern void _decideTriangleRemainingNodes(const int *pelm, const int elm0, const int elm1, Vector8i& quad);
extern void _decideQuadRemainingNodes(const int *pelm, const int elm0, const int elm1, Vector8i& quad);


#define T CThinshellElement
#define CSimuThinshellT CSimuThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 

#define T CThinshellSimpShearElement
#define CSimuThinshellT CSimuSimpShearThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 

#define T CThinshellSpringElement
#define CSimuThinshellT CSimuSpringThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 

#define T CThinshellGyrodElement
#define CSimuThinshellT CSimuGyrodThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 

#define T CThinshellElementFEM
#define CSimuThinshellT CSimuFEMThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 

#define T CThinshellBridsonElement
#define CSimuThinshellT CSimuBridsonThinshell
#include "simu_thinshell.inl"
#undef T 
#undef CSimuThinshellT 


#endif