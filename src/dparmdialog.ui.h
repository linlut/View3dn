/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qfiledialog.h>
#include <qapplication.h>
#include <qimage.h>

//void DGDrawParms::onCheckBoxShadow(){}

void DGDrawParms::onRadioButtonLine()
{
	if (radioButtonLine->isChecked())
		m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_LINE;
}

void DGDrawParms::onRadioButtonHiddenline()
{
	if (radioButtonHiddenline->isChecked())
		m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_HIDDENLINE;
}


void DGDrawParms::onRadioButtonFill()
{
	if (radioButtonFillpoly->isChecked())
		m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_SHADING;
}


void DGDrawParms::onRadioButtonPoint()
{
//	if (radioButtonVertex->isChecked())
//		m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_POINT_EMBOSS;
}


void DGDrawParms::onCheckBoxLighting()
{
	m_DrawParms.m_bEnableLighting = checkBoxLighting->isChecked();
}


void DGDrawParms::onCheckBoxDepthtest()
{
	m_DrawParms.m_bEnableDepthTest = checkBoxDepthtest->isChecked();
}


void DGDrawParms::onCheckBoxSmoothing()
{
	m_DrawParms.m_bEnableSmoothing = checkBoxSmoothing->isChecked();
}


void DGDrawParms::onCheckBoxTexturing()
{
	bool f = checkBoxTexturing->isChecked();
	m_DrawParms.m_bEnableTexture1D = f;
	checkBoxTexturing2->setChecked(f);
    buttonLoadTexture->setEnabled(f);
	m_DrawParms.m_bRecomputeTexture1DParameter = f;
	m_DrawParms.m_bRecomputeTexture1DRange = true;
}

void DGDrawParms::onCheckBoxVBO()
{
	m_DrawParms.m_bUseVbo = checkBoxVBO->isChecked();
}

void DGDrawParms::onCheckBoxDispList()
{
	m_DrawParms.m_bUseDispList = checkBoxDispList->isChecked();
}

void DGDrawParms::onLineEditLineColor()
{
	float r, g, b;
	const float K = 1.0f/255;
	QString s = lineEditLineColor->text();
	sscanf(s.ascii(), "%f %f %f", &r, &g, &b);
	m_DrawParms.m_cHLineColor.x = r*K, 
	m_DrawParms.m_cHLineColor.y = g*K, 
	m_DrawParms.m_cHLineColor.z = b*K;
}


void DGDrawParms::onLineEditLineWidth()
{
	float linewidth = 2;
	QString s = lineEditLineWidth->text();
	sscanf(s.ascii(), "%f", &linewidth);
	m_DrawParms.m_fLineWidth = linewidth;
}


void DGDrawParms::onLineEditFaceColor()
{
	float r, g, b;
	const float K = 1.0f/255;
	QString s = lineEditFaceColor->text();
	sscanf(s.ascii(), "%f %f %f", &r, &g, &b);
	m_DrawParms.m_cObjectColor.x = r*K, 
	m_DrawParms.m_cObjectColor.y = g*K, 
	m_DrawParms.m_cObjectColor.z = b*K;
}


void DGDrawParms::onLineEditBgColor()
{
	float r, g, b;
	const float K = 1.0f/255;
	QString s = lineEditBackgroundColor->text();
	sscanf(s.ascii(), "%f %f %f", &r, &g, &b);
	m_DrawParms.m_cBackgroundColor.x = r*K, 
	m_DrawParms.m_cBackgroundColor.y = g*K, 
	m_DrawParms.m_cBackgroundColor.z = b*K;
}


void DGDrawParms::onCheckBoxShowAxis()
{
	bool f = false;
	if (checkBoxShowAxis->isChecked()) f = true;
	m_DrawParms.m_bShowAixes = f;
}


void DGDrawParms::onCheckBoxShowCamera()
{
		m_DrawParms.m_bShowCamera = checkBoxShowCamera->isChecked();
}


void DGDrawParms::onCheckBox1DTexture()
{
		bool f = checkBoxTexturing2->isChecked();
		m_DrawParms.m_bEnableTexture1D = f;
		m_DrawParms.m_bEnableTexture2D = f;
    buttonLoadTexture->setEnabled(f);
		checkBoxTexturing->setChecked(f);
}


void DGDrawParms::onButtonLoadTexture()
{
	static QString dirpath;
	QImage _image;

    QString filename = QFileDialog::getOpenFileName(dirpath, QString::null, this);
    if (filename.isEmpty())
		return;

    bool ok = FALSE;
	QApplication::setOverrideCursor( waitCursor );	// this might take time
	ok = _image.load(filename, 0);
	if (_image.isNull())
		return;
	if (_image.depth()!=32)
		return;

	QImage tt = _image.swapRGB ();
	_image = tt;
	int w = _image.width();
	int h = _image.height();
	QApplication::restoreOverrideCursor();			// restore original cursor

	//copy the image to the drawparms data structure
	unsigned int *pcolor = (unsigned int *)m_DrawParms.m_texture1D;
	unsigned char *pbits = _image.bits();
	for (int i=0; i<256; i++, pbits+=4){
		unsigned char *pdst = (unsigned char*)(&pcolor[i]);
		pdst[0] = pbits[0];
		pdst[1] = pbits[1];
		pdst[2] = pbits[2];
		pdst[3] = pbits[3];
	}
}


void DGDrawParms::onButtonApply()
{
    *m_pDrawParms = m_DrawParms;
	m_pDrawParms->m_bRecomputeTexture1DParameter = true;
	m_pDrawParms->m_bRecomputeTexture1DRange = true;

	//read vertex color field
	float r, g, b;
	QString s = lineEditVertexColor->text();
    sscanf(s.ascii(), "%f %f %f", &r, &g, &b);
    m_pDrawParms->m_cVertexColor = Vector3f(r, g, b) * (1.0f/255);

	//set vertex texture
	m_pDrawParms->m_bEnableVertexTexture = checkBoxVertexTexture->isChecked();
	m_pDrawParms->m_bRecomputeTexture1DRange = true;
	//vertex dir;
	m_pDrawParms->m_bEnableVertexRandDir = checkBoxVertexDir->isChecked();

}


void DGDrawParms::onButtonOK()
{
	onButtonApply();
	accept();
}


void DGDrawParms::onButtonCancel()
{
	reject();
}


void DGDrawParms::onRadioButtonLinearMapping()
{
	if (radioButtonLinearMapping->isChecked())
		m_DrawParms.m_nMappingType = ScalarMappingLinear;
}


void DGDrawParms::onRadioButtonLog2Mapping()
{
	if (radioButtonLog2Mapping->isChecked())
		m_DrawParms.m_nMappingType = ScalarMappingLog2;
}


void DGDrawParms::onRadioButtonLog10Mapping()
{
	if (radioButtonLog10Mapping->isChecked())
		m_DrawParms.m_nMappingType = ScalarMappingLog10;
}


void DGDrawParms::onComboBoxChooseComp()
{    
	int n=  comboBoxChooseComp->currentItem();
	m_DrawParms.m_nTextureCoorType = (ScalarToTextureCoorMappingType) n;
	m_DrawParms.m_bRecomputeTexture1DParameter = true;
	m_DrawParms.m_bRecomputeTexture1DRange = true;
}


void DGDrawParms::onLineEditLegendLevel()
{
    QString s = lineEditLegendLevel->text();
    int lvl;
    sscanf(s.ascii(), "%d", &lvl);
	if (lvl<2)
		lvl = 2;
	else if (lvl>32)
		lvl = 32;
    m_DrawParms.m_nLegendLevel = lvl;
}


void DGDrawParms::onRadioButtonShowLegend()
{
    bool f = radioButtonShowLegend->isChecked();
    m_DrawParms.m_bShowLegend = f;
}


void DGDrawParms::onLineEditLegendTextColor()
{
    float r, g, b;
    QString s = lineEditLegendTextColor->text();
    sscanf(s.ascii(), "%f %f %f", &r, &g, &b);
    m_DrawParms.m_cLegendTextColor = Vector3f(r, g, b) * (1.0f/255);
}


void DGDrawParms::onComboBoxLegendPosition()
{
	int n=  comboBoxLegendPosition->currentItem();
	m_DrawParms.m_cLegendPosition=n;
}


void DGDrawParms::onComboBoxLegendOrientation()
{
	int n=  comboBoxLegendOrientation->currentItem();
	m_DrawParms.m_cLegendOrientation= n;
}


void DGDrawParms::onRadioButtonDrawNothing()
{
	if (radioButtonDrawNothing->isChecked())
		m_DrawParms.m_nDrawType = CGLDrawParms::DRAW_MESH_NONE;

}


void DGDrawParms::onRadioButtonShowVertexValue()
{
	m_DrawParms.m_bShowVertexAttribValue = radioButtonShowVertexValue->isChecked();

}


void DGDrawParms::onRadioButtonVertPoint()
{
    if (radioButtonVertPoint->isChecked())
		m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_POINT;
}


void DGDrawParms::onRadioButtonVertSphere()
{
    if (radioButtonVertSphere->isChecked())
		m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_SPHERE;
}


void DGDrawParms::onRadioButtonVertCube()
{
    if (radioButtonVertCube->isChecked())
		m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_CUBE;
}


void DGDrawParms::onRadioButtonVertTetra()
{
    if (radioButtonVertTetra->isChecked())
		m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_TETRA;
}


void DGDrawParms::onRadioButtonVertNone()
{
    if (radioButtonVertNone->isChecked())
		m_DrawParms.m_nDrawVertexStyle = CGLDrawParms::DRAW_VERTEX_NONE;
}


void DGDrawParms::onLineEditVertScale()
{
	float scale;
	QString s = lineEditVertScale->text();
	sscanf(s.ascii(), "%f", &scale);
	m_DrawParms.m_fVertexSizeScale = scale;
}
