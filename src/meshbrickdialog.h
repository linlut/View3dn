#ifndef _INC_MESHBRICK_DIALOG1362ag0904_H_
#define _INC_MESHBRICK_DIALOG1362ag0904_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshBrickDialog: public CMeshPrimitiveDialog
{
private:
	static Vector3d m_vCenter;
	static Vector3d m_vLowleft;
	static Vector3d m_vUpright;
	static int m_nx;
	static int m_ny;
	static int m_nz;

public:

public:
    CMeshBrickDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshBrickDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif