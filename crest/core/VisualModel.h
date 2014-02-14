/******************************************************************************
*       CIA3D Framework, Version 1.0									      *
*                (c) 2007 Nan Zhang @ Univeristy of Minnesota                 *
*                                                                             *
*                                                                             *
* Contact information: nanzhang790@yahoo.com	                              *
*                                                                             *
* Authors: Nan Zhang													      *
******************************************************************************/
/******************************************************************************
   File Name: visualmodel.h
   Author: Nan Zhang
   Date: Aug 20, 2008
*******************************************************************************/

#ifndef CIA3DFRAMEWORK_CORE_VISUALMODEL_H
#define CIA3DFRAMEWORK_CORE_VISUALMODEL_H

#include <crest/core/baseobject.h>

namespace cia3d{

namespace core{

/*! \class VisualModel
  *  \brief An interface which all VisualModel inherit.
  *
  *	 <P> This Interface is used for the VisualModel. VisualModel is an interface implemented by all VisualModel.<BR>
  *  VisualModel is drawn so it has a function draw. The function updatePosition is used to compute the displacement of vertices (only in OBJModel)<BR>
  *  It's a generic interface that is bound by a Mapping with a BehaviorModel<BR>
  *  This Interface will change a lot when a SceneGraph Manager like OpenSceneGraph is used <BR></P>
  */
class VisualModel : public virtual ZBaseObject
{
public:
	virtual ~VisualModel() { }

	/*! \fn void initTextures() 
	 *  \brief initialize the textures
	 */
	virtual void initTextures() = 0;

	/*! \fn void draw() 
	 *  \brief display the VisualModel object.
	 */
	virtual void draw() = 0;

	/*! \fn void drawTransparent()
	 *  \brief display transparent surfaces.
	 */
	virtual void drawTransparent()
	{
	}

	/*! \fn void drawShadow() 
	 *  \brief display shadow-casting surfaces.
	 */
	virtual void drawShadow()
	{
		draw();
	}

	/*! \fn void update()
	 *  \brief used to compute the displacement of the model
	 */
	virtual void update() = 0;
	
	/*! \fn void addBBox()
	 *  \brief used to add the bounding-box of this visual model to the
	 *  given bounding box in order to compute the scene bounding box or
	 *  cull hidden objects.
	 *
	 *  Return false if the visual model does not define any bounding box,
	 *  which should only be the case for "debug" objects, as this lack of
	 *  information might affect performances and leads to incorrect scene
	 *  bounding box.
	 */
	virtual bool addBBox(double* /*minBBox*/, double* /*maxBBox*/)
	{
		return false;
	}
	
	/// Append this mesh to an OBJ format stream.
	/// The number of vertices position, normal, and texture coordinates already written is given as parameters
	/// This method should update them
	virtual void exportOBJ(std::string /*name*/, std::ostream* /*out*/, std::ostream* /*mtl*/, int& /*vindex*/, int& /*nindex*/, int& /*tindex*/){};

};

} // namespace core

} // namespace cia3d

#endif
