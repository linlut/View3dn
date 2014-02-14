include(view3dn-inc.pro)

SOURCES += main.cpp $${MAINFILES} $${LIBSRCFILES} $${HLINEFILES}

DESTDIR	= $${HOMEDIR}/bin

TARGET 	= view3dn

CONFIG += app qt thread warn_off release opengl

