
/*! \file etkPlatform.hpp
    \brief Detect compiler and platform
 */

#ifndef etkPlatform_hpp
#define etkPlatform_hpp

  // Define compiler
  // ---------------

  // VCC_COMPILER = Microsoft Visual C++
  // BCC_COMPILER = Borland C++ Builder 
  // GCC_COMPILER = GCC or generic compiler

  #ifdef __WIN32__
    #define BCC_COMPILER
    #define ETK_WIN
  #else
    #ifdef WIN32
      #define VCC_COMPILER
      #define ETK_WIN
    #else
      #define GCC_COMPILER
      #define ETK_UNIX
    #endif
  #endif
#endif
