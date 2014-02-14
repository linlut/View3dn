// ===================================================================
// volume3d.cpp
//	Volumetric object support routines for OORT.
//
//		 The Object-Oriented Ray Tracer (OORT)
//		Copyright (C) 1993 by Nicholas Wilt.
//
// This software product may be freely copied and distributed in
// unmodified form but may not be sold.  A nominal distribution
// fee may be charged for media and handling by freeware and
// shareware distributors.  The software product may not be
// included in whole or in part into any commercial package
// without the express written consent of the author.
// 
// This software product is provided as is without warranty of
// any kind, express or implied, including but not limited to
// the implied warranties of merchantability and fitness for a
// particular purpose.  The author assumes no liability for any
// alleged or actual damages arising from the use of this
// software.  The author is under no obligation to provide 
// service, corrections or upgrades to the software.
//
// ------------------------------------------------------------
//
// Please contact me with questions, comments, suggestions or
// other input about OORT.  My Compuserve account number is
// [75210,2455] (Internet sites can reach me at 
// 75210.2455@compuserve.com).
//					--Nicholas Wilt
// ===================================================================

#include "oort.h"
#include "volume3d.h"

// Volume3D constructors: from density and distance volumes.
void Volume3D::initVolumeObject()
{ 
	// read threshold value and profile type from densGrid
	cerr<<"Reconstruction parameters: ";
	bool thresholdSet(false);
	bool centerSet(false);
	float cx, cy, cz;
	Vector3D center(0);

	for(int i=0; i<densGrid->getNComm(); i++){
	float ht, lt, rad;
	if(sscanf(densGrid->getComm(i), "vxt center %f %f %f",&cx, &cy, &cz) 
									== 3){
		centerSet = true;
		center = Vector3D(cx, cy, cz);
	} else 
	if(sscanf(densGrid->getComm(i), "vxt threshold %f",&thr) == 1){
		thresholdSet = true;
	} else 
	if(sscanf(densGrid->getComm(i), "vxt density %f %f",&lt, &ht) == 2){
		if(!thresholdSet) thr = (lt+ht) / 2;
	} else 
	if(sscanf(densGrid->getComm(i), "vxt profile gaussian %f", &rad)==1){
		densGrid->defGradFilter(f3dIntCub, f3dGradGabor);
		cerr<<" Gaussian, Filter = "<<f3dIntCub<<", "<<f3dGradGabor;
	} else 
	if(sscanf(densGrid->getComm(i), "vxt profile linear %f", &rad)==1){
		//densGrid->defGradFilter(f3dIntLin, f3dGradLin);
		//densGrid->defGradFilter(f3dIntLin, f3dGradQuad3EF);
		// f3dGradQuad2EF is the best
		densGrid->defGradFilter(f3dIntLin, f3dGradQuad2EF);
		cerr<<" Linear, Filter = "<<f3dIntLin<<", "<<f3dGradLin;
	} else 
	if(sscanf(densGrid->getComm(i), "vxt profile quadratic %f", &rad)==1){
		densGrid->defGradFilter(f3dIntCub, f3dGradGabor);
		cerr<<" Quadratic, Filter = "<<f3dIntCub<<", "<<f3dGradGabor;
	}
	}
	cerr<<": Threshold = "<<thr<<endl;

	// initialize the bounding box

	// get real scene dimensions
	Vector3D realDim(densGrid->getGWSizeX(), 
			  densGrid->getGWSizeY(),
			  densGrid->getGWSizeZ());
	if(!centerSet) center = realDim/2;

	vol = AxisAlignedBox(Vector3D(0), realDim);
	ApplyTransform(TranslationMatrix(-center));
	
	// test, if density and dist grid are compatible
	Vector3D densDim = Vector3D(densGrid->getNX(), 
				 densGrid->getNY(), densGrid->getNZ());
	Vector3D distDim = Vector3D(distGrid->getNX(), 
				 distGrid->getNY(), distGrid->getNZ());

	if( ! (distDim + Vector3D(1) == densDim) ){
	cerr<<"Volume3D::Volume3D: distGrid"<<distDim<<" and densGrid"<<
		 densDim<<" have incorrect dimensions\n";
	exit(1);
	}
}

// Volume3D constructors: from density and distance volumes.
/*
void Volume3D::initVolumeObject()
{ 
	// read threshold value and profile type from densGrid
	cerr<<"Reconstruction parameters: ";
	for(int i=0; i<densGrid->getNComm(); i++){
	double ht, lt, rad;
	if(sscanf(densGrid->getComm(i), "vxt density %f %f",&lt, &ht) == 2)
		thr = (lt+ht) / 2;

	if(sscanf(densGrid->getComm(i), "vxt profile gaussian %f", &rad)==1){
		densGrid->defGradFilter(f3dIntCub, f3dGradGabor);
		cerr<<" Gaussian, Filter = "<<f3dIntCub<<", "<<f3dGradGabor;
	}
	if(sscanf(densGrid->getComm(i), "vxt profile linear %f", &rad)==1){
		//densGrid->defGradFilter(f3dIntLin, f3dGradLin);
		//densGrid->defGradFilter(f3dIntLin, f3dGradQuad3EF);
		// f3dGradQuad2EF is the best
		densGrid->defGradFilter(f3dIntLin, f3dGradQuad2EF);
		cerr<<" Linear, Filter = "<<f3dIntLin<<", "<<f3dGradLin;
	}
	if(sscanf(densGrid->getComm(i), "vxt profile quadratic %f", &rad)==1){
		densGrid->defGradFilter(f3dIntCub, f3dGradGabor);
		cerr<<" Quadratic, Filter = "<<f3dIntCub<<", "<<f3dGradGabor;
	}
	}
	cerr<<": Threshold = "<<thr<<endl;

	// get dimendions
	Vector3D densDim = Vector3D(densGrid->getNX(), 
				 densGrid->getNY(), densGrid->getNZ());
	Vector3D distDim = Vector3D(distGrid->getNX(), 
				 distGrid->getNY(), distGrid->getNZ());

	// initialize the remaining members
	vol = AxisAlignedBox(Vector3D(0), distDim);

	// do some tests

	if( ! (distDim + Vector3D(1) == densDim) ){
	cerr<<"Volume3D::Volume3D: distGrid"<<distDim<<" and densGrid"<<
		 densDim<<" have incorrect dimensions\n";
	exit(1);
	}

	// center the volume  and scale it to 1
	double mdim = MAX(MAX(densDim.x,densDim.y),densDim.z);
	ApplyTransform(ScaleMatrix(1/mdim,1/mdim,1/mdim)); 
	ApplyTransform(TranslationMatrix(-densDim/2/mdim));
}
*/


// Compute the nearest intersection along the ray, if any.
// This function transforms the ray to canonical space before
// passing it along to the Intersect routine for the object.
int
Volume3D::NearestInt(const Ray& ray, double& t1, double t2)
{
	Ray locRay = InvertRay(ray);

	double	   tmin, tmax;
	Statistics::Intersections::Volume3D++;
	// first, get intersection with the box
	if (! vol.ComputeMinMaxT(locRay, &tmin, &tmax)) return 0;

	// not solved in ComputeMinMaxT
	if (tmax < tmin || tmax < Limits::Threshold) return 0;

	// find intersection within the box
	if(tmin < Limits::Threshold) tmin = Limits::Threshold;
	if(getNearestIntersection(locRay, tmin, tmax)){
	if(tmin > Limits::Threshold && tmin < t2){  // intersection is OK
		t1 = tmin;
		return 1;
	}
	}
	return 0;
}

SpanList *
Volume3D::FindAllIntersections(const Ray& ray)
{
	Ray 	locRay = InvertRay(ray);
	SpanList 	*ret = 0;
	double	t1, t2;
	const int	iMax(100);	// max number of intersections
	double	iList[iMax];	// list of intersections
	int		ni;		// number of intersections found

	Statistics::Intersections::Volume3D++;
	if (! vol.ComputeMinMaxT(locRay, &t1, &t2))
	return ret;
	if (t2 < t1)	// no intersection
	return ret;

	// If both parameters are less than threshold, return no-intersection.
	// copied from Sphere::FindAllIntersections
	if (t1 < Limits::Threshold && t2 < Limits::Threshold)
	return 0;

	if( (ni = getAllIntersections(locRay, t1, t2, iList, iMax)) == 0)
	return 0;		// no intersections found!!
	
	if( ni % 2){
	cerr<<"\nVolume3D::FindAllIntersections -- Odd number of intersections found!";
	} else {
	int i;
	ret = new SpanList(Span(iList[0], this, iList[1], this));	
	for(i=1; i<ni/2; i++)
		ret->AddSpan(Span(iList[2*i], this, iList[2*i+1], this));
	}
	return ret;
}

// Check, if there is intersection of the ray with the object 
// not surface but volume 
// This function transforms the ray to canonical space before
// passing it along to the Intersect routine for the object.
int
Volume3D::HitAtAll(Ray& ray, double t2)
{
	Ray locRay = InvertRay(ray);

	double	   tmin, tmax;
	Statistics::Intersections::Volume3D++;
	// first, get intersection with the box
	if (! vol.ComputeMinMaxT(locRay, &tmin, &tmax)) return 0;

	// not solved in ComputeMinMaxT
	if (tmax < tmin || tmax < Limits::Threshold) return 0;

	// find intersection within the box
	if(tmin < Limits::Threshold) tmin = Limits::Threshold;
	if(getAtAll(locRay, tmin, tmax)){
	if(tmin > Limits::Threshold && tmin < t2){  // intersection is OK
		return 1;
	}
	}
	return 0;
}

// Volume3D normal-finding routine.
Vector3D
Volume3D::FindNormal(const Vector3D& intersection)
{
	// convert to canonical coordinates
	Vector3D p(InvertPt(intersection));
	// convert back to world coordinates
	return(Normalize(RotateOnly(mat,densGrid->intGrad(p))));
}

// Duplicate the Volume
Object3D *
Volume3D::Dup() const
{
	return(new Volume3D(*this));
}

int
Volume3D::IsInside(const Vector3D& v)
{
	Vector3D p(InvertPt(v));
	//return vol.Min()<p && p<vol.Max();
	return densGrid->intDens(p) >= thr;
}

double
Volume3D::PtDistance(const Vector3D& v)
{
	Vector3D p(InvertPt(v));
	Vector3D mmin(vol.Min()-Vector3D(Limits::Threshold));
	Vector3D mmax(vol.Max()+Vector3D(Limits::Threshold));
	return (mmin<p && p<mmax) ? -1 : 1;
}

// Describe sphere.  Indent spaces first.
void
Volume3D::Describe(int ind) const
{
	indent(ind);
	cout<<"Volume3D\n";
	//vol.Describe();
}

void
Volume3D::ApplyTransform(const Matrix& tform)
{
	InverseRayObject::ApplyTransform(tform);
}

AxisAlignedBox Volume3D::BBox() const
{
	//return Transform(vol, getTransform());
	/* do not include the volume objects into hierarchy of bounding volumes
	1. it is nor efficient repeating the same computation twice
	2. id does not work -- incorrect reflections -- do no know why
	*/

	return AxisAlignedBox(Vector3D(-MAXFLOAT), Vector3D(MAXFLOAT));
}

//----------Local methods-----------------------------------------------

// intersection of the ray with the density data
// t1 and t2 a guaranteed to be inside of the volume
int Volume3D::getNearestIntersection(const Ray& ray, double& t1, double t2)
{
	long	interpolations(0), raySteps(0);
	double	t1Old=t1;
	Vector3D p = ray.Extrap(t1);
	double	densOld(densGrid->intDens(p) - thr);

	
	float vsize; (densGrid->getGDim(&vsize));
	double step(vsize/
		MAX(MAX (ABS(ray.dir.x), ABS(ray.dir.y)), ABS(ray.dir.z))
		);

	while ( t1 < t2 ){
		raySteps++;	
		Vector3D p = ray.Extrap(t1);
		Vector3D dp = densGrid->toGrid(p);
		int m = distGrid->get((int)dp.x, (int)dp.y, (int)dp.z);
//cerr<<m<<dp<<endl;
		if( m <= 1 ){
			interpolations++;
			double dens = densGrid->intDens(p) - thr;
			if(densOld*dens <= 0) {
				locateSurface(ray, t1Old, &t1);
				Statistics::RayTraversal::
					TraversalStepsHit += raySteps;
				Statistics::RayTraversal::
					SampleInterpolationsHit += interpolations;
				Statistics::RayTraversal::VolumeRaysHit++;
				return 1;
			}
		}
		t1Old = t1;
		t1 += MAX(m-1,0.3) * step;
	}
	Statistics::RayTraversal::TraversalStepsMiss += raySteps;
	Statistics::RayTraversal::SampleInterpolationsMiss += interpolations;
	Statistics::RayTraversal::VolumeRaysMiss++;
	return 0;
}

// intersection of the ray with the density data
// t1 and t2 are guaranteed to be inside of the volume
int Volume3D::getAtAll(const Ray& ray, double& t1, double t2)
{
	long	interpolations(0), raySteps(0);
	Vector3D p = ray.Extrap(t1);
	double	densOld(densGrid->intDens(p) - thr);

	float vsize; (densGrid->getGDim(&vsize));
	double step(vsize/
		MAX(MAX (ABS(ray.dir.x), ABS(ray.dir.y)), ABS(ray.dir.z))
		);

	while ( t1 < t2 ){
		raySteps++;	
		Vector3D p = ray.Extrap(t1);
		Vector3D dp = densGrid->toGrid(p);
		int m = distGrid->get((int)dp.x, (int)dp.y, (int)dp.z);
		if( m <= 1 ){
			interpolations++;
			double dens = densGrid->intDens(p) - thr;
			if(dens > 0) {
				Statistics::RayTraversal::
					TraversalStepsHit += raySteps;
				Statistics::RayTraversal::
				SampleInterpolationsHit += interpolations;
				Statistics::RayTraversal::VolumeRaysHit++;
				return 1;
			}
		}
		t1 += MAX(m-1,0.3) * step;
	}
	Statistics::RayTraversal::TraversalStepsMiss += raySteps;
	Statistics::RayTraversal::SampleInterpolationsMiss += interpolations;
	Statistics::RayTraversal::VolumeRaysMiss++;
	return 0;
}

//
// Compute exact position of the ray-surface intersection point by bisection
//
void Volume3D::locateSurface(const Ray& ray, double x1, double *x)
{
	const int	JMAX(50);
	const double acc(0.00001);
	double dx, fmid, xmid, x2(*x);
	int	j;

	if(x1 == x2) {
		//cerr<<"*";
		return;
	}
	double f1 = densGrid->intDens( ray.Extrap(x1) );
	double f2 = densGrid->intDens( ray.Extrap(x2) );

	//x2 = (x2-x1)*(thr-f1)/(f2-f1) + x1;

	if( (f1-thr)*(f2-thr) >=0) return;

	double rtb = f1 < thr ? (dx=x2-x1, x1) : (dx=x1-x2, x2);
	for (j=1;j<=JMAX;j++) {
		fmid=densGrid->intDens(ray.Extrap(xmid=rtb+(dx *= 0.5)));
		if (fmid <= thr) rtb=xmid;
		if (ABS(dx) < acc || fmid == thr){
			*x = rtb;
			return;
		}
	}
	*x = rtb;
}

// all intersections of the ray with the density data
// tList[0] and tList[1] are guaranteed to be inside of the volume
int Volume3D::getAllIntersections(const Ray& ray, double t1, double t2, 
						double *tList, int maxCnt)
{
	int		iCnt(0);	// number of intersections found
	long	interpolations(0), raySteps(0);
	double	t1Old=t1;
	Vector3D p = ray.Extrap(t1);
	double	densOld(densGrid->intDens(p) - thr);

	float vsize; (densGrid->getGDim(&vsize));
	double step(vsize/
		MAX(MAX (ABS(ray.dir.x), ABS(ray.dir.y)), ABS(ray.dir.z))
		);

	// is the first intersection already on the surface?
	if (densOld >= 0) {
	tList[0] = t1;
	iCnt++;
	}
	while ( t1 < t2 ){
		raySteps++;	
		Vector3D p = ray.Extrap(t1);
		Vector3D dp = densGrid->toGrid(p);
		int m = distGrid->get((int)dp.x, (int)dp.y, (int)dp.z);
		if( m <= 1 ){
			interpolations++;
			double dens = densGrid->intDens(p) - thr;
			if(densOld*dens <= 0) {
			tList[iCnt] = t1;
				locateSurface(ray, t1Old, &tList[iCnt]);
			iCnt++;
			densOld = dens;
				Statistics::RayTraversal::
					TraversalStepsHit += raySteps;
				Statistics::RayTraversal::
					SampleInterpolationsHit += interpolations;
				Statistics::RayTraversal::VolumeRaysHit++;
			}
		}
		t1Old = t1;
		t1 += MAX(m-1,1) * step;
	}
	// is the last intersection on the surface?
	if (densGrid->intDens(ray.Extrap(t2)) - thr >= 0) {
	tList[iCnt] = t2;
	iCnt++;
	}
	Statistics::RayTraversal::TraversalStepsMiss += raySteps;
	Statistics::RayTraversal::SampleInterpolationsMiss += interpolations;
	Statistics::RayTraversal::VolumeRaysMiss++;
	return iCnt;
}
