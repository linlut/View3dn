//File: Main2.h

#ifndef CIA3D_GUI_QT_MAIN_H
#define CIA3D_GUI_QT_MAIN_H

#include <crest/simulation/GNode.h>

class QWidget;

int MainLoop(const char* argv0, 
             const char* filename = NULL,
             bool use_docked_windows = false);

bool InsertTab(QWidget* tab, const char* name);

GNode* CurrentSimulation();

void Redraw();

extern const char* progname;


#endif
