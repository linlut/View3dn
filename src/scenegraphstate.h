//FILE: scenegraphstate.h

#ifndef __INC_SCENEGRAPH_STATE_H_
#define __INC_SCENEGRAPH_STATE_H_

#include "scenegraph.h"


//===============================================================

void addMirroredSceneNodes(CSceneGraph &m_SceneGraph, const Vector3f v[3]);

void addMirroredSceneNodes(CSceneGraph &m_SceneGraph);

void addRotAngleSceneNodes(CSceneGraph &m_SceneGraph, const float ang, const Vector3f v[2]);

void addRotAngleSceneNodes(CSceneGraph &m_SceneGraph, const float ang);



class CSceneGraphState
{

private:
	//scene graph state push/pop for loading consecutive objs;
	bool bm;	//has mirror 
	bool b90;	//has rot 90?
	bool b180;	//has rot 180?
	bool b270;	//has rot 270?
	CSceneNode *pbaknode;
	CSceneNode *pactivenode;
	CSceneNode * pActiveSceneNode4Face;	//active face node;
	CSceneNode * pActiveSceneNode4Edge;	//active line node;
	int fid;
	int lineid0, lineid1;

	Vector3f mirrorplane[3];
	Vector3f rotline[2];

	int nNodeCount;
	CSceneGraph *pScenegraph;

public:

	void restoreOldSceneGraph(void);

	void clearSceneGraphState(void);

	CSceneGraphState(CSceneGraph &s);

	~CSceneGraphState(void)
	{
		clearSceneGraphState();
	}

	void useSceneGraphState(const bool recomputeTexture1DRange=true);
};



#endif