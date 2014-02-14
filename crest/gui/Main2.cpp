#include "QtViewer.h"
#include "RealGUI.h"
#include "Main2.h"

//#include <sofa/simulation/automatescheduler/Automate.h>
//#include <sofa/simulation/automatescheduler/ThreadSimulation.h>
//#include <sofa/simulation/tree/Simulation.h>

#include <crest/core/Factory.h>

#include <iostream>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#ifndef QT_MODULE_QT3SUPPORT
#include <qlistview.h>
#include <qheader.h>
#endif


::GUI* gui = NULL;
QApplication* application = NULL;

//using namespace sofa::simulation::automatescheduler;
const char* progname="";
//using sofa::simulation::tree::GNode;
/*
void dumpGraph(GNode* node, Q3ListViewItem* item)
{
	if (std::string(node->getName(),0,7) != "default")
		item->setText(0, node->getName().c_str());
	Q3ListViewItem* last = NULL;
	for(GNode::ObjectIterator it = node->object.begin(); it != node->object.end(); ++it)
	{
		core::objectmodel::BaseObject* o = *it;
		Q3ListViewItem* child = (last = new Q3ListViewItem(item, last));
		std::string name = sofa::helper::gettypename(typeid(*o));
		std::string::size_type pos = name.find('<');
		if (pos != std::string::npos)
			name.erase(pos);
		if (std::string(o->getName(),0,7) != "default")
		{
			name += "  ";
			name += o->getName();
		}
		child->setText(0, name.c_str());
	}
	for(GNode::ChildIterator it = node->child.begin(); it != node->child.end(); ++it)
	{
		dumpGraph(*it, (last = new Q3ListViewItem(item, last)));
	}
	item->setOpen(true);
}
*/


int MainLoop(const char* pname, const char* filename, bool use_docked_windows)
{
	progname = pname;
	{
		int argc=1;
		char* argv[1];
		argv[0] = strdup(progname);
		application = new QApplication(argc,argv);
		free(argv[0]);
	}
	// create interface
    gui = new RealGUI(filename, use_docked_windows);	
	application->setMainWidget( gui );	
	gui->show();
	return application->exec();
}

bool InsertTab(QWidget* tab, const char* name)
{
	if (gui==NULL) return false;
	gui->tabs->insertTab(tab,name);
	gui->tabs->showPage(tab);
	return true;
}

void Redraw()
{
	if (gui==NULL) return;
	gui->viewer->update();
}

GNode* CurrentSimulation()
{
    if (gui==NULL) return NULL;
    //return gui->viewer->getScene();
	return NULL;
}


void Draw3DText(double x, double y, double z, char *txt)
{
	QGLWidget *pwin = gui->viewer;
	pwin->renderText(x, y, z, QString(txt));
}
