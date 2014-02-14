//FILE: drawparms.h
#ifndef __INC_DRAWPARMS_H__
#define __INC_DRAWPARMS_H__

#include <sysconf.h>
#include <vectorall.h>


#define AVL_PARALLEL_PROJECTION 0
#define AVL_PERSPECTIVE_PROJECTION 1

//return the color buffer and number of colors used in CGLDrawParms class
Vector3f * getColorLibrary(int & ncolor);


enum ScalarToScalarMappingType{
	ScalarMappingLinear=0, 
	ScalarMappingLog2, 
	ScalarMappingLog10
};


#define ScalarToTextureCoorMappingType int
#define COLOR_LEVEL_BUFFLENGTH 17

class CGLDrawParms
{
private: 
	static Vector3f m_cColorLevel[COLOR_LEVEL_BUFFLENGTH];

public:
	enum CONSTANTS
	{
		NAME_ARRAY_SIZE = 50
	};

	enum glDrawMeshStyle{
		DRAW_MESH_SHADING=0, 
		DRAW_MESH_HIDDENLINE, 
		DRAW_MESH_LINE,
		DRAW_MESH_POINT_EMBOSS,
		DRAW_MESH_LINE_POINT_EMBOSS,
		//---------------------------
		DRAW_VOLUME_TRANSPARENT,
		DRAW_VOLUME_SHADING,
		DRAW_MESH_NONE
	};

	enum glDrawVertexStyle{
		DRAW_VERTEX_POINT=0, 
		DRAW_VERTEX_POINTSPRITE, 
		DRAW_VERTEX_SPHERE, 
		DRAW_VERTEX_CUBE, 
		DRAW_VERTEX_TETRA, 
		DRAW_VERTEX_NONE 
	};

	bool m_bValidDrawParms;			//is the class enabled/disabled??
	bool m_bMirrorFlag;				//mirrorred node??

	bool m_bEnableLighting;
	bool m_bEnableDepthTest;
	bool m_bEnableSmoothing;
	bool m_bEnableShadow;

	bool m_bEnableTexture1D;
	bool m_bEnableTexture2D;
	bool m_bEnableTexture3D;
	bool m_bEnableVertexTexture;

	bool m_bEnableVertexRandDir;	//random direction
	bool m_bEnablePNTriangleSubdiv;	//subdivide a tri in Bezier triangle patch

	bool m_bShowVertexNormal;
	bool m_bShowAixes;
	bool m_bShowBoudingCube;
	bool m_bShowCamera;
	bool m_bShowSphereControl;

    bool m_bUseDispList;
    bool m_bUseVbo;

	glDrawMeshStyle		m_nDrawType;
	glDrawVertexStyle	m_nDrawVertexStyle;

	Vector3f m_cObjectColor;
	Vector3f m_cHLineColor;
	Vector3f m_cVertexColor;
	Vector3f m_cNormalColor;
	Vector3f m_cBackgroundColor;

	float m_fLineWidth;
	float m_fAxisLineWidth;
	float m_fVertexSizeScale;
	float m_fNormalLengthScale;

	short m_nShowMinOctreeLevel;
	short m_nShowMaxOctreeLevel;
	int m_nShowOctMinX;
	int m_nShowOctMaxX;
	int m_nShowOctMinY;
	int m_nShowOctMaxY;
	int m_nShowOctMinZ;
	int m_nShowOctMaxZ;

	bool m_bDrawSelectedCell;
	bool m_bProjectionMode;
	bool m_bUseVertexNormal;
	bool m_bInEPSSnag;

	//The parameters for LOD rendering;
	double m_ModelView[16];
	double m_Projection[16];
	double m_Identity[16];
	int m_Viewport[4];
	Vector3f m_ViewPos;
	Vector3f m_ViewDir;
	float m_fFovy;
	float m_fFocalLength;
	float m_x0, m_x1;
	float m_y0, m_y1;
	float m_fViewConstant;

	//==================================
	Vector3f m_cLevel0Color;
	Vector3f m_cLevelNColor;

	Vector3d vertex1;
	Vector3d vertex2;

	//========SHOW PICKINIG==============
	int m_nPickingTYpe;			//pick obj, face, edge, vertex, etc...
	int m_nPickedEdgeID;
	Vector3d m_vPickedVertex0;	//first picking line
	Vector3d m_vPickedVertex1;

	Vector3d m_vPickedVertex2;	//second picking line
	Vector3d m_vPickedVertex3;

	bool m_bRecomputeTexture1DParameter;
	bool m_bRecomputeTexture1DRange;

	//===============================
	unsigned char m_texture1D[256][4];
	ScalarToScalarMappingType m_nMappingType;
	ScalarToTextureCoorMappingType m_nTextureCoorType;
	unsigned int m_nTextureHandle;
	float m_fMinRange;
	float m_fMaxRange;
	
	//the variable names used for texture mapping;
	char * m_strVarNames[NAME_ARRAY_SIZE];

	//Legend stuff
	int m_nLegendLevel;
	Vector3f m_cLegendTextColor;
	bool m_bShowLegend;
	bool m_bShowVertexAttribValue;
	unsigned char m_cLegendPosition;
	unsigned char m_cLegendOrientation;

	//for volume rendering, transfer func pointer
	unsigned char ** m_ppColorTable;
	unsigned int * m_pFboMRT;		//frame buffer object in the window

public:
	CGLDrawParms(const int colorstep=0);

	~CGLDrawParms(){}

	void BeginDrawing();

	void PostDrawing();

	void setupMaterial(void) const; 

	void reCalcLevelColor(const Vector3f& cLow, const Vector3f& cHigh, const int maxLevel);

	Vector3f getLevelColor(const int i) const
	{
		ASSERT0(i>=0 && i<COLOR_LEVEL_BUFFLENGTH);
		return m_cColorLevel[i];
	}


	//==========================LOD rendering==============================
	void WorldTransform(const double v[3], double vt[3]);

	void WorldTransform(const Vector3d& v, Vector3d & vt)
	{
		WorldTransform(&v.x, &vt.x);
	}

	void WorldTransform(const Vector3f &v, Vector3f &vt)
    {
        double p[3]={v.x, v.y, v.z};
        double q[3];
	    WorldTransform(p, q);
        vt.x=q[0], vt.y=q[1], vt.z=q[2];
    }


	//world point to screen point
	void World2Screen(const double v[3], double winxyz[3]);
	void World2Screen(const Vector3d& v, Vector3d & winv)
	{
		World2Screen(&v.x, &winv.x);
	}
	void World2Screen(const Vector3f& v, Vector3f & winv)
	{
        Vector3d p(v.x, v.y, v.z), q;
		World2Screen(&p.x, &q.x);
        winv.x=q.x, winv.y=q.y, winv.z=q.z;
	}

	//screen point to world point
	void Screen2World(const double winxyz[3], double v[3]);
	void Screen2World(const Vector3d& winv, Vector3d & v)
	{
		Screen2World(&winv.x, &v.x);
	}
	void Screen2World(const Vector3f& winv, Vector3f & v)
	{
        Vector3d p(winv.x, winv.y, winv.z), q;
		Screen2World(&p.x, &q.x);
        v.x=q.x, v.y=q.y, v.z=q.z;
    }
	
	bool VertexVisible(const Vector3d& v, Vector3d & winv)
	{
		World2Screen(v, winv);
		if (winv.x >= m_Viewport[0] && winv.x< m_Viewport[2] &&
			winv.y >= m_Viewport[1] && winv.y< m_Viewport[3])
			return true;
		else
			return false;
	}

    bool VertexVisible(const Vector3f& v, Vector3f & winv)
	{
		World2Screen(v, winv);
		if (winv.x >= m_Viewport[0] && winv.x< m_Viewport[2] &&
			winv.y >= m_Viewport[1] && winv.y< m_Viewport[3])
			return true;
		else
			return false;
	}

	bool CellVisible(const int l, const Vector3I pos);

	bool ScreenErrorHigh(const Vector3d& winv, const float qerror)
	{
		float dist2 = winv.z - 0.5;
		dist2*=dist2;
		float c = m_fViewConstant * dist2; 
		if (qerror > c)
			return true;
		return false;
	}

	bool ScreenErrorHigh(const Vector3f& winv, const float qerror)
	{
        Vector3d p(winv.x, winv.y, winv.z);
	    return ScreenErrorHigh(p, qerror);
	}

	void ComputeLODConstant(const float tao) const
	{
		float fovy = m_fFovy*PI/180;
		float h = m_Viewport[3]-m_Viewport[1];
		float * t = (float *)(&m_fViewConstant);
		*t = tao * fovy / h;
		*t *= *t;
	}

	bool Qrefine(const int level, const Vector3I& pos, const Vector3f&v, const float qerror, bool & visible);
	bool Qrefine2(const int level, const Vector3I& pos, const Vector3f&v, const float qerror, bool & visible);

	//==========================LOD rendering==============================

	void DrawPickedLine(void);

	//=====every time a new object is created, object change color ========
	static Vector3f allocateNewColor(void);

	static Vector3f previewNewColor(void);

	//rest the color pointer to 0
	static void resetColorIndex(void);
};

#endif //drawparms.h
