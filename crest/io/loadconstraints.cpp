//FILE: loadconstraints.cpp

#include <string.h>
#include <stdio.h>
#include <qfile.h>
#include <qdom.h>
#include <crest/io/nametableitem.h>
#include <oglmaterial.h>
#include <crest/simulation/simu_engine.h>
#include <crest/simulation/constraints.h>
#include <crest/simulation/simu_thinshell.h>

typedef cia3d::io::CNameTableItem  NameTableItem;

static void parseConstraintBase(
	QDomElement &element, 
	int& nPrior,
	double& startTime,
	double& endTime,
	int& nVertexCount,
	int pVertexArray[ ])
{
	startTime = 0;
	endTime = 0;
	nVertexCount = 0;
	nPrior=10;
	pVertexArray[0]=0;

	NameTableItem table[]={
		NameTableItem("starttime", 'd', &startTime), 
		NameTableItem("endtime", 'd', &endTime), 
		NameTableItem("vertexcount", 'i', &nVertexCount), 
		NameTableItem("vertexarray", 'I', pVertexArray), 
		NameTableItem("prior", 'i', &nPrior), 
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}
	
	//check input data integraty
	if (nVertexCount!=pVertexArray[0]) nVertexCount=pVertexArray[0];
	for (int i=0; i<nVertexCount; i++)
		pVertexArray[i] = pVertexArray[i+1];
}


static inline CFixedPositionConstraint *buildFixedPositionConstraint(QDomElement &element)
{
	//load base constraint
	double startTime=0, endTime=0;
	int nVertexCount=0, pVertexArray[1000];
	int nnprior=10;
	parseConstraintBase(element, nnprior, startTime, endTime, nVertexCount, pVertexArray);

	//load its own paramter;
	//create instance
	if (nVertexCount==0) return NULL;
	CFixedPositionConstraint *p = new CFixedPositionConstraint(
		nnprior, startTime, endTime, nVertexCount, pVertexArray);
	return p;
}


static inline CPlanarRotationConstraint *buildPlanarRotationConstraint(QDomElement &element)
{
	//load base constraint
	double startTime=0, endTime=0;
	int nVertexCount=0, pVertexArray[1000];
	int nnprior=10;

	//load its own paramter;
	Vector3d center(0,0,0);			//rotation center;
	Vector3d norm(0,0,1);			//rotation plane normal direction
	double av=PI*1e-3;				//angular velocity per ms
	NameTableItem table[]={
		NameTableItem("center", 'V', &center.x), 
		NameTableItem("norm", 'V', &norm.x), 
		NameTableItem("av", 'd', &av), 
	};
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	//create instance
	parseConstraintBase(element, nnprior, startTime, endTime, nVertexCount, pVertexArray);
	if (nVertexCount==0) return NULL;
	CPlanarRotationConstraint *p = new CPlanarRotationConstraint(
		nnprior, startTime, endTime, nVertexCount, pVertexArray,
		center, norm, av);
	return p;
}


CConstraint * buildConstraintFromXML(QDomElement &element)
{
	CConstraint *p = NULL;
	std::string classname = element.attribute("class", "");
	if (classname == "FixedPositionConstraint"){
		p = buildFixedPositionConstraint(element);
	}
	else if (classname == "PlanarRotationConstraint"){
		p = buildPlanarRotationConstraint(element);
	}

	return p;
}

