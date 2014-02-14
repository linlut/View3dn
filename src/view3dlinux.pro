HOMEDIR = /afs/ahpcrc.org/home/nanzhang

LIBSRC = $${HOMEDIR}/libsrc

LIBDIR = $${HOMEDIR}/lib

INCLUDEPATH = $${HOMEDIR}/include \
	  $${HOMEDIR}/package/libQGLViewer-1.3.9/ \
	  $${HOMEDIR}/package/qt/include
			  
DESTDIR	= $${HOMEDIR}/bin

DEFINES+= QT_THREAD_SUPPORT

TEMPLATE = app

TARGET 	= view3dn

REUIRES	= opengl

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
!win32: LIBS += $${LIBDIR}/libQGLViewer.a
win32: DEFINES += WIN32

MAINFILES 	=main.cpp glwin.cpp glwin2.cpp meshattrib.cpp view3d.cpp tabdialog.cpp\
                qmeshobj.cpp cia3dobj.cpp tmeshobj.cpp pointobj.cpp polyobj.cpp

HEADERS	= glwin.h tabdialog.h dparmdialogex.h polyobj.h\
        animcontroldialog.ui.h  glwin.h qmeshobj.h \
        animcontroldialogex.h   scenegraph.h\
        cia3dobj.h  cia3dfloor.h        cia3dfloor.h     mirrordialog.ui.h   \ 
        mirrordialogex.h        tmeshobj.h \
        dparmdialog.ui.h        pointobj.h              view3d.h 

SOURCES += $${MAINFILES} $${LIBSRCFILES} 

FORMS	= dparmdialog.ui animcontroldialog.ui mirrordialog.ui

CONFIG += qt thread warn_on release  opengl

