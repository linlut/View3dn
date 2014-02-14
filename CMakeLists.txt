project(View3d)
cmake_minimum_required(VERSION 2.6.2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

if(WIN32)
  SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /wd4251")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
  SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MP")
  SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:CONSOLE")
else(WIN32)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
  IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
  ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  IF(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ppc -arch i686 -arch x86_64")
  ENDIF(APPLE)
endif(WIN32)

#--------------------------------------------------------------------------------
# This sets the windows build that will need the special winmain@16 call. Qt provides
#  this for us in the qtmain.lib file. Using this cmake code will ensure we have it
#  linked into our build. Not needed on Unix/OS X/Linux.
IF (WIN32)
  SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32)

#--------------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
  #current fix for compiliting on linux to enable include of stdint.h in glext.h
  add_definitions(-D__SCO__)
endif(UNIX AND NOT APPLE)

#--------------------------------------------------------------------
# Output paths for the executables and libraries
#
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Static (including dll's import) library path.")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Module library path.")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Executable (including dll) path.")

#--------------------------------------------------------------------
# If the generator is VS8, set the compiler to VS8
#
if(${CMAKE_GENERATOR} MATCHES "Visual Studio 8 2005")
  if(NOT ${CMAKE_CXX_COMPILER} MATCHES "8")
    string(REPLACE "9.0" "8" CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
    string(REPLACE "9.0" "8" CMAKE_C_COMPILER   ${CMAKE_C_COMPILER})
  endif(NOT ${CMAKE_CXX_COMPILER} MATCHES "8")
endif(${CMAKE_GENERATOR} MATCHES "Visual Studio 8 2005")

#--------------------------------------------------------------------
# Disable deprecation warnings for standard C functions
#
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

option(DEBUG_POSTFIX "Add the post-fix d to library DLLs when in debug mode" OFF)

#--------------------------------------------------------------------
# Set preprocessor definition for ?? to indicate whether 
# to use the debug postfix
if(DEBUG_POSTFIX)
  add_definitions(-D_DEBUG_POSTFIX)
endif(DEBUG_POSTFIX)

FIND_PACKAGE(OpenGL REQUIRED)
#FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui QtOpenGL Qt3Support)
FIND_PACKAGE(Qt3 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTXML TRUE)

ADD_DEFINITIONS(${QT_DEFINITIONS})
#INCLUDE(${QT_USE_FILE})

#--------------------------------------------------------------------
SET(QGLVIEWER_DIR "${CMAKE_SOURCE_DIR}/extern/libQGLViewer-2.3.17") 
ADD_SUBDIRECTORY("${QGLVIEWER_DIR}/QGLViewer")

#--------------------------------------------------------------------
IF(WIN32)
#FIND_PACKAGE(GLEW)
  IF (GLEW_FOUND)
    SET(GLEW "")
    SET(glew "")
  ELSE(GLEW_FOUND)
    SET(GLEW "glew")
    SET(glew "glew")
    SET(GLEW_HOME_DIR "${CMAKE_SOURCE_DIR}/extern/glew-1.8.0" CACHE PATH "GLEW home path.")
    SET(GLEW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extern/glew-1.8.0/include" CACHE PATH "GLEW include path.")
    ADD_SUBDIRECTORY(extern/glew-1.8.0)
    SET(GLEW_FOUND true)
  ENDIF(GLEW_FOUND)
ELSE(WIN32)
  SET(GLEW "glew")
  SET(glew "glew")
  SET(GLEW_HOME_DIR "${CMAKE_SOURCE_DIR}/extern/glew-1.8.0" CACHE PATH "GLEW home path.")
  SET(GLEW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extern/glew-1.8.0/include" CACHE PATH "GLEW include path.")
  ADD_SUBDIRECTORY(extern/glew-1.8.0)
ENDIF(WIN32)

#--------------------------------------------------------------------
#add_subdirectory(extern/PropertyEditor)

#--------------------------------------------------------------------
ADD_SUBDIRECTORY(extern/libMeshRender)

ADD_SUBDIRECTORY(src)
