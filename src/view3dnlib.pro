include(view3dn-inc.pro)

SOURCES += $${MAINFILES} $${LIBSRCFILES} $${HLINEFILES}

DESTDIR = $${HOMEDIR}/lib

TARGET  = View3dn

TEMPLATE = lib

win32 {	
  staticlib {
    # DEFINES *= VIEW3D_STATIC
  } else {
    # CONFIG *= dll
    # DEFINES *= CREATE_QGLVIEWER_DLL
  }
}

#CONFIG += qt warn_on release opengl thread create_prl
#CONFIG += qt warn_on release incremental link_prl opengl thread create_prl moc dll
CONFIG += qt warn_on release incremental link_prl opengl thread create_prl moc


