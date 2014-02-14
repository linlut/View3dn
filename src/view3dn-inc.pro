HOMEDIR = /users/nanzhang

DEFINES += QT_THREAD_SUPPORT

LIBSRC = $${HOMEDIR}/libsrc

LIBDIR = $${HOMEDIR}/lib

INCLUDEPATH = \
	$${HOMEDIR}/include \
	$${HOMEDIR}/package/libQGLViewer/libQGLViewer-2.2.3-1
	      			  			  
LIBSRCFILES = \
	$${LIBSRC}/drawparms.cpp \
	$${LIBSRC}/matrix.cpp \
	$${LIBSRC}/memmgr.cpp \
	$${LIBSRC}/ply.c \
	$${LIBSRC}/triangle_mesh.cpp \
	$${LIBSRC}/triangle_mesh2.cpp \
	$${LIBSRC}/triangle_mesh3.cpp 

# Used by Qt4 only
QT += xml opengl

#======== Additional changed to be fixed================
win32{ 
	#INCLUDEPATH += C:/Progra~1/Micros~2/VC98/Include
	LIBS += $${LIBDIR}/qglviewer223.lib
  	CONFIG *= rtti
	DEFINES += WIN32
}

macx{
	INCLUDEPATH += /sw/include/qt /usr/X11R6/include 
 	LIBS += $${LIBDIR}/libQGLViewer.a -lGL -lGLU
}

unix{
	INCLUDEPATH += $${HOMEDIR}/package/qt/include
	LIBS += $${LIBDIR}/libQGLViewer.a
}

MAINFILES = \	
								 meshconedialog.cpp \
								 meshcylinderdialog.cpp \
	                             meshcylindershelldialog.cpp \
	cia3dfile.cpp                meshfrustumdialog.cpp \
	cia3dobj.cpp                 \
	clipplane.cpp                meshprismdialog.cpp \
								 meshspheredialog.cpp \
	dynamicmesh.cpp              meshsphereshelldialog.cpp \
	fortranarray.cpp             meshsurfbox_dlg.cpp \
	fortranarray2.cpp            meshsurfcone_dlg.cpp \
	fortranarray3.cpp            meshsurfcyl_dlg.cpp \
	gleps.cpp                    meshsurffrustum_dlg.cpp \
	glpreviewwin.cpp             meshsurfsphere_dlg.cpp \
	glwin.cpp                    meshtetradialog.cpp \
	glwin2.cpp                   \
	glwineps.cpp                 op_dyn.cpp \
	hexobj.cpp                   op_imp.cpp \
	hlineview.cpp                op_output.cpp \
	listviews.cpp                pointobj.cpp \
	                             polyobj.cpp \
	mainfunc.cpp                 qmeshobj.cpp \
	marchingtet.cpp              scenegraph.cpp \
	materialtree.cpp             shapefunc.cpp \
	mesh2dcircle_dlg.cpp         tabdialog.cpp \
	mesh2dcircleshell_dlg.cpp    tetraobj.cpp \
	mesh2drect_dlg.cpp           tmeshobj.cpp \
	mesh2dtri_dlg.cpp            vertexinfo.cpp \
	meshattrib.cpp               vertexsmoothing.cpp \
	meshbrickdialog.cpp          view3d.cpp \
	meshbrickdialog2.cpp         \
	meshcbrickdialog.cpp         xmldlgex.cpp	
	
HLINEFILES = \
	../hline/cut.cpp ../hline/hline_t.cpp ../hline/hline_removal.cpp \
	../hline/int.cpp ../hline/jline.cpp ../hline/maths.cpp \
	../hline/maths2.cpp ../hline/rmline.cpp \
	../hline/sameline.cpp ../hline/stack.cpp 

HEADERS	= \
	animcontroldialog.ui.h     meshprimitivedialog.ui.h \
	animcontroldialogex.h      meshprismdialog.h \
	                           meshspheredialog.h \
	cia3dfile.h                meshsphereshelldialog.h \
	cia3dfloor.h               meshsurfbox_dlg.h \
	cia3dobj.h                 meshsurfcone_dlg.h \
	dlinkedlist.h              meshsurfcyl_dlg.h \
					           meshsurffrustum_dlg.h \
							   meshsurfsphere_dlg.h \
	dparmdialog.ui.h           meshtetradialog.h \
	dparmdialogex.h             \
	dynamicmesh.h              mirrordialog.ui.h \
	fortranarray.h             mirrordialogex.h \
	gleps.h                    nametableitem.h \
	glpreviewwin.h             op_dyn.h \
	glwin.h                    op_imp.h \
	glwineps.h                 op_output.h \
	hexobj.h                   pointobj.h \
	hlineview.h                pointorgnizer.h \
	listviews.h                polyobj.h \
	marchingtet.h              qmeshobj.h \
	materialtree.h             scenegraph.h \
	mesh2dcircle_dlg.h         shapefunc.h \
	mesh2dcircleshell_dlg.h    tabdialog.h \
	mesh2drect_dlg.h           tetraobj.h  \
	mesh2dtri_dlg.h            tmeshobj.h  \
	meshbrickdialog.h          vertexinfo.h \
	meshbrickdialog2.h         vertexsmoothing.h \
	meshcbrickdialog.h         view3d.h \
	meshconedialog.h           \
	meshcylinderdialog.h       xmldlg.ui.h \
	meshcylindershelldialog.h  xmldlgex.h \
	meshfrustumdialog.h 

FORMS = \
	dparmdialog.ui animcontroldialog.ui xmldlg.ui \
	mirrordialog.ui meshprimitivedialog.ui
