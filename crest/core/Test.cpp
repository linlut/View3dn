//test core functions
//#include <base/fixed_array.h>
//#include <base/config_assert.h>
//#include <base/static_assert.h>
//#include <base/vec.h>
//#include <base/mat.h>
//#include <core/rigidtypes.h>
//#include <core/mappedmodel.h>

/*
#include <io/mesh.h>
#include <io/meshobj.h>
//#include <core/visualmodel.h>

//#include <crest/ogl/oglmodel.h>
#include <core/mappedmodel.h>

#include <vector>
#include <iostream>
#include <string>
#include <base/vec.h>
#include <base/vectorall.h>
#include <base/quat.h>
#include <core/rigidtypes.h>
#include <core/visualmodel.h>
#include <core/baseobjectdescription.h>
#include <ogl/texture.h>
#include <ogl/oglmaterial.h>

*/

//#define X ( x )  a
//#define sofa_do_concat2( a , b ) a ## b
//int sofa_do_concat2( X ( x ) , b );


void mainxx(int argc, char **argv)
{

//	Xb=1;
//	ab = 1;
	//Xb=1;
	//Yb=1;


}



//	cia3d::zcore::MappedModel< ExtVectorTypes<Vec<3,float>, Vec<3,float> > > zz;

#include <stdio.h>
#define MACRO1(A, B)  MACRO2 ( A ) ## B
#define MACRO2(A)      AA 

void mainyy()
{

	int MACRO1(A, X) = 1;


}

#define MODULE_VAR(A, B)  MODULE_NAME(A) ## B ## C
#define MODULE_NAME(A)    AA_mp_

void mainzz()
{
	bool MODULE_VAR(A, X);
}

