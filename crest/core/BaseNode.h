//FILE: BaseNode.h

#ifndef CIA3D_CORE_OBJECTMODEL_BASENODE_H
#define CIA3D_CORE_OBJECTMODEL_BASENODE_H

#include "BaseContext.h"

namespace cia3d{
namespace core{

class BaseObject;

/// Base class for simulation nodes.
class BaseNode : public virtual Base
{
public:
	virtual ~BaseNode() {}
	
	/// @name Scene hierarchy
	/// @{
	
	/// Get parent node (or NULL if no hierarchy or for root node)
	virtual BaseNode* getParent() = 0;
	
	/// Get parent node (or NULL if no hierarchy or for root node)
	virtual const BaseNode* getParent() const = 0;
	
	/// Add a child node
	virtual void addChild(BaseNode* node) = 0;
	
	/// Remove a child node
	virtual void removeChild(BaseNode* node) = 0;
	
	/// Add a generic object
	virtual bool addObject(BaseObject* obj) = 0;
	
	/// Remove a generic object
	virtual bool removeObject(BaseObject* obj) = 0;

	/// Get this node context
	virtual BaseContext* getContext() = 0;

	/// Get this node context
	virtual const BaseContext* getContext() const = 0;
	
	/// @}
};

} // namespace core
} // namespace cia3d


#endif
