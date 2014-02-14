/********************************************************************** 
   Adaptive-sampled Voxelization Library (AVL) V1.0
   Copyright (C) 2002 Nan Zhang, State University of New York at Stony Brook

This file is part of AVL.

AVL is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

AVL is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Subdivide; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. 

  
File name:	syslog.cpp
Date:		March 26, 2004
Author:		Nan Zhang
Version: 
History:

***********************************************************************/
//#include <windows.h>
//#include <GL/gl.h>
#include "sysconf.h"
#include "syslog.h"


CRuntimeLog::CRuntimeLog(int width, int length, char **names, char *datatype)
{
	int i;
	assert(width<_LOG_BUFFER_LENGTH);

	for (i=0; i<width; i++){
		int len = strlen(names[i]);
		m_pStrNames[i] = new char [len + 10];
		strcpy(m_pStrNames[i], names[i]);
	}

	m_nValues = new int [width*length];

	m_nWidth = width;
	m_nLength = length;
	m_nRowPtr = 0;

	for (i=0; i<width; i++)
		m_cDataTypes[i] = datatype[i];
}


CRuntimeLog::~CRuntimeLog()
{
	for (int i=0; i<m_nWidth; i++){
		delete [] m_pStrNames[i];
	}
	delete [] m_nValues;
}



void CRuntimeLog::Output(char *fname)
{
	FILE *fp;
	int i;

	fp =fopen(fname, _WA_);
	assert(fp!=NULL);

	for (i=0; i<m_nWidth; i++){
		fprintf(fp, "## Colume %d title: %s\n", i, m_pStrNames[i]);
	}
	fprintf(fp, "\n\n");

	for (i=0; i<m_nLength; i++){
		write_a_row(fp, i);
		printf("Line %d:\n", i);
	}
	fclose(fp);
}


void CRuntimeLog::AddRow(void *value)
{
	if (m_nRowPtr>m_nLength)
		return;

	if (m_nRowPtr == m_nLength){
		printf("\nLog file is written!\n");
	    Output("./runlog.txt");
		m_nRowPtr++;
		return;
	}

	const int pos = m_nRowPtr * m_nWidth;
	int *sd = (int *)value;
	float *sf =(float *)value;
	int *td =(int*)&m_nValues[pos];
	float *tf =(float*)&m_nValues[pos];

	for (int i=0; i<m_nWidth; i++){
		if (m_cDataTypes[i]=='d'){
			td[i] = sd[i];
		}
		else{
			tf[i] = sf[i];
		}
	}
	m_nRowPtr++;
}
