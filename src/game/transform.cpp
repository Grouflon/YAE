#include "transform.h"

#include <yae/Application.h>
#include <yae/containers/containers.h>
#include <yae/math_3d.h>

namespace yae {

Matrix4 Transform::toMatrix4() const
{
	return Matrix4::FromTransform(position, rotation, scale);
}

Transform Transform::operator*(const Transform& _rhs) const
{
	Transform t;
	t.position = toMatrix4() * _rhs.position;
	t.rotation = _rhs.rotation * rotation;
	t.scale = scale * _rhs.scale;
	return t;
}

const NodeID NodeID::INVALID = ~u64(0);

SpatialNode* NodeID::get() const
{
	return spatialSystem().findNode(*this);
}

SpatialNode& NodeID::operator*() const
{
	return *get();
}

SpatialNode* NodeID::operator->() const
{
	return get();
}

Vector3 SpatialNode::getWorldPosition() const
{
	_refreshWorldTransform();
	return m_worldTransform.position;
}

Quaternion SpatialNode::getWorldRotation() const
{
	_refreshWorldTransform();
	return m_worldTransform.rotation;
}

Vector3 SpatialNode::getWorldScale() const
{
	_refreshWorldTransform();
	return m_worldTransform.scale;
}

Transform SpatialNode::getWorldTransform() const
{
	_refreshWorldTransform();
	return m_worldTransform;
}


void SpatialNode::setWorldPosition(const Vector3& _position)
{
	SpatialNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		Matrix4 inverseParentMatrix = math::inverse(parent->getWorldMatrix());
		setLocalPosition(inverseParentMatrix * _position);
	}
	else
	{
		setLocalPosition(_position);
	}
}

void SpatialNode::setWorldRotation(const Quaternion& _rotation)
{
	SpatialNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		Quaternion inverseParentRotation = math::inverse(parent->getWorldRotation());
		setLocalRotation(inverseParentRotation * _rotation);
	}
	else
	{
		setLocalRotation(_rotation);
	}
}

void SpatialNode::setWorldScale(const Vector3& _scale)
{
	SpatialNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		YAE_ASSERT(false);
	}
	else
	{
		setLocalScale(_scale);
	}
}

void SpatialNode::setWorldTransform(const Transform& _transform)
{
	SpatialNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		YAE_ASSERT(false);
	}
	else
	{
		setLocalTransform(_transform);
	}
}


Vector3 SpatialNode::getLocalPosition() const
{
	return m_localTransform.position;
}

Quaternion SpatialNode::getLocalRotation() const
{
	return m_localTransform.rotation;
}

Vector3 SpatialNode::getLocalScale() const
{
	return m_localTransform.scale;
}

Transform SpatialNode::getLocalTransform() const
{
	return m_localTransform;
}

void SpatialNode::setLocalPosition(const Vector3& _position)
{
	m_localTransform.position = _position;
	_setWorldTransformDirty();
}

void SpatialNode::setLocalRotation(const Quaternion& _rotation)
{
	m_localTransform.rotation = _rotation;
	_setWorldTransformDirty();
}

void SpatialNode::setLocalScale(const Vector3& _scale)
{
	m_localTransform.scale = _scale;
	_setWorldTransformDirty();
}

void SpatialNode::setLocalTransform(const Transform& _transform)
{
	m_localTransform = _transform;
	_setWorldTransformDirty();
}

void SpatialNode::setParent(NodeID _parentID)
{
	if (_parentID == m_parent)
		return;

	SpatialSystem& space = spatialSystem();
	if (m_parent != NodeID::INVALID)
	{
		YAE_VERIFY(containers::remove(m_parent->m_children, m_id) != 0);
		space.registerRoot(m_id);
	}

	m_parent = _parentID;

	if (m_parent != NodeID::INVALID)
	{
		space.unregisterRoot(m_id);
		YAE_ASSERT(containers::find(m_parent->m_children, m_id) == nullptr);
		m_parent->m_children.push_back(m_id);
	}
	_setWorldTransformDirty();
}

NodeID SpatialNode::getParent() const
{
	return m_parent;
}

DataArray<NodeID> SpatialNode::getChildren() const
{
	return m_children;
}

NodeID SpatialNode::getID() const
{
	return m_id;
}

Matrix4 SpatialNode::getWorldMatrix() const
{
	_refreshWorldTransform();
	return m_worldTransform.toMatrix4();
}

Matrix4 SpatialNode::getLocalMatrix() const
{
	return m_localTransform.toMatrix4();
}

void SpatialNode::_setWorldTransformDirty()
{
	m_isWorldTransformDirty = true;
	for (NodeID child : m_children)
	{
		child->_setWorldTransformDirty();
	}
}

void SpatialNode::_refreshWorldTransform() const
{
	if (!m_isWorldTransformDirty)
		return;

	SpatialNode* parent = m_parent.get();
	Transform parentWorldTransform = parent != nullptr ? parent->getWorldTransform() : Transform();
	m_worldTransform = parentWorldTransform * m_localTransform;
	m_isWorldTransformDirty = false;
}

NodeID SpatialSystem::createNode()
{
	PoolID poolId = m_nodes.add(SpatialNode());
	NodeID nodeId = NodeID(poolId);
	SpatialNode* node = nodeId.get();
	node->m_id = nodeId;
	registerRoot(nodeId);
	return nodeId;
}

void SpatialSystem::destroyNode(NodeID _id)
{
	YAE_VERIFY(m_nodes.remove(_id.id));
}

const DataArray<NodeID> SpatialSystem::getRoots() const
{
	return m_roots;
}

void SpatialSystem::registerRoot(NodeID _id)
{
	YAE_ASSERT_MSGF(containers::find(m_roots, _id) == nullptr, "Registering root NodeID %lld twice", _id.id);
	m_roots.push_back(_id);
}

void SpatialSystem::unregisterRoot(NodeID _id)
{
	NodeID* nodePtr = containers::find(m_roots, _id);
	YAE_ASSERT_MSGF(nodePtr != nullptr, "NodeID %lld not present in roots list", _id.id);
	m_roots.erase(nodePtr);
}

SpatialNode* SpatialSystem::findNode(NodeID _id) const
{
	return m_nodes.get(_id.id);
}

SpatialSystem& spatialSystem()
{
	SpatialSystem* spatialSystem = (SpatialSystem*)(app().getUserData("spatialSystem"));
	YAE_ASSERT(spatialSystem);
	return *spatialSystem;
}

} // namespace yae
