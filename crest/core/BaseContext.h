//FILE: BaseContext.h

#ifndef CIA3D_CORE_OBJECTMODEL_BASECONTEXT_H
#define CIA3D_CORE_OBJECTMODEL_BASECONTEXT_H

#include <set>
#include <crest/base/vec.h>
#include <crest/core/Base.h>
//#include <crest/defaulttype/SolidTypes.h>
//#include <sofa/defaulttype/SofaBaseMatrix.h>

namespace cia3d
{
namespace core
{

class BaseObject;
class Event;

/// Base class for storing shared variables and parameters.
class BaseContext : public virtual Base
{

public:
	/*
    typedef defaulttype::SolidTypes<double> SolidTypes;
    typedef SolidTypes::Transform Frame;
    typedef SolidTypes::Vec Vec3;
    typedef SolidTypes::Rot Quat;
    typedef SolidTypes::Mat Mat33;
    typedef SolidTypes::SpatialVector SpatialVector;
*/
    BaseContext();
    virtual ~BaseContext();

    static BaseContext* getDefault();

    /// @name Parameters
    /// @{

    /// Simulation time
    virtual double getTime() const;

    /// Simulation timestep
    virtual double getDt() const;

    /// Animation flag
    virtual bool getAnimate() const;

    /// MultiThreading activated
    virtual bool getMultiThreadSimulation() const;

    /// Display flags: Collision Models
    virtual bool getShowCollisionModels() const;

    /// Display flags: Bounding Collision Models
    virtual bool getShowBoundingCollisionModels() const;

    /// Display flags: Behavior Models
    virtual bool getShowBehaviorModels() const;

    /// Display flags: Visual Models
    virtual bool getShowVisualModels() const;

    /// Display flags: Mappings
    virtual bool getShowMappings() const;

    /// Display flags: Mechanical Mappings
    virtual bool getShowMechanicalMappings() const;

    /// Display flags: ForceFields
    virtual bool getShowForceFields() const;

    /// Display flags: InteractionForceFields
    virtual bool getShowInteractionForceFields() const;

    /// Display flags: WireFrame
    virtual bool getShowWireFrame() const;

    /// Display flags: Normals
    virtual bool getShowNormals() const;

    /// Multiresolution
    virtual int getCurrentLevel() const;
    virtual int getCoarsestLevel() const;
    virtual int getFinestLevel() const;
    virtual unsigned int nbLevels() const;

    /// @}


    /// @name Local Coordinate System
    /// @{
    /// Projection from the local coordinate system to the world coordinate system.
    //virtual const Frame& getPositionInWorld() const;
    /// Projection from the local coordinate system to the world coordinate system.
    //virtual void setPositionInWorld(const Frame&){}

    /// Spatial velocity (linear, angular) of the local frame with respect to the world
    //virtual const SpatialVector& getVelocityInWorld() const;

    /// Spatial velocity (linear, angular) of the local frame with respect to the world
    //virtual void setVelocityInWorld(const SpatialVector&){}

    /// Linear acceleration of the origin induced by the angular velocity of the ancestors
    //virtual const Vec3& getVelocityBasedLinearAccelerationInWorld() const;

    /// Linear acceleration of the origin induced by the angular velocity of the ancestors
    //virtual void setVelocityBasedLinearAccelerationInWorld(const Vec3& ){}
    /// @}


    /// Gravity in local coordinates
    virtual Vec3 getLocalGravity() const;
    /// Gravity in local coordinates
    //virtual void setGravity( const Vec3& ) { }
    /// Gravity in world coordinates
    virtual const Vec3& getGravityInWorld() const;
    /// Gravity in world coordinates
    virtual void setGravityInWorld( const Vec3& )
    { }

    /// @name Variables
    /// @{

    /// Mechanical Degrees-of-Freedom
    virtual BaseObject* getMechanicalState() const;

    /// Topology
    virtual BaseObject* getTopology() const;

    /// Topology
    virtual BaseObject* getMainTopology() const;

    /// @}

    /// @name Parameters Setters
    /// @{


    /// Simulation timestep
    virtual void setDt( double )
    { }

    /// Animation flag
    virtual void setAnimate(bool )
    { }

    /// MultiThreading activated
    virtual void setMultiThreadSimulation(bool )
    { }

    /// Display flags: Collision Models
    virtual void setShowCollisionModels(bool )
    { }

    /// Display flags: Bounding Collision Models
    virtual void setShowBoundingCollisionModels(bool )
    { }

    /// Display flags: Behavior Models
    virtual void setShowBehaviorModels(bool )
    { }

    /// Display flags: Visual Models
    virtual void setShowVisualModels(bool /*val*/)
    { }

    /// Display flags: Mappings
    virtual void setShowMappings(bool /*val*/)
    { }

    /// Display flags: Mechanical Mappings
    virtual void setShowMechanicalMappings(bool /*val*/)
    { }

    /// Display flags: ForceFields
    virtual void setShowForceFields(bool /*val*/)
    { }

    /// Display flags: InteractionForceFields
    virtual void setShowInteractionForceFields(bool /*val*/)
    { }

    /// Display flags: WireFrame
    virtual void setShowWireFrame(bool /*val*/)
    { }

    /// Display flags: Normals
    virtual void setShowNormals(bool /*val*/)
    { }
	
	/// Multiresolution
	virtual bool setCurrentLevel(int ){return false;}  ///< set the current level, return false if l >= coarsestLevel
	virtual void setCoarsestLevel(int ){}
	virtual void setFinestLevel(int ){}

    /// @}

    /// @name Variables Setters
    /// @{

    /// Mechanical Degrees-of-Freedom
    virtual void setMechanicalState( BaseObject* )
    { }

    /// Topology
    virtual void setTopology( BaseObject* )
    { }

    /// @}

    /// @name Adding/Removing objects. Note that these methods can fail if the context don't support attached objects
    /// @{

    /// Add an object, or return false if not supported
    virtual bool addObject( BaseObject* /*obj*/ )
    {
        return false;
    }

    /// Remove an object, or return false if not supported
    virtual bool removeObject( BaseObject* /*obj*/ )
    {
        return false;
    }

    /// @}

    /// @name Actions.
    /// @{

    /// Propagate an event 
    //virtual void propagateEvent( Event* );

    /// apply an action
    //virtual void executeAction( simulation::tree::Action* );

    /// @}
};


} // namespace core
} // namespace cia3d

#endif


