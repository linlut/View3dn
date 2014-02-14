//FILE: collision-interface.h

#ifndef _INC_COLLISION_INTERFACE_ZN08012008_H_
#define _INC_COLLISION_INTERFACE_ZN08012008_H_

#include <sysconf.h>
#include <vectorall.h>


//===========================================================
#define _DISPACEMENT_BUFFER_LENGTH 1000
class CCollisionResponseBuffer
{
private:
	static CCollisionResponseBuffer _buffer;

public:
	int DISPACEMENT_BUFFER_LENGTH;
	Vector3d dispbuffer[_DISPACEMENT_BUFFER_LENGTH];
	int indexbuffer[_DISPACEMENT_BUFFER_LENGTH];
	int nBufferLength;

	//a counter for counting the run time
	static int nDisplacementCounter;
	
	//increase the counter
	void increaseDisplacementCounter(void)
	{
		nDisplacementCounter++;
	}

	CCollisionResponseBuffer(void)
	{
		nBufferLength = 0;
		DISPACEMENT_BUFFER_LENGTH = _DISPACEMENT_BUFFER_LENGTH;
	}

	void beginCollision(void)
	{
		nBufferLength = 0;
	}

	void endCollision(void)
	{
		if (nBufferLength>0)
			nDisplacementCounter ++;
	}

	static CCollisionResponseBuffer* getInstance(void)
	{
		return &_buffer;
	}

};
#undef _DISPACEMENT_BUFFER_LENGTH

extern void 
getMouseGrabberPositionConstraints(CSceneGraph &laptool, const vector<Vector3d> *deformedBuf, CCollisionResponseBuffer &colbuf);

extern 
void collisonInstrumentTissue(				//Input parameters:
    const double &curtime,					//current time, in mili-sec
	CSceneGraph &tissue,					//The scene graph with tissue objects in
	CSceneGraph &instrument,				//instruments
	const vector<Vector3d> *pDeformedVerticesBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	const vector<Vector4i> *pDeformedSurfacesBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	const vector<int> *pDeformedSurfacesSizeBuf,	//the deformed vertices, in fortran array (starting at 1, 0 is for omit the adding 1)
	CCollisionResponseBuffer &colbuf				//returned data
	);

#endif