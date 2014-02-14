HOMEDIR = /users/nanzhang

LIBSRC = $${HOMEDIR}/libsrc

LIBDIR = $${HOMEDIR}/lib

INCLUDEPATH = $${HOMEDIR}/include \
			  $${HOMEDIR}/package/libQGLViewer-1.3.9/
			  
DESTDIR	= /users/nanzhang/project/view3d/objs

TARGET 	= view3d

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
!win32: LIBS += $${LIBDIR}/qglviewer.a
win32: DEFINES += WIN32

MAINFILES 	= main.cpp glwin.cpp glwin2.cpp meshattrib.cpp view3d.cpp tabdialog.cpp

HEADERS	= glwin.h tabdialog.h

SOURCES += $${MAINFILES} $${LIBSRCFILES} 

FORMS	= dparmdialog.ui animcontroldialog.ui mirrordialog.ui

CONFIG += qt winapp warn_on debug opengl

