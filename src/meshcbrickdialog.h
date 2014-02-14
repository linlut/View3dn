#ifndef _INC_MESH_CBK_DLG111_H_
#define _INC_MESH_CBK_DLG111_H_

#include <vector3d.h>
#include "meshprimitivedialog.h"



class CMeshCbrickDialog: public CMeshPrimitiveDialog
{
private:	
	static Vector3d m_vVertex[20];
	static Vector3d m_vTrans;
	static int m_nArcDiv;
	static int m_nThkDiv;
	static int m_nHgtDiv;

public:

public:
    CMeshCbrickDialog(QWidget* parent = 0, const char* name = 0);

    virtual ~CMeshCbrickDialog(void);

    virtual void prepareNameTable(void);

	virtual void onApply(void);
};

#endif