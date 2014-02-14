# Target is a library: sofaguiqt

SOFA_DIR = ../..
TEMPLATE = lib
include($$SOFA_DIR/sofa.cfg)

TARGET = sofaguiqt$$LIBSUFFIX
CONFIG += $$CONFIGLIBRARIES qt uic3
QT += opengl qt3support
LIBS += $$SOFA_FRAMEWORK_LIBS $$SOFA_MODULES_LIBS
LIBS += $$SOFA_EXT_LIBS

win32 {
	FORMS3 += GUI.ui 
	FORMS3 += BaseGenGraphForm.ui 
	INCLUDEPATH += E:/Qt/3.3.3/include
}
else {
	FORMS += GUI.ui 
	FORMS += BaseGenGraphForm.ui 
}

HEADERS += QtViewer.h \
	       Main2.h \
           RealGUI.h \
           GenGraphForm.h 

SOURCES += QtViewer.cpp \
	   	   Main2.cpp \
           RealGUI.cpp \
           GenGraphForm.cpp 

