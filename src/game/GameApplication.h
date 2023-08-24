#pragma once

#include <yae/Application.h>
#include <game/transform.h>

namespace yae {

class MeshFile;
class TextureFile;
class FontFile;
class ShaderProgram;

class GAME_API GameApplication : public Application
{
public:
	GameApplication();
	virtual ~GameApplication();

	void _onStart() override;
	void _onStop() override;
	void _onUpdate(float _dt) override;
	virtual bool _onSerialize(Serializer* _serializer) override;

	float pitch = 0.f;
	float yaw = 0.f;
	Vector3 cameraPosition = Vector3::ZERO();
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = Matrix4::IDENTITY();
	Matrix4 mesh2Transform = Matrix4::IDENTITY();
	Matrix4 fontTransform = Matrix4::IDENTITY();

	MeshFile* mesh = nullptr;
	MeshFile* ladybugMesh = nullptr;
	TextureFile* texture = nullptr;
	TextureFile* ladybugTexture = nullptr;
	FontFile* font = nullptr;
	ShaderProgram* meshShader = nullptr;

	NodeID node1;
	NodeID node2;
	NodeID node3;
	NodeID node4;
};

} // namespace yae
