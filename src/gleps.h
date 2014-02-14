/*
% $Id: gleps.h,v 2.10 2003/03/28 16:47:46 fuhrmann Exp $

\section{gleps  - Encapsulated Postscript Dump}
$Revision: 2.10 $\\
$Date: 2003/03/28 16:47:46 $\\
Author: J\"urgen Fuhrmann\\
\bigskip
%--------------------------------------------------------------------

This module  provides the possibility  to dump rendered  graphics into
vector postscript files using  the feedback buffer rendering mechanism
of  OpenGL. It  is in  a  beta state  and possibly  will remain  there
because  it  is  not  that  easy  to  map  correctly  all  the  OpenGL
functionality to Postscript. It should reasonably well render graphics
which remains in  the limits of the OpenGL features  used by the other
gltools parts.


This code  would not  exist if Mark  Kilgard wouldn't have  placed his
rendereps  sample code  onto the  net, and  if there  would  not exist
Frederic Delhoume's free gouraudtriangle postscript code.  From Mark's
code, handling of lines and  polygons has been taken.  String handling
is new.

I tried  to keep the generated  postscript files as  human readable as
possible.  Especially, there is a "tuning section" in the prolog where
the user can change fonts, font sizes etc.

The main problems  with the output are caused by  the fact that OpenGL
does not know alignment of  bitmaps (bitmaps are used by glxUseXFont),
and that there  seems to be no simple possibility  to align strings in
OpenGL output. So  only bottom left alignment of  strings is possible,
this is not quite beautiful. This problem also leads to the fact that
Bounding box calculation in presence of strings may be incorrect.

Until now, I  only see the possibility to tune the  eps files by hand.
In the ps  header there are defined some  alingned show commands which
can be  used to improve  label placement in  the ps file. Any  idea to
cope with this situation is appreciated.

*/

//class Viewer;
//#define glWindow Viewer *
class QGLViewer;
#define glWindow QGLViewer *


/*\func{glepsDumpUnSorted}*/
void glepsDumpUnSorted(glWindow w, FILE *file,int crop);

/*

  Create vector postscript dump using the feedback buffer
   mechanism, but without hidden surface removal.
  The crop flag is used to decide whether to crop ps output to
  the actually drawn area or not.
   */


/*\func{glepsDumpSorted}*/
void glepsDumpSorted(glWindow w, FILE *file, int crop);
/*
  Create vector postscript dump using the feedback buffer
  mechanism, with hidden surface removal based on sorting
  the feedback buffer before the dump.
  The crop flag is used to decide whether to crop ps output to
  the actually drawn area or not.

  */ 


/*\func{glepsSetOutputFormat}*/
void glepsSetOutputFormat(char * coord_fmt, char *color_fmt);
/*
  Set the output format for floating point numbers  (different for coordinates and colors)
  in the postscript file.
  (The more accurate the format, the longer the file...). There have to
  be a trailing spaces both formats.
*/
void glepsEmitHeader(glWindow w, FILE *file);
void glepsEmitTrailer(glWindow w, FILE *file, int npages);
void glepsGrabEPSFrame(glWindow w, FILE *file, int npage);

/*
\begin{comment}
 *
 *$Log: gleps.h,v $
 *Revision 2.10  2003/03/28 16:47:46  fuhrmann
 *pdelib1.15_alpha1
 *
 *Revision 2.9  2003/03/28 11:20:23  fuhrmann
 *pdelib2.0_alpha1
 *
 *Revision 2.8  2002/09/24 08:55:44  fuhrmann
 *scales
 *
 *Revision 2.6  2000/11/22 15:20:49  fuhrmann
 *ICE hack
 *1 mpeg/eps per variation
 *variation[i]-> variationi
 *bias=... option
 *
 *Revision 2.5  2000/10/25 10:49:59  fuhrmann
 *crop stuff for graphics
 *
 *Revision 2.4  2000/07/28 13:40:47  schmelze
 **** empty log message ***
 *
 *Revision 2.3  2000/01/13 17:36:41  fuhrmann
 *optmized ps output
 *
 *Revision 2.2  1999/12/21 17:18:55  fuhrmann
 *doc update for gltools-2-3
 *
 *Revision 2.1  1999/06/29 17:56:24  fuhrmann
 *Feedback buffer is used now...
 *
\end{comment}
 */


