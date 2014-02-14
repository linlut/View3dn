project(View3dn)
cmake_minimum_required(VERSION 2.6.2)


#--------------------------------------------------------------------------------
# This sets the windows build that will need the special winmain@16 call. Qt provides
#  this for us in the qtmain.lib file. Using this cmake code will ensure we have it
#  linked into our build. Not needed on Unix/OS X/Linux.
IF (WIN32)
    SET (QT_USE_QTMAIN TRUE)
    SET (UNIX_SYS_INCLUDE_DIR "")
ELSE(WIN32)
    SET (UNIX_SYS_INCLUDE_DIR "/usr/X11/include/")
ENDIF (WIN32)
#--------------------------------------------------------------------------------

ADD_DEFINITIONS(${QT_DEFINITIONS})
#INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
    ${UNIX_SYS_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GLEW_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${QGLVIEWER_INCLUDE_DIR}
    ${MESHRENDER_INCLUDE_DIR}
    #${CMAKE_SOURCE_DIR}/extern/PropertyEditor/src
)

SET(LIB_SOURCES 
    ${CMAKE_SOURCE_DIR}/libsrc/memmgr.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/drawparms.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/objrw.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/ply.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/triangle_mesh.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/triangle_mesh2.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/triangle_mesh3.cpp
    ${CMAKE_SOURCE_DIR}/libsrc/ztime.cpp
)
SOURCE_GROUP(libsrc FILES ${LIB_SOURCES} )

SET(SOURCES
${LIB_SOURCES} 
#bmp2avi.cpp
#cia3ddlg.cpp
cia3dfile.cpp
cia3dobj.cpp
clipplane.cpp
d3plot.cpp
dynamicmesh.cpp
edgetable.cpp
fortranarray.cpp
fortranarray2.cpp
fortranarray3.cpp
gleps.cpp
gllight.cpp
#glpreviewwin.cpp
glwin.cpp
glwin2.cpp
glwineps.cpp
grasper.cpp
hexobj.cpp
#hlineview.cpp
import-ansys.cpp
lineobj.cpp	
listviews.cpp
main.cpp
mainfunc.cpp	
marchingtet.cpp
materialtree.cpp
#mesh2dcircle_dlg.cpp
#mesh2dcircleshell_dlg.cpp
#mesh2drect_dlg.cpp
#mesh2dtri_dlg.cpp
#meshattrib.cpp
#meshbrickdialog.cpp
#meshbrickdialog2.cpp
#meshcbrickdialog.cpp
#meshconedialog.cpp
#meshcylinderdialog.cpp
#meshcylindershelldialog.cpp
#meshfrustumdialog.cpp
#meshprismdialog.cpp
#meshspheredialog.cpp
#meshsphereshelldialog.cpp
#meshsurfbox_dlg.cpp
#meshsurfcone_dlg.cpp
#meshsurfcyl_dlg.cpp
#meshsurffrustum_dlg.cpp
#meshsurfsphere_dlg.cpp
#meshtetradialog.cpp
objectlistview.cpp
op_dyn.cpp
op_imp.cpp
op_output.cpp
pointobj.cpp
polyobj.cpp
qmeshobj.cpp
rodobj.cpp
scenegraph.cpp
scenegraphstate.cpp
scenenode.cpp
shapefunc.cpp
sphere16.cpp
springcontainer.cpp
tabdialog.cpp
tetraobj.cpp
texturedqmeshobj.cpp
texturedtmeshobj.cpp
tmeshobj.cpp
trusslink.cpp
vertexinfo.cpp
vertexsmoothing.cpp
view3d.cpp
volumemeshio.cpp
xmldlgex.cpp
#xmlpropertylist.cpp			
)

SET(HEADERS 
animcontroldialog.ui.h
animcontroldialogex.h
#cia3ddlg.h
cia3dfile.h
cia3dfloor.h
cia3dobj.h
dlinkedlist.h
doxymain.h
dparmdialog.ui.h
dparmdialogex.h
dynamicmesh.h
edgetable.h
facetable.h
fortranarray.h
gleps.h
gllight.h
#glpreviewwin.h
glwin.h
glwineps.h
grasper.h
hexobj.h
hlineview.h
import-ansys.h
lineobj.h
listviews.h
marchingtet.h
materialtree.h
#mesh2dcircle_dlg.h
#mesh2dcircleshell_dlg.h
#mesh2drect_dlg.h
#mesh2dtri_dlg.h
#meshbrickdialog.h
#meshbrickdialog2.h
#meshcbrickdialog.h
#meshconedialog.h
#meshcylinderdialog.h
#meshcylindershelldialog.h
#meshfrustumdialog.h
#meshprimitivedialog.ui.h
#meshprismdialog.h
#meshspheredialog.h
#meshsphereshelldialog.h
#meshsurfbox_dlg.h
#meshsurfcone_dlg.h
#meshsurfcyl_dlg.h
#meshsurffrustum_dlg.h
#meshsurfsphere_dlg.h
#meshtetradialog.h
mirrordialog.ui.h
mirrordialogex.h
nametableitem.h
objectlistview.h
op_dyn.h
op_imp.h
op_output.h
pointobj.h
pointorgnizer.h
polyobj.h
qmeshobj.h
rodobj.h
scenegraph-deformpatch.h
scenegraph.h
scenegraphstate.h
scenenode.h
shapefunc.h
sphere16.h
springcontainer.h
tabdialog.h
tetraobj.h
texturedqmeshobj.h
texturedtmeshobj.h
tmeshobj.h
trusslink.h
vertexinfo.h
vertexsmoothing.h
view3d.h
volumemeshio.h
xmldlg.ui.h
xmldlgex.h
#xmlpropertylist.h
)

SET(HEADERS_QT 
#cia3ddlg.h
glwin.h
listviews.h
materialtree.h
#meshprimitivedialog.h
#mirrordialog.h
objectlistview.h
scenegraph.h
#scenegraph2.h
tabdialog.h
view3d.h
#xmlpropertylist.h
)

SET(UIS 
  animcontroldialog.ui	
  mirrordialog.ui
  dparmdialog.ui 
  #meshprimitivedialog.ui	
  xmldlg.ui
)

QT_WRAP_CPP(View3dn HEADERS_MOC ${HEADERS_QT})
QT_WRAP_UI(View3dn UIS_HEADER UIS_CPP ${UIS})

ADD_EXECUTABLE(${PROJECT_NAME} MACOSX_BUNDLE WIN32
    ${SOURCES} 
    ${HEADERS} 
    ${HEADERS_MOC} 
    ${UIS_HEADER}
    ${UIS_CPP}
)

ADD_DEPENDENCIES(${PROJECT_NAME} 
    glew
    libQGLViewer
    libMeshRender
)

ADD_DEFINITIONS(
    -DPROJ_PATH="${CMAKE_SOURCE_DIR}"
    -DUSE_QT_LIB
    -DQGLVIEWER_STATIC
    -DGLEW_STATIC
)

LINK_DIRECTORIES( 
    ${LINK_DIRECTORIES} 
    ${GLEW_LIBRARY_DIRS}
    ${QT_LIBRARY_DIR}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    glew
    libQGLViewer
    libMeshRender
    #PropertyEditor
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/lib/glut32.lib
)


#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")
IF(APPLE)
  SET(plugin_dest_dir ${PROJECT_NAME}.app/Contents/MacOS)
  SET(qtconf_dest_dir ${PROJECT_NAME}.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS ${PROJECT_NAME} 
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)

#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
SET(DIRS ${QT_LIBRARY_DIRS})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
INSTALL(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
SET(CPACK_BINARY_DRAGNDROP ON)
INCLUDE(CPack)
