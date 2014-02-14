#ifndef __INC_NAME_TABLE2008JULY_H__
#define __INC_NAME_TABLE2008JULY_H__

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

//class QDomNode;
//namespace cia3d{
//namespace io{

#define _NAME_BUFFER_LENGTH  31

inline const char *_SKIP_NUMBER_TOKEN(const char *p)
{
	//first, find the first digit
	const char *c = p;
	while (!(c[0]>='0' && c[0]<='9')) c++;
	//skip digits
	while ((c[0]>='0' && c[0]<='9')) c++;
	return c;
}

class CNameTableItem
{
public:
	char m_sName[_NAME_BUFFER_LENGTH+1];
	char m_Type;		//variable type
	char __pad[3];		//padding, unused
	int *m_pAddr;		//the address storing the variable
	char *m_sOption;	//option string, must be statically allocated
	char *m_sComment;	//comment string, must be statically allocated

	CNameTableItem()
	{
		m_sName[0]=0;
		m_Type=0;
		m_pAddr = NULL;
		m_sOption=NULL;
		m_sComment=NULL;
	}

	CNameTableItem(const char * str, const char type, const void *p, char *option=NULL, char *strcomment=NULL)
	{
		strncpy(m_sName, str, _NAME_BUFFER_LENGTH);
		m_Type = type;
		m_sOption = option;
		m_sComment = strcomment;
		m_pAddr = (int*)p;
	}

	~CNameTableItem(){}

	bool isNull(void)
	{
		return (m_pAddr==NULL);
	}

	//Convert the string rep. of the variable 
	void setValue(const char *_val)
	{
		float x,y,z;
        double lx, ly, lz;
		char val[4096];
		if (_val==NULL || _val[0]==0)	//if empty string, no change
			return;
		const int _len = strlen(_val);
		for (int i=0; i<_len; i++){		//strdup, use space as the uniform separator
			char ch = _val[i];
			if (ch==',' || ch==';') ch=' ';
			val[i]=ch;
		}
		val[_len]=0;

		switch (m_Type){
		case 'x':		//double matrix 4x4
			{
			float m[16];
			sscanf(val, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
						&m[0], &m[1], &m[2], &m[3], 
						&m[4], &m[5], &m[6], &m[7], 
						&m[8], &m[9], &m[10], &m[11], 
						&m[12], &m[13], &m[14], &m[15]);
			float *fptr = (float*)m_pAddr;
			for (int i=0; i<16; i++) fptr[i]=m[i];
			}
			break;
		case 'X':		//double matrix 4x4
			{
			double m[16];
			sscanf(val, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
						&m[0], &m[1], &m[2], &m[3], 
						&m[4], &m[5], &m[6], &m[7], 
						&m[8], &m[9], &m[10], &m[11], 
						&m[12], &m[13], &m[14], &m[15]);
			double *lfptr = (double*)m_pAddr;
			for (int i=0; i<16; i++) lfptr[i]=m[i];
			}
			break;
		case 'V':		//double x, y, z
			{
			sscanf(val, "%lf %lf %lf", &lx, &ly, &lz);
			double* lfptr = (double*)m_pAddr;
			lfptr[0]=lx, lfptr[1]=ly, lfptr[2]=lz;
			}
			break;
		case 'v':		//float point x, y, z
			{
			sscanf(val, "%f %f %f", &x,&y,&z);
			float *fptr = (float*)m_pAddr;
			fptr[0]=x, fptr[1]=y, fptr[2]=z;
			}
			break;
		case 'i':		//integer type
		case 'm':		//multiple choices of integer type
			*m_pAddr=atoi(val);
			break;
		case 'I':		//integer array
			{
				int *buff = &m_pAddr[1];
				int& i = m_pAddr[0];  i=0;
				int x;
				const char *p=val;
				do{
					int n = sscanf(p, "%d", &x);
					if (n==1){
						buff[i++] = x;
						p=_SKIP_NUMBER_TOKEN(p);
					}
					else
						break;
				}while(1);
				break;
			}
		case 'd':		//double number
			*((double*)m_pAddr)=atof(val);
			break;
		case 'f':		//float point number
			*((float*)m_pAddr)=(float)atof(val);
			break;
		case 'c':		//char or bool
			*((char*)m_pAddr)=val[0];
			break;
		case 'b':
			*((char*)m_pAddr)=atoi(val);
			break;
		default:
			fprintf(stderr, "Unknow type in CNameTableItem.\n");
			break;
		}
	}

	//Convert to the string rep. The val buffer is an internal work buffer, 
	// all other buffers are pointing to valid memory locations
	void toString(char *&name, char *&type, char *&val, char *&option, char *&comm)
	{
		static char *ptypes[] = 
			{"int", "float", "multi", "char", "bool", 
			 "vec3", "VEC3","double", "Matrix4x4d", "Matrix4x4f"
			};
		static char valbuffer[100];
		float *fp;
        double *lfp;

		name = m_sName;
		val = valbuffer;
		comm = m_sComment;
		option = m_sOption;

		switch (m_Type){
		case 'x':
			{
			float* fp=(float*)m_pAddr;
			type = ptypes[9];
			sprintf(valbuffer, "%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g", 
					fp[0], fp[1], fp[2], fp[3], 
					fp[4], fp[5], fp[6], fp[7], 
					fp[8], fp[9], fp[10], fp[11], 
					fp[12], fp[13], fp[14], fp[15]
					);
			}
			break;
		case 'X':
			{
			double *fp=(double*)m_pAddr;
			type = ptypes[8];
			sprintf(valbuffer, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", 
					fp[0], fp[1], fp[2], fp[3], 
					fp[4], fp[5], fp[6], fp[7], 
					fp[8], fp[9], fp[10], fp[11], 
					fp[12], fp[13], fp[14], fp[15]
					);
			}
			break;
		case 'V':
			type = ptypes[6];
			lfp = (double*)m_pAddr;
			sprintf(valbuffer, "%.14lg,%.14lg,%.14lg", lfp[0], lfp[1], lfp[2]);
			break;
		case 'v':
			type = ptypes[5];
			fp = (float*)m_pAddr;
			sprintf(valbuffer, "%g,%g,%g", fp[0], fp[1], fp[2]);
			break;
		case 'b':
			type = ptypes[4];
			sprintf(valbuffer, "%d", (int)(*m_pAddr));
			break;
		case 'c':
			type = ptypes[3];
			sprintf(valbuffer, "%c", *m_pAddr);
			break;
		case 'i':
			type = ptypes[0];
			sprintf(valbuffer, "%d", *m_pAddr);
			break;
		case 'd':
			type = ptypes[7];
			sprintf(valbuffer, "%.14lg", *((double*)m_pAddr));
			break;
		case 'f':
			type = ptypes[1];
			sprintf(valbuffer, "%g", *((float*)m_pAddr));
			break;
		case 'm':
			type = ptypes[2];
			sprintf(valbuffer, "%d", *m_pAddr);
			break;
		default:
			assert(0);
		}
	}
};

inline void 
setNameTableValue(CNameTableItem* table, const int N, const char *name, const char* val)
{
	if (val==NULL || val[0]==0) return;
	for (int i=0; i<N; i++){
		CNameTableItem *p = &table[i];
		if (strcmp(p->m_sName, name)==0){
			p->setValue(val);
			break;
		}
	}
}

/*
inline void beginXMLCLASS(char *sbuffer, int &slen, const char *classname)
{
	sprintf(&sbuffer[slen], "<xmlclass name=\"%s\">\n", classname);
	slen=strlen(sbuffer);
}

inline void endXMLCLASS(char *sbuffer, int &slen, const char *)
{
	sprintf(&sbuffer[slen], "</xmlclass>\n");
	slen=strlen(sbuffer);
}

inline void writeXMLItem(char *sbuffer, int &slen, 
	   const char *name, const char *type, const char *value, 
	   const char *option=NULL, const char *comment=NULL)
{
	if (option==NULL || option[0]==0){
		if (comment==NULL || comment[0]==0)
			sprintf(&sbuffer[slen], "<xmlitem name=\"%s\" type=\"%s\" value=\"%s\"/>\n", name, type, value);
		else
			sprintf(&sbuffer[slen], "<xmlitem name=\"%s\" type=\"%s\" value=\"%s\" comment=\"%s\"/>\n", name, type, value, comment);
	}
	else{
		if (comment==NULL || comment[0]==0)
			sprintf(&sbuffer[slen], "<xmlitem name=\"%s\" type=\"%s\" value=\"%s\" option=\"%s\"/>\n", name, type, value, option);
		else
			sprintf(&sbuffer[slen], "<xmlitem name=\"%s\" type=\"%s\" value=\"%s\" option=\"%s\" comment=\"%s\"/>\n", name, type, value, option, comment);
	}
	slen=strlen(sbuffer);
}

inline QDomNode* _searchItemNodeByName( const QDomNode & anode, const QString &id)
{
	static QDomNode staticnode;
	QDomNode node = anode.firstChild();
	while (!node.isNull()) {
		const QString nodename = node.nodeName();
		if (nodename == "xmlitem"){
			QString propname = node.toElement().attribute( "name");
			if (propname==id){
				staticnode = node;
				return &staticnode;
			}
		}
		else{
			QDomNode * r = _searchItemNodeByName(node, id);
			if (r) return r;
		}
		node = node.nextSibling();
	}
	return NULL;
}
*/

//}//end io
//}//end cia3d

#endif