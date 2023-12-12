#pragma once

#include <yae/Application.h>
#include <yae/ID.h>

namespace yae {

class MeshFile;
class TextureFile;
class FontFile;
class ShaderProgram;
class Scene;
class Entity;

class GAME_API GameApplication : public Application
{
public:
	GameApplication();
	virtual ~GameApplication();

	void _onStart() override;
	void _onStop() override;
	void _onUpdate(float _dt) override;
	virtual bool _onSerializeSettings(Serializer& _serializer) override;

	float pitch = 0.f;
	float yaw = 0.f;
	Vector3 cameraPosition = Vector3::ZERO();
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = Matrix4::IDENTITY();
	Matrix4 mesh2Transform = Matrix4::IDENTITY();
	Matrix4 fontTransform = Matrix4::IDENTITY();

	MeshFile* mesh = nullptr;
	MeshFile* ladybugMesh = nullptr;
	MeshFile* pyramidMesh = nullptr;
	TextureFile* texture = nullptr;
	TextureFile* ladybugTexture = nullptr;
	FontFile* font = nullptr;
	ShaderProgram* meshShader = nullptr;

	ID<Scene> scene;
	ID<Entity> entity1;
	ID<Entity> entity2;
};

} // namespace yae
