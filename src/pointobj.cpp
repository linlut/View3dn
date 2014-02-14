//pointobj.cpp

#include "pointobj.h"
#include "sphere16.h"


int CPointObj::LoadPltFileWithoutHeader(FILE *fp, const int nv, const int, const int nTotalAttrib)
{
	//read vertices;
	m_nVertexCount = nv;
	m_nPolygonCount = nv;
	LoadPltVertices(fp, nv, nTotalAttrib);

	//assign the radii for particles
	m_pRadius = new float[m_nPolygonCount];
	assert(m_pRadius!=NULL);
	float *pInputRadius = NULL;
	int rpos= this->GetFAttributeIndexByName("R");
    if (rpos>0) pInputRadius = m_pFAttributes[rpos];
	if (pInputRadius){
		for (int i=0; i<nv; i++) 
            m_pRadius[i] = pInputRadius[i];
	}
	else{
		for (int i=0; i<nv; i++) 
            m_pRadius[i] = 0;
	}
	
	//assign ellipsoid transform matrices
	m_pMatrix = NULL;
	const float K = 1.0f;
	int evXpos= this->GetFAttributeIndexByName("A");
	int etXXpos= this->GetFAttributeIndexByName("AX");
	int etXYpos= this->GetFAttributeIndexByName("AY");
	int etXZpos= this->GetFAttributeIndexByName("AZ");
	int evYpos= this->GetFAttributeIndexByName("B");
	int etYXpos= this->GetFAttributeIndexByName("BX");
	int etYYpos= this->GetFAttributeIndexByName("BY");
	int etYZpos= this->GetFAttributeIndexByName("BZ");
	int evZpos= this->GetFAttributeIndexByName("C");
	int etZXpos= this->GetFAttributeIndexByName("CX");
	int etZYpos= this->GetFAttributeIndexByName("CY");
	int etZZpos= this->GetFAttributeIndexByName("CZ");
	if (evXpos>=0 && etXXpos>=0 && etXYpos>=0 && etXZpos>=0 &&
		evYpos>=0 && etYXpos>=0 && etYYpos>=0 && etYZpos>=0 &&
		evZpos>=0 && etZXpos>=0 && etZYpos>=0 && etZZpos>=0){
		float *rx = m_pFAttributes[evXpos];
		float *ry = m_pFAttributes[evYpos];
		float *rz = m_pFAttributes[evZpos];
		float *vxx = m_pFAttributes[etXXpos];
		float *vxy = m_pFAttributes[etXYpos];
		float *vxz = m_pFAttributes[etXZpos];
		float *vyx = m_pFAttributes[etYXpos];
		float *vyy = m_pFAttributes[etYYpos];
		float *vyz = m_pFAttributes[etYZpos];
		float *vzx = m_pFAttributes[etZXpos];
		float *vzy = m_pFAttributes[etZYpos];
		float *vzz = m_pFAttributes[etZZpos];
		m_pMatrix = new float3x3[nv];
		for (int i=0; i<nv; i++){
			float3x3& m = m_pMatrix[i];
			Vector3f* dx = (Vector3f*)(&m.x[0]);
			Vector3f* dy = (Vector3f*)(&m.x[3]);
			Vector3f* dz = (Vector3f*)(&m.x[6]);
			//normalize, right-handrize the 3 axes
			*dx = Vector3f(vxx[i], vxy[i], vxz[i]);
			(*dx).normalize();
			*dy = Vector3f(vyx[i], vyy[i], vyz[i]);
			(*dy).normalize();
			(*dz) = CrossProd(*dx, *dy);
			//*dz = Vector3f(vzx[i], vzy[i], vzz[i]);
			(*dz).normalize();
			float3x3 s; 
			s.setIdentityMatrix();
			s.x[0]=(fabs(rx[i]))*K;
			s.x[4]=(fabs(ry[i]))*K;
			s.x[8]=(fabs(rz[i]))*K;
			m*=s;
		}
	}

	return 1;
}


int CPointObj::LoadFile(FILE *fp, const char *ftype)
{
		return 1;
}


int CPointObj::SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
{
	Vector3d lowleft, upright;
	for (int i=0; i<bufflen; i++){
		CPointObj *p = new CPointObj;
		assert(p!=NULL);
		_copyToObject(*p, objidbuff[i]);
		p->ComputeBoundingBox(lowleft, upright);
		p->SetBoundingBox(AxisAlignedBox(lowleft, upright));
		obj[i] = p;
	}
	return bufflen;
}



void CPointObj::_copyToObject(CPointObj & aobj, const int objid)
{
	int i, c, *vindex;
	float* pid = GetVertexObjectIDPointer();
	assert(pid!=NULL);
	assert(objid>=0);

	//count how many polygons;
	for (i=c=0; i<m_nVertexCount; i++){
		int idx = (int)pid[i];
		if (idx==objid) c++;
	}
	assert(c>0);
	aobj.m_nPolygonCount = 0;
	aobj.m_nVertexCount = c;
	aobj.m_pVertex = new Vector3d[c];
	assert(aobj.m_pVertex!=NULL);

	//copy the vertex buffer;
	vindex = new int [m_nVertexCount];
	for (i=c=0; i<m_nVertexCount; i++){
		vindex[i]=-1;
		int idx = (int)pid[i];
		if (idx==objid){ 
			vindex[i]=c;
			aobj.m_pVertex[c] = m_pVertex[i];
			c++;
		}
	}
	assert(c==aobj.m_nVertexCount);

	//copy radius buffer;
	aobj.m_pRadius = new float[aobj.m_nVertexCount];
	for (int i=0; i<m_nVertexCount; i++){
		int k = vindex[i];
		if (k>=0)
			aobj.m_pRadius[k] = m_pRadius[i];
	}

	//copy point matrix buffer;
	if (this->m_pMatrix){
		aobj.m_pMatrix = new float3x3[aobj.m_nVertexCount];
		for (int i=0; i<m_nVertexCount; i++){
			int k = vindex[i];
			if (k>=0)
				aobj.m_pMatrix[k] = m_pMatrix[i];
		}
	}

	//copy other polyobj class attributes;
	CPolyObj::CopyAttributes(aobj, vindex);
	delete []vindex;
}


void CPointObj::glDrawElementFaceGoround(const int eid, void *pvoid)
{
	ASSERT0(eid>=0 && eid<m_nVertexCount);
	Vector3d& v = m_pVertex[eid];

	bool reverse_normal=false;
	if (pvoid) reverse_normal=true;

	if (this->NeedTexture1D() && m_pVertexTexCoor1D){
		const float tx = m_pVertexTexCoor1D[eid];
		glTexCoord1f(tx);
	}

	if (!m_pMatrix){
		float r=0;
		if (m_pRadius) r= m_pRadius[eid];
		if (r==0) r = m_fGivenRad;
		r *= m_pDrawParms->m_fVertexSizeScale;
		CSphere16::getInstance().glDraw(v, r, reverse_normal);
	}
	else{
		Vector3f vf(v.x, v.y, v.z);
		float3x3 mat = m_pMatrix[eid]*m_pDrawParms->m_fVertexSizeScale;
		mat.Transpose();
		CSphere16::getInstance().glDraw(vf, mat, reverse_normal);
	}
}


void CPointObj::glDrawElementFacePhong(const int eid, void *pvoid)
{
	glDrawElementFaceGoround(eid, pvoid);
}


void CPointObj::glDrawElementLine(const int eid, const float offset, const bool needtex)
{
	assert(0);
}


void CPointObj::glDrawVertexSphere(const int eid, void *pvoid)
{
	glDrawElementFaceGoround(eid, pvoid);
}



//======================Picking functions===============================
void CPointObj::DrawPickingObject(const int objid)
{
	int f, name = objid;

	glEnable(GL_DEPTH_TEST);      
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMatrixMode(GL_MODELVIEW);
	//GLUquadric * quad = gluNewQuadric();
	//gluQuadricDrawStyle(quad, GLU_FILL);

    glPushName(objid);
	for (f=0; f<m_nVertexCount; f++)
		glDrawElementFaceGoround(f, NULL);
    glPopName();

	//gluDeleteQuadric(quad);
}


void CPointObj::DrawPickingObjectFace(const int objid)
{
	DrawPickingObject(objid);
}

void CPointObj::DrawPickingObjectLine(const int objid)
{
	DrawPickingObject(objid);
}


void CPointObj::_exportPovrayFile(FILE *fp, const double *matrix)
{
	int i;
	double v[3], r;

	//output vertices;
	fprintf(fp, "union {\n");
	//fprintf(fp, "  vertex_vectors {\n");
	//fprintf(fp, "\t%d\n", m_nVertexCount); 
	for (i=0; i<m_nVertexCount; i++){
		r = m_pRadius[i];
		if (r==0)
		r = m_fGivenRad;
		r *= m_pDrawParms->m_fVertexSizeScale;

		if (matrix)
			TransformVertex3dToVertex3d(m_pVertex[i], matrix, v);
		else{
			const double * ptr = &m_pVertex[i].x;
			v[0]=ptr[0], v[1]=ptr[1], v[2]=ptr[2];
		}
		fprintf(fp, "\tsphere {<%lf,%lf,%lf>, %lf}\n", v[0], v[1], -v[2], r);
	}
	fprintf(fp, "}\n");	
}


//export data using the specified format and transform
void CPointObj::exportFile(FILE *fp, const char *format, const double * matrix)
{
	if (strcmp(format, ".pov")==0 || strcmp(format, ".POV")==0)
		_exportPovrayFile(fp, matrix);

}

