#ifndef _CIA3D_SOFACONFIG_H_
#define _CIA3D_SOFACONFIG_H_

#ifdef WIN32
#define NOMINMAX
#include <windows.h>

#define snprintf _snprintf
#endif

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // required to get M_PI from math.h
#endif

#define sofa_concat(a,b) sofa_do_concat(a,b)
#define sofa_do_concat(a,b) sofa_do_concat2(a,b)
#define sofa_do_concat2(a,b) a##b

#define CIA3D_DECL_CLASS(name) extern "C" { int sofa_concat(class_,name) = 0; }
#define CIA3D_LINK_CLASS(name) extern "C" { extern int sofa_concat(class_,name); int sofa_concat(link_,name) = sofa_concat(class_,name); }

#endif
