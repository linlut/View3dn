project(Animation)
cmake_minimum_required(VERSION 2.6.2)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	./../../
)

SET(SOURCES main.cpp animation.cpp)
SET(HEADERS animation.h)
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})

ADD_EXECUTABLE(${PROJECT_NAME} 
	${SOURCES} ${HEADERS} ${HEADERS_MOC} 
)

target_link_libraries(${PROJECT_NAME}
    libQGLViewer
	${QT_LIBRARIES}
	${OPENGL_LIBRARIES}
)

ADD_DEPENDENCIES(${PROJECT_NAME} libQGLViewer)
