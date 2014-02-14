//FILE: xmlrw.cpp

#include <string.h>
#include <stdio.h>
#include <qfile.h>
#include <qdom.h>
//=================================
#include <view3dn/scenegraph.h>
#include <view3dn/cia3dobj.h>
#include <view3dn/texturedtmeshobj.h>

#include <crest/core/BaseObject.h>
#include <crest/core/BaseObjectDescription.h>
#include <crest/core/BaseContext.h>
#include <crest/core/ObjectFactory.h>
#include <crest/core/ObjectLib.h>
#include <crest/io/nametableitem.h>
#include <oglmaterial.h>
#include <crest/simulation/truss_thread.h>
#include <crest/simulation/simu_engine.h>
#include <crest/simulation/constraints.h>
#include <crest/simulation/simu_thinshell.h>
#include <crest/simulation/obj_vertexringsolid.h>
#include <crest/simulation/obj_vertexringshell.h>
#include <crest/simulation/obj_spring.h>
#include <crest/simulation/obj_tetfem.h>


using namespace cia3d::core;
using namespace cia3d::simulation;
extern int USE_INTERNAL_SIMULATOR;

static QDomDocument* _loadXMLSceneFile(const QString &fileName)
{
	QDomDocument *pDomTree = new QDomDocument;
	if (pDomTree == NULL) 
		return NULL;

    //Read the XML file and create DOM tree
    QFile opmlFile( fileName );
    if (!opmlFile.open(IO_ReadOnly)){
		delete pDomTree;
		return NULL;
    }

    if (!pDomTree->setContent(&opmlFile)){
		printf("XML file parse error: %s.\n", fileName.ascii());
		opmlFile.close();
		delete pDomTree;
		return NULL;
    }

	//Setup return val
    opmlFile.close();
	return pDomTree;
}


static CPolyObj* loadSolidMesh(const char *fname, const char*objname)
{
	CPolyObj *p = NULL;
	p = CPolyObjLoaderFactory::LoadPolygonMesh(fname);
	if (p){
		if (p->m_pElement==NULL){
			printf("Not a solid mesh!\n");
			delete p;
			return NULL;
		}
		p->SetObjectName(objname);
		printf("Mesh obj %s loaded, file=%s\n", objname, fname);
	}
	return p;
}


static CPolyObj* loadSurface(const char *fname, const char*objname)
{
	CPolyObj* p = CPolyObjLoaderFactory::LoadPolygonMesh(fname);
	if (p){
		p->SetObjectName(objname);
		printf("Surface obj %s loaded, file=%s\n", objname, fname);
	}
	return p;
}

inline bool isZeroMatrix(const Matrix2d & m)
{
	bool r=false;
	if (m.x[0][0]==0 && m.x[0][1]==0 && m.x[0][2]==0 && m.x[0][3]==0 &&
		m.x[1][0]==0 && m.x[1][1]==0 && m.x[1][2]==0 && m.x[1][3]==0 &&
		m.x[2][0]==0 && m.x[2][1]==0 && m.x[2][2]==0 && m.x[2][3]==0 &&
		m.x[3][0]==0 && m.x[3][1]==0 && m.x[3][2]==0 && m.x[3][3]==0)
		r = true;
	return r;
}

static void setTransformation(const QDomElement &element, CPolyObj *poly)
{
	typedef cia3d::io::CNameTableItem  NameTableItem;
	Vector3d translation(0,0,0), scale(1,1,1);
	Matrix2d mat; IdentityMatrix(mat);
	Matrix2d II = mat;
	NameTableItem table[]={
		NameTableItem("translation", 'V', &translation), 
		NameTableItem("scale", 'V', &scale), 
		NameTableItem("transform", 'X', &mat.x)
	};
	
	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	if (isZeroMatrix(mat-II)){
		if (!(scale==Vector3d(1,1,1)))
			poly->applyScaling(scale.x, scale.y, scale.z);
		if (!(translation==Vector3d(0,0,0)))
			poly->applyTranslation(translation.x, translation.y, translation.z);
	}
	else{
		poly->ApplyTransform(mat);
	}

	Vector3d lowleft, upright;
	poly->ComputeBoundingBox(lowleft, upright);
	poly->SetBoundingBox(AxisAlignedBox(lowleft, upright));
}

//The created object is returned, at the same time, the corresponding pointer
//in the scenenode class is modified!
static CObjectBase* _buildObject(QDomElement &element, CSceneNode &snode, CConstraint *&pconstraint)
{
	CObjectBase *object = NULL;
	std::string tpname = element.attribute("type", "");

	if (tpname=="DeformableSolid"){		
		std::string fname = element.attribute("filename", "");
		std::string objname = element.attribute("name", "nonamesolid");
		CPolyObj *poly = loadSolidMesh(fname.c_str(), objname.c_str());
		if (poly) setTransformation(element, poly);
		snode.m_pSimulationObject = poly;
		object = poly;
	}
	else if (tpname=="DeformableSurface" || tpname=="DeformableShell"){
		std::string fname = element.attribute("filename", "");
		std::string objname = element.attribute("name", "nonamesurface");
		CPolyObj *poly = loadSurface(fname.c_str(), objname.c_str());
		if (poly) setTransformation(element, poly);
		snode.m_pSimulationObject = poly;
		object = poly;
	}
	else if (tpname=="Tool"){
		std::string classname = element.attribute("class", "");
		std::string fname = element.attribute("filename", "");
		std::string objname = element.attribute("name", "nonamelaptool");
		if (classname == "Pusher"){
			CPolyObj *poly = loadSurface(fname.c_str(), objname.c_str());
			if (poly) setTransformation(element, poly);
			snode.m_pCollisionObject = poly;
			object = poly;
		}
		else if (classname == "Grasper"){
			CPolyObj *poly = loadSurface(fname.c_str(), objname.c_str());
			if (poly) setTransformation(element, poly);
			CTexturedTriangleObj *pwaveobj = dynamic_cast<CTexturedTriangleObj*>(poly);
			if (pwaveobj!=NULL){
				pwaveobj->initJaws();
			}
			snode.m_pCollisionObject = poly;
			object = poly;
		}
		else{
			BaseContext context;
			ObjectFactory *pFactory = ObjectFactory::getInstance();
			CPolyObj *tool = (CPolyObj *)pFactory->createObject(&context, &element);
			snode.m_pCollisionObject = tool;
			object = tool;
		}
	}
	else if (tpname=="VisualSurface"){
		std::string fname = element.attribute("filename", "");
		std::string objname = element.attribute("name", "nonamesurface");
		CPolyObj *poly = loadSurface(fname.c_str(), objname.c_str());
		if (poly) setTransformation(element, poly);
		snode.m_pObject = poly;
		object = poly;
	}
	else if (tpname=="Mapping"){ 
		BaseContext context;
		ObjectFactory *pFactory = ObjectFactory::getInstance();
		CBaseMapping *mapping = (CBaseMapping *)pFactory->createObject(&context, &element);
		snode.m_pMapping = mapping;
		object = mapping;
	}
	else if (tpname=="Rendering"){ 
		BaseContext context;
		ObjectFactory *pFactory = ObjectFactory::getInstance();
		CBaseRendering *rendering = (CBaseRendering *)pFactory->createObject(&context, &element);
		snode.m_pRendering = rendering;
		object = rendering;
	}
	else if (tpname=="Constraint"){ 
		extern CConstraint * buildConstraintFromXML(QDomElement &element);
		pconstraint = buildConstraintFromXML(element);
	}
	return object;
}


static void _getNodeAttributes(QDomElement &element, CGLDrawParms &draw, CObjectFemAttrib &fem)
{
	typedef cia3d::io::CNameTableItem  NameTableItem;
	int renderstyle = (int)draw.m_nDrawType;
	int rendervertexstyle = (int)draw.m_nDrawVertexStyle;
	NameTableItem table[]={
		//Rendering attributes--------------------------------------
		NameTableItem("color", 'v', &draw.m_cObjectColor), 
		NameTableItem("objectcolor", 'v', &draw.m_cObjectColor), 
		NameTableItem("linecolor", 'v', &draw.m_cHLineColor), 
		NameTableItem("vertexcolor", 'v', &draw.m_cVertexColor), 
		NameTableItem("normalcolor", 'v', &draw.m_cNormalColor), 

		NameTableItem("vertexsizescale", 'f', &draw.m_fVertexSizeScale), 
		NameTableItem("normallengthscale", 'f', &draw.m_fNormalLengthScale), 
		NameTableItem("linewidth", 'f', &draw.m_fLineWidth), 

		NameTableItem("texturing", 'b', &draw.m_bEnableTexture2D), 
		NameTableItem("texture1d", 'b', &draw.m_bEnableTexture1D), 
		NameTableItem("texture2d", 'b', &draw.m_bEnableTexture2D), 
		NameTableItem("texture3d", 'b', &draw.m_bEnableTexture3D), 
		NameTableItem("smoothing", 'b', &draw.m_bEnableSmoothing), 
		NameTableItem("showvertexnormal", 'b', &draw.m_bShowVertexNormal), 

		NameTableItem("renderstyle", 'i', &renderstyle), 
		NameTableItem("rendervertexstyle", 'i', &rendervertexstyle), 

		//FEM attributes-----------------------------------------------
		NameTableItem("deformationmethod", 'i', &fem.m_nDeformationMethod), 
						//1: FEM or 0: TRUSS method
		NameTableItem("springtype", 'i', &fem.m_nSpringType), 
						// 0: Adaptive, meshfree type of truss
						// 1: Nonadaptive truss, basically 
						// 2: G's method       ---could only be used on tet mesh
						// 3: TVCG 2007 method ---could only be used on tet mesh
		NameTableItem("young", 'd', &fem.m_young), 
		NameTableItem("poisson", 'd', &fem.m_poisson), 
		NameTableItem("rho", 'd', &fem.m_rho), 
		NameTableItem("velocitydamp", 'd', &fem.m_velocity_damp),
		NameTableItem("massdamp", 'd', &fem.m_mass_damp),
		NameTableItem("gravity", 'V', &fem.m_vLoad),
		NameTableItem("rigid", 'i', &fem.m_nRigid),

	};

	const int N = sizeof(table)/sizeof(NameTableItem);
	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		const QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}
	draw.m_nDrawType = (CGLDrawParms::glDrawMeshStyle)renderstyle;
	draw.m_nDrawVertexStyle = (CGLDrawParms::glDrawVertexStyle)rendervertexstyle;
}


static void _buildNode(QDomElement &parentElement, const char* objstr, CSceneNode &snode, CConstraint *&pconstraintlist)
{
	bool isItemNode;
    QDomNode node = parentElement.firstChild();
	snode = CSceneNode();
	snode.m_pMapping = NULL;
	snode.m_pObject = NULL;
	snode.m_pSimulationObject = NULL;
	pconstraintlist = NULL;

    while ( !node.isNull() ) {
		CConstraint *pconstraint=NULL;
		const char *nodename = node.nodeName();
		printf("%s\n", nodename);
		isItemNode = (strcmp(nodename, objstr)==0);
		if (node.isElement() && isItemNode){
			CObjectBase* pobj = _buildObject(node.toElement(), snode, pconstraint);
			if (pobj!=NULL)
				ObjectLib::getInstance()->push_back(pobj);	//append to the object LIB
			if (pconstraint!=NULL){//insert into a constraint list
				pconstraint->m_pNext = pconstraintlist;
				pconstraintlist = pconstraint;
			}
		}
		node = node.nextSibling();
    }
	
	//get other parameters
	_getNodeAttributes(parentElement, snode.m_DrawParms, snode.m_FemAttrib);
}


static void _addOneObjectIntoSimulationEngine(CSceneNode *pnode, const CConstraint *pconstraintlist)
{
	CSimuEngine *pEngine = CSimuEngine::getInstance();
	CPolyObj *pobj = pnode->m_pSimulationObject;
	if (pobj){
		CObjectFemAttrib &attrib = pnode->m_FemAttrib;
		CMeMaterialProperty mtl;
		mtl.setYoung(attrib.m_young);
		mtl.setPoisson(attrib.m_poisson);
		mtl.setDensity(attrib.m_rho);
		mtl.m_rigid = attrib.m_nRigid;
		mtl.setMassDamping(attrib.m_mass_damp);
		mtl.setDamping(0.10, 0.10);
		//surface object
		CSimuEntity *psimuobj = NULL;
		if (pobj->m_pElement == NULL && pobj->m_pPolygon!=NULL){
			//==============================================
			const Vector3d *pVertex = pobj->m_pVertex;
			const int nv = pobj->m_nVertexCount;
			const Vector3d init_velocity(0, 0, 0);
			const int *pElement = pobj->m_pPolygon;
			const int nv_per_elm = pobj->m_nPlyVertexCount;
			const int nelm = pobj->m_nPolygonCount;
			const double shell_thickness=1e-3;
			const int isStatic=0;
			//psimuobj = new CSimuThinshell(pVertex, nv, init_velocity, pElement, nv_per_elm, nelm, shell_thickness, mtl, isStatic);
			psimuobj = new CSimuVertexRingShell(1, pVertex, nv, init_velocity, pElement, nv_per_elm, nelm, shell_thickness, mtl);
			assert(psimuobj!=NULL);
			//================Apply constraints & gravity================
			psimuobj->setGravityAccelertion(attrib.m_vLoad);
			const CConstraint *pc = pconstraintlist;
			while (pc!=NULL){
				psimuobj->addConstraint(pc);
				pc = pc->m_pNext;
			}
			//================Add obj========================
			pEngine->addSimuObject(psimuobj);
		}
		else{
			const Vector3d *pVertex = pobj->m_pVertex;
			const int nv = pobj->m_nVertexCount;
			const Vector3d init_velocity(0, 0, 0);
			const int *pElement = pobj->m_pElement;
			const int nv_per_elm = pobj->m_nElmVertexCount;
			const int nelm = pobj->m_nElementCount;
			const int *pSurfacePoly = pobj->m_pPolygon;
			const int nSurfacePoly = pobj->m_nPolygonCount;
			const int nSurfaceType = pobj->m_nPlyVertexCount;
			const int springtype=1;
			psimuobj = new CSimuTetra(false, pVertex, nv,init_velocity, pElement, nelm, pSurfacePoly, nSurfacePoly, mtl);
			//psimuobj = new CSimuVertexRingSolid(springtype, pVertex, nv, init_velocity, pElement, nv_per_elm, nelm, pSurfacePoly, nSurfaceType, nSurfacePoly, mtl);
			//CSimuSpringObj *spobj = new CSimuSpringObj(springtype, pVertex, nv, init_velocity, 1, pElement, nv_per_elm, nelm, 1, mtl);
			//spobj->setBoundarySurface(pSurfacePoly, nSurfaceType, nSurfacePoly);
			//psimuobj = spobj;
			assert(psimuobj!=NULL);
			//================Apply constraints & gravity================
			psimuobj->setGravityAccelertion(attrib.m_vLoad);
			const CConstraint *pc = pconstraintlist;
			while (pc!=NULL){
				psimuobj->addConstraint(pc);
				pc = pc->m_pNext;
			}
			//================Add obj========================
			pEngine->addSimuObject(psimuobj);
		}	
	}
}


static void _buildTree(const QDomElement &parentElement, const char *nodestr, const char* objstr, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG)
{
	bool isClassNode;
    QDomNode node = parentElement.firstChild();
	CSceneNode snode;
	CConstraint *pconstraintlist;

    while ( !node.isNull() ) {
		const char *nodename = node.nodeName();
		//printf("Node: %s\n", nodename);
		isClassNode = (strcmp(nodename, nodestr)==0);
		if (node.isElement() && isClassNode){
			_buildNode(node.toElement(), objstr, snode, pconstraintlist);
			if (snode.isEmptyNode()){
				fprintf(stderr, "Error: Empty scene node!\n");
			}
			else if (snode.m_pSimulationObject!=NULL){
				//at this step we should decide where the node should go
				//YES, simulation object
				CSceneNode *pnode = pSG->AddSceneNode(snode);
				if (USE_INTERNAL_SIMULATOR){
					_addOneObjectIntoSimulationEngine(pnode, pconstraintlist);
				}
			}
			else if (snode.m_pCollisionObject!=NULL){
				//YES, laparoscopic tool object
				//create a local frame used for manipulation
				qglviewer::ManipulatedFrame *pFrame = new qglviewer::ManipulatedFrame;
				assert(pFrame!=NULL);
				CSceneNode* pnode = pLaptoolSG->AddSceneNode(snode, pFrame);
			}
			else{
				//Object is the background
				pBackgroundSG->AddSceneNode(snode);
			}
		}
		node = node.nextSibling();
    }
}


static void _getSceneAttributes(QDomElement &element, CGLDrawParms &draw)
{
	typedef cia3d::io::CNameTableItem  NameTableItem;
	double timesteplength=1e-3;		//1ms
	NameTableItem table[]={
		//Rendering attributes
		NameTableItem("backgroundcolor", 'v', &draw.m_cBackgroundColor), 
		NameTableItem("timesteplength", 'd', &timesteplength), 
	};
	const int N = sizeof(table)/sizeof(NameTableItem);

	for (int i=0; i<N; i++){
		NameTableItem *p = &table[i];
		QString val = element.attribute(p->m_sName);
		p->setValue(val.ascii());
	}

	CSimuEngine::getInstance()->setTimestep(timesteplength);

}


static inline bool 
_parseScene(QDomDocument* pDomTree, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, CGLDrawParms &draw)
{
	const char* HEADSTR="head";
	const char* BODYSTR="body";
	const char* nodestr="node";
	const char* objstr="object";
    //Get the header information from the DOM
    QDomElement root = pDomTree->documentElement();
    QDomNode node = root.firstChild();

	//Process header information
    while ( !node.isNull() ) {
		if ( node.isElement() && node.nodeName() == HEADSTR ) {
			//QDomElement header = node.toElement();
			//getHeaderInformation( header );
			break;
		}
		node = node.nextSibling();
    }

    //Create the tree view out of the DOM
    node = root.firstChild();
    while ( !node.isNull() ) {
		if ( node.isElement() && node.nodeName() == BODYSTR ) {
			QDomElement& body = node.toElement();
			_buildTree(body, nodestr, objstr, pSG, pLaptoolSG, pBackgroundSG);
			_getSceneAttributes(body, draw);
			break;
		}
		node = node.nextSibling();
    }
	return true;
}

//Load a XML file, parse data into 3 screnegraph objects, then construct the simulation.
void loadXMLScene(const char *strfilename, CSceneGraph* pSG, CSceneGraph* pLaptoolSG, CSceneGraph* pBackgroundSG, CGLDrawParms &draw)
{
	//stop the simulation thread first
	CDeformationThread::terminateInstance();
	CSimuEngine::getInstance()->clear();

	//then, clear the scene
	pSG->RemoveAll();
	pLaptoolSG->RemoveAll();
	pBackgroundSG->RemoveAll();
	ObjectLib::getInstance()->clear();
	OglTextureFactory::getInstance()->clear();
	extern void clearCollisonBuffer(void);
	clearCollisonBuffer();

	//load the file
	QString fileName(strfilename);
	QDomDocument* p=_loadXMLSceneFile(fileName);
	if (p==NULL) return;
	bool r = _parseScene(p, pSG, pLaptoolSG, pBackgroundSG, draw);
	if (r==false)
		fprintf(stderr, "Parsing file %s failed!\n", strfilename);
	delete p;

	CSceneNode *plapnode = pLaptoolSG->GetSceneNode(0);
	pLaptoolSG->setActiveSceneNode(plapnode);

}

