//FILE: Context.h
#ifndef SOFA_CORE_OBJECTMODEL_CONTEXT_H
#define SOFA_CORE_OBJECTMODEL_CONTEXT_H

#include <iostream>
#include <map>
#include <crest/core/BaseContext.h>
#include <crest/core/DataField.h>
#include <crest/base/vec.h>

namespace cia3d{
namespace core{

class Context : public BaseContext
{

public:
    //begin ContextData
    //typedef BaseContext::Frame Frame;
    //typedef BaseContext::Vec3 Vec3;
    //typedef BaseContext::Quat Quat;
    //typedef BaseContext::SpatialVector SpatialVector;
        
    DataField<Vec3> worldGravity_;  ///< Gravity IN THE WORLD COORDINATE SYSTEM.
    DataField<double> dt_;
    DataField<double> time_;
    DataField<bool> animate_;
    DataField<bool> showCollisionModels_;
    DataField<bool> showBoundingCollisionModels_;
    DataField<bool> showBehaviorModels_;
    DataField<bool> showVisualModels_;
    DataField<bool> showMappings_;
    DataField<bool> showMechanicalMappings_;
    DataField<bool> showForceFields_;
    DataField<bool> showInteractionForceFields_;
    DataField<bool> showWireFrame_;
    DataField<bool> showNormals_;
    DataField<bool> multiThreadSimulation_;
	/// for multiresolution usage
	DataField<int> currentLevel_;
	DataField<int> coarsestLevel_;
	DataField<int> finestLevel_;
	
	
    //Frame localFrame_;
    //SpatialVector spatialVelocityInWorld_;
    Vec3 velocityBasedLinearAccelerationInWorld_;
    
    // end ContextData   
    
//  typedef BaseContext::Frame Frame;
// 	typedef BaseContext::Quat Quat;
// 	typedef BaseContext::SpatialVector SpatialVector;

	Context();
	virtual ~Context(){}
        
	static BaseContext* getDefault();
    
    /// @name Parameters
    /// @{

    /// Gravity in the local coordinate system
    virtual Vec3 getLocalGravity() const;
    /// Gravity in the local coordinate system
    //virtual void setGravity(const Vec3& );
	/// Gravity in world coordinates
    virtual const Vec3& getGravityInWorld() const;
	/// Gravity in world coordinates
    virtual void setGravityInWorld( const Vec3& );
	
    /// Simulation timestep
    virtual double getDt() const;

    /// Simulation time
    virtual double getTime() const;

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
	

    /// @}

	
    /// @name Local Coordinate System
    /// @{
	/// Projection from the local coordinate system to the world coordinate system.
	//virtual const Frame& getPositionInWorld() const;
	/// Projection from the local coordinate system to the world coordinate system.
	//virtual void setPositionInWorld(const Frame&);

	/// Spatial velocity (linear, angular) of the local frame with respect to the world
    //    virtual const SpatialVector& getVelocityInWorld() const;
	/// Spatial velocity (linear, angular) of the local frame with respect to the world
      //  virtual void setVelocityInWorld(const SpatialVector&);

	/// Linear acceleration of the origin induced by the angular velocity of the ancestors
        virtual const Vec3& getVelocityBasedLinearAccelerationInWorld() const;
	/// Linear acceleration of the origin induced by the angular velocity of the ancestors
        virtual void setVelocityBasedLinearAccelerationInWorld(const Vec3& );
	/// @}

/*
    /// @name Variables
    /// @{

	/// Mechanical Degrees-of-Freedom
	virtual BaseObject* getMechanicalState() const;

	/// Topology
	virtual BaseObject* getTopology() const;

	/// @}
*/

	/// @name Parameters Setters
	/// @{


	/// Simulation timestep
	virtual void setDt( double dt );

	/// Simulation time
	virtual void setTime( double t );

	/// Animation flag
	virtual void setAnimate(bool val);

	/// MultiThreading activated
	virtual void setMultiThreadSimulation(bool val);

	/// Display flags: Collision Models
	virtual void setShowCollisionModels(bool val);

	/// Display flags: Behavior Models
	virtual void setShowBehaviorModels(bool val);

	/// Display flags: Bounding Collision Models
	virtual void setShowBoundingCollisionModels(bool val);

	/// Display flags: Visual Models
	virtual void setShowVisualModels(bool val);

	/// Display flags: Mappings
	virtual void setShowMappings(bool val);

	/// Display flags: Mechanical Mappings
	virtual void setShowMechanicalMappings(bool val);

	/// Display flags: ForceFields
	virtual void setShowForceFields(bool val);

	/// Display flags: InteractionForceFields
	virtual void setShowInteractionForceFields(bool val);

    /// Display flags: WireFrame
    virtual void setShowWireFrame(bool val);

    /// Display flags: Normals
    virtual void setShowNormals(bool val);

	/// Multiresolution
	virtual bool setCurrentLevel(int l);  ///< set the current level, return false if l >= coarsestLevel
	virtual void setCoarsestLevel(int l);
	virtual void setFinestLevel(int l);

	/// @}
	//static Context* getDefault();

	void copyContext(const Context& c);

	friend std::ostream& operator << (std::ostream& out, const Context& c );

};

} // namespace core
} // namespace cia3d

#endif
