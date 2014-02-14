//FILE: main.cpp

#include <vector>
#include <iostream>
#include <fstream>
#include <crest/system/FileRepository.h>
#include <crest/simulation/truss_thread.h>
#include <crest/core/Factory.h>
#include <crest/gui/Main2.h>
#include <crest/GPU/Cuda/cuda0.h>


using namespace cia3d::simulation;

#include <virtualbuffer.h>
void testmem(void)
{
	CPagedVirtualBuffer b(sizeof(int));
	
	unsigned int i = b.allocElement();
	unsigned int *pi = (unsigned int *)b.getElementPtr(i);
	*pi = 100;

	unsigned int j = b.allocElement();
	unsigned int *pj = (unsigned int *)b.getElementPtr(j);
	*pj = 200;

	b.deleteElement(i);
	b.deleteElement(j);
	unsigned int k = b.allocElement();
	unsigned int m = b.allocElement();

	unsigned int *pk = (unsigned int *)b.getElementPtr(k);
	*pk = 300;

}


#ifndef WIN32
#include <dlfcn.h>
bool loadPlugin(const char* filename) 
{
  void *handle;
  handle=dlopen(filename, RTLD_LAZY);
  if (!handle)
  {
    std::cerr<<"Error loading plugin "<<filename<<": "<<dlerror()<<std::endl;
    return false;
  }
  std::cerr<<"Plugin "<<filename<<" loaded."<<std::endl;
  return true;
}
#else
bool loadPlugin(const char* filename)
{
	std::cerr << "Plugin loading not supported on this platform.\n";
	return false;
}
#endif 

void testMem(void)
{
	int *p = new int [10];
	for (int i=0; i<20; i++){
		int x = p[i];
		printf("%d: x=%d\n", i, x);
	}
}

void printUsage(void)
{
	printf("Save a serrial of snapshot image, press V key.\n");
}

// ---------------------------------------------------------------------
// ---
// ---------------------------------------------------------------------
int main(int argc, char** argv)    
{
	extern void createBleedingImages();
	extern void createBubbleImages();
	extern void createLaserImages();
	extern void createFiberImages(void);
	//createBleedingImages();
	//createBubbleImages();
	//createLaserImages();
	//testmem();
	//createFiberImages();
	//return 1;

	printf("STEP0\n");
	printUsage();

	//testMem();
	//CUDA init
	printf("STEP1: CUDA\n");
	cudaDeviceInit();

	std::string fileName ;
	bool        startAnim = false;
	bool        printFactory = false;
	std::vector<std::string> plugins;
	std::vector<std::string> files; 
/*
	sofa::helper::parse(&files, "This is a SOFA application. Here are the command line arguments")
	.option(&fileName,'f',"file","scene file")
	.option(&startAnim,'s',"start","start the animation loop")
	.option(&printFactory,'p',"factory","print factory logs") 
	.option(&gui,'g',"gui","choose the UI (none"
#ifdef SOFA_GUI_FLTK
		"|fltk"
#endif
#ifdef SOFA_GUI_QT
		"|qt"
#endif
		")"
	)
	.option(&plugins,'l',"load","load given plugins")
	(argc,argv);
*/

	if (printFactory){
		std::cout << "////////// FACTORY //////////" << std::endl;
		//sofa::helper::printFactoryLog();
		std::cout << "//////// END FACTORY ////////" << std::endl;
	}


    if (fileName.empty()){
        fileName = "default.xml";
		cia3d::system::DataRepository.findFile(fileName);
    }

	printf("STEP2: before mainloop\n");
	MainLoop(argv[0], fileName.c_str());

	//CUDA close
	printf("STEP3: CUDA close\n");
	cudaDeviceClose(argc, argv);

	return 0;
}

//void Draw3DText(double x, double y, double z, char *txt)
//{
//	QGLWidget *pwin = pmainwindow->m_pGLUIWin;
//	pwin->renderText(x, y, z, QString(txt));
//}

//#include <objfactory_io.h>