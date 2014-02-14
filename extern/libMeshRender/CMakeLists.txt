######################################################
# This file exports: ${QGLVIEWER_INCLUDE_DIR}
#
######################################################
project(libMeshRender)
cmake_minimum_required(VERSION 2.6.2)
MESSAGE("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
MESSAGE("PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
MESSAGE("EXECUTABLE_OUTPUT_PATH = ${EXECUTABLE_OUTPUT_PATH}")
 
FIND_PACKAGE(OpenGL REQUIRED)
#FIND_PACKAGE(GLEW REQUIRED)
ADD_DEFINITIONS(
    -DGLEW_STATIC
    -D_USE_MATH_DEFINES
    -DPROJ_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
)

SET(INC_HEADERS
    src/inc/matrix3x3.h
    src/inc/vector2f.h
    src/inc/vector2i.h
    src/inc/vector3d.h
    src/inc/vector3f.h
    src/inc/vector3i.h
    src/inc/vector4d.h
    src/inc/vector4f.h
    src/inc/vector4i.h
    src/inc/vector5d.h
    src/inc/vector6d.h
    src/inc/vector7d.h
	src/inc/vectorall.h
)

SOURCE_GROUP("Internal Files" FILES ${INC_HEADERS} )

SET(HEADERS
    ${INC_HEADERS}
	include/MeshRenderLib.h
	include/MeshRender.h
    src/GLSLProgramObject.h
	src/MeshRenderingContext.h
	src/NormalCompresser.h
    src/RenderingContext.h
    src/ShadersManagement.h
    src/ShaderPath.h
    src/ShaderUtils.h
)

SET(SOURCES 
    src/main.cpp
    src/GLSLProgramObject.cpp
	src/MeshRender.cpp
	src/MeshRenderingContext.cpp
	src/NormalCompresser.cpp
    src/ShaderPath.cpp	
    src/ShadersManagement.cpp
    src/RenderingContext.cpp
)

SET(SHADER_FILES
    src/shaders/common_uniforms.glsl
    src/shaders/flatshading_vp.glsl
    src/shaders/flatshading_fp.glsl
    src/shaders/gouraudshading_vp.glsl
    src/shaders/gouraudshading_fp.glsl
    src/shaders/hiddenline_gp.glsl
    src/shaders/normal_decompression.glsl
    src/shaders/phongshading_vp.glsl
    src/shaders/phongshading_fp.glsl
    src/shaders/shading.glsl
)

SOURCE_GROUP("GLSL Files" FILES ${SHADER_FILES} )

INCLUDE_DIRECTORIES(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"    
    ${GLEW_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
)

#Build a tool "glsl2c" to convert all glsl files into a C++ header file
ADD_EXECUTABLE(glsl2c "${CMAKE_CURRENT_SOURCE_DIR}/src/glsl2c.cpp")
set_target_properties( glsl2c 
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXECUTABLE_OUTPUT_PATH} 
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXECUTABLE_OUTPUT_PATH} 
)

#Build the lib for mesh rendering
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${SHADER_FILES})

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND "${EXECUTABLE_OUTPUT_PATH}/glsl2c" ARGS -d ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_CURRENT_SOURCE_DIR}/src/AllShaders.h ${SHADER_FILES}
    COMMENT "Compiling GLSL shaders into .h file")
                   
#Library exports
SET(MESHRENDER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)

