static char *rcsid= "$Id: gleps.c,v 2.33 2003/03/28 16:47:46 fuhrmann Exp $";
/* This code has been cloned by J.Fuhrmann from rendereps.c by Mark Kilgard 
   which had the following header notice.*/
/****BEGIN MJK HEADER*/

/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

/* Example showing how to use OpenGL's feedback mode to capture
   transformed vertices and output them as Encapsulated PostScript.
   Handles limited hidden surface removal by sorting and does
   smooth shading (albeit limited due to PostScript). */

/****END MJK HEADER*/

/*
  From Mark's code, handling of lines&polygons had been taken.
  String handling is new.
  
  For more information about this stuff, see 
  http://reality.sgi.com/opengl/tips/Feedback.html
  */
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/gl.h>
#include "gleps.h"
#include "glwineps.h"


#define Max(a,b) (((a)>(b))?(a):(b))
	
#define EPS_SMOOTH_LINE_FACTOR 0.06  /* Lower for better smooth 
                                        lines. */
#define EPS_GOURAUD_THRESHOLD 0.1  /* Lower for better (slower) 
                                      smooth shading. */
	
#define PS_UPI 72.0
#define A4_XINCH 8.27
#define A4_YINCH 11.69

/* OpenGL's GL_3D_COLOR feedback vertex format. */
typedef struct _FBColorVertex {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat red;
  GLfloat green;
  GLfloat blue;
  GLfloat alpha;
} FBColorVertex;

#define FBColorVertexSize (sizeof(FBColorVertex)/sizeof(GLfloat))

static GLfloat pointSize=0;

static char *gouraudtriangleEPS[] =
{
  "/bd{bind def}bind def /triangle { aload pop   setrgbcolor  aload pop 5 3",
  "roll 4 2 roll 3 2 roll exch moveto lineto lineto closepath fill } bd",
  "/computediff1 { 2 copy sub abs threshold ge {pop pop pop true} { exch 2",
  "index sub abs threshold ge { pop pop true} { sub abs threshold ge } ifelse",
  "} ifelse } bd /computediff3 { 3 copy 0 get 3 1 roll 0 get 3 1 roll 0 get",
  "computediff1 {true} { 3 copy 1 get 3 1 roll 1 get 3 1 roll 1 get",
  "computediff1 {true} { 3 copy 2 get 3 1 roll  2 get 3 1 roll 2 get",
  "computediff1 } ifelse } ifelse } bd /middlecolor { aload pop 4 -1 roll",
  "aload pop 4 -1 roll add 2 div 5 1 roll 3 -1 roll add 2 div 3 1 roll add 2",
  "div 3 1 roll exch 3 array astore } bd /gouraudtriangle { computediff3 { 4",
  "-1 roll aload 7 1 roll 6 -1 roll pop 3 -1 roll pop add 2 div 3 1 roll add",
  "2 div exch 3 -1 roll aload 7 1 roll exch pop 4 -1 roll pop add 2 div 3 1",
  "roll add 2 div exch 3 -1 roll aload 7 1 roll pop 3 -1 roll pop add 2 div 3",
  "1 roll add 2 div exch 7 3 roll 10 -3 roll dup 3 index middlecolor 4 1 roll",
  "2 copy middlecolor 4 1 roll 3 copy pop middlecolor 4 1 roll 13 -1 roll",
  "aload pop 17 index 6 index 15 index 19 index 6 index 17 index 6 array",
  "astore 10 index 10 index 14 index gouraudtriangle 17 index 5 index 17",
  "index 19 index 5 index 19 index 6 array astore 10 index 9 index 13 index",
  "gouraudtriangle 13 index 16 index 5 index 15 index 18 index 5 index 6",
  "array astore 12 index 12 index 9 index gouraudtriangle 17 index 16 index",
  "15 index 19 index 18 index 17 index 6 array astore 10 index 12 index 14",
  "index gouraudtriangle 18 {pop} repeat } { aload pop 5 3 roll aload pop 7 3",
  "roll aload pop 9 3 roll 8 index 6 index 4 index add add 3 div 10 1 roll 7",
  "index 5 index 3 index add add 3 div 10 1 roll 6 index 4 index 2 index add",
  "add 3 div 10 1 roll 9 {pop} repeat 3 array astore triangle } ifelse } bd",
  NULL
};

#define EPS 1.0e-3
#define NEQ(a,b) ((a)<(b)-EPS || (a)>(b)+EPS)
/* put primitive at fb buffer location loc */
static char crdformat[32]={'%','.','1','f',' ','\0'};
static char colformat[32]={'%','.','2','f',' ','\0'};




void glepsSetOutputFormat(char * crd, char *col)
{

  strcpy(crdformat,crd);
  strcpy(colformat,col);

}

#define PutCrd(f,x) fprintf(file,crdformat,(x));
#define PutCol(f,x) fprintf(file,colformat,(x));
#define PutCmd(f,x) fprintf(file,"%s",(x));

typedef struct 
{
  double  xmin,xmax,ymin,ymax;
}BBox;

static void checkbbox(  FBColorVertex *vertex, int nvert, BBox *bbox)
{ 
  int i;
  for(i=0;i<nvert;i++)
    {
      if (vertex[i].x<bbox->xmin) bbox->xmin=vertex[i].x;
      if (vertex[i].x>bbox->xmax) bbox->xmax=vertex[i].x;
      if (vertex[i].y<bbox->ymin) bbox->ymin=vertex[i].y;
      if (vertex[i].y>bbox->ymax) bbox->ymax=vertex[i].y;
    }
}

 /* Count how many primitives there are, at the same time get bouning
     box. Bounding box may not work if text is involved. */
static void CheckBBox(GLfloat *buffer,GLint size, BBox *bbox, int *np)
{
  int token,nvertices,nprimitives;
  GLfloat *loc,*end;
  //FBColorVertex *vertex;
  
  end = buffer + size;
  
  loc = buffer;
  nprimitives=0;
  while (loc < end) 
    {
      token = *loc;
      loc++;
      switch (token) {
      case GL_LINE_TOKEN:
      case GL_LINE_RESET_TOKEN:
	checkbbox((FBColorVertex*)loc,2,bbox);
	loc += 2*FBColorVertexSize;
	break;
      case GL_POLYGON_TOKEN:
	nvertices = *loc;
	loc++;
	checkbbox((FBColorVertex*)loc,nvertices,bbox);
	loc += (FBColorVertexSize * nvertices);
	break;
      case GL_POINT_TOKEN:
	checkbbox((FBColorVertex*)loc,1,bbox);
	loc += FBColorVertexSize;
	break;
      case GL_PASS_THROUGH_TOKEN: 
	checkbbox((FBColorVertex*)(loc-FBColorVertexSize-1),1,bbox); 
	loc++; 
	break;
      case GL_BITMAP_TOKEN: 
      case GL_DRAW_PIXEL_TOKEN: 
      case GL_COPY_PIXEL_TOKEN: 
	checkbbox((FBColorVertex*)loc,1,bbox);
	loc+=FBColorVertexSize;
	break;
      default:
	printf("gleps error: Check bbox 0: Unexpected token (%d).\n",token);
	return;
      }
	nprimitives++;
    }
  *np=nprimitives;
}

#define ESTRLEN 132
static GLfloat *PutPrimitive(glWindow w, 
			     FILE * file, 
			     GLfloat * loc)
{
  int token;
  int nvertices, i, j;
  GLfloat red, green, blue;
  int smooth;
  //double ssize;
  GLfloat dx, dy, dr, dg, db, absR, absG, absB, colormax;
  int steps;
  FBColorVertex *vertex;
  GLfloat xstep, ystep, rstep, gstep, bstep;
  GLfloat xnext, ynext, rnext, gnext, bnext, distance;
  GLfloat colfac;
  char *str;
  char estr[ESTRLEN];

  token = *loc;
  loc++;
  switch (token) 
    {
    case GL_LINE_RESET_TOKEN:
    
    case GL_LINE_TOKEN:
      vertex = (FBColorVertex *) loc;
      colfac=1.0;
      {
	for (i = 0; i < 2; i++) 
	  if (vertex[i].red>2.0|| vertex[i].blue>2.0|| vertex[i].green>2.0) 
	    {
	      colfac=2.0/65535.0;
	      break;
	    }
      }
	 
      dr = (vertex[1].red*colfac - vertex[0].red*colfac);
      dg = (vertex[1].green*colfac - vertex[0].green*colfac);
      db = (vertex[1].blue*colfac - vertex[0].blue*colfac);
      
      if (NEQ(dr,0.0) || NEQ(dg,0.0) || NEQ(db , 0.0)) {
	/* Smooth shaded line. */
	dx = vertex[1].x - vertex[0].x;
	dy = vertex[1].y - vertex[0].y;
	distance = sqrt(dx * dx + dy * dy);
	
	absR = fabs(dr);
	absG = fabs(dg);
	absB = fabs(db);
	
	colormax = Max(absR, Max(absG, absB));
	steps = Max(1.0, colormax * distance * EPS_SMOOTH_LINE_FACTOR);
	
	xstep = dx / steps;
	ystep = dy / steps;
	
	rstep = dr / steps;
	gstep = dg / steps;
	bstep = db / steps;
	
	xnext = vertex[0].x;
	ynext = vertex[0].y;
	rnext = vertex[0].red*colfac;
	gnext = vertex[0].green*colfac;
	bnext = vertex[0].blue*colfac;
	
	/* Back up half a step; we want the end points to be
	   exactly the their endpoint colors. */
	xnext -= xstep / 2;
	ynext -= ystep / 2;
	rnext -= rstep / 2;
	gnext -= gstep / 2;
	bnext -= bstep / 2;
      } else {
	/* Single color line. */
	steps = 0;
      }
      PutCol(file,vertex[0].red*colfac);
      PutCol(file,vertex[0].green*colfac);
      PutCol(file,vertex[0].blue*colfac);
      PutCmd(file,"C\n");
      PutCrd(file,vertex[0].x);
      PutCrd(file,vertex[0].y);
      PutCmd(file,"M\n");
      
      for (i = 0; i < steps; i++) {
	xnext += xstep;
	ynext += ystep;
	rnext += rstep;
	gnext += gstep;
	bnext += bstep;
	PutCrd(file,xnext);
	PutCrd(file,ynext);
	PutCmd(file,"LS\n");
        PutCol(file,rnext);
        PutCol(file,gnext);
        PutCol(file,bnext);
	PutCmd(file,"C\n");
	PutCrd(file,xnext);
	PutCrd(file,ynext);
	PutCmd(file,"M\n");
      }
      PutCrd(file,vertex[1].x);
      PutCrd(file,vertex[1].y);
      PutCmd(file,"LS\n");

      loc += 2*FBColorVertexSize; 
      
      break;

    case GL_POLYGON_TOKEN:
      nvertices = *loc;
      loc++;
      
      vertex = (FBColorVertex *) loc;
      /* this is a hack to correct a bug in (older ?) Mesa versions.*/
      if (nvertices > 0) 
	{ 
	  /* no colfac calculations for exact comparison!!! */
	  i=0;
	  red = vertex[i].red;
	  green = vertex[i].green;
	  blue =  vertex[i].blue;
	  
	  smooth = 0;
	  for (i = 1; i < nvertices; i++) 
	    if (NEQ(red,vertex[i].red) || NEQ(green,vertex[i].green) || NEQ(blue,vertex[i].blue)) 
	      {
		smooth=1; 
		break;
	      }
	  
	  colfac=1.0;
	  /*if (glwIsMesa(w))*/
	  {
	    for (i = 0; i < nvertices; i++) 
	      if (vertex[i].red>2|| vertex[i].blue>2|| vertex[i].green>2) 
		{
		  colfac=2/65535.0f;
		  break;
		}
	  }
      
	  
	  
	  if (smooth) 
	    {
	      /* Smooth shaded polygon; varying colors at vertices. */
	      int triOffset;
	      
	      /* Break polygon into "nvertices-2" triangle fans. */
	      for (i = 0; i < nvertices - 2; i++) 
		{
		  triOffset = i * FBColorVertexSize;
		  PutCmd(file,"[");
		  PutCrd(file,vertex[0].x);
		  PutCrd(file,vertex[i + 1].x);
		  PutCrd(file,vertex[i + 2].x);
		  PutCrd(file,vertex[0].y);
		  PutCrd(file,vertex[i + 1].y);
		  PutCrd(file,vertex[i + 2].y);
		  PutCmd(file,"] [");
		  PutCol(file,vertex[0].red*colfac);
		  PutCol(file,vertex[0].green*colfac);
		  PutCol(file,vertex[0].blue*colfac);
		  PutCmd(file,"] [");
		  PutCol(file,vertex[i+1].red*colfac);
		  PutCol(file,vertex[i+1].green*colfac);
		  PutCol(file,vertex[i+1].blue*colfac);
		  PutCmd(file,"] [");
		  PutCol(file,vertex[i+2].red*colfac);
		  PutCol(file,vertex[i+2].green*colfac);
		  PutCol(file,vertex[i+2].blue*colfac);
		  PutCmd(file,"] GT\n");
		}
	    } 
	  else 
	    {
	      
	      /* Flat shaded polygon; all vertex colors the same. */
	      PutCol(file,red*colfac);
	      PutCol(file,green*colfac);
	      PutCol(file,blue*colfac);
	      PutCmd(file,"C\n");
	      
	      /* Draw a filled polygon. */
	      PutCmd(file,"NP\n");
	      PutCrd(file,vertex[0].x);
	      PutCrd(file,vertex[0].y);
	      PutCmd(file,"M\n");
	      for (i = 1; i < nvertices; i++) 
		  {
		    PutCrd(file,vertex[i].x);
		    PutCrd(file,vertex[i].y);
		    PutCmd(file,"L\n");
		  }
	      PutCmd(file,"CF\n\n");
	    }
	}
      loc += nvertices * FBColorVertexSize;  /* Each vertex element in the
						feedback buffer is 7 GLfloats. */
      break;
      
    case GL_POINT_TOKEN:
      vertex = (FBColorVertex *) loc;

      colfac=1.0f;
      if (vertex[0].red>2.0|| vertex[0].blue>2.0|| vertex[0].green>2.0) 
	{
	  colfac=2/65535.0f;
	}
   
      PutCol(file,vertex[0].red*colfac);
      PutCol(file,vertex[0].green*colfac);
      PutCol(file,vertex[0].blue*colfac);
      PutCmd(file,"C\n");
      PutCrd(file,vertex[0].x);
      PutCrd(file,vertex[0].y);
      PutCmd(file,"PT\n");


      loc += FBColorVertexSize;           /* Each vertex element in the feedback
					     buffer is 7 GLfloats. */
      break;

    case GL_PASS_THROUGH_TOKEN: 
      vertex=(FBColorVertex *) (loc-FBColorVertexSize-1);

      /*        fprintf(file, "%g FS\n",glwGetFBStringSize(w,(int)*loc));
      ssize=glwGetFBStringSize(w,(int)*loc);
      */
      colfac=1.0;
      if (vertex[0].red>2.0 || vertex[0].blue>2.0 || vertex[0].green>2.0) 
	{
	  colfac=2/65535.0f;
	}
      PutCol(file,vertex[0].red*colfac);
      PutCol(file,vertex[0].green*colfac);
      PutCol(file,vertex[0].blue*colfac);
      PutCmd(file,"C\n");
      PutCrd(file,vertex[0].x);
      PutCrd(file,vertex[0].y);
      PutCmd(file,"M\n");

      str=glwGetFBString(w,(int)*loc);
      for(i=j=0;str[i]!='\0';i++)
	{
	  switch (str[i])
	    {
	    case '(' : estr[j++]='\\';estr[j++]='(';  break;
	    case ')' : estr[j++]='\\';estr[j++]=')';  break;
	    case '\\': estr[j++]='\\';estr[j++]='\\'; break;
	    default  : estr[j++]=str[i];
	    }
	  if (i>=ESTRLEN) break;
	}
      estr[j]='\0';
      
      fprintf(file,"(%s) LBshow\n",estr);
      loc++; 
      break;



    case GL_BITMAP_TOKEN: 
    case GL_DRAW_PIXEL_TOKEN: 
    case GL_COPY_PIXEL_TOKEN: 
      loc+=FBColorVertexSize; 
      break;

      
      
    default:
      printf("gleps: PutPrimitive: Unexpected token (%d).\n",token);
      return NULL;
    }
  return loc;
}

static void PutUnsortedFeedback(glWindow w, 
				FILE * file, 
				GLint size, 
				GLfloat * buffer)
{
  GLfloat *loc, *end;
  
  loc = buffer;
  end = buffer + size;
  while (loc < end) 
    {
      loc = PutPrimitive(w,file, loc);
      if (loc==NULL) return;
    }
}

typedef struct _DepthIndex {
  GLfloat *ptr;
  GLfloat depth;
} DepthIndex;

static int compare(const void *a, const void *b)
{
  DepthIndex *p1 = (DepthIndex *) a;
  DepthIndex *p2 = (DepthIndex *) b;
  GLfloat diff = p2->depth - p1->depth;
  
  if (diff > 0.0) 
    {
      return 1;
    } 
  else if (diff < 0.0) 
    {
      return -1;
    } 
  else 
    {
      return 0;
    }
}

static void PutSortedFeedback(glWindow w, 
			      FILE * file, 
			      GLint size, 
			      GLfloat * buffer, 
			      int nprimitives)

{
  int token;
  GLfloat *loc,*end;
  FBColorVertex *vertex;
  GLfloat depthSum,lastdepth;
  int item;
  DepthIndex *prims;
  int nvertices, i;
  
  end = buffer + size;
  nprimitives=0;
  /* Count how many primitives there are. */
  if (nprimitives ==0)
    {
      loc = buffer;
      while (loc < end) 
	{
	  token = *loc;
	  loc++;
	  switch (token) {
	  case GL_LINE_TOKEN:
	  case GL_LINE_RESET_TOKEN:
	    loc += 2*FBColorVertexSize;
	    break;
	  case GL_POLYGON_TOKEN:
	    nvertices = *loc;
	    loc++;
	    loc += (FBColorVertexSize * nvertices);
	    break;
	  case GL_POINT_TOKEN:
	    loc += FBColorVertexSize;
	    break;
	  case GL_PASS_THROUGH_TOKEN: loc++; break;
	  case GL_BITMAP_TOKEN: 
	  case GL_DRAW_PIXEL_TOKEN: 
	  case GL_COPY_PIXEL_TOKEN: 
	    loc+=FBColorVertexSize;
	    /*	printf("Bitmaps & Pixels not supported in feedback mode\n");*/
	    break;
	  default:
	    printf("gleps error: Sorted FB 0: Unexpected token (%d).\n",token);
	    return;
	  }
	  nprimitives++;
	}
    }
  /* 
     Allocate an array of pointers that will point back at
     primitives in the feedback buffer.  There will be one
     entry per primitive.  This array is also where we keep the
     primitive's average depth.  There is one entry per
     primitive  in the feedback buffer. 
  */
  
  prims = (DepthIndex *) malloc(sizeof(DepthIndex) * nprimitives);
  
  item = 0;
  loc = buffer;
  while (loc < end) 
    {
      prims[item].ptr = loc;  /* Save this primitive's location. */
      token = *loc;
      loc++;
      switch (token) 
	{
	  
	case GL_LINE_TOKEN:
	case GL_LINE_RESET_TOKEN:
	  vertex = (FBColorVertex *) loc;
	  depthSum = vertex[0].z + vertex[1].z;
	  prims[item].depth = depthSum / 2.0;
	  loc += 2*FBColorVertexSize;
	  break;
      
	case GL_POLYGON_TOKEN:
	  nvertices = *loc;
	  loc++;
	  vertex = (FBColorVertex *) loc;
	  depthSum = vertex[0].z;
	  for (i = 1; i < nvertices; i++) {
	    depthSum += vertex[i].z;
	  }
	  prims[item].depth = depthSum / nvertices;
	  loc += (FBColorVertexSize * nvertices);
	  break;
      
	case GL_POINT_TOKEN:
	  vertex = (FBColorVertex *) loc;
	  prims[item].depth = vertex[0].z;
	  loc += FBColorVertexSize;
	  break;
	  
	  
	case GL_PASS_THROUGH_TOKEN: 
	  /* Pass through is only used for string handling. It 
	     assumes that before it has been called, a dummy bitmap
	     has been invoked to get the position.
	  */
	  prims[item].depth=lastdepth;
	  loc++;  
	  break;
      
	case GL_BITMAP_TOKEN: 
	case GL_DRAW_PIXEL_TOKEN: 
	case GL_COPY_PIXEL_TOKEN: 
	  /* It is assumed that this stuff is currently used only to 
	     handle strings. In fact, marker bitmaps are written to just 
	     get some position.
	  */
	  vertex = (FBColorVertex *) loc;
	  lastdepth=prims[item].depth = vertex[0].z;
	  loc+=FBColorVertexSize;
	  break;
	  
	default:
	  printf("gleps error: Sorted FB 1: Unexpected token (%d).\n",token);
	  return;
	}
      item++;
    }
 
  assert(item == nprimitives);
  
  /* Sort the primitives back to front. */
  qsort(prims, nprimitives, sizeof(DepthIndex), compare);
  
  /* XXX Understand that sorting by a primitives average depth
     doesn't allow us to disambiguate some cases like self
     intersecting polygons.  Handling these cases would require
     breaking up the primitives.  That's too involved for this
     example.  Sorting by depth is good enough for lots of
     applications. */
  
  /* Emit the Encapsulated PostScript for the primitives in
     back to front order. */
  for (item = 0; item < nprimitives; item++) 
    {
      (void) PutPrimitive(w,file, prims[item].ptr);
    }
  
  free(prims);
}


void scale_output(
		  int ww,
		  int wh,
		  BBox *bbox,
		  double *xtran,
		  double *ytran,
		  double *output_scale,
		  BBox *tbbox)
{
  /* we want to place our stuff nicely @ A4 Potrait... */
  *output_scale=(A4_XINCH-1.0)*PS_UPI/(float)ww;
  *xtran=(A4_XINCH*PS_UPI-(float)ww* *output_scale)/2.0;
  *ytran=((A4_YINCH-0.5)*PS_UPI-wh* *output_scale);

  tbbox->xmin=*xtran+bbox->xmin* *output_scale;
  tbbox->xmax=*xtran+bbox->xmax* *output_scale;
  tbbox->ymin=*ytran+bbox->ymin* *output_scale;
  tbbox->ymax=*ytran+bbox->ymax* *output_scale;
}



void gleps_emit_file_header(glWindow w,
			    FILE *file,
			    int npages, 
			    BBox *bbox)
{
  time_t t;
  double xtran,ytran,output_scale;
  GLfloat clearColor[4];
  GLfloat lineWidth;
  int ww,wh;
  int i;
  BBox xbbox,tbbox;
  assert(file);
  glwGetWindowSize(w,&ww,&wh);
  if (bbox==NULL)
    {
      xbbox.xmin=0.0;
      xbbox.xmax=ww;
      xbbox.ymin=0.0;
      xbbox.ymax=wh;
      bbox=&xbbox;
    }
  scale_output(ww,wh,bbox,&xtran,&ytran,&output_scale,&tbbox);
  
  /* Read back a bunch of OpenGL state to help make the EPS
     consistent with the OpenGL clear color, line width, point
     size, and viewport. */

  /* This is still inconsistent when multiple viewports, line sizes etc are
     used. Possibly, we will have to introduce some glwNewObject stuff to
     catch the necessary values when they are changed */


  glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
  glGetFloatv(GL_LINE_WIDTH, &lineWidth);
  glGetFloatv(GL_POINT_SIZE, &pointSize);

  t=time(&t); 

  fprintf(file,"%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(file,"%%%%Title: %s\n",glwGetTitle(w));
  fprintf(file,"%%%%Creator: gltools/gleps.c $Revision: 2.33 $ (c) Juergen Fuhrmann <fuhrmann@wias-berlin.de> \n");
  fprintf(file,"%%%%CreationDate: %s",ctime(&t));
  if (npages)
    fprintf(file,"%%%%Pages: %d\n",npages);
  else
    fprintf(file,"%%%%Pages: atend\n");
    
  fprintf(file,"%%%%BoundingBox: %5.0f %5.0f %5.0f %5.0f\n",
	  tbbox.xmin,
	  tbbox.ymin,
	  tbbox.xmax,
	  tbbox.ymax);

  fprintf(file,"%%%%EndComments\n");
  fprintf(file,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  fprintf(file,"%%BEGIN OF TUNING SECTION\n");  
  fprintf(file,"%%\n");
  fprintf(file,"%%Fonts:\n");
  fprintf(file,"%%By changing the definition of Fontscale and Fontname\n");
  fprintf(file,"%%you can modify the font for your needs\n");
  fprintf(file,"/Fontscale %g def\n",glwGetFontSize(w)*wh);
  fprintf(file,"/Fontname  /Courier def\n%%\n");
  fprintf(file,"%%\n");
  fprintf(file,"%%Triangle shading:\n");
  fprintf(file,"%%By changing the definition of GouraudThreshold\n");
  fprintf(file,"%%you can modify the quality of rendering of shaded triangles.\n");
  fprintf(file,"%%Lower values mean increased smoothness at the cost of longer printing times\n");
  fprintf(file, "/GouraudThreshold %g def\n", EPS_GOURAUD_THRESHOLD);
  fprintf(file,"%%\n");
  fprintf(file,"%%Line width\n");
  fprintf(file, "/DefaultLineWidth %g def\n", lineWidth-1.0);
  fprintf(file,"%%PointSize\n");
  fprintf(file, "/DefaultPointSize %g def\n", pointSize-1.0);
  fprintf(file,"%%Background color in rgb values\n");
  fprintf(file, "/DefaultBackgroundColor { %g %g %g } def\n", clearColor[0], clearColor[1], clearColor[2]);
  fprintf(file,"%%\n");
  fprintf(file,"%%END OF TUNING SECTION\n");  
  fprintf(file,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");


  /* Output Frederic Delhoume's "gouraudtriangle" PostScript
     fragment. */
  fputs("% The gouraudtriangle PostScript fragement below is free\n", file);
  fputs("% written by Frederic Delhoume <delhoume@ilog.fr>\n", file);
  fprintf(file, "/threshold GouraudThreshold def\n");
  for (i = 0; gouraudtriangleEPS[i]; i++) 
    fprintf(file, "%s\n", gouraudtriangleEPS[i]);
  fprintf(file,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  fprintf(file,"/GT {gouraudtriangle} bind def\n");
  fprintf(file,"/M {moveto} bind def\n");
  fprintf(file,"/L {lineto} bind def\n");
  fprintf(file,"/LS {lineto stroke} bind def\n");
  fprintf(file,"/CF {closepath fill} bind def\n");
  fprintf(file,"/N {newpath} bind def\n");
  fprintf(file,"/C {setrgbcolor} bind def\n");
  fprintf(file,"/NP {newpath} bind def\n");
  fprintf(file,"/PT {DefaultPointSize 0 360 arc fill} bind def\n");

  fprintf(file,"/LBshow {0  0  rmoveto show} bind def\n");
  fprintf(file,"/LCshow {0 Fontscale 2 div neg   rmoveto show} bind def\n");
  fprintf(file,"/LTshow {0 Fontscale neg rmoveto show} bind def\n");
  fprintf(file,"/CBshow {dup stringwidth pop 2 div neg   0 rmoveto  show} bind def\n");
  fprintf(file,"/CCshow {dup stringwidth pop 2 div neg  Fontscale 2 div neg  rmoveto  show} bind def\n");
  fprintf(file,"/CTshow {dup stringwidth pop 2 div neg  Fontscale neg rmoveto  show} bind def\n");
  fprintf(file,"/RBshow {dup stringwidth pop neg  0  rmoveto  show} bind def\n");
  fprintf(file,"/RCshow {dup stringwidth pop neg Fontscale 2 div neg   rmoveto  show} bind def\n");
  fprintf(file,"/RTshow {dup stringwidth pop neg Fontscale neg rmoveto  show} bind def\n");
  fprintf(file,"/F { findfont Fontscale  scalefont setfont} bind def\n");



  fprintf(file,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  fprintf(file,"%%%%EndProlog\n");

}

static void gleps_emit_page_header(
				   glWindow w, 
				   FILE *file, 
				   int page, 
				   BBox *bbox)
{
  double xtran,ytran,output_scale;
  int ww,wh;
  BBox xbbox,tbbox;
  
  glwGetWindowSize(w,&ww,&wh);
  if (bbox==NULL)
    {
      xbbox.xmin=0.0;
      xbbox.xmax=ww;
      xbbox.ymin=0.0;
      xbbox.ymax=wh;
      bbox=&xbbox;
    }
  scale_output(ww,wh,bbox,&xtran,&ytran,&output_scale,&tbbox);

  fprintf(file,"%%%%Page: %d %d\n",page,page);
  fprintf(file,"gsave\n");
  fprintf(file,"%g %g  translate %g %g scale\n",
	xtran,ytran,output_scale, output_scale);
  fprintf(file, "DefaultLineWidth setlinewidth\n");
  fprintf(file, "DefaultBackgroundColor setrgbcolor\n");
  fprintf(file, "%g %g %g %g rectfill\n\n", 
	  bbox->xmin, 
	  bbox->ymin, 
	  bbox->xmax-bbox->xmin,
	  bbox->ymax-bbox->ymin);
  fprintf(file,"Fontname F\n");

}

static void gleps_emit_page_trailer(glWindow w, FILE *file, int page)
{
  fputs("showpage\ngrestore\n", file);

}

static void gleps_emit_file_trailer(glWindow w, FILE *file,int pages)
{
  fprintf(file,"%%%%Trailer\n");
  fprintf(file,"%%%%Pages: %d\n",pages);
}

static void Put(glWindow w, 
		FILE * file, 
		int doSort,
		int size, 
		GLfloat * buffer,
		int crop)
{
  BBox bbox,*xbbox=NULL;
  int nprimitives=0;
  if (size<=0 || buffer==NULL) return;

  if (crop){
      bbox.xmin=1.0e30;
      bbox.xmax=-1.0e30;
      bbox.ymin=1.0e30;
      bbox.ymax=-1.0e30;
      CheckBBox(buffer,size,&bbox,&nprimitives);
      xbbox=&bbox;
  }    

  //gleps_emit_file_header(w,file,1,xbbox);
  gleps_emit_page_header(w,file,1,xbbox);
  
  if (doSort) 
    PutSortedFeedback(w,file, size, buffer,nprimitives);
  else 
    PutUnsortedFeedback(w,file, size, buffer);
  
  //gleps_emit_page_trailer(w,file,1);
  //gleps_emit_file_trailer(w,file,1);
}


void glepsEmitHeader(glWindow w, FILE *file)
{
  gleps_emit_file_header(w,file,0,0);
}

void glepsEmitTrailer(glWindow w, FILE *file, int npages)
{
  gleps_emit_file_trailer(w,file,npages);
}

void glepsGrabEPSFrame(glWindow w, FILE *file, int npage)
{
  glwRenderFB(w);

//  gleps_emit_page_header(w,file,npage,0);
//  PutUnsortedFeedback(w,file, glwGetFBSize(w), glwGetFB(w));
	int dosort = 1;
	int crop = 0;
	Put(w, file, dosort, glwGetFBSize(w), glwGetFB(w), crop); 

  gleps_emit_page_trailer(w,file,1);
  glwResetFB(w);  
}

void glepsDumpSorted(glWindow w, FILE *file, int crop)
{
  assert(w!=NULL);
  assert(file!=NULL);
  glwRenderFB(w);
  Put(w,file, 1 ,glwGetFBSize(w), glwGetFB(w),crop);
  glwResetFB(w);
}

void glepsDumpUnSorted(glWindow w, FILE *file,int crop)
{
  assert(w!=NULL);
  assert(file!=NULL);
  glwRenderFB(w);
  Put(w,file, 0 ,glwGetFBSize(w), glwGetFB(w),crop);
  glwResetFB(w);
}

void glepsDump(glWindow w, FILE *file)
{
  glepsDumpSorted(w,file,0);
}

void glepsDump2(glWindow w, FILE *file,int sort,int crop)
{
  if (sort)
    glepsDumpSorted(w,file,crop);
  else
    glepsDumpUnSorted(w,file,crop);
}


