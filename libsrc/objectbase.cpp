

#include "objectbase.h"

// =========================================================
// Object3D::ShadeObject
//	Called by World when it is determined that the 
//	object was the closest along the given ray.
// =========================================================
RGBColor
Object3D::ShadeObject(World& world, Ray& ray, Vector3D& Intersection, int depth)
{
/*
    ShadingInfo shade;

    shade.p = Intersection;
    shade.normal = FindNormal(Intersection);
    shade.incident = ray.dir;
    shade.reflected = ray.ReflectRay(shade.normal);
    shade.obj = this;
    shade.ray = &ray;
    shade.world = &world;
    return surf->ComputeColor(shade, depth);
*/
	return RGBColor(0,0,0);
}

int
Object3D::HitAtAll(Ray& ray, float maxt)
{
    float tempt;
    return NearestInt(ray, tempt, maxt) != 0;
}

int
Object3D::IsInside(Vector3D& v)
{
    float dist = PtDistance(v);
    return dist < 0 || fabs(dist) < 1E-10;
}

void
Object3D::TransformSurface(SurfaceList& clist, const Matrix& tform)
{
	/*
    if (flags & BOUNDING) {
	((BoundingVolume *) this)->TransformSurface(clist, tform);
    }	
    else if (! (flags & CSG)) {
	if (surf && ! clist.SurfaceThere(surf)) {
	    clist.AddToList(surf);
	    surf->PreMulTransform(tform);	
	}
    }
	*/
}

int
SurfaceList::SurfaceThere(Surface *probe)
{
    for (Iterator sc(*this); sc.Valid(); sc.GotoNext()) {
	if (sc.Contents() == probe)
	    return 1;
    }
    return 0;
}

void
ObjectList::TransformList(const Matrix& Transform)
{
    for (Iterator sc(*this); sc.Valid(); sc.GotoNext())
	sc.Contents()->ApplyTransform(Transform);
}

void
ObjectList::TransformSurfaces(SurfaceList& clist, const Matrix& Transform)
{
    for (Iterator sc(*this); sc.Valid(); sc.GotoNext())
	sc.Contents()->TransformSurface(clist, Transform);
}

