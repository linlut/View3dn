/*------------------------------------------------------ 
//
//cia3dfile.h
//
//------------------------------------------------------*/

#ifndef __INC_CIA3DFILE_H_311_
#define __INC_CIA3DFILE_H_311_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <sysconf.h>
#include <vectorall.h>
#include <qdom.h>


class CPolyObj;
class CNameTableItem;

const int _CIA3D_MAX_OBJECT_COUNT = 20;



class CC3d1inIOControl
{
public:
	int m_nIStat;		//Option for analysis package
						//1: static problem
						//2: dynamic problem
						//3: static contact problem
						//4: dynamic contact/impact problem
						//5: dynamic penetration problem
	//Group 1
    int m_nVertex;		//number of vertices (nodes);
	int m_nElement;		//number of elements (including tet and hex)
	int m_nMaterial;	//number of materials

	int m_nStep;		//load step or time step
	int m_nAlgo;		//indicator of Newton Raphson Iteration
	int m_nAlgType;		//there are three types in alg: 
						//  1--Finite Element Method (FEM)
						//  4--Particle Method (PM)
						//  5--Finite Mass Method (FMM)


	int m_nMiter;		//max number of iteration
	double m_fTol;		//accuracy tolerance
	int m_nType;		//number of element types, currently only 1 type 

	int m_nVfix;		//number of fixed vertices;
	int m_nPrev;		//indicator of initial stree
	int m_nLaps;		//indicator for finite formulation

	//Group 2
	double m_fFrict;	//coefficient of Coulmb friction
	int m_nMethod;		//indicator of frictional contact method
	int m_nIseah;		//indicator of contact surface reach	
	int m_nReqc;		//gloabl point to output
	double m_fPenan;	//penalty for normal direction
	double m_fPenat;	//penalty for tengential direction
	
	int m_nObjectType[_CIA3D_MAX_OBJECT_COUNT];
	int m_nObjectElementCount[_CIA3D_MAX_OBJECT_COUNT];

	Vector3d * m_pVertex;
	Vector8i * m_pElement;
	int *m_pElementObjectID;


private:

	void _readConnectivity(FILE *fp);

	void _readVertices(FILE *fp);

	int _getVertexCount(const int matid);

	int _getElementCount(const int matid)
	{
		return m_nObjectElementCount[matid];
	}


public:

	void deleteBuffers()
	{
		SafeDeleteArray(m_pVertex);
		SafeDeleteArray(m_pElement);
		SafeDeleteArray(m_pElementObjectID);
		m_pVertex = NULL;
		m_pElement = NULL;
		m_pElementObjectID = NULL;
	}

	void initVariables(void)
	{
		m_nIStat = 5;	//6: fast truss element method
						//5: new algorithm supports hybrid rod, fem, etal
						//4: dynamic contact/impact problem

		m_nVertex=0;	//number of vertices (nodes);
		m_nElement=0;	//number of elements (including tet and hex)
		m_nMaterial=0;	//number of materials
		m_nStep=2000000;//load step or time step
		m_nAlgo=1;		//indicator of Newton Raphson Iteration
		m_nAlgType = 1; //1: FM, 4: particle, 5: fmm
		m_nMiter=25;	//max number of iteration
		m_fTol=1e-9;	//accuracy tolerance
		m_nType = 1;	//one element type

		m_nVfix=0;		//number of fixed vertices;
		m_nPrev=0;		//indicator of initial stree
		m_nLaps=3;		//indicator for finite formulation, see cia3d manual
						//0: linear problem
						//1: elasto-platic large deformation
						//2: elastic large deformation by T.L.F.
						//3: elasto-plastic large deformation by T.L.R.
						//4, 5, .....

		m_fFrict=0;		//coefficient of Coulmb friction
		m_nMethod=1;	//indicator of frictional contact method
                        // 1: automatic search
		m_nIseah=0;		//indicator of contact surface reach	
		m_nReqc=0;		//gloabl point to output
		m_fPenan=0;		//penalty for normal direction
		m_fPenat=0;		//penalty for tengential direction

		m_pVertex = NULL;
		m_pElement = NULL;
		m_pElementObjectID = NULL;
	}

	CC3d1inIOControl()
	{
		initVariables();
	}

	~CC3d1inIOControl()
	{
		deleteBuffers();
	}

	bool loadC3dinFile(const char *fname);

	bool loadXMLFile(const char *fname);

	void saveXMLFile(const char *fname);

	//Get a copy of the single object, where the vertices and mesh is indexed independently
	void getSingleObject(const int objid, Vector3d *& pVertex, int &nVertex, void* &pElement, int &nElement);

	QDomDocument *createDomDocument(void);

	void convertFromDomDocument(QDomDocument *pdom);

	void checkVertexWithNoMatID(void);

};



extern void computeBoundarySurface(const Vector4i *pTet, const int ntet, const int nvert, Vector3i *& pTri, int &nTri);

extern void computeBoundarySurface(const Vector8i *pTet, const int ntet, const int nvert, Vector4i *& pTri, int &nTri);

extern void PrepareXmlSymbolTable(CC3d1inIOControl &cio, CNameTableItem table[]);

extern CPolyObj* loadNeutralMesh(const char *fname);

extern CPolyObj* loadHMMesh(const char *fname);


#endif