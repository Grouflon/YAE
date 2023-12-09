#include "SceneGraphNode.h"

#include <yae/math_3d.h>
#include <yae/SceneSystem.h>

namespace yae {

Vector3 SceneGraphNode::getWorldPosition() const
{
	_refreshWorldTransform();
	return m_worldTransform.position;
}

Quaternion SceneGraphNode::getWorldRotation() const
{
	_refreshWorldTransform();
	return m_worldTransform.rotation;
}

Vector3 SceneGraphNode::getWorldScale() const
{
	_refreshWorldTransform();
	return m_worldTransform.scale;
}

Transform SceneGraphNode::getWorldTransform() const
{
	_refreshWorldTransform();
	return m_worldTransform;
}


void SceneGraphNode::setWorldPosition(const Vector3& _position)
{
	SceneGraphNode* parent = m_parent.get();
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

void SceneGraphNode::setWorldRotation(const Quaternion& _rotation)
{
	SceneGraphNode* parent = m_parent.get();
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

void SceneGraphNode::setWorldScale(const Vector3& _scale)
{
	SceneGraphNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		YAE_ASSERT(false);
	}
	else
	{
		setLocalScale(_scale);
	}
}

void SceneGraphNode::setWorldTransform(const Transform& _transform)
{
	SceneGraphNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		YAE_ASSERT(false);
	}
	else
	{
		setLocalTransform(_transform);
	}
}


Vector3 SceneGraphNode::getLocalPosition() const
{
	return m_localTransform.position;
}

Quaternion SceneGraphNode::getLocalRotation() const
{
	return m_localTransform.rotation;
}

Vector3 SceneGraphNode::getLocalScale() const
{
	return m_localTransform.scale;
}

Transform SceneGraphNode::getLocalTransform() const
{
	return m_localTransform;
}

void SceneGraphNode::setLocalPosition(const Vector3& _position)
{
	m_localTransform.position = _position;
	_setWorldTransformDirty();
}

void SceneGraphNode::setLocalRotation(const Quaternion& _rotation)
{
	m_localTransform.rotation = _rotation;
	_setWorldTransformDirty();
}

void SceneGraphNode::setLocalScale(const Vector3& _scale)
{
	m_localTransform.scale = _scale;
	_setWorldTransformDirty();
}

void SceneGraphNode::setLocalTransform(const Transform& _transform)
{
	m_localTransform = _transform;
	_setWorldTransformDirty();
}

void SceneGraphNode::setParent(ID<SceneGraphNode> _parentID)
{
	setParent(_parentID.get());
}

void SceneGraphNode::setParent(SceneGraphNode& _parent)
{
	setParent(&_parent);
}

void SceneGraphNode::setParent(SceneGraphNode* _parent)
{
	SceneGraphNode* previousParent = m_parent.get();
	if (_parent == previousParent)
		return;

	if (previousParent != nullptr)
	{
		previousParent->m_children.erase(m_parent);
	}

	m_parent = _parent != nullptr ? _parent->m_id : ID<SceneGraphNode>::INVALID;
	if (_parent != nullptr)
	{
		_parent->m_children.push_back(m_id);
	}

	_setWorldTransformDirty();
}

ID<SceneGraphNode> SceneGraphNode::getParent() const
{
	return m_parent;
}

const DataArray<ID<SceneGraphNode>>& SceneGraphNode::getChildren() const
{
	return m_children;
}

ID<SceneGraphNode> SceneGraphNode::getID() const
{
	return m_id;
}

Matrix4 SceneGraphNode::getWorldMatrix() const
{
	_refreshWorldTransform();
	return Matrix4::FromTransform(m_worldTransform);
}

Matrix4 SceneGraphNode::getLocalMatrix() const
{
	return Matrix4::FromTransform(m_localTransform);
}

void SceneGraphNode::setWorldMatrix(const Matrix4& _matrix)
{
	SceneGraphNode* parent = m_parent.get();
	if (parent != nullptr)
	{
		YAE_ASSERT(false);
	}
	else
	{
		setLocalMatrix(_matrix);
	}
}

void SceneGraphNode::setLocalMatrix(const Matrix4& _matrix)
{
	yae::math::decompose(_matrix, m_localTransform.position, m_localTransform.rotation, m_localTransform.scale);
}

void SceneGraphNode::_setWorldTransformDirty()
{
	m_isWorldTransformDirty = true;
	for (ID<SceneGraphNode> childID : m_children)
	{
		SceneGraphNode* child = childID.get();
		YAE_ASSERT(child != nullptr);
		child->_setWorldTransformDirty();
	}
}

void SceneGraphNode::_refreshWorldTransform() const
{
	if (!m_isWorldTransformDirty)
		return;

	SceneGraphNode* parent = m_parent.get();
	Transform parentWorldTransform = parent != nullptr ? parent->getWorldTransform() : Transform();
	m_worldTransform = parentWorldTransform * m_localTransform;
	m_isWorldTransformDirty = false;
}

} // namespace yae
