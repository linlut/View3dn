/****************************************************************************

 Copyright (C) 2002-2008 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.3.1.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#include <quaternion.h>
#include <stdlib.h> 
#include <assert.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433
#endif

// All the methods are declared inline in Quaternion.h
using namespace std;

/*! Constructs a Quaternion that will rotate from the \p from direction to the \p to direction.

Note that this rotation is not uniquely defined. The selected axis is usually orthogonal to \p from
and \p to. However, this method is robust and can handle small or almost identical vectors. */
Quaternion::Quaternion(const Vector3d& from, const Vector3d& to)
{
	assert(0);
	const double epsilon = ZERO_QUAT_NORM*ZERO_QUAT_NORM;
	const double fromSqNorm = squaredNorm(from);	//from.squaredNorm();
	const double toSqNorm   = squaredNorm(to);		//to.squaredNorm();
	// Identity Quaternion when one vector is null
	if ((fromSqNorm < epsilon) || (toSqNorm < epsilon)){
		q[0]=q[1]=q[2]=0.0; q[3]=1.0;
	}
	else{
		Vector3d axis = CrossProd(from, to);
		const double axisSqNorm = squaredNorm(axis);

		// Aligned vectors, pick any axis, not aligned with from or to
		//if (axisSqNorm < epsilon)
		//	axis = from.orthogonalVec();
		double angle = asin(sqrt(axisSqNorm / (fromSqNorm * toSqNorm)));
		if (from*to < 0.0)
			angle = M_PI-angle;
		setAxisAngle(axis, angle);
	}
}

/*! Set the Quaternion from a (supposedly correct) 3x3 rotation matrix.

  The matrix is expressed in European format: its three \e columns are the images by the rotation of
  the three vectors of an orthogonal basis. Note that OpenGL uses a symmetric representation for its
  matrices.

  setFromRotatedBasis() sets a Quaternion from the three axis of a rotated frame. It actually fills
  the three columns of a matrix with these rotated basis vectors and calls this method. */
void Quaternion::setFromRotationMatrix(const double m[3][3])
{
	// Compute one plus the trace of the matrix
	const double onePlusTrace = 1.0 + m[0][0] + m[1][1] + m[2][2];
	if (onePlusTrace > 0){
		// Direct computation
		const double s = sqrt(onePlusTrace) * 2.0;
		const double s1 = 1.0/s;
		q[0] = (m[2][1] - m[1][2]) * s1;
		q[1] = (m[0][2] - m[2][0]) * s1;
		q[2] = (m[1][0] - m[0][1]) * s1;
		q[3] = 0.25 * s;
	}
	else{
		// Computation depends on major diagonal term
		if ((m[0][0] > m[1][1]) & (m[0][0] > m[2][2])){ 
			const double s = sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2.0; 
			const double s1 = 1.0/s;
			q[0] = 0.25 * s;
			q[1] = (m[0][1] + m[1][0]) * s1; 
			q[2] = (m[0][2] + m[2][0]) * s1; 
			q[3] = (m[1][2] - m[2][1]) * s1;
		}
		else if (m[1][1] > m[2][2]){ 
			const double s = sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2.0; 
			const double s1 = 1.0/s;
			q[0] = (m[0][1] + m[1][0]) * s1; 
			q[1] = 0.25 * s;
			q[2] = (m[1][2] + m[2][1]) * s1; 
			q[3] = (m[0][2] - m[2][0]) * s1;
		}
		else{ 
			const double s = sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2.0; 
			const double s1 = 1.0/s;
			q[0] = (m[0][2] + m[2][0]) * s1; 
			q[1] = (m[1][2] + m[2][1]) * s1; 
			q[2] = 0.25 * s;
			q[3] = (m[0][1] - m[1][0]) * s1;
		}
	}

	normalize();
}

#ifndef DOXYGEN
void Quaternion::setFromRotationMatrix(const float m[3][3])
{
	//qWarning("setFromRotationMatrix now waits for a double[3][3] parameter");
	double mat[3][3];
	double *p = &mat[0][0]; 
	const float *q = &m[0][0];
	p[0]=(double)q[0]; p[1]=(double)q[1]; p[2]=(double)q[2];
	p[3]=(double)q[3]; p[4]=(double)q[4]; p[5]=(double)q[5];
	p[6]=(double)q[6]; p[7]=(double)q[7]; p[8]=(double)q[8];
	setFromRotationMatrix(mat);
}

void Quaternion::setFromRotatedBase(const Vector3d& X, const Vector3d& Y, const Vector3d& Z)
{
	//qWarning("setFromRotatedBase is deprecated, use setFromRotatedBasis instead");
	setFromRotatedBasis(X,Y,Z);
}
#endif

/*! Sets the Quaternion from the three rotated vectors of an orthogonal basis.

  The three vectors do not have to be normalized but must be orthogonal and direct (X^Y=k*Z, with k>0).

  \code
  Quaternion q;
  q.setFromRotatedBasis(X, Y, Z);
  // Now q.rotate(Vector3d(1,0,0)) == X and q.inverseRotate(X) == Vector3d(1,0,0)
  // Same goes for Y and Z with Vector3d(0,1,0) and Vector3d(0,0,1).
  \endcode

  See also setFromRotationMatrix() and Quaternion(const Vector3d&, const Vector3d&). */
void Quaternion::setFromRotatedBasis(const Vector3d& X, const Vector3d& Y, const Vector3d& Z)
{
	double m[3][3];
	double normX = normLength(X);
	double normY = normLength(Y);
	double normZ = normLength(Z);
	for (int i=0; i<3; ++i){
		m[i][0] = X[i] / normX;
		m[i][1] = Y[i] / normY;
		m[i][2] = Z[i] / normZ;
    }
	setFromRotationMatrix(m);
}

/*! Returns the axis vector and the angle (in radians) of the rotation represented by the Quaternion.
 See the axis() and angle() documentations. */
void Quaternion::getAxisAngle(Vector3d& axis, double& angle) const
{
	angle = 2.0*acos(q[3]);
	axis = Vector3d(q[0], q[1], q[2]);
	const double sinus = normLength(axis);
	if (sinus > ZERO_QUAT_NORM)
		axis /= sinus;
	if (angle > M_PI){
		angle = 2.0*M_PI - angle;
		axis = -axis;
    }
}

/*! Returns an XML \c QDomElement that represents the Quaternion.

 \p name is the name of the QDomElement tag. \p doc is the \c QDomDocument factory used to create
 QDomElement.

 When output to a file, the resulting QDomElement will look like:
 \code
 <name q0=".." q1=".." q2=".." q3=".." />
 \endcode

 Use initFromDOMElement() to restore the Quaternion state from the resulting \c QDomElement. See
 also the Quaternion(const QDomElement&) constructor.

 See the Vector3d::domElement() documentation for a complete QDomDocument creation and saving example.

 See also Frame::domElement(), Camera::domElement(), KeyFrameInterpolator::domElement()... */
/*
QDomElement Quaternion::domElement(const QString& name, QDomDocument& document) const
{
  QDomElement de = document.createElement(name);
  de.setAttribute("q0", QString::number(q[0]));
  de.setAttribute("q1", QString::number(q[1]));
  de.setAttribute("q2", QString::number(q[2]));
  de.setAttribute("q3", QString::number(q[3]));
  return de;
}
*/

/*! Restores the Quaternion state from a \c QDomElement created by domElement().

 The \c QDomElement should contain the \c q0, \c q1 , \c q2 and \c q3 attributes. If one of these
 attributes is missing or is not a number, a warning is displayed and these fields are respectively
 set to 0.0, 0.0, 0.0 and 1.0 (identity Quaternion).

 See also the Quaternion(const QDomElement&) constructor. */
/*
void Quaternion::initFromDOMElement(const QDomElement& element)
{
  Quaternion q(element);
  *this = q;
}
*/

/*! Constructs a Quaternion from a \c QDomElement representing an XML code of the form
  \code< anyTagName q0=".." q1=".." q2=".." q3=".." />\endcode

  If one of these attributes is missing or is not a number, a warning is displayed and the associated
  value is respectively set to 0, 0, 0 and 1 (identity Quaternion).

  See also domElement() and initFromDOMElement(). */
/*
Quaternion::Quaternion(const QDomElement& element)
{
  QStringList attribute;
  attribute << "q0" << "q1" << "q2" << "q3";
#if QT_VERSION >= 0x040000
  for (int i=0; i<attribute.size(); ++i)
#else
  for (unsigned int i=0; i<attribute.count(); ++i)
#endif
    q[i] = DomUtils::floatFromDom(element, attribute[i], ((i<3)?0.0f:1.0f));
}
*/

/*! Returns the Quaternion associated 4x4 OpenGL rotation matrix.

 Use \c glMultMatrixd(q.matrix()) to apply the rotation represented by Quaternion \c q to the
 current OpenGL matrix.

 See also getMatrix(), getRotationMatrix() and inverseMatrix().

 \attention The result is only valid until the next call to matrix(). Use it immediately (as shown
 above) or consider using getMatrix() instead.

 \attention The matrix is given in OpenGL format (row-major order) and is the transpose of the
 actual mathematical European representation. Consider using getRotationMatrix() instead. */
const GLdouble* Quaternion::matrix() const
{
	static GLdouble m[4][4];
	getMatrix(m);
	return (const GLdouble*)(m);
}

/*! Fills \p m with the OpenGL representation of the Quaternion rotation.

Use matrix() if you do not need to store this matrix and simply want to alter the current OpenGL
matrix. See also getInverseMatrix() and Frame::getMatrix(). */
void Quaternion::getMatrix(GLdouble m[4][4]) const
{
	const double q00 = 2.0l * q[0] * q[0];
	const double q11 = 2.0l * q[1] * q[1];
	const double q22 = 2.0l * q[2] * q[2];
	const double q01 = 2.0l * q[0] * q[1];
	const double q02 = 2.0l * q[0] * q[2];
	const double q03 = 2.0l * q[0] * q[3];
	const double q12 = 2.0l * q[1] * q[2];
	const double q13 = 2.0l * q[1] * q[3];
	const double q23 = 2.0l * q[2] * q[3];

	m[0][0] = 1.0 - q11 - q22;
	m[1][0] =       q01 - q23;
	m[2][0] =       q02 + q13;

	m[0][1] =       q01 + q23;
	m[1][1] = 1.0 - q22 - q00;
	m[2][1] =       q12 - q03;

	m[0][2] =       q02 - q13;
	m[1][2] =       q12 + q03;
	m[2][2] = 1.0 - q11 - q00;

	m[0][3] = 0.0l;
	m[1][3] = 0.0l;
	m[2][3] = 0.0l;

	m[3][0] = 0.0l;
	m[3][1] = 0.0l;
	m[3][2] = 0.0l;
	m[3][3] = 1.0l;
}

/*! Same as getMatrix(), but with a \c GLdouble[16] parameter. See also getInverseMatrix() and Frame::getMatrix(). */
void Quaternion::getMatrix(GLdouble m[16]) const
{
	static GLdouble mat[4][4];
	getMatrix(mat);
	int count = 0;
	for (int i=0; i<4; ++i){
		for (int j=0; j<4; ++j)
			m[count++] = mat[i][j];
	}
}

/*! Fills \p m with the 3x3 rotation matrix associated with the Quaternion.

  See also getInverseRotationMatrix().

  \attention \p m uses the European mathematical representation of the rotation matrix. Use matrix()
  and getMatrix() to retrieve the OpenGL transposed version. */
void Quaternion::getRotationMatrix(float m[3][3]) const
{
	static GLdouble mat[4][4];
	getMatrix(mat);
	for (int i=0; i<3; ++i){
		for (int j=0; j<3; ++j){
			// Beware of transposition
			m[i][j] = (float)mat[j][i];
		}
	}
}

/*! Returns the associated 4x4 OpenGL \e inverse rotation matrix. This is simply the matrix() of the
  inverse().

  \attention The result is only valid until the next call to inverseMatrix(). Use it immediately (as
  in \c glMultMatrixd(q.inverseMatrix())) or use getInverseMatrix() instead.

  \attention The matrix is given in OpenGL format (row-major order) and is the transpose of the
  actual mathematical European representation. Consider using getInverseRotationMatrix() instead. */
const GLdouble* Quaternion::inverseMatrix() const
{
	static GLdouble m[4][4];
	getInverseMatrix(m);
	return (const GLdouble*)(m);
}

/*! Fills \p m with the OpenGL matrix corresponding to the inverse() rotation.

Use inverseMatrix() if you do not need to store this matrix and simply want to alter the current
OpenGL matrix. See also getMatrix(). */
void Quaternion::getInverseMatrix(GLdouble m[4][4]) const
{
	inverse().getMatrix(m);
}

/*! Same as getInverseMatrix(), but with a \c GLdouble[16] parameter. See also getMatrix(). */
void Quaternion::getInverseMatrix(GLdouble m[16]) const
{
	inverse().getMatrix(m);
}

/*! \p m is set to the 3x3 \e inverse rotation matrix associated with the Quaternion.
 \attention This is the classical mathematical rotation matrix. The OpenGL format uses its
 transposed version. See inverseMatrix() and getInverseMatrix(). */
void Quaternion::getInverseRotationMatrix(float m[3][3]) const
{
	static GLdouble mat[4][4];
	getInverseMatrix(mat);
	for (int i=0; i<3; ++i){
		for (int j=0; j<3; ++j){
			// Beware of transposition
			m[i][j] = (float)mat[j][i];
		}
	}
}

/*! Returns the slerp interpolation of Quaternions \p a and \p b, at time \p t.

 \p t should range in [0,1]. Result is \p a when \p t=0 and \p b when \p t=1.

 When \p allowFlip is \c true (default) the slerp interpolation will always use the "shortest path"
 between the Quaternions' orientations, by "flipping" the source Quaternion if needed (see
 negate()). */
Quaternion Quaternion::slerp(const Quaternion& a, const Quaternion& b, const double &t, const bool allowFlip)
{
	double cosAngle = Quaternion::dot(a, b);
	double c1, c2;
	// Linear interpolation for close orientations
	if (fabs(cosAngle) > 0.99){
		c1 = 1.0 - t;
		c2 = t;
	}
	else{
		// Spherical interpolation
		double angle = acos(fabs(cosAngle));
		double sinAngle = sin(angle);
		c1 = sin(angle * (1.0 - t)) / sinAngle;
		c2 = sin(angle * t) / sinAngle;
    }

	// Use the shortest path
	if (allowFlip && (cosAngle < 0.0)) 
		c1 = -c1;
	const double dx = c1*a[0] + c2*b[0];
	const double dy = c1*a[1] + c2*b[1];
	const double dz = c1*a[2] + c2*b[2];
	const double dw = c1*a[3] + c2*b[3];
	return Quaternion(dx, dy, dz, dw);
}

/*! Returns the slerp interpolation of the two Quaternions \p a and \p b, at time \p t, using
  tangents \p tgA and \p tgB.

  The resulting Quaternion is "between" \p a and \p b (result is \p a when \p t=0 and \p b for \p
  t=1).

  Use squadTangent() to define the Quaternion tangents \p tgA and \p tgB. */
Quaternion Quaternion::squad(const Quaternion& a, const Quaternion& tgA, const Quaternion& tgB, const Quaternion& b, const double& t)
{
	Quaternion ab = Quaternion::slerp(a, b, t);
	Quaternion tg = Quaternion::slerp(tgA, tgB, t, false);
	return Quaternion::slerp(ab, tg, 2.0*t*(1.0-t), false);
}

/*! Returns the logarithm of the Quaternion. See also exp(). */
Quaternion Quaternion::log()
{
	Quaternion r;
	const double len = sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2]);
	if (len < ZERO_QUAT_NORM)
		r = Quaternion(q[0], q[1], q[2], 0.0);
	else{
		const double coef = acos(q[3]) / len;
		r = Quaternion(q[0]*coef, q[1]*coef, q[2]*coef, 0.0);
	}
	return r;
}

/*! Returns the exponential of the Quaternion. See also log(). */
Quaternion Quaternion::exp()
{
	Quaternion r;
	const double theta = sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2]);
	if (theta < ZERO_QUAT_NORM)
		r = Quaternion(q[0], q[1], q[2], cos(theta));
	else{
		const double coef = sin(theta) / theta;
		r = Quaternion(q[0]*coef, q[1]*coef, q[2]*coef, cos(theta));
	}
	return r;
}

/*! Returns log(a. inverse() * b). Useful for squadTangent(). */
Quaternion Quaternion::lnDif(const Quaternion& a, const Quaternion& b)
{
	Quaternion dif = a.inverse()*b;
	dif.normalize();
	return dif.log();
}

/*! Returns a tangent Quaternion for \p center, defined by \p before and \p after Quaternions.

 Useful for smooth spline interpolation of Quaternion with squad() and slerp(). */
Quaternion Quaternion::squadTangent(const Quaternion& before, const Quaternion& center, const Quaternion& after)
{
	Quaternion l1 = Quaternion::lnDif(center,before);
	Quaternion l2 = Quaternion::lnDif(center,after);
	Quaternion e;
	for (int i=0; i<4; ++i)
		e.q[i] = -0.25 * (l1.q[i] + l2.q[i]);
	e = center*(e.exp());
	// if (Quaternion::dot(e,b) < 0.0)
    // e.negate();
	return e;
}

ostream& operator<<(ostream& o, const Quaternion& Q)
{
	return o << Q[0] << '\t' << Q[1] << '\t' << Q[2] << '\t' << Q[3];
}

/*! Returns a random unit Quaternion.

You can create a randomly directed unit vector using:
\code
Vector3d randomDir = Quaternion::randomQuaternion() * Vector3d(1.0, 0.0, 0.0); // or any other Vector3d
\endcode

\note This function uses rand() to create pseudo-random numbers and the random number generator can
be initialized using srand().*/
Quaternion Quaternion::randomQuaternion()
{
	// The rand() function is not very portable and may not be available on your system.
	// Add the appropriate include or replace by an other random function in case of problem.
	double seed = rand()/(float)RAND_MAX;
	double r1 = sqrt(1.0 - seed);
	double r2 = sqrt(seed);
	double t1 = 2.0 * M_PI * (rand()/(float)RAND_MAX);
	double t2 = 2.0 * M_PI * (rand()/(float)RAND_MAX);
	return Quaternion(sin(t1)*r1, cos(t1)*r1, sin(t2)*r2, cos(t2)*r2);
}

int testfunc(void)
{
	{
		const double angle = 0;
		Quaternion a(Vector3d(0, 0, 1), angle);
		a.invert();
		Quaternion b(Vector3d(0, 0, 1), angle);
		Quaternion c = Quaternion::slerp_midpoint(a, b);
	}
	{
		Vector3d axis1, axis2, axis3, axis4, axis5, axis6;
		double angle1, angle2, angle3, angle4, angle5, angle6;
		Quaternion a(1, 1, 1, 1);
		a.normalize();
		a.getAxisAngle(axis1, angle1);
		Quaternion b(1, 1, 1, -1);
		b.normalize();
		b.getAxisAngle(axis2, angle2);
		Quaternion b2(-1, -1, -1, 1);
		b2.normalize();
		b2.getAxisAngle(axis3, angle3);

		Quaternion c = Quaternion::slerp_midpoint(a, b2);
		c.getAxisAngle(axis4, angle4);
		Quaternion d = Quaternion::slerp(a, b, 0.5, true);
		d.getAxisAngle(axis5, angle5);
		Quaternion e = Quaternion::slerp(a, b2, 0.5, true);
		e.getAxisAngle(axis6, angle6);
		Quaternion g1=e; g1.inverse();
	}
	{
		Vector3d axis1, axis2, axis3, axis4, axis5, axis6;
		double angle1, angle2, angle3, angle4, angle5;
		Quaternion a(Vector3d(0, 0, 1), -M_PI);
		a.normalize();
		a.getAxisAngle(axis1, angle1);
		Quaternion b(Vector3d(0, 0, 1), M_PI);
		b.normalize();
		b.getAxisAngle(axis2, angle2);
		Quaternion b2 = a; 
		b2.invert();
		
		double q0, q1, q2, q3;
		double m[4][4];
		b2.getValue(q0, q1, q2, q3);
		Quaternion b3(q0, q1, q2, -q3); 

		Quaternion c = Quaternion::slerp_midpoint(a, b);
		c.getAxisAngle(axis3, angle3);
		c.getMatrix(m);
		Quaternion d = Quaternion::slerp_midpoint(a, b2);
		d.getAxisAngle(axis4, angle4);
		Quaternion e = Quaternion::slerp_midpoint(a, b3);
		e.getAxisAngle(axis5, angle5);
		int x=1;
	}
	return 1;
}

static int testquat = testfunc();