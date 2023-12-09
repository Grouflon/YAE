#pragma once

#include <game/game.h>

#include <core/containers/Pool.h>
#include <yae/math_types.h>

namespace yae {

class SpatialNode;

struct GAME_API NodeID
{
	NodeID() {}
	NodeID(u64 _id) : id(_id) {}

	u64 id;

	SpatialNode* get() const;
	SpatialNode& operator*() const;
	SpatialNode* operator->() const;

	bool operator==(const NodeID& _rhs) const { return id == _rhs.id; }
	bool operator!=(const NodeID& _rhs) const { return id != _rhs.id; }

	static const NodeID INVALID;
};

class GAME_API SpatialNode
{
public:
	Vector3 getWorldPosition() const;
	Quaternion getWorldRotation() const;
	Vector3 getWorldScale() const;
	Transform getWorldTransform() const;

	void setWorldPosition(const Vector3& _position);
	void setWorldRotation(const Quaternion& _rotation);
	void setWorldScale(const Vector3& _scale);
	void setWorldTransform(const Transform& _transform);

	Vector3 getLocalPosition() const;
	Quaternion getLocalRotation() const;
	Vector3 getLocalScale() const;
	Transform getLocalTransform() const;

	void setLocalPosition(const Vector3& _position);
	void setLocalRotation(const Quaternion& _rotation);
	void setLocalScale(const Vector3& _scale);
	void setLocalTransform(const Transform& _transform);	

	void setParent(NodeID _parentID);
	NodeID getParent() const;
	DataArray<NodeID> getChildren() const;

	NodeID getID() const;
	Matrix4 getWorldMatrix() const;
	Matrix4 getLocalMatrix() const;

//private:
	void _setWorldTransformDirty();
	void _refreshWorldTransform() const; // not actually const, but we allow cache refresh in const functions

	NodeID m_id = NodeID::INVALID;
	DataArray<NodeID> m_children;
	NodeID m_parent = NodeID::INVALID;

	Transform m_localTransform;
	mutable Transform m_worldTransform;
	mutable bool m_isWorldTransformDirty = true;
};

class GAME_API SpatialSystem
{
public:

	NodeID createNode();
	void destroyNode(NodeID _id);

	const DataArray<NodeID> getRoots() const;

	void registerRoot(NodeID _id);
	void unregisterRoot(NodeID _id);

	SpatialNode* findNode(NodeID _id) const;


//private:
	Pool<SpatialNode> m_nodes;

	DataArray<NodeID> m_roots;
};

GAME_API SpatialSystem& spatialSystem();

} //namespace yae
