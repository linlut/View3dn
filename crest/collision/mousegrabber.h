//mousegrabber.h

#ifndef __INC_CIAMOUSEGRABBER_H__

#define __INC_CIAMOUSEGRABBER_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <view3dn/pointobj.h>

namespace cia3d{
namespace collision{


class CMouseGrabber: public CPointObj
{

private:
	CPolyObj *m_pTargetObj;		/// The target polyobj to grab
	int m_nTargetVertexID;		/// the target vertex id of the obj
	int m_nRealtimePos;			/// store the real object's position index into this var, an index val
								/// the corresponding vertex's manipulated position is stored in
								/// m_pVertex[0], the first pos
	Vector3d m_realtimePos;		/// store the real object's position into this var
	Vector3d m_initialPos;		/// store the initial mouse manipulated vertex position, in world coord

	void _init(const CPolyObj *pobj, const int vertID);

public:

	CMouseGrabber(void): CPointObj()
	{
		m_pTargetObj = NULL;
		m_nTargetVertexID = 0;
		m_nRealtimePos = -1;
	}

	CMouseGrabber(Vector3d *pVertex, const int nVertex, const float r, const bool allocbuffer=false):
		CPointObj()
	{
		m_pTargetObj = NULL;
		m_nTargetVertexID = 0;
		m_nRealtimePos = -1;
	}

	void getGrabberInfo(CPolyObj **ppTargetObj, int *pTargetVertID)
	{
		*ppTargetObj = m_pTargetObj;
		*pTargetVertID = m_nTargetVertexID;
	}

	int getRealtimePosID(void)
	{
		return m_nRealtimePos;
	}

	int findGrabbingVertex(const double *pmatrix, const Vector3d *pVertexArray, const int n);

	virtual ~CMouseGrabber(void)
	{
	}

	//=====================Drawing functions================================

	virtual void glDraw(const CGLDrawParms &dparms);

	virtual void parse(ZBaseObjectDescription* arg);

	virtual const char *Description(void)
	{
		return "mousegrabber";
	}

};


}//end namespace collison
}//end namespace cia3d


#endif