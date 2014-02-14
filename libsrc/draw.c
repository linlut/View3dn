/*****************************************************************************
                 Some simple SGI GL-based routines for CS480
******************************************************************************
    Author: Stan Sclaroff
            Boston University Computer Science Dept.
            January 27, 1995
******************************************************************************
 Modifications:

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/device.h>
#include "draw.h"

     
/* This routine opens a window.  Returns window id.  If window open
fails, then it returns window id = 0 */

/* This routine also initializes the following input devices:
the left mouse button, right mouse button, Q-Key, E-Key, and the
expose event (redraw).  */

long open_window(width,height,name)
long width,height;
char *name;
{

  long winid;

  /* put the graphical process in the foreground so that we can
     interact with it */

  foreground();

  /* set the prefered size of the window */

  prefsize(width,height);

  /* open the window, return 0 if failure */

  winid = winopen(name);
  if (winid == 0) {
      fprintf(stderr,"Cannot open window\n");
      return(0);
    }
  
  /* set default settings (this includes color map mode) */
  greset();
  clear();

  /* specify clipping region */
  ortho2(0,width,0,height);

  /* select devices to queue */
  qdevice(LEFTMOUSE);
  qdevice(RIGHTMOUSE);
  qdevice(QKEY);
  qdevice(EKEY);
  qdevice(TKEY);
  qdevice(REDRAW);
  return(winid);
}

/* close a window */
void close_window(winid)
long winid;
{
  winclose(winid);
}


/* allocate and initialize a pixmap */
/* this routine assumes that we're using color map mode. */
Pixmap *create_pixmap(winid,width,height)
     long winid;
     long width,height;
{
  Pixmap *pixmap;

  /* allocate structure */
  pixmap = (Pixmap *)malloc(sizeof(Pixmap));
  if(pixmap == 0)
    {
      fprintf(stderr,"Error allocating pixmap\n");
      return(0);
    }

  /* allocate the pixmap memory */
  pixmap->pixels = (Colorindex *)malloc(sizeof(Colorindex)*width*height);
  if(pixmap == 0)
    {
      fprintf(stderr,"Error allocating pixels for pixmap \n");
      return(0);
    }

  /* fill in the pertinent information */
  pixmap->w = width;
  pixmap->h = height;
  pixmap->winid = winid;

  /* return the initialized structure */
  return(pixmap);
}

/* free a pixmap */
void free_pixmap(pixmap)
     Pixmap *pixmap;
{
  if(pixmap==0 || pixmap->pixels==0)
    {
      fprintf(stderr,"Error freeing pixmap \n");
      return;
    }
  free(pixmap->pixels);
  free(pixmap);
}

/* post a pixmap to the screen */
/* color map mode is assumed */
void post_pixmap(pixmap)
     Pixmap *pixmap;
{
  winset(pixmap->winid);
  rectwrite(0,0,pixmap->w-1,pixmap->h-1,pixmap->pixels);
}

/* set the the whole pixmap to a particular color */
/* color map mode is assumed */
void set_pixmap(pixmap,color)
     Pixmap *pixmap;
     Colorindex color;
{
  int i,j;
  
  for(i=0;i<pixmap->h;++i)
    for(j=0;j<pixmap->w;++j)
      write_pixel(pixmap,j,i,color);
}


/* this is a pixmap that defines the pattern for a blue point symbol */
/* on a black background */
Colorindex symbol[7][7] = {BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK,
			     BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK,
			     BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK,
			     BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,BLUE,
			     BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK,
			     BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK,
			     BLACK,BLACK,BLACK,BLUE,BLACK,BLACK,BLACK};

/* routine to draw a point symbol in the specified window */
/* at the appropriate location.  Color map mode is assumed */
void draw_point(winid,point)
long winid;
Screencoord *point;
{
  winset(winid);
  rectwrite(point[0]-3,point[1]-3,point[0]+3,point[1]+3,&symbol[0][0]);
}

/* get an event, and return the mouse position if relevant */
Device get_event(winid,mouse_pos)
long winid;
Screencoord mouse_pos[2];
{
  short data;
  long x_origin,y_origin;
  Device event;
  
  /* set the window */
  winset(winid);

  /* read an event */
  do
    event = qread(&data);
  while (data != TRUE); 

  /* if it is a mouse event, then read the mouse position */
  /* calculate the mouse position relative to the window origin */
  if(event == RIGHTMOUSE || event == LEFTMOUSE)
    {
      getorigin(&x_origin,&y_origin);
      mouse_pos[0] = getvaluator(MOUSEX) - x_origin;
      mouse_pos[1] = getvaluator(MOUSEY) - y_origin;
    }

  /* return the event id */
  return event;
}
	









