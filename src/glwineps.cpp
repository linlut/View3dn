#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/gl.h>
#include "view3d.h"
#include "gleps.h"
#include "glwineps.h"




double glwGetFontSize(glWindow w)
{
  return 11; //w->font_size;
}

char * glwGetTitle(glWindow w)
{
  return "window";
}


void glwGetWindowSize(glWindow w, int *width, int *height)
{
  *width=w->width();
  *height=w->height();
}

char *glwGetDefault(char *resource, char *dflt)
{
    return NULL;
}



/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static FILE *eps_file=NULL;
static int eps_recording=0;
static int eps_recorded_frame_num=0;
static int eps_frame_num=0;
static int eps_skip=0;  


#define GLW_FEEDBACK_BUFSIZE 1000000
#define GLW_FEEDBACK_MAX_STRINGS 1000
typedef void (*glwRedrawFunc)(glWindow w,void * user_data);

static int fb_mode;
static int fb_num_strings;
static char *fb_strings[GLW_FEEDBACK_MAX_STRINGS];
static double fb_string_size[GLW_FEEDBACK_MAX_STRINGS];
static GLfloat *fb_buffer=NULL;
static int fb_initial_size=2L*1024*1024;
static int fb_size=0;
static int font_size=11;
static glwRedrawFunc pRedrawFunc;



void glwStartEPSRecording(
		       glWindow win, 
		       char *fileName,
		       int skip
		       )
{
	eps_file=fopen(fileName,"w");
	eps_recording=1;
    
	assert(eps_file!=NULL);
	eps_recorded_frame_num=0;
	eps_frame_num=0;
	if (skip<0) skip=0;
	eps_skip=skip+1;  
}


void glwStopEPSRecording(glWindow win)
{	
	//in file gleps.cpp
	//extern void glepsEmitTrailer(glWindow w, FILE *file, int npages);
	glepsEmitTrailer(win,eps_file,eps_recorded_frame_num);

	if (eps_recording==1){
		fclose(eps_file);
		printf("%d eps frames recorded\n",eps_recorded_frame_num);
	}
	eps_recording=0;
	eps_file = NULL;
}


void glwDumpOnSwap(glWindow w, glwRedrawFunc pfunc)
{
	//set draw func;
	pRedrawFunc = pfunc;

	//write header;
	if(eps_recorded_frame_num==0)
		glepsEmitHeader(w, eps_file); // emitting header here solves clear color problem 

	//write the frame;
	glepsGrabEPSFrame(w, eps_file, eps_recorded_frame_num+1);
	eps_recorded_frame_num++;	
}


bool ExportEPSFile(const char *fname, glWindow win, void *drawfunc)
{
	assert(drawfunc!=NULL);
	assert(win!=NULL);
	glwStartEPSRecording(win, (char*)fname, 0);
	glwDumpOnSwap(win, (glwRedrawFunc)drawfunc);
	glwStopEPSRecording(win);
	return true;
}

 
void glwStartMPEGRecording(
		       glWindow win, 
		       char *fileNameStub,
		       int skip
		       )
{
}

void glwStopMPEGRecording(glWindow win)
{
}


#define GLW_SCREEN_WIDTH 2048
char pixelbuf[GLW_SCREEN_WIDTH*3];

void glwGrabPPM(glWindow win, FILE *f)
{
  register int i;
  int w,h;
  glwGetWindowSize(win,&w,&h);
  assert(f!=NULL);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
  fprintf(f, "P6\n");
  fprintf(f, "%d %d\n", w, h);
  fprintf(f, "%d\n", 255);
  for(i=1;i<=h;i++)
    {
      glReadPixels(0, h-i, w,1, GL_RGB, GL_UNSIGNED_BYTE, pixelbuf);
      fwrite(pixelbuf, 3, w, f);
    }
}

void  glwGrabPPMAndPrint(glWindow w)
{
  FILE *f;
  char *command;
  command=glwGetDefault("printerCommand","pnmtops | lpr");
  f=fopen(command,"w");
  glwGrabPPM(w,f);
  fclose(f);
}

void  glwDump(
              glWindow win, 
              void * info, 
              char *fileName, 
              int w, 
              int h
              )
{
  FILE *f=fopen(fileName,"w");
  glwGrabPPM(win,f);
  fclose(f);
}


/*******************************************/

void glwRenderFB(glWindow w)
{
	int size=fb_initial_size;
	fb_size=0;
	while (fb_size<=0){ 
		fb_buffer =(GLfloat*) calloc(size, sizeof(GLfloat));
		if (fb_buffer==NULL){
			fprintf(stderr, "Not enough memory for feedback buffer");
			return;
		}

		glFeedbackBuffer(size, GL_3D_COLOR, fb_buffer);
		glRenderMode(GL_FEEDBACK);
		fb_mode=1;

		///===redraw the window===
		assert(pRedrawFunc!=NULL);
		pRedrawFunc(w, NULL);
		///=======================

		fb_mode=0;
		fb_size = glRenderMode(GL_RENDER);
		if (fb_size<=0){
			glwResetFB(w);
			fprintf(stderr,"resizig feedback buffer to %.3g MB\n", ((double)size)/(1024.0*1024.0));
			size=2*size;
		}
    }
	fb_initial_size=size;
}


void glwResetFB(glWindow w)
{
	int i;
	for (i=0;i<fb_num_strings;i++){  
		if (fb_strings[i])
			free(fb_strings[i]);
		fb_strings[i]=NULL;
    }
	fb_num_strings=0;  
	fb_size=0;
	if (fb_buffer!=NULL) 
		free(fb_buffer);
}


float *glwGetFB(glWindow w)
{
	return fb_buffer;
}

int glwGetFBSize(glWindow w)
{
	return fb_size;
}

char *glwGetFBString(glWindow w,int istring)
{
	return fb_strings[istring];
}

double glwGetFBStringSize(glWindow w,int istring)
{
	return fb_string_size[istring];
}

int glwPrintFB(glWindow w, char *s)
{
	if (fb_mode){
		GLint buf=0;
		if (fb_num_strings<GLW_FEEDBACK_MAX_STRINGS){
			glDrawPixels(1,1,GL_COLOR_INDEX,GL_INT,&buf); // draw some cheap dummy 
			glPassThrough(fb_num_strings);
			fb_strings[fb_num_strings]=(char*)malloc(strlen(s));
			strcpy(fb_strings[fb_num_strings],s);
			fb_string_size[fb_num_strings]=font_size;
			fb_num_strings++;
		}
		return 1;
    }
	else
		return 0;
}


void glwInitFB(glWindow w)
{
	fb_num_strings=0;
	fb_buffer=0;
	fb_initial_size=2*1024*1024; /*  MB */
}

