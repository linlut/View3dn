//FILE: PointOrgnizer.h

#ifndef __INC_POINTORGNIZER_ID909585_H__
#define __INC_POINTORGNIZER_ID909585_H__

#include <memmgr.h>
#include <vector3d.h>


#define __GRIDSIZE_XYZ_SUM__ 48


class CPointOrgnizer3D
{
private:
	class CPointList3211
	{
	public:
		Vector3d* m_pPoint;
		int m_nID;
		CPointList3211 *m_pNext;
	};

	inline int _searchPointInPointList3211(const CPointList3211 *phead, const Vector3d& v, const double SQRDIST)
	{
		int rtval = -1;
		const CPointList3211 *p = phead;   
		while (p!=NULL){
			const double d2 = Distance2(*(p->m_pPoint), v);
			if (d2<SQRDIST){
				rtval = p->m_nID;
				break;
			}
			p=p->m_pNext;
		}
		return rtval;
	}

	inline int _searchNearestPointInPointList3211(const CPointList3211 *phead, const Vector3d& v, double& SQRDIST)
	{
		int rtval = -1;
		SQRDIST = 1e60;
		const CPointList3211 *p = phead;   
		while (p!=NULL){
			const double d2 = Distance2(*(p->m_pPoint), v);
			if (d2<SQRDIST){
				SQRDIST = d2;
				rtval = p->m_nID;
			}
			p=p->m_pNext;
		}
		return rtval;
	}

private:
    CMemoryMgr m_memMgr;
    Vector3d m_vLowLeft;    //boundary lowleft corner
    Vector3d m_vUpRight;    //bounding box upright corner
    int m_nx;
    int m_ny;
    int m_nz;
    double m_fUnitX1;         //grid size x
    double m_fUnitY1;         //grid size y
    double m_fUnitZ1;         //grid size z

    CPointList3211 **** m_pPoint;

    void _init(void)
    {
        m_vLowLeft = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
        m_vUpRight = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
        m_nx = m_ny = m_nz = 0;
        m_pPoint=NULL;
    }

    void _computeBoundingBox(const Vector3d* v, const int vlen, Vector3d &lowleft, Vector3d &upright)
    {
        lowleft = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
        upright = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
        for (int i=0; i<vlen; i++){
            const Vector3d& vv=v[i];
            const Vector3d r(vv.x, vv.y, vv.z);
            Minimize(lowleft, r);
            Maximize(upright, r);
        }
    }

    void _computeBoundingBox(const Vector3d* v, const int *pindex, const int vindexlen, Vector3d &lowleft, Vector3d &upright)
    {
        lowleft = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
        upright = Vector3d(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
        for (int i=0; i<vindexlen; i++){
            const int j=pindex[i];
            const Vector3d& vv=v[j];
            const Vector3d r(vv.x, vv.y, vv.z);
            Minimize(lowleft, r);
            Maximize(upright, r);
        }
    }

    void _computeGridSize(const Vector3d &lowleft, const Vector3d& upright)
    {
        Vector3d dif1 = (upright-lowleft);  
        Vector3d dif = dif1 * 0.0011f;
        m_vLowLeft = lowleft - dif;
        m_vUpRight = upright + dif;
        dif = m_vUpRight - m_vLowLeft;
        const double fsize = fabs(dif.x+dif.y+dif.z);
        const double stepsize = fsize/__GRIDSIZE_XYZ_SUM__;

        m_nx = (int)ceil(dif.x/stepsize);
        m_ny = (int)ceil(dif.y/stepsize);
        m_nz = (int)ceil(dif.z/stepsize);
        if (m_nx<1) m_nx=1;
        if (m_ny<1) m_ny=1;
        if (m_nz<1) m_nz=1;
        
        m_fUnitX1= 1/stepsize;
        m_fUnitY1= m_fUnitX1;
        m_fUnitZ1= m_fUnitX1;
    }

    void _getGridPos(const Vector3d& p, int &x, int &y, int &z)
    {
        Vector3d dist;
        dist.x= p.x-m_vLowLeft.x;
        dist.y= p.y-m_vLowLeft.y;
        dist.z= p.z-m_vLowLeft.z;
        x = (int)(dist.x*m_fUnitX1);
        y = (int)(dist.y*m_fUnitY1);
        z = (int)(dist.z*m_fUnitZ1);
    }

    void _allocGridBuffer(const int nx, const int ny, const int nz)
    {
        m_nx = nx; m_ny = ny; m_nz = nz;
        char***p= m_memMgr.Malloc3DArray(nx, ny, nz, sizeof(CPointList3211*));
        assert(p!=NULL);
        m_pPoint = (CPointList3211 ****)p;
        for (int k=0; k<m_nz; k++){
            for (int j=0; j<m_ny; j++){
                for (int i=0; i<m_nx; i++){
                    m_pPoint[i][j][k]=NULL;
                }
            }
        }
    }

    void _orgnizeVertexArray(const Vector3d* v, const int vlen)
    {
        _computeBoundingBox(v, vlen, m_vLowLeft, m_vUpRight);
        initOrgnizerWithBoundingBox(m_vLowLeft, m_vUpRight);
        appendVertices(v, vlen);
    }

    void _orgnizeVertexArray(const Vector3d* v, const int *pindex, const int vindexlen)
    {
        _computeBoundingBox(v, pindex, vindexlen, m_vLowLeft, m_vUpRight);
        initOrgnizerWithBoundingBox(m_vLowLeft, m_vUpRight);

        for (int i=0; i<vindexlen; i++){
            const int j = pindex[i];
            int ix, iy, iz;
            _getGridPos(v[j], ix, iy, iz);
            ASSERT0(ix<m_nx && iy<m_ny && iz<m_nz);
            CPointList3211* phead=m_pPoint[ix][iy][iz];
            CPointList3211* pnode=(CPointList3211*)m_memMgr.Malloc(sizeof(CPointList3211));
            pnode->m_nID = j;
            pnode->m_pPoint = (Vector3d*)&v[j];
            pnode->m_pNext = phead;
            m_pPoint[ix][iy][iz] = pnode;
        }
    }

	//error tol SQRDIST(square distance error tol)
    int _findNearestPointInGridCell(const int ix, const int iy, const int iz, const Vector3d & v, double& SQRDIST)
    {
        if (ix<0 || ix>=m_nx) return -1;
        if (iy<0 || iy>=m_ny) return -1;
        if (iz<0 || iz>=m_nz) return -1;
		CPointList3211 *phead = m_pPoint[ix][iy][iz];
        const int id = _searchNearestPointInPointList3211(phead, v, SQRDIST);
		return id;
    }

	//Return the minimal dist of current input vs. an array of searched results
	void _minimizeDistance(int &ID, double &MIN_DIST2, const int ids[], const double min_dist2[], const int len)
	{
		for (int i=0; i<len; i++){
			const int id = ids[i];
			if (id!=-1){
				if (min_dist2[i]<MIN_DIST2){
					MIN_DIST2 = min_dist2[i];
					ID = id;
				}
			}
		}
	}

    int _findNearestPointInGridCellPlaneX_YZ(const int ix, const int iy, const int iz, const int r, const int sign, const Vector3d & v, double& SQRDIST)
	{
		int ID = -1;
		double min_dist2, MIN_DIST2 = 1e60;
		const int i = ix + r*sign;
		for (int j=iy-r; j<=iy+r; j++){
			for (int k=iz-r; k<=iz+r; k++){
				const int id = _findNearestPointInGridCell(i, j, k, v, min_dist2);
				_minimizeDistance(ID, MIN_DIST2, &id, &min_dist2, 1);
			}
		}
		return ID;
	}

	int _findNearestPointInGridCellPlaneY_ZX(const int ix, const int iy, const int iz, const int r, const int sign, const Vector3d & v, double& SQRDIST)
	{
		int ID = -1;
		double min_dist2, MIN_DIST2 = 1e60;
		const int j = iy + r*sign;
		for (int k=iz-r; k<=iz+r; k++){
			for (int i=ix-r; i<=ix+r; i++){
				const int id = _findNearestPointInGridCell(i, j, k, v, min_dist2);
				_minimizeDistance(ID, MIN_DIST2, &id, &min_dist2, 1);
			}
		}
		return ID;
	}

	int _findNearestPointInGridCellPlaneZ_XY(const int ix, const int iy, const int iz, const int r, const int sign, const Vector3d & v, double& SQRDIST)
	{
		int ID = -1;
		double min_dist2, MIN_DIST2 = 1e60;
		const int k = iz + r*sign;
		for (int j=iy-r; j<=iy+r; j++){
			for (int i=ix-r; i<=ix+r; i++){
				const int id = _findNearestPointInGridCell(i, j, k, v, min_dist2);
				_minimizeDistance(ID, MIN_DIST2, &id, &min_dist2, 1);
			}
		}
		return ID;
	}

public:

    CPointOrgnizer3D(void):
        m_memMgr()
    {
        _init();
    }

    ~CPointOrgnizer3D(void)
    {
        m_memMgr.Free();
    }

    void initOrgnizerWithBoundingBox(const Vector3d &lowleft, const Vector3d &upright)
    {
        m_vLowLeft = lowleft, m_vUpRight = upright;
        _computeGridSize(m_vLowLeft, m_vUpRight);
        _allocGridBuffer(m_nx, m_ny, m_nz);
    }

    void appendVertices(const Vector3d* v, const int vlen)
    {
        for (int i=0; i<vlen; i++){
            int ix, iy, iz;
            _getGridPos(v[i], ix, iy, iz);
            ASSERT0(ix<m_nx && iy<m_ny && iz<m_nz);
            CPointList3211* phead=m_pPoint[ix][iy][iz];
            CPointList3211* pnode=(CPointList3211*)m_memMgr.Malloc(sizeof(CPointList3211));
            pnode->m_nID = i;
            pnode->m_pPoint = (Vector3d*)&v[i];
            pnode->m_pNext = phead;
            m_pPoint[ix][iy][iz] = pnode;
        }
    }

	//This function for the volume data
    CPointOrgnizer3D(const Vector3d* v, const int vlen):
        m_memMgr()
    {
        _init();
        _orgnizeVertexArray(v, vlen);
    }

	//This function for surface points in a volumetric data
    CPointOrgnizer3D(const Vector3d* v, const int *pindex, const int vindexlen):
        m_memMgr()
    {
        _init();
        _orgnizeVertexArray(v, pindex, vindexlen);
    }

	//search the point cloud to find the first vertex ID within a given 
	//error tol SQRDIST(square distance error tol)
    int searchPoint(const Vector3d & q, const double& SQRDIST)
    {
        int i, j, k, ix, iy, iz;
        _getGridPos(q, ix, iy, iz);
        int i0=ix-1, i1=ix+1;
        int j0=iy-1, j1=iy+1;
        int k0=iz-1, k1=iz+1;
        if (i0<0) i0=0;
        if (i1>=m_nx) i1=m_nx-1;
        if (j0<0) j0=0;
        if (j1>=m_ny) j1=m_ny-1;
        if (k0<0) k0=0;
        if (k1>=m_nz) k1=m_nz-1;

        for (k=k0; k<=k1; k++){
            for (j=j0; j<=j1; j++){
                for (i=i0; i<=i1; i++){
                    CPointList3211 *phead = m_pPoint[i][j][k];
                    const int id = _searchPointInPointList3211(phead, q, SQRDIST);
                    if (id!=-1) return id;
                }
            }
        }
        return -1;
    }

	int searchNearestPoint(const Vector3d & q)
    {
		double MIN_DIST2=1e60;
        int ix, iy, iz, ID=-1, R_MAX;
        _getGridPos(q, ix, iy, iz);

		//search in the closest neighbourhood, it is most likely
        int i0=ix-1, i1=ix+1;
        int j0=iy-1, j1=iy+1;
        int k0=iz-1, k1=iz+1;
        if (i0<0) i0=0;
        if (i1>=m_nx) i1=m_nx-1;
        if (j0<0) j0=0;
        if (j1>=m_ny) j1=m_ny-1;
        if (k0<0) k0=0;
        if (k1>=m_nz) k1=m_nz-1;
        for (int k=k0; k<=k1; k++){
            for (int j=j0; j<=j1; j++){
                for (int i=i0; i<=i1; i++){
                    CPointList3211 *phead = m_pPoint[i][j][k];
					double min_dist2;
                    const int id = _searchNearestPointInPointList3211(phead, q, min_dist2);
					_minimizeDistance(ID, MIN_DIST2, &id, &min_dist2, 1);
                }
            }
        }
		//if the closest point is found, then return
        if (ID!=-1) return ID;

		//if not found, we need to increase the search radius
		{
			int r1, r2;
			r1 = ix, r2 = m_nx - ix; R_MAX=_MAX_(r1, r2);
			r1 = iy, r2 = m_ny - iy; R_MAX=_MAX3_(R_MAX, r1, r2);
			r1 = iz, r2 = m_nz - iz; R_MAX=_MAX3_(R_MAX, r1, r2);
		}
		for (int r=2; r<R_MAX; r++){
			double mindist[6];
			int id[6];
		    id[0] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, 1, q, mindist[0]);
		    id[1] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, -1, q, mindist[1]);
		    id[2] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, 1, q, mindist[0]);
		    id[3] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, -1, q, mindist[1]);
		    id[4] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, 1, q, mindist[0]);
		    id[5] = _findNearestPointInGridCellPlaneX_YZ(ix, iy, iz, r, -1, q, mindist[1]);
			_minimizeDistance(ID, MIN_DIST2, id, mindist, 6);
			if (ID!=-1) return ID;
		}

		//the closest point should be found
		if (ID==-1){
			for (int k=0; k<m_nz; k++){
				for (int j=0; j<m_ny; j++){
					for (int i=0; i<m_nx; i++){
						CPointList3211 *phead = m_pPoint[i][j][k];
						double min_dist2;
						const int id = _searchNearestPointInPointList3211(phead, q, min_dist2);
						_minimizeDistance(ID, MIN_DIST2, &id, &min_dist2, 1);
					}
				}
			}
		}

		if (ID!=-1)
			return ID;
		else{
			assert(0);
			return ID;
		}
    }


};







#endif