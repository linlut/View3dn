//FILE: vrmlrw.cpp 
//Purpose: read/write VRML 2.0 data file. This format is used for tools
//Author: Nan Zhang
//Date: 

BOOL CGeoModel::ReaderVrml(CString Path)
{
	CFile file;
	CFileException ex;
	long fileLength;
	LPTSTR pFileBuffer;
	TCHAR c;

	CString str_vrml, str;
	long nCoord_def, nTexCoord_def, nCoordIndex_def , nTexCoordIndex_def; // Array anchor
	long nCoord_end, nTexCoord_end, nCoordIndex_end, nTexCoordIndex_end;
	long nColor_def, nColor_end;
	long index, length, n, a;
	nCoord_def=nTexCoord_def=nCoordIndex_def=nTexCoordIndex_def=-1;
	nCoord_end=nTexCoord_end=nCoordIndex_end=nTexCoordIndex_end=-1;
	nColor_def=nColor_end=-1;
	index=length=n=a=-1;
	float* pf;
	//float fScaler=1.0f;//0.3f;//
	//float translation[3]={0,0,0};

	if(m_vertex) delete []m_vertex;
	if(m_vertex0) delete []m_vertex0;
	if(m_vertex1) delete []m_vertex1;
	if(m_triangle) delete []m_triangle;
	if(m_textCoord) delete []m_textCoord;
	if(m_textIndex) delete []m_textIndex;
	if(m_normal) delete []m_normal;
	if(m_pNails) delete []m_pNails;
	m_vertex=m_vertex0=m_vertex1=m_triangle=m_textCoord=m_textIndex=m_normal=NULL;
	m_pNails=NULL;

	// File access------------------------------------------
	if(!file.Open(Path, CFile::modeRead | CFile::typeBinary,&ex))
	{
		#ifdef _DEBUG
		  afxDump << "File could not be opened " << ex.m_cause << "\n";
		#endif
		TRACE("unable to open file for reading\n");
		return FALSE;
	}
	fileLength = file.GetLength();
	pFileBuffer=str_vrml.GetBufferSetLength((long)fileLength);
	if(pFileBuffer == NULL)
	{
		TRACE("str_vrml.GetBufferSetLength error\n");
		file.Close();
		return FALSE;
	}
	TRY{ file.Read(pFileBuffer,fileLength);}
	CATCH(CFileException, e)
	{
		#ifdef _DEBUG
				afxDump << "Error during reading" << e->m_cause << "\n";
		#endif
		TRACE("error during file reading in CShare\n");
		file.Close();
		return FALSE;
	}
	END_CATCH
	file.Close();
	str_vrml.ReleaseBuffer();
	if(str_vrml.Find("#VRML V2.0")==-1)
	{
		TRACE("#VRML V2.0 file format error\n");
		return FALSE;
	}

	// Transform -------------------------------------------
	if( (index=str_vrml.Find("Transform"))!=-1
		&& (index=str_vrml.Find("{", index))!=-1 )
	{
		a=index+1;
		if( (index=str_vrml.Find("translation", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("translation"));
			pf=m_translate;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("rotation", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("rotation"));
			pf=m_rotate;
			while(n<4)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("scale", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("scale"));
			pf=m_scale;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("center", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("center"));
			pf=m_center;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("scaleOrientation", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("scaleOrientation"));
			pf=m_scaleOrientation;
			while(n<4)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
	}

	// Material --------------------------------------------
	if( (index=str_vrml.Find("material"))!=-1
		&& (index=str_vrml.Find("{", index))!=-1 )
	{
		a=index+1;
		if( (index=str_vrml.Find("ambientIntensity", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("ambientIntensity"));
			pf=m_material.ambientIntensity;
			while(n<1)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
			pf[1]=pf[2]=pf[0];
		}
		if( (index=str_vrml.Find("diffuseColor", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("diffuseColor"));
			pf=m_material.diffuseColor;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("emissiveColor", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("emissiveColor"));
			pf=m_material.emissiveColor;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("shininess", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("shininess"));
			pf=&(m_material.shininess);
			while(n<1)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("specularColor", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("specularColor"));
			pf=m_material.specularColor;
			while(n<3)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
		if( (index=str_vrml.Find("transparency", a))!=-1 )
		{
			n=0;
			str="";
			c= str_vrml.GetAt(index+=strlen("transparency"));
			pf=&(m_material.transparency);
			while(n<1)
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					pf[n++]=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
		}
	}

	// Texture path-----------------------------------------
	if( (index=str_vrml.Find("texture ImageTexture"))!=-1
		&& (index=str_vrml.Find("url \"", index))!=-1
		&& (n=str_vrml.Find("\"", index+=5))!=-1 )
		{
			m_texPath=str_vrml.Mid(index, n-index);
			index=Path.ReverseFind('/');
			m_texPath=Path.Left(index+1)+m_texPath;
			if(!e_mi.m_graphic.PreTexture(m_texPath,m_pBmi,m_pBit,m_nTexWidth,m_nTexHeight))
				m_texPath="invalid";
			//CGraphic::PreTexture(m_texPath,m_pBmi,m_pBit,m_nTexWidth,m_nTexHeight);
		}
	else m_texPath="invalid";

	// DEF array anchor and size -----------------
	// Coordinate anchor
	if( (index=str_vrml.Find("geometry"))<0 ||
		(index=str_vrml.Find("Coordinate", index))<0 ||
		(index=str_vrml.Find("point", index))<0 ||
		(index=str_vrml.Find("[", index))<0 )
	{
		TRACE("VRML V2.0 file format error in geometry DEF/Coordinate\n");
		return FALSE;
}	
	nCoord_def=index+1; //+3; // Start after [
	nCoord_end=str_vrml.Find("]", index);
	if(nCoord_end <= nCoord_def)
	{
		TRACE("VRML V2.0 file format error in Coordinate\n");
		return FALSE;
	}
	// Count m_nLength_v
	index=str_vrml.Find(',', nCoord_def);
	while(index>-1 && index<nCoord_end)
	{
		m_nLength_v++;
		index=str_vrml.Find(',', index+1);
	}
	m_nLength_v++;
	if(m_nLength_v==0)
	{
		TRACE("VRML V2.0 file format error: m_nLength_v==0\n");
		return FALSE;
	}
	// TextureCoordinate anchor
	if( (index=str_vrml.Find("TextureCoordinate", nCoord_end))>0 &&
		(index=str_vrml.Find("point", index))>0 &&
		(index=str_vrml.Find("[", index))>0 )
	{
		nTexCoord_def=index+1;
		nTexCoord_end=str_vrml.Find("]", index);
		if(nTexCoord_end <= nTexCoord_def)
		{
			TRACE("VRML V2.0 file format error in TextureCoordinate\n");
			return FALSE;
		}
		// Count m_iLength_tc
		index=str_vrml.Find(',', nTexCoord_def);
		while(index>-1 && index<nTexCoord_end)
		{
			m_iLength_tc++;
			index=str_vrml.Find(',', index+1);
		}
		m_iLength_tc++;
		if(m_iLength_tc==0)
		{
			TRACE("VRML V2.0 file format error: m_iLength_tc=0\n");
			return FALSE;
		}
	}
	// coordIndex anchor
	index=nTexCoord_end;
	if(index<=0) index=nCoord_end;
	if( (index=str_vrml.Find("coordIndex", index))<0 ||
		(index=str_vrml.Find("[", index))<0 )
	{
		TRACE("VRML V2.0 file format error in gcoordIndex\n");
		return FALSE;
	}
	nCoordIndex_def=index+1;
	nCoordIndex_end=str_vrml.Find("]", index);
	if(nCoordIndex_end <= nCoordIndex_def)
	{
		TRACE("VRML V2.0 file format error in coordIndex\n");
		return FALSE;
	}
	// Count m_iLength_t;
	index=str_vrml.Find("-1", nCoordIndex_def);
	while(index>-1 && index<nCoordIndex_end)
	{
		m_iLength_t++;
		index=str_vrml.Find("-1", index+2);
	}
	if(m_iLength_t==0)
	{
		TRACE("VRML V2.0 file format error: m_iLength_t==0\n");
		return FALSE;
	}
	// texCoordIndex anchor
	if( (index=str_vrml.Find("texCoordIndex", nCoordIndex_end))>0 &&
		(index=str_vrml.Find("[", index))>0 )
	{
		nTexCoordIndex_def=index+1;
		nTexCoordIndex_end=str_vrml.Find("]", index);
		if(nTexCoordIndex_end <= nTexCoordIndex_def)
		{
			TRACE("VRML V2.0 file format error in texCoordIndex\n");
			return FALSE;
		}
		// Count whether equal to m_iLength_t;
		long nLength_ti=0;
		index=str_vrml.Find("-1", nTexCoordIndex_def);
		while(index>-1 && index<nTexCoordIndex_end)
		{
			nLength_ti++;
			index=str_vrml.Find("-1", index+2);
		}
		if(nLength_ti!=m_iLength_t)
		{
			TRACE("VRML V2.0 file format error: m_iLength_t==nLength_ti\n");
			return FALSE;
		}
	}

	m_triangle=new vertex[m_iLength_t];
	if(m_triangle==NULL){
		TRACE("Insufficient memory: m_triangle[m_iLength_t]\n");
		return FALSE;
	}
	m_vertex=new vertex[m_nLength_v];
	if(m_vertex==NULL){
		TRACE("Insufficient memory: m_vertex[m_nLength_v]\n");
		return FALSE;
	}
	m_vertex0=new vertex[m_nLength_v];
	if(m_vertex0==NULL){
		TRACE("Insufficient memory: m_vertex0[m_nLength_v]\n");
		return FALSE;
	}
	m_vertex1=new vertex[m_nLength_v];
	if(m_vertex1==NULL){
		TRACE("Insufficient memory: m_vertex1[m_nLength_v]\n");
		return FALSE;
	}
	m_normal=new vertex[m_nLength_v];
	if(m_normal==NULL){
		TRACE("Insufficient memory: m_normal[m_nLength_v]\n");
		return FALSE;
	}
	long* triangles=(long*)m_triangle;
	float* vertices=(float*)m_vertex;

	//load vertices, triangle-------------------------------
	length=m_nLength_v*3;
	index=nCoord_def;
	n=0;
	str="";
	c= str_vrml.GetAt(index);
	while(n<length)
	{
		while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
			str+=c;
			c= str_vrml.GetAt(++index);
		}
		if(str.GetLength()>0){
			vertices[n++]=atof(str);
			str="";
		}
		c= str_vrml.GetAt(++index);
	}
	// memcpy(m_vertex0,m_vertex,sizeof(vertex) * m_nLength_v);

	length=m_iLength_t*3;
	index=nCoordIndex_def;
	n=0;
	str="";
	c= str_vrml.GetAt(index);
	while(n<length)
	{
		while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
			str+=c;
			c= str_vrml.GetAt(++index);
		}
		if(str.GetLength()>0 && str.Find("-1")==-1){
			triangles[n++]=atof(str);
		}
		str="";
		c= str_vrml.GetAt(++index);
	}

	// Apply transform -------------------------------------
	float T[16], R[16], S[16], Sor[16], Sori[16];
	bool bTransform=FALSE;
	vertex zero={0.0, 0.0, 0.0};
	//vertex xyz;
	SetIdentityMatrix(T);
	SetIdentityMatrix(R);
	SetIdentityMatrix(S);
	SetIdentityMatrix(Sor);
	SetIdentityMatrix(Sori);
	if( m_translate[0]!=0.0 || m_translate[1]!=0.0 || m_translate[2]!=0.0 ){
		SetTranslateMatrix2(T, m_translate[0], m_translate[1], m_translate[2]);
		bTransform=TRUE;
	}
	if( m_rotate[3]!=0.0 ){
		SetRotateMatrix(R, zero, *((vertex*)m_rotate), m_rotate[3]);
		bTransform=TRUE;
	}
	if( m_scale[0]!=1.0 || m_scale[1]!=1.0 || m_scale[2]!=1.0){
		SetScaleMatrix(S, m_scale[0], m_scale[1], m_scale[2], *((vertex*)m_center));
		bTransform=TRUE;
	}
	if( m_scaleOrientation[3]!=0.0 ){
		SetRotateMatrix(Sor, zero, *((vertex*)m_scaleOrientation), m_scaleOrientation[3]);
		SetRotateMatrix(Sori, zero, *((vertex*)m_scaleOrientation), -1.0*m_scaleOrientation[3]);
		bTransform=TRUE;
	}
	if(bTransform){
		Multiply44(S, Sori);
		Multiply44(Sor, Sori);
		Multiply44(R, Sori);
		Multiply44(T, Sori);
		Transform(Sori, m_vertex, m_vertex0, m_nLength_v);
		memcpy(m_vertex,m_vertex0,sizeof(vertex) * m_nLength_v);
/*		Multiply44(T, R);
		Multiply44(R, Sori);
		Multiply44(Sori, S);
		Multiply44(S, Sor);
		Transform(Sor, m_vertex0, m_vertex, m_nLength_v); */
	}
	else{ memcpy(m_vertex0,m_vertex,sizeof(vertex) * m_nLength_v); }

	// load texture info if exists------------------------
 	if( m_texPath!="invalid" && nTexCoord_def>0 && nTexCoordIndex_def>0 )
	{
		m_textCoord=new vertex[m_iLength_tc];
		m_textIndex=new vertex[m_iLength_t];
		if(m_textCoord==NULL){
			TRACE("Insufficient memory: m_textCoord[m_iLength_tc]\n");
			return FALSE;
		}
		if(m_textIndex==NULL){
			TRACE("Insufficient memory: CShare::m_textureIndex[m_iLength_t]\n");
			return FALSE;
		}
		float* textCoord=(float*)m_textCoord;
		length=m_iLength_tc*2;
		index=nTexCoord_def;
		n=0;
		str="";
		c= str_vrml.GetAt(index);
		while(n<length)
		{
			while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
				str+=c;
				c= str_vrml.GetAt(++index);
			}
			if(str.GetLength()>0){
				textCoord[n++]=atof(str);
				str="";
			}
			c= str_vrml.GetAt(++index);
		}

		long* textIndex=(long*)m_textIndex;
		length=m_iLength_t*3;
		index=nTexCoordIndex_def;
		n=0;
		str="";
		c= str_vrml.GetAt(index);
		while(n<length)
		{
			while(c!=' ' && c!='\r' && c!='\n' && c!='\t'){
				str+=c;
				c= str_vrml.GetAt(++index);
			}
			if(str.GetLength()>0 && str.Find("-1")==-1){
				textIndex[n++]=atof(str);
			}
			str="";
			c= str_vrml.GetAt(++index);
		}
	}

	// Color Keys
	if( index=str_vrml.Find("colorPerVertex TRUE")>0 )
	{
		if( (index=str_vrml.Find(" Color", index))>0 &&
			(index=str_vrml.Find("color", index))>0 &&
			(index=str_vrml.Find("[", index))>0 )
		{
			nColor_def=index+1;
			nColor_end=str_vrml.Find("]", index);
			if(nColor_end <= nColor_def)
			{
				TRACE("VRML V2.0 file format error in Color\n");
				return FALSE;
			}
			// Count m_nNails...
			m_nNails=0;
			index=str_vrml.Find("0 0 0", nColor_def);
			while(index>-1 && index<nColor_end)
			{
				m_nNails++;
				index=str_vrml.Find("0 0 0", index+5);
			}
			// Count jaw and medial
			m_nMedialLeft=m_nMedialRight=m_nJawLeft=m_nJawRight=0;
			index=str_vrml.Find("1 0 0", nColor_def);
			while(index>-1 && index<nColor_end)
			{
				m_nJawLeft++;
				index=str_vrml.Find("1 0 0", index+5);
			}
			index=str_vrml.Find("1 1 0", nColor_def);
			while(index>-1 && index<nColor_end)
			{
				m_nMedialLeft++;
				index=str_vrml.Find("1 1 0", index+5);
			}
			index=str_vrml.Find("0 0 1", nColor_def);
			while(index>-1 && index<nColor_end)
			{
				m_nJawRight++;
				index=str_vrml.Find("0 0 1", index+5);
			}
			index=str_vrml.Find("0 1 1", nColor_def);
			while(index>-1 && index<nColor_end)
			{
				m_nMedialRight++;
				index=str_vrml.Find("0 1 1", index+5);
			}
			// Extract nails id
			if(m_nNails>0 && m_nNails<=m_nLength_v){
				m_pNails=new long[m_nNails];
				if(m_pNails==NULL){
					TRACE("Insufficient memory: m_pNails\n");
					return FALSE;
				}
			}
			if(m_nJawLeft>0 && m_nJawLeft<=m_nLength_v){
				m_pJawLeft=new long[m_nJawLeft+m_nMedialLeft];
				if(m_pJawLeft==NULL){
					TRACE("Insufficient memory: m_pJawLeft\n");
					return FALSE;
				}
			}
			if(m_nJawRight>0 && m_nJawRight<=m_nLength_v){
				m_pJawRight=new long[m_nJawRight+m_nMedialRight];
				if(m_pJawRight==NULL){
					TRACE("Insufficient memory: m_pJawRight\n");
					return FALSE;
				}
			}
			if(m_nMedialLeft>0 && m_nMedialLeft<=m_nLength_v){
				m_pMedialLeft=new long[m_nMedialLeft];
				if(m_pMedialLeft==NULL){
					TRACE("Insufficient memory: m_pMedialLeft\n");
					return FALSE;
				}
			}
			if(m_nMedialRight>0 && m_nMedialRight<=m_nLength_v){
				m_pMedialRight=new long[m_nMedialRight];
				if(m_pMedialRight==NULL){
					TRACE("Insufficient memory: m_pMedialRight\n");
					return FALSE;
				}
			}

			n=0;
			a=0;
			long lastIndex=-1;
			long nJawLeft=0;
			long nJawRight=0;
			long nMedialLeft=0;
			long nMedialRight=0;
			index=str_vrml.Find(',', nColor_def);
			while(index>-1 && index<nColor_end)
			{
				str=str_vrml.Mid(index-5, 5);
				if("0 0 0"==str){
					m_pNails[n++]=a;
				}
				// Left jaw
				else if("1 0 0"==str){
					m_pJawLeft[nJawLeft++]=a;
				}
				// Left medial
				else if("1 1 0"==str){
					m_pMedialLeft[nMedialLeft++]=a;
				}
				// Right jaw
				else if("0 0 1"==str){
					m_pJawRight[nJawRight++]=a;
				}
				// Right medial
				else if("0 1 1"==str){
					m_pMedialRight[nMedialRight++]=a;
				}
				a++;
				lastIndex=index;
				index=str_vrml.Find(',', index+1);
			}
			//if(n<m_nNails && a==m_nLength_v-1) m_pNails[n]=a;// When the last one is nail
			if( (index=str_vrml.Find("0 0 0", lastIndex)) < nColor_end && index > lastIndex)
				m_pNails[n]=a;
			else if( (index=str_vrml.Find("1 0 0", lastIndex)) < nColor_end && index > lastIndex )
				m_pJawLeft[nJawLeft]=a;
			else if( (index=str_vrml.Find("1 1 0", lastIndex)) < nColor_end && index > lastIndex )
				m_pMedialLeft[nMedialLeft]=a;
			else if( (index=str_vrml.Find("0 0 1", lastIndex)) < nColor_end && index > lastIndex )
				m_pJawRight[nJawRight]=a;
			else if( (index=str_vrml.Find("0 1 1", lastIndex)) < nColor_end && index > lastIndex )
				m_pMedialRight[nMedialRight]=a;
		}
	}
	else // Color per stupid Maya polygon that demands additional coding here
	{
		if( (index=str_vrml.Find(" Color", index))>0 &&
			(index=str_vrml.Find("color", index))>0 &&
			(index=str_vrml.Find("[", index))>0 )
		{
			long nColorP=m_iLength_t*3;
			long nColorV=m_nLength_v;
			vertex* ColorP=new vertex[nColorP];
			vertex* ColorV=new vertex[nColorV];
			float* pf=(float*)ColorP;
			long i=0;
			str="";
			c= str_vrml.GetAt(++index);
			while(i<nColorP*3)//per float
			{
				while(c!=' ' && c!='\r' && c!='\n' && c!='\t' && c!=']' && c!=','){
					str+=c;
					c= str_vrml.GetAt(++index);
				}
				if(str.GetLength()>0){
					*(pf+i++)=atof(str);
					str="";
				}
				c= str_vrml.GetAt(++index);
			}
			for(int i=0;i<m_iLength_t;i++)
			{
				*(ColorV+((m_triangle+i)->p1))=*(ColorP+i*3);
				*(ColorV+((m_triangle+i)->p2))=*(ColorP+i*3+1);
				*(ColorV+((m_triangle+i)->p2))=*(ColorP+i*3+2);
			}
			const vertex ColorNail={0.0f, 0.0f, 0.0f};
			const vertex ColorJL={1.0f, 0.0f, 0.0f};
			const vertex ColorJR={0.0f, 0.0f, 1.0f};
			const vertex ColorML={1.0f, 1.0f, 0.0f};
			const vertex ColorMR={0.0f, 1.0f, 1.0f};
			m_nNails=m_nMedialLeft=m_nMedialRight=m_nJawLeft=m_nJawRight=0;
			for(int i=0;i<nColorV;i++)
			{
				if( !memcmp(&ColorNail, (ColorV+i), sizeof(vertex)) ) m_nNails++;
				else if ( !memcmp(&ColorJL, (ColorV+i), sizeof(vertex)) ) m_nJawLeft++;
				else if ( !memcmp(&ColorJR, (ColorV+i), sizeof(vertex)) ) m_nJawRight++;
				else if ( !memcmp(&ColorML, (ColorV+i), sizeof(vertex)) ) m_nMedialLeft++;
				else if ( !memcmp(&ColorMR, (ColorV+i), sizeof(vertex)) ) m_nMedialRight++;
			}
			// Extract nails id
			if(m_nNails>0 && m_nNails<=m_nLength_v){
				m_pNails=new long[m_nNails];
				if(m_pNails==NULL){
					TRACE("Insufficient memory: m_pNails\n");
					return FALSE;
				}
			}
			if(m_nJawLeft>0 && m_nJawLeft<=m_nLength_v){
				m_pJawLeft=new long[m_nJawLeft+m_nMedialLeft];
				if(m_pJawLeft==NULL){
					TRACE("Insufficient memory: m_pJawLeft\n");
					return FALSE;
				}
			}
			if(m_nJawRight>0 && m_nJawRight<=m_nLength_v){
				m_pJawRight=new long[m_nJawRight+m_nMedialRight];
				if(m_pJawRight==NULL){
					TRACE("Insufficient memory: m_pJawRight\n");
					return FALSE;
				}
			}
			if(m_nMedialLeft>0 && m_nMedialLeft<=m_nLength_v){
				m_pMedialLeft=new long[m_nMedialLeft];
				if(m_pMedialLeft==NULL){
					TRACE("Insufficient memory: m_pMedialLeft\n");
					return FALSE;
				}
			}
			if(m_nMedialRight>0 && m_nMedialRight<=m_nLength_v){
				m_pMedialRight=new long[m_nMedialRight];
				if(m_pMedialRight==NULL){
					TRACE("Insufficient memory: m_pMedialRight\n");
					return FALSE;
				}
			}
			long nJawLeft=0;
			long nJawRight=0;
			long nMedialLeft=0;
			long nMedialRight=0;
			long nNails=0;
			for(int i=0;i<nColorV;i++)
			{
				if( !memcmp(&ColorNail, (ColorV+i), sizeof(vertex)) ) m_pNails[nNails++]=i;
				else if( !memcmp(&ColorJL, (ColorV+i), sizeof(vertex)) ) m_pJawLeft[nJawLeft++]=i;
				else if( !memcmp(&ColorML, (ColorV+i), sizeof(vertex)) ) m_pMedialLeft[nMedialLeft++]=i;
				else if( !memcmp(&ColorJR, (ColorV+i), sizeof(vertex)) ) m_pJawRight[nJawRight++]=i;
				else if( !memcmp(&ColorMR, (ColorV+i), sizeof(vertex)) ) m_pMedialRight[nMedialRight++]=i;
			}	
			delete []ColorP;
			delete []ColorV;
		}
	}

	if(m_nMedialLeft && m_nJawLeft){
		memcpy(m_pJawLeft+m_nJawLeft, m_pMedialLeft, m_nMedialLeft*sizeof(long));
		m_nJawLeft+=m_nMedialLeft;
	}
	if(m_nMedialRight && m_nJawRight){
		memcpy(m_pJawRight+m_nJawRight, m_pMedialRight, m_nMedialRight*sizeof(long));
		m_nJawRight+=m_nMedialRight;
	}

	// Automatically locate the jaw reference vector
	float fJoint=0.0;
	for(n=0;n<m_nJawLeft;n++) fJoint=__max(fJoint, (m_vertex0 + *(m_pJawLeft+n))->z);
	for(n=0;n<m_nJawRight;n++) fJoint=__max(fJoint, (m_vertex0 + *(m_pJawRight+n))->z);
	m_jawRefer.z=fJoint;


	return TRUE;
}
