//File: BarycentricMapping.h--Moving Least Square based mapping

#ifndef CIA3D_COMPONENT_BARYCENTRICPOLAR_MAPPING_H
#define CIA3D_COMPONENT_BARYCENTRICPOLAR_MAPPING_H

#include <vector>
#include <crest/component/BarycentricMapping.h>

namespace cia3d{
namespace component{


class BarycentricPolarMapping : public BarycentricMapping
{	
private:

	vector<Vector4f> m_elmQuat;
	vector<Vector4f> m_vertQuat;
	void init(CPolyObj *src, CPolyObj *dst);
	void _computeElementQuats(const CPolyObj *poly, const Vector3d *deformv, vector<Vector4f>& quat);
	void _computeRotation(const Vector3d *vin);

public:
	
	BarycentricPolarMapping(void): BarycentricMapping()
	{
		m_elmQuat.clear();
		m_vertQuat.clear();
	}
	
	virtual ~BarycentricPolarMapping()
	{
		m_elmQuat.clear();
		m_vertQuat.clear();
	}
	
	virtual bool canDeformNormal(void)
	{
		return true;
	}

	virtual void updateMapping(const Vector3d* in, Vector3d* out);
	
	virtual void updateMapping(const Vector3d* vin, Vector3f* vout);

	virtual void parse(ZBaseObjectDescription* arg)
	{
		BarycentricMapping::parse(arg);
		init(m_pSrcObj, m_pDstObj);
	}
};


} // namespace component
} // namespace cia3d

#endif
