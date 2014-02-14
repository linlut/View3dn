
#ifndef SOFA_GUI_QT_GENGRAPHFORM_H
#define SOFA_GUI_QT_GENGRAPHFORM_H

#include "BaseGenGraphForm.h"
#ifdef QT_MODULE_QT3SUPPORT
#include "QProcess"
#include "QStringList"
#else
#include "qprocess.h"
#include "qstringlist.h"
#endif

#include <iostream>
#include <list>
#include <map>
#include <set>

#include <crest/simulation/GNode.h>


class GenGraphForm : public BaseGenGraphForm
{
	Q_OBJECT
public:
	GenGraphForm();

	void setScene(GNode* scene);

public slots:
	virtual void change();
	virtual void doBrowse();
	virtual void doExport();
	virtual void doDisplay();
	virtual void doClose();
	virtual void taskFinished();
	virtual void changeFilter();
	virtual void setFilter();

protected:
	QString exportedFile;
	GNode* graph;
	std::list<QStringList> tasks;
	QProcess* currentTask;

	void addTask(QStringList argv);
	void runTask();
	void killAllTasks();

	std::map<std::string, std::set<std::string> > presetFilters;
	bool settingFilter;
	std::set<std::string> getCurrentFilter();
};


#endif
