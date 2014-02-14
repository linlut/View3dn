#include <GL/glew.h>
#include "glwin.h"
#include <qapplication.h>
#include <time.h>
#include <zntoolbox.h>

using namespace std;
using namespace qglviewer;

extern CGLWin* pmainwindow;

inline double fsqrt (double y) {
    double x, z, tempf;
    unsigned long *tfptr = ((unsigned long *)&tempf) + 1;

	tempf = y;
	*tfptr = (0xbfcdd90a - *tfptr)>>1; /* estimate of 1/sqrt(y) */
	x =  tempf;
	z =  y*0.5;                        /* hoist out the         */
	x = (1.5*x) - (x*x)*(x*z);         /* iteration formula     */
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	return x*y;
 }

void mult_timing(void)
{
	const int N=10000000;
	float x=1, y=1, z;


	double t1= GetClockInSecond();

	for (int i=0; i<N; i++){
		/*
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		z = x+y;
		x+=0.1f;
		y+=0.1f;
		*/
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
		z = fsqrt(x);
	}
	printf("Z=%lf\n", (double)z);
	double t2= GetClockInSecond();
	double r = (double)N/(t2-t1+1e-30)*10;
	printf("Computation of Mult speed is %lf per sec.\n", r);
}


/*-----------------------------------------
Some testing results for the computation: on Intel Core2Duo 2.67GHZ
double precision:
  fsqrt: 12.8e+6
  sqrt   96.0e+6 per sec.
  +   1.64e+8 per sec.
  *   1.42e+8 per sec

single precision:
  *   10.64e+8 per sec
  +   10.90e+8 per sec
-------------------------------------------*/

int main(int argc, char** argv)
{
	// Read command lines arguments.
	QApplication application(argc,argv);
	//const QString appdir = application.applicationDirPath();

	// Instantiate the viewer.
	CGLWin v;

	//random number init
	srand(static_cast<unsigned>(time(0)));

	// Make the viewer window visible on screen.
	v.setCaption(APP_NAME);
	v.show();
	v.setGoodWindowSize(512,512);

	// Set the viewer as the application main widget.
	application.setMainWidget(&v);
	pmainwindow = &v;

	// Run main loop.
	return application.exec();
}
