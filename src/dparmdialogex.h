
#ifndef __INC_DGDRAWPARMSEX_H__
#define __INC_DGDRAWPARMSEX_H__

#include <assert.h>
#include <string.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "dparmdialog.h"


class CDGDrawParms : public DGDrawParms
{

public:
    CDGDrawParms( CGLDrawParms *pDrawParms, QWidget* parent, const char* name):
		DGDrawParms(parent, name)
	{
		int i;
		const float K=255;
		char buff[256];
		CGLDrawParms *p = pDrawParms;
		Vector3f clr;
		
		//set dlg name
		setCaption(name);

		m_DrawParms = *pDrawParms;
		m_pDrawParms = pDrawParms;
			
		sprintf(buff, "%g %g %g", p->m_cHLineColor.x*K, p->m_cHLineColor.y*K, p->m_cHLineColor.z*K);
		lineEditLineColor->setText(buff);

		sprintf(buff, "%g %g %g", p->m_cBackgroundColor.x*K, p->m_cBackgroundColor.y*K, p->m_cBackgroundColor.z*K);
		lineEditBackgroundColor->setText(buff);

		sprintf(buff, "%g", p->m_fLineWidth);
		lineEditLineWidth->setText(buff);

		sprintf(buff, "%g %g %g", p->m_cObjectColor.x*K, p->m_cObjectColor.y*K, p->m_cObjectColor.z*K);
		lineEditFaceColor->setText(buff);

		radioButtonFillpoly->setChecked(p->m_nDrawType==CGLDrawParms::DRAW_MESH_SHADING);
		radioButtonHiddenline->setChecked(p->m_nDrawType==CGLDrawParms::DRAW_MESH_HIDDENLINE);
		radioButtonLine->setChecked(p->m_nDrawType==CGLDrawParms::DRAW_MESH_LINE);
		radioButtonDrawNothing->setChecked(p->m_nDrawType==CGLDrawParms::DRAW_MESH_NONE);

		checkBoxLighting->setChecked(p->m_bEnableLighting);
		checkBoxDepthtest->setChecked(p->m_bEnableDepthTest);
		checkBoxTexturing->setChecked(p->m_bEnableTexture1D);
		checkBoxSmoothing->setChecked(p->m_bEnableSmoothing);

		checkBoxVBO->setChecked(p->m_bUseVbo);
		checkBoxDispList->setChecked(p->m_bUseDispList);

		checkBoxTexturing->setChecked(p->m_bEnableTexture1D);
		checkBoxTexturing2->setChecked(p->m_bEnableTexture1D);

		checkBoxShowAxis->setChecked(p->m_bShowAixes);
		checkBoxShowCamera->setChecked(m_DrawParms.m_bShowCamera);

		//mapping
		radioButtonLinearMapping->setChecked(p->m_nMappingType == ScalarMappingLinear);
		radioButtonLog2Mapping->setChecked(p->m_nMappingType == ScalarMappingLog2);
		radioButtonLog10Mapping->setChecked(p->m_nMappingType == ScalarMappingLog10);

		//mapping source from ..
		for (i=0; i<CGLDrawParms::NAME_ARRAY_SIZE; i++){
			char *str = p->m_strVarNames[i];
			if (str!=NULL && str[0]!=0)
				comboBoxChooseComp->insertItem(str, -1);
			else
				break;
		}
		int n = (ScalarToTextureCoorMappingType)p->m_nTextureCoorType;
		comboBoxChooseComp->setCurrentItem(n);

		//legend init;
		radioButtonShowLegend->setChecked(p->m_bShowLegend);
		radioButtonShowVertexValue->setChecked(p->m_bShowVertexAttribValue);

		sprintf(buff, "%d", p->m_nLegendLevel);
		lineEditLegendLevel->setText(buff);

		clr = p->m_cLegendTextColor*K;
		sprintf(buff, "%g %g %g", clr.x, clr.y, clr.z);
		lineEditLegendTextColor->setText(buff);

		char * posoptions[] = {"LowLeft", "LowRight", "UpLeft", "UpRight", 0};
		for (i=0; i<4; i++)
			comboBoxLegendPosition->insertItem(posoptions[i]);
		comboBoxLegendPosition->setCurrentItem(p->m_cLegendPosition);

		char * orientoptions[]= {"Horizontal", "Vertical", 0};
		for (i=0; i<2; i++)
			comboBoxLegendOrientation->insertItem(orientoptions[i]);
		comboBoxLegendOrientation->setCurrentItem(p->m_cLegendOrientation);

		//set vertex draw style;
		radioButtonVertPoint->setChecked((p->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_POINT));
		radioButtonVertSphere->setChecked((p->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_SPHERE));		
		radioButtonVertCube->setChecked((p->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_CUBE));		
		radioButtonVertTetra->setChecked((p->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_TETRA));		
		radioButtonVertNone->setChecked((p->m_nDrawVertexStyle==CGLDrawParms::DRAW_VERTEX_NONE));	
		sprintf(buff, "%g", p->m_fVertexSizeScale);
		lineEditVertScale->setText(buff);

		clr = p->m_cVertexColor*K;
		clr.x = (int)(clr.x+0.5f);
		clr.y = (int)(clr.y+0.5f);
		clr.z = (int)(clr.z+0.5f);
		sprintf(buff, "%g %g %g", clr.x, clr.y, clr.z);
		lineEditVertexColor->setText(buff);

		//set vertex texture
		checkBoxVertexTexture->setChecked(p->m_bEnableVertexTexture); 
		checkBoxVertexDir->setChecked(p->m_bEnableVertexRandDir);
	}

};

#endif 
