SOFA_DIR=.
TEMPLATE = subdirs

include($$SOFA_DIR/sofa.cfg)

SUBDIRS += base
SUBDIRS += collision
SUBDIRS += core
SUBDIRS += data
SUBDIRS += io
SUBDIRS += ogl
SUBDIRS += system
SUBDIRS += xml
SUBDIRS += gui

