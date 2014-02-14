
#ifndef _CREST_GUI_QT_REALGUI_H
#define _CREST_GUI_QT_REALGUI_H

#include "GUI.h"
#include "QtViewer.h"
#include <crest/simulation/GNode.h>

class GraphListenerQListView;
class CPolyObj;
//class Q3DockWindow;

class RealGUI : public GUI
{

private:
	CGLDrawParms m_DrawParms;		//rendering configuration
	CSceneGraph m_SceneGraph;		//scengraph, objects that can be deformed in CIA3d
	CSceneGraph m_LaptoolSceneGraph;//scengraph for laproscopic instrument objects
	CSceneGraph m_BgSceneGraph;		//scengraph for Background objects
	std::string sceneFileName;		//scene file name

	void clearAll(void);			//clean the buffers and object for quit/destruction

public:
    RealGUI(const char* filename=NULL, const bool use_docked_windows = true);
    ~RealGUI();

    virtual void fileOpen(const char* filename, bool resetView=true);
    virtual void fileSaveAs(const char* filename);
	virtual void setScene(CGLDrawParms* pDraw, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, const char* filename, bool resetView=true);
    virtual void setTitle( const char* windowTitle );

//public slots:
    virtual void fileNew();
    virtual void fileOpen();
    //virtual void fileSave();
    virtual void fileSaveAs();
    virtual void fileReload();
    //virtual void filePrint();
    virtual void fileExit();
    virtual void saveXML();
    //virtual void editUndo();
    //virtual void editRedo();
    //virtual void editCut();
    //virtual void editCopy();
    //virtual void editPaste();
    //virtual void editFind();
    //virtual void helpIndex();
    //virtual void helpContents();
    //virtual void helpAbout();

	//Nan Zhang added funcs
	virtual void showEntireScene(void);
	virtual void runTetCSG(void);
	virtual void playpause(bool);
	virtual void stepAnimation(void);
	virtual void saveSnapshot(void);
	virtual void saveCamera(void);
	virtual void loadCamera(void);
	virtual void exportGraph(void);

protected:
	GraphListenerQListView* graphListener;
};

#endif // 
