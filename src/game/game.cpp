#include "game.h"

#include <yae/hash.h>
#include <yae/serialization.h>
#include <yae/resources/FileResource.h>
#include <yae/math_types.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <stdio.h>
#include <vector>

MIRROR_CLASS_DEFINITION(ConfigData);

using namespace yae;


struct Quat : public glm::quat
{
	Quat() : glm::quat() {};
	Quat(const glm::quat& _q) : glm::quat(_q) {};
	Quat(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

	operator glm::quat()
	{
		return *((glm::quat*)(this));
	}
	static const Quat IDENTITY;
};

const Quat Quat::IDENTITY = Quat(0.f, 0.f, 0.f, 1.f);

struct Matrix44 : public glm::mat4x4
{
	Matrix44() : glm::mat4x4() {}
	Matrix44(const glm::mat4x4& _m) : glm::mat4x4(_m) {}
	Matrix44(
		float _x0, float _y0, float _z0, float _w0,
		float _x1, float _y1, float _z1, float _w1,
		float _x2, float _y2, float _z2, float _w2,
		float _x3, float _y3, float _z3, float _w3
	) : glm::mat4x4(_x0, _y0, _z0, _w0, _x1, _y1, _z1, _w1, _x2, _y2, _z2, _w2, _x3, _y3, _z3, _w3)
	{

	}

	operator glm::mat4x4()
	{
		return *((glm::mat4x4*)(this));
	}

	Vector3 getTranslation() const;
	Quat getRotation() const;
	Vector3 getScale() const;

	static const Matrix44 IDENTITY;
};

const Matrix44 Matrix44::IDENTITY = Matrix44(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
);


Vector3 Matrix44::getTranslation() const
{
	return Vector3((*this)[3][0], (*this)[3][1], (*this)[3][2]);
}


Quat Matrix44::getRotation() const
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	// @TODO(remi): Should extract the rotation part from this function
	bool ret = glm::decompose(*this, scale, rotation, translation, skew, perspective);
	YAE_ASSERT(ret);
	rotation = glm::conjugate(rotation);
	return Quat(rotation.x, rotation.y, rotation.z, rotation.w);
}


Vector3 Matrix44::getScale() const
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	// @TODO(remi): Should extract the scale part from this function
	bool ret = glm::decompose(*this, scale, rotation, translation, skew, perspective);
	YAE_ASSERT(ret);
	return Vector3(scale.x, scale.y, scale.z);
}


class TransformComponent
{
public:
	TransformComponent();
	~TransformComponent();

	void setLocalPosition(const Vector3& _position);
	void setLocalRotation(const Quat& _rotation);
	void setLocalScale(const Vector3& _scale);

	Vector3 getLocalPosition() const;
	Quat getLocalRotation() const;
	Vector3 getLocalScale() const;

	Vector3 getWorldPosition() const;
	Quat getWorldRotation() const;
	Vector3 getWorldScale() const;
	Matrix44 getWorldMatrix() const;

private:
	bool _isWorldMatrixDirty() const;
	void _computeMatrix() const;

	Vector3 m_position = Vector3::ZERO;
	Quat m_rotation = Quat::IDENTITY;
	Vector3 m_scale = Vector3::ONE;
	TransformComponent* m_parent = nullptr;

	mutable Matrix44 m_worldMatrix;
	mutable bool m_isWorldMatrixDirty = true;
};


TransformComponent::TransformComponent()
{

}


TransformComponent::~TransformComponent()
{

}


void TransformComponent::setLocalPosition(const Vector3& _position)
{
	m_position = _position;
	m_isWorldMatrixDirty = true;
}


void TransformComponent::setLocalRotation(const Quat& _rotation)
{
	m_rotation = _rotation;
	m_isWorldMatrixDirty = true;
}


void TransformComponent::setLocalScale(const Vector3& _scale)
{
	m_scale = _scale;
	m_isWorldMatrixDirty = true;
}


Vector3 TransformComponent::getLocalPosition() const
{
	return m_position;
}


Quat TransformComponent::getLocalRotation() const
{
	return m_rotation;
}


Vector3 TransformComponent::getLocalScale() const
{
	return m_scale;
}


Vector3 TransformComponent::getWorldPosition() const
{
	return getWorldMatrix().getTranslation();
}


Quat TransformComponent::getWorldRotation() const
{
	return getWorldMatrix().getRotation();
}


Vector3 TransformComponent::getWorldScale() const
{
	return getWorldMatrix().getScale();
}

Matrix44 TransformComponent::getWorldMatrix() const
{
	if (_isWorldMatrixDirty())
	{
		_computeMatrix();
	}
	return m_worldMatrix;
}

bool TransformComponent::_isWorldMatrixDirty() const
{
	if (m_isWorldMatrixDirty)
	{
		return true;
	}

	if (m_parent)
	{
		return m_parent->_isWorldMatrixDirty();
	}
	return false;
}


void TransformComponent::_computeMatrix() const
{
	Matrix44 m = Matrix44::IDENTITY;
	Matrix44 t = Matrix44(1.f, 0.f, 0.f, 0.f,  0.f, 1.f, 0.f, 0.f,  0.f, 0.f, 1.f, 0.f,  m_position.x, m_position.y, m_position.z, 1.f);
	Matrix44 r = glm::toMat4(m_rotation);
	Matrix44 s = Matrix44(m_scale.x, 0.f, 0.f, 0.f,  0.f, m_scale.y, 0.f, 0.f,  0.f, 0.f, m_scale.z, 0.f,  0.f, 0.f, 0.f, 1.f);
	m = t * r * s;
	if (m_parent)
	{
		m = m_parent->getWorldMatrix() * m;
	}
	m_worldMatrix = m;
	m_isWorldMatrixDirty = false;
}


void onLibraryLoaded()
{
	TransformComponent c;
	c.setLocalPosition(Vector3(1.f, 2.f, 3.f));
	c.setLocalScale(Vector3(1.f, 2.f, 3.f));
	Matrix44 m = c.getWorldMatrix();
	int a = 0;

	//printf("\x1b[31mBA.\r\n");
	YAE_LOG("Bonjour");
	//
	/*{
		FileResource* configFile = findOrCreateResource<FileResource>("./config.json");
		configFile->useLoad();

		//for (int i = 0; i < 10000; ++i)
		ConfigData config;
		JsonSerializer serializer;
		serializer.beginRead(configFile->getContent(), configFile->getContentSize());
		serializer.serialize(&config, ConfigData::GetClass());
		serializer.endRead();

		const void* buffer;
		size_t bufferSize;
		serializer.beginWrite();
		serializer.serialize(&config, ConfigData::GetClass());
		serializer.endWrite(&buffer, &bufferSize);

		printf("hello: %s", (const char*)buffer);

		configFile->releaseUnuse();
	}

	mirror::PointerTypeDesc* pointerType = (mirror::PointerTypeDesc*)(mirror::GetTypeDesc<ConfigData*>());
	mirror::TypeDesc* type = pointerType->getSubType();

	mirror::TypeSet* typeSet = mirror::GetTypeSet();
	std::vector<mirror::TypeDesc*> types;
	for (auto& type : typeSet->getTypes())
	{
		YAE_LOG(type->getName());
		types.push_back(type);
	}*/


}

void onLibraryUnloaded()
{
}

void initGame()
{
}

void updateGame()
{
	//printf("Hello again World!\n");
}

void shutdownGame()
{

}
