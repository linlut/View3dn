
#include <vectorall.h>


void ConvertAttrib2ColorIndex(Vector3f *pArray, const int nArray, const int channel, unsigned int * pColor)
{
	int i;

	///find max value
	float maxval = -1e30;
	float minval = +1e30;
	float dx;

	for (i=0; i<nArray; i++){
		Vector3f & v= pArray[i];
		const float x = v[channel];
		if (x>maxval)
			maxval = x;
		else if (x<minval)
			minval = x;

	}

	dx = maxval - minval; 
	if (dx < 1e-20f)
		dx = 1e-20f;
	dx = 255 / dx;

	for (i=0; i<nArray; i++){
		Vector3f & v= pArray[i];
		const float x = v[channel];
		unsigned char * clr = (unsigned char* )(&pColor[i]);
		clr[3] = (unsigned char)((x - minval) * dx);
	}
	
}