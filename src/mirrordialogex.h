#ifndef __INC_MIRROR_DIAG_EX_H__
#define __INC_MIRROR_DIAG_EX_H__

#include "mirrordialog.h"

class CMirrorObjectDialog : public MirrorObjectDialog
{

public:
	CMirrorObjectDialog( QWidget * parent, CGLDrawParms *d0, CGLDrawParms ** d1 ):
		MirrorObjectDialog(parent, "Options for mirrors",  true, 0)
	{
    	_pDrawParms = d0;
    	_ppDrawParmsM3 = d1;

		Vector3d * p0 = &_pDrawParms->m_vPickedVertex0;
		Vector3d * p1 = &_pDrawParms->m_vPickedVertex1;
		{
			QString s1, s2, s3;
			s1.setNum(p0->x);
			s2.setNum(p0->y);
			s3.setNum(p0->z);
			lineEditP0->setText(s1+ " " + s2 + " " + s3);
		}
		{
			QString s1, s2, s3;
			s1.setNum(p1->x);
			s2.setNum(p1->y);
			s3.setNum(p1->z);
			lineEditP1->setText(s1+ " " + s2 + " " + s3);
		}

		checkBoxM1->setChecked(_ppDrawParmsM3[0]->m_bValidDrawParms);				
		checkBoxM2->setChecked(_ppDrawParmsM3[1]->m_bValidDrawParms);
		checkBoxM3->setChecked(_ppDrawParmsM3[2]->m_bValidDrawParms);
	}
};

#endif