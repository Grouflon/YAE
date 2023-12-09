#pragma once

#include <yae/types.h>
#include <yae/ID.h>

#include <core/containers/Pool.h>

namespace yae {

class YAE_API SceneGraphNode
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

	ID<SceneGraphNode> getID() const;
	void setParent(ID<SceneGraphNode> _parentID);
	void setParent(SceneGraphNode& _parent);
	void setParent(SceneGraphNode* _parent);
	ID<SceneGraphNode> getParent() const;
	const DataArray<ID<SceneGraphNode>>& getChildren() const;

	Matrix4 getWorldMatrix() const;
	Matrix4 getLocalMatrix() const;
	void setWorldMatrix(const Matrix4& _matrix);
	void setLocalMatrix(const Matrix4& _matrix);

//private:
	void _setWorldTransformDirty();
	void _refreshWorldTransform() const; // not actually const, but we allow cache refresh in const functions

	ID<SceneGraphNode> m_id;
	ID<SceneGraphNode> m_parent;
	DataArray<ID<SceneGraphNode>> m_children;

	Transform m_localTransform = Transform::IDENTITY();
	mutable Transform m_worldTransform;
	mutable bool m_isWorldTransformDirty = true;
};

} // namespace yae
