HOMEDIR = /users/nanzhang

LIBSRC = $${HOMEDIR}/libsrc

LIBDIR = $${HOMEDIR}/lib

INCLUDEPATH = $${HOMEDIR}/include \
	      /sw/include/qt \
	      /usr/X11R6/include/ \
	      $${HOMEDIR}/package/libQGLViewer-2.1.1-7/
			  
DESTDIR	= $${HOMEDIR}/bin

TARGET 	= view3dn

LIBSRCFILES = \
			$${LIBSRC}/drawparms.cpp \
			$${LIBSRC}/matrix.cpp \
			$${LIBSRC}/memmgr.cpp \
			$${LIBSRC}/ply.c \
			$${LIBSRC}/triangle_mesh.cpp \
			$${LIBSRC}/triangle_mesh2.cpp \
			$${LIBSRC}/triangle_mesh3.cpp 

### Additional libs to be linked
win32: LIBS += $${LIBDIR}/qglviewer.lib
!win32: LIBS += $${LIBDIR}/libQGLViewer.dylib -lGL -lGLU
win32: DEFINES += WIN32

MAINFILES 	= glwin.cpp glwin2.cpp meshattrib.cpp view3d.cpp tabdialog.cpp\
		qmeshobj.cpp cia3dobj.cpp tmeshobj.cpp \
		mainfunc.cpp hexobj.cpp \
		pointobj.cpp polyobj.cpp tetraobj.cpp \
		dynamicmesh.cpp scenegraph.cpp listviews.cpp \
		gleps.cpp glwineps.cpp hlineview.cpp\
		marchingtet.cpp vertexinfo.cpp vertexsmoothing.cpp materialtree.cpp
	
HLINEFILES	= ../hline/cut.cpp ../hline/hline_t.cpp ../hline/hline_removal.cpp \
		../hline/int.cpp ../hline/jline.cpp ../hline/maths.cpp\
		../hline/maths2.cpp ../hline/rmline.cpp ../hline/sameline.cpp ../hline/stack.cpp 

HEADERS	= glwin.h tabdialog.h dparmdialogex.h polyobj.h\
	animcontroldialog.ui.h  glwin.h qmeshobj.h \
	animcontroldialogex.h   scenegraph.h\
	cia3dobj.h cia3dfloor.h mirrordialog.ui.h   \ 
        mirrordialogex.h tmeshobj.h tetraobj.h \
	dparmdialog.ui.h pointobj.h view3d.h \
	dynamicmesh.h listviews.h hexobj.h \
	gleps.h glwineps.h hlineview.h \
	marchingtet.h materialtree.h vertexinfo.h vertexsmoothing.h


SOURCES += main.cpp $${MAINFILES} $${LIBSRCFILES} $${HLINEFILES}

FORMS	= dparmdialog.ui animcontroldialog.ui mirrordialog.ui

CONFIG += qt app warn_off release  opengl

