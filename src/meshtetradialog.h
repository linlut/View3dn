#ifndef _INC_MESHTETRA_DIALOG_XXX_H_
#define _INC_MESHTETRA_DIALOG_XXX_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshTetraDialog: public CMeshPrimitiveDialog
{
private:	
	static int m_nX;
	static int m_nY;
	static int m_nZ;
	static Vector3d m_vVertex[4];

public:

public:
    CMeshTetraDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshTetraDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif