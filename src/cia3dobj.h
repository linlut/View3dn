//tmeshobj.h

#ifndef __INC_CIA3DOBJ_H__
#define __INC_CIA3DOBJ_H__


#include "object3d.h"

#define CIA3D_SUBOBJ_LIMIT 32

class CPolyObj;
class CCia3dObj;

class CCia3dObj: public CObject3D
{
public:
	CObject3D * m_pObjList[CIA3D_SUBOBJ_LIMIT];

private:
	//return 1: success, 0: fail
	int _loadPltFile(const char *fileName);
	int _loadD3PlotFile(const char *fileName);

public:
	CCia3dObj(void): CObject3D()
	{
		int i;
		for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++)
			m_pObjList[i]=NULL;
	}

	virtual ~CCia3dObj()
	{
		int i;
		for (i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
			if (m_pObjList[i])
				delete m_pObjList[i];
			m_pObjList[i] = NULL;
		}
	}

	//in case when there are multiple object in an input, 
	//split them and generate several ones
	int ObjectCount(int objbuff[], const int bufflen);

	int ObjectCount(void)
	{
		const int idsize = 2048;
		int objidbuff[idsize];	
		int n = ObjectCount(objidbuff, idsize);
		return n;
	}

	int UnsplitObjectCount(void)
	{
		int c=0;
		for (int i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
			if (m_pObjList[i])
				c++;
			else
				break;
		}
		return c;
	}

	int GetTotalTriangleCount(void);

    int indexOf(const CObject3D *p) const
    {
        int c=-1;
		for (int i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
            if (m_pObjList[i]==p){
				c=i;
                break;
            }
		}
        return c;
    }

	void addObject(CObject3D * p)
	{
		int n= UnsplitObjectCount();
		assert(n<CIA3D_SUBOBJ_LIMIT);
		m_pObjList[n] = p;

		//take care of bounding box;
		AxisAlignedBox bbox1, bbox2;
		this->GetBoundingBox(bbox1);
		p->GetBoundingBox(bbox1);
		bbox1 =  Union(bbox1, bbox2);
		this->SetBoundingBox(bbox1);
	}

    void removeObject(const CObject3D *p)
    {
        int i=indexOf(p);
        if (i<0) return;
        m_pObjList[i]=NULL;
    }

    int countObjects(void) const
    {
        int c=0;
		for (int i=0; i<CIA3D_SUBOBJ_LIMIT; i++){
            if (m_pObjList[i]) c++;
		}
        return c;
    }

	int ObjectSplit(CCia3dObj * buff[], const int bufflen);

	void scaleObject(const double factor);

	//load file
	virtual int LoadFile(const char *fname, const double factor);

	virtual const char *Description(void) const
	{
		static char* name = "cia3dobj";
		return name;
	}
};


extern int LoadAndSetMultipleObjs(
	const char *fname, 
	CCia3dObj * pCia3dObjs[], 
	const int OBUFFLENGTH, 
	const double scalingfactor);


//extern CPolyObj* loadNeutralMesh(const char *fname);
//extern CPolyObj* loadHMMesh(const char *fname);
//extern CPolyObj* loadPolyFamily(const char *fname);


#endif