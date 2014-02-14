#ifndef _INC_MESHSLANTBRICK_DIALOG13xx_H_
#define _INC_MESHSLANTBRICK_DIALOG13xx_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshSlantBrickDialog: public CMeshPrimitiveDialog
{
private:
	static Vector3d m_vVertex[8];
	static int m_nx;
	static int m_ny;
	static int m_nz;

public:

public:
    CMeshSlantBrickDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshSlantBrickDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif