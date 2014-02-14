/*

readbpfile
*/

#include "sysconf.h"
#include <bprw.h>

#define MAX(x,y) ((x)>(y)?(x):(y))


//free the point and block list 
void free_data(Block_Info *b_info)
{
	if (b_info==NULL)
		return;
	if (b_info->bp_list!=NULL)
	{
		free(b_info->bp_list);
		b_info->bp_list=NULL;
	}
	if (b_info->point_list!=NULL)
	{
		free(b_info->point_list);
		b_info->point_number=NULL;
	}
}//free_data()



//write out the organized block points in bp format
//if block_compress, 0 point in cell, use runlength : 0-length
//if has_color, there is color component of the point
int Block_Info::saveFile(const char* file_name, const int block_compress)
{
	FILE *fp;
	int i;
	int count;

	if ((fp=fopen(file_name,"w"))==NULL){
		printf("cannot open %s for write.\n",file_name);
		return 0;
	}

	fprintf(fp,"#BLOCKPOINT\n");
	fprintf(fp,"%d %d %d\n",m_x,m_y,m_z);
	fprintf(fp,"%f %f %f\n",m_dx,m_dy,m_dz);
	fprintf(fp,"%d\n",point_number);

	if (m_bPosition)
		fprintf(fp,";position3f\n");
	if (m_bNormal)
		fprintf(fp,";normal3f\n");
	if (m_bSize)
		fprintf(fp,";size1f\n");
	if (m_bColor)
		fprintf(fp,";color3d\n");
	if (m_bIndex)
		fprintf(fp,";index1d\n");


	//first block info
	if (!block_compress){
		fprintf(fp,"#BLOCK_INFO\n");
		for (i=0;i<m_x*m_y*m_z;i++)
			fprintf(fp,"%d %d\n", bp_list[i].num, bp_list[i].index);
	}
	else{
		fprintf(fp,"#COMP_BLOCK_INFO\n");
		i=0;
		while (i<m_x*m_y*m_z){
			if (bp_list[i].num==0)
			{
				count=1;
				i++;
				while((i<m_x*m_y*m_z)&&(bp_list[i].num==0))
				{
					count++;
					i++;
				}
				fprintf(fp,"0-%d\n",count);
			}
			else//>0
			{
				fprintf(fp,"%d %d\n",bp_list[i].num,bp_list[i].index);
				i++;
			}
		}
	}

	fprintf(fp,"#POINT_INFO\n");

	for (i=0;i<point_number;i++){
		Vertex *p = &point_list[i];
		if (m_bPosition)
			fprintf(fp,"%f %f %f ", p->x, p->y, p->z);
		if (m_bNormal)
			fprintf(fp,"%f %f %f ", p->nx,p->ny,p->nz);
		if (m_bSize)
			fprintf(fp,"%f ", p->radius);
		if (m_bColor)
			fprintf(fp,"%d %d %d ", (unsigned int)p->red,(unsigned int)p->green,(unsigned int)p->blue);
		if (m_bIndex)
			fprintf(fp,"%d", p->vindex);
		fprintf(fp, "\n");
	}
	
	fclose(fp);

	return 1;
}



Vertex* Block_Info::processing(float minp[3], float maxp[3])
{
	int i;
	int x,y,z;
	int index;
	int nverts=point_number;
	Vertex* vlist=new Vertex[nverts];
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	minX= minY= minZ = 1e38f;
	maxX= maxY= maxZ =-1e38f;

	for (i=0;i<nverts;i++)
	{
		vlist[i] = point_list[i];
		index=find_block_index(i);
		x=index%m_x;
		y=((index-x)/m_x)%m_y;
		z=index/(m_x*m_y);
		vlist[i].x=point_list[i].x+x;
		vlist[i].y=point_list[i].y+y;
		vlist[i].z=point_list[i].z+z;
		if (minX>vlist[i].x)	minX=vlist[i].x;
		if (maxX<vlist[i].x)	maxX=vlist[i].x;
		if (minY>vlist[i].y)	minY=vlist[i].y;
		if (maxY<vlist[i].y)	maxY=vlist[i].y;
		if (minZ>vlist[i].z)	minZ=vlist[i].z;
		if (maxZ<vlist[i].z)	maxZ=vlist[i].z;
	}

	minp[0]=minX;
	minp[1]=minY;
	minp[2]=minZ;
	maxp[0]=maxX;
	maxp[1]=maxY;
	maxp[2]=maxZ;
	return vlist;
}


int Block_Info::loadFile(const char* file_name)
{
	FILE *fp;
	int i,j, count,cur_pos;
	char str[512];
	int block_comp=0;

	m_bColor = m_bNormal = m_bSize = m_bIndex = false;
	m_bPosition = true;

	if ((fp=fopen(file_name,"r"))==NULL){
		printf("Cannot open %s.\n",file_name);
		return 0;
	}

	if (fgets(str,512,fp)!=NULL)	{
		if (strncmp(str,"#BLOCKPOINT",11))  //not block point file
		{
			fclose(fp);
			return 0;
		}
	}
	fgets(str,512,fp);
	m_x=atoi(strtok(str," "));
	m_y=atoi(strtok(NULL," "));
	m_z=atoi(strtok(NULL," "));

	fgets(str,512,fp);
	m_dx=(float)atof(strtok(str," "));
	m_dy=(float)atof(strtok(NULL," "));
	m_dz=(float)atof(strtok(NULL," "));

	fgets(str,512,fp);
	point_number=atoi(str);

	do{
		fgets(str,512,fp);
		if (str[0]==';')
		{
			if (!strncmp(str+1,";position",9))
			{
				m_bPosition=true;
			}
			else if (!strncmp(str,";normal",7))
			{
				m_bNormal=true;
			}
			else if (!strncmp(str,";size",5))
			{
				m_bSize=true;
			}
			else if (!strncmp(str,";color",6))
			{
				m_bColor=true;
			}
			else if (!strncmp(str,";index1d",8))
			{
				m_bIndex=true;
			}
		}
	}
	while (str[0]==';');

	bp_list= new Block_Point[m_x*m_y*m_z];
	point_list=new Vertex[point_number];
	assert(bp_list!=NULL);
	assert(point_list!=NULL);

	if (!strncmp(str,"#BLOCK_INFO",11))
	{
		block_comp=0;
	}
	else if (!strncmp(str,"#COMP_BLOCK_INFO",16))
	{
		block_comp=1;
	}

	if (block_comp==0)
	{
		for (i=0;i<m_x*m_y*m_z;i++)
		{
			fgets(str,512,fp);
			bp_list[i].num=atoi(strtok(str," "));
			bp_list[i].index=atoi(strtok(NULL," "));
		}
	}
	else if (block_comp)
	{
		j=0;
		cur_pos=0;
		while (j<m_x*m_y*m_z)
		{
			fgets(str,512,fp);
			if ((str[0]=='0')&&(str[1]=='-'))
			{
				count=atoi(str+2);
				for (i=j;i<j+count;i++)
				{
					bp_list[i].num=0;
					bp_list[i].index=cur_pos;
				}
				j+=count;
			}
			else
			{
				bp_list[j].num=atoi(strtok(str," "));
				bp_list[j].index=atoi(strtok(NULL," "));
				cur_pos=bp_list[j].index+bp_list[j].num;
				j++;
			}
		}
	}

	//then point list
	fgets(str,512,fp);
	if (strncmp(str,"#POINT_INFO",11))//not the line: #POINT_INFO
	{
		printf("bad file format!\n");
		free(bp_list);
		free(point_list);
		fclose(fp);
		return 0;
	}

	for (i=0;i<point_number;i++)
	{
		fgets(str,512,fp);
		//x, y, z
		point_list[i].x=(float)atof(strtok(str," "));
		point_list[i].y=(float)atof(strtok(NULL," "));
		point_list[i].z=(float)atof(strtok(NULL," "));
		if (m_bNormal)
		{
			point_list[i].nx=(float)atof(strtok(NULL," "));
			point_list[i].ny=(float)atof(strtok(NULL," "));
			point_list[i].nz=(float)atof(strtok(NULL," "));
		}
		else{
			point_list[i].nx=0; 
			point_list[i].ny=0; 
			point_list[i].nz=1; 
		}

		if (m_bSize){
			point_list[i].radius=(float)atof(strtok(NULL," "));
		}
		else{
			point_list[i].radius=0.02f;
		}

		if (m_bColor){
			point_list[i].red=atoi(strtok(NULL," "));
			point_list[i].green=atoi(strtok(NULL," "));
			point_list[i].blue=atoi(strtok(NULL," "));
		}
		else{
			point_list[i].red=200;
			point_list[i].green=200;
			point_list[i].blue=0; 
		}
		if (m_bIndex){
			point_list[i].vindex=atoi(strtok(NULL," "));
		}
		else{
			point_list[i].vindex=0;
		}
	}

	fclose(fp);
	return 1;
}

