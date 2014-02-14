cmake_minimum_required(VERSION 2.8)

project(libdeform)

#--------------------------------------------------------------------
# Define the compiler flags
##
if(WIN32)
  add_definitions(-DINVENTOR_EXPORTS)
endif(WIN32)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DDARWIN)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


#--------------------------------------------------------------------
# Define the include directories
##
include_directories(
  	.
)

#--------------------------------------------------------------------
# Build projects inside subdirectories
##
set(PUBLIC_HEADERS
    maths.h   
    matrix2d.h  
    matrix2f.h  
    matrix3x3.h      
    memmgr.h
    vector2f.h vector4d.h 
    vector2i.h 
    vector3d.h vector3f.h vector3i.h
    vector4d.h vector4f.h vector4i.h  
    virtualarray.h  
    virtualbuffer.h
    vector6d.h
    vectorall.h
)

set(HEADERS
    CGSolver.h            
    element_tetfem2.h       save_stiffness.h   tetra_element.h
    element_tetfem3.h       
    shear_element.h    
    GNode.h               element_trianglepair.h  simu_engine.h      
    QuadArray.h           gspring_element.h       simu_entity.h      
    QuadProg++.h          gyrod_lut.h             simu_shell.h       
    constraints.h         mechanical_property.h   simu_springobj.h   
    element_base.h        obj_hexfem.h            simu_staticrod.h   truss_element.h
    element_cubefem.h     obj_hexshell.h          simu_tetra2.h      truss_thread.h
    element_gyrod.h       obj_spring.h            simu_thinshell.h   vertexring_element.h
    element_hexshell.h    obj_tetfem.h            simu_thinshell2.h  zdiag_matrix.h
    element_hexshell2.h   obj_tetfem2.h           simu_trussobj.h    zicprecond_matrix.h
    element_shear.h       obj_vertexring.h        sparseMatrix.h     
    element_simp_shear.h  obj_vertexringshell.h   spring_element.h   
    element_tetfem.h      obj_vertexringsolid.h   stiffimage.h  	
    thinshell_element.h
    thinshell2_element.h  
    thinshell3_element.h
    thinshell4_element.h
    time_integration.h	
    zsparse_matrix.h
    zsparse_solver.h
)

set(SOURCES
    element_tetfem3.cpp       simu_tetra2.cpp
    element_trianglepair.cpp  simu_thinshell.cpp
    gspring_element.cpp       simu_thinshell2.cpp
    gyrod_lut.cpp             simu_trussobj.cpp
    meshfileio.cpp            sparseMatrix.cpp
    obj_hexfem.cpp            spring_element.cpp
    obj_hexshell.cpp          tetra_element.cpp
    obj_spring.cpp            
    obj_tetfem.cpp             
    obj_tetfem2.cpp            
    CGSolver.cpp                     
    
    QuadArray.cpp              
    QuadProg++.cpp          rod3d.cpp                  
    constraints.cpp         shear_element.cpp         
    element_cubefem.cpp     simmu-core.cpp            
    element_gyrod.cpp       simu_demo_sig09.cpp       
    element_hexshell.cpp    simu_engine.cpp           
    element_hexshell2.cpp   simu_entity.cpp           truss_element.cpp
    element_shear.cpp       simu_shell.cpp            truss_thread.cpp
    element_simp_shear.cpp  simu_springobj.cpp        vertexring_element.cpp
    element_tetfem.cpp      simu_staticrod.cpp        
    element_tetfem2.cpp     simu_test.cpp 
    obj_vertexring.cpp            
    obj_vertexringshell.cpp
    obj_vertexringsolid.cpp
    thinshell_element.cpp
    thinshell2_element.cpp
    thinshell3_element.cpp
    thinshell3_element_fullangle.cpp
    thinshell4_element.cpp
    time_integration.cpp
    zicprecond_matrix.cpp
    zsparse_matrix.cpp
)


#--------------------------------------------------------------------
# Compile the library
#
add_library(${PROJECT_NAME} static
	${SOURCES} 
	${HEADERS} 
    ${PUBLIC_HEADERS}
)

set(EXE_VERSION "")    
set(CMAKE_DEBUG_POSTFIX "d")
if (WIN32)
    set(EXE_NAME ${PROJECT_NAME}${PLATFORM_BINTYPE}${EXE_VERSION})
	set_target_properties(${PROJECT_NAME} PROPERTIES
		VERSION ${EXE_VERSION}
		CLEAN_DIRECT_OUTPUT 1
		OUTPUT_NAME ${EXE_NAME}
		DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)	
else(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
		VERSION ${EXE_VERSION}
		SOVERSION ${EXE_VERSION}
		CLEAN_DIRECT_OUTPUT 1
		DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)
endif(WIN32)

#--------------------------------------------------------------------
# Compile the executable
#
if(WIN32)
  #add_custom_command(TARGET ${PROJECT_NAME}
  #  PRE_BUILD COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../gensources.bat)
else(WIN32)
  ##add_executable(ppp tools/ppp/ppp.c++)
  #add_custom_command(TARGET ${PROJECT_NAME}
  #  PRE_BUILD COMMAND gensources.sh)
endif(WIN32)

#--------------------------------------------------------------------
# Link the library
#
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Static (including dll's import) library path.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "Module library path.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Executable (including dll) path.")

if(WIN32)
    set (OPENGL_LIBRARIES "opengl32.lib glu32.lib")
else(WIN32)

endif(WIN32)

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
)

#--------------------------------------------------------------------
# Install the library
#

######################set installation#########################
install(TARGETS ${PROJECT_NAME} 
    CONFIGURATIONS Debug LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH}
    CONFIGURATIONS Debug ARCHIVE DESTINATION ${LIBRARY_OUTPUT_PATH}
    CONFIGURATIONS Debug RUNTIME DESTINATION ${EXECUTABLE_OUTPUT_PATH}
)
install(TARGETS ${PROJECT_NAME} 
    CONFIGURATIONS Release LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH}
    CONFIGURATIONS Release ARCHIVE DESTINATION ${LIBRARY_OUTPUT_PATH}    
    CONFIGURATIONS Release RUNTIME DESTINATION ${EXECUTABLE_OUTPUT_PATH}
)

if (WIN32)
    set (PDBFILED ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/debug/${EXE_NAME}${CMAKE_DEBUG_POSTFIX}.pdb)
    install(FILES ${PDBFILED} CONFIGURATIONS Debug DESTINATION ${EXECUTABLE_OUTPUT_PATH})
    #set (PDBFILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/release/${EXE_NAME}.pdb)
    #install(FILES ${PDBFILE} CONFIGURATIONS Release DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()

