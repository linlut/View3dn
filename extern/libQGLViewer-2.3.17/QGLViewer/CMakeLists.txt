######################################################
# This file exports: ${QGLVIEWER_INCLUDE_DIR}
#
######################################################
project(libQGLViewer)
cmake_minimum_required(VERSION 2.6.2)
SET(CMAKE_AUTOMOC ON)
#FIND_PACKAGE(OpenGL REQUIRED)
#FIND_PACKAGE(Qt3 REQUIRED)
ADD_DEFINITIONS(${QT_DEFINITIONS})
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTXML TRUE)

SET(VRender_SOURCES
	./VRender/BackFaceCullingOptimizer.cpp
	./VRender/BSPSortMethod.cpp
	./VRender/EPSExporter.cpp
	./VRender/Exporter.cpp
	./VRender/FIGExporter.cpp
	./VRender/gpc.cpp
	./VRender/NVector3.cpp
	./VRender/ParserGL.cpp
	./VRender/Primitive.cpp
	./VRender/PrimitivePositioning.cpp
	./VRender/TopologicalSortMethod.cpp
	./VRender/Vector2.cpp
	./VRender/Vector3.cpp
	./VRender/VisibilityOptimizer.cpp
	./VRender/VRender.cpp
)

SET(VRender_HEADERS
	./VRender/AxisAlignedBox.h
	./VRender/Exporter.h
	./VRender/gpc.h
	./VRender/NVector3.h
	./VRender/Optimizer.h
	./VRender/ParserGL.h
	./VRender/Primitive.h
	./VRender/PrimitivePositioning.h
	./VRender/SortMethod.h
	./VRender/Types.h
	./VRender/Vector2.h
	./VRender/Vector3.h
	./VRender/VRender.h
)

SET(SOURCES 
	${VRender_SOURCES}
	camera.cpp
	constraint.cpp
	frame.cpp
	keyFrameInterpolator.cpp
	manipulatedCameraFrame.cpp
	manipulatedFrame.cpp
	mouseGrabber.cpp
	qglviewer.cpp
	quaternion.cpp
	saveSnapshot.cpp
	vec.cpp
)

SET(HEADERS
	${VRender_HEADERS}
	camera.h
	config.h
	constraint.h
	domUtils.h
	frame.h
	keyFrameInterpolator.h
	manipulatedCameraFrame.h
	manipulatedFrame.h
	mouseGrabber.h
	qglviewer.h
	quaternion.h
	vec.h
)

SET(FORMS VRenderInterface.Qt3.ui ImageInterface.Qt3.ui)
SET(RESOURCES qglviewer-icon.xpm)

QT_WRAP_CPP(libQGLViewer HEADERS_MOC ${HEADERS})
MESSAGE("HEADERS_MOC = ${HEADERS_MOC}")

QT_WRAP_UI(libQGLViewer UI_HEADERS UI_HEADERS_CPP ${FORMS})
MESSAGE("UI_HEADERS = ${UI_HEADERS}")
MESSAGE("UI_HEADERS_CPP = ${UI_HEADERS_CPP}")

SET(QGLVIEWER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../" PARENT_SCOPE) 
INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR} 
    ${QGLVIEWER_INCLUDE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
)
##INCLUDE(${QT_USE_FILE})

ADD_DEFINITIONS("-DQGLVIEWER_STATIC")
ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS} ${HEADERS_MOC} ${UI_HEADERS} ${UI_HEADERS_CPP})
