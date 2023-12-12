#include "GameApplication.h"

#include <core/containers/Array.h>
#include <core/filesystem.h>
#include <core/Module.h>
#include <core/Program.h>
#include <core/serialization/BinarySerializer.h>
#include <core/serialization/JsonSerializer.h>
#include <core/serialization/serialization.h>
#include <core/string.h>

#include <yae/Application.h>
#include <yae/im3d_extension.h>
#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>
#include <yae/math_3d.h>
#include <yae/math_types.h>
#include <yae/rendering/Renderer.h>
#include <yae/resource.h>
#include <yae/ResourceManager.h>
#include <yae/resources/File.h>
#include <yae/resources/FontFile.h>
#include <yae/resources/MeshFile.h>
#include <yae/resources/ShaderFile.h>
#include <yae/resources/ShaderProgram.h>
#include <yae/resources/TextureFile.h>
#include <yae/SceneSystem.h>

#include <game/transform.h>

#include <im3d.h>
#include <imgui.h>
#include <mirror/mirror.h>

#include <stdio.h>

namespace yae {

static void drawNode(NodeID _nodeID)
{
	SpatialNode* node = _nodeID.get();
	Vector3 p = node->getWorldPosition();

	Matrix3 r = Matrix3::FromRotation(node->getWorldRotation());
	Vector3 s = node->getWorldScale();
	if (Im3d::Gizmo(Im3d::MakeId((void*)(uintptr_t)_nodeID.id), math::data(p), math::data(r), math::data(s)))
	{
		Transform t(p, Quaternion::FromMatrix3(r), s);
		node->setWorldTransform(t);
	}
	Im3d::Text(p, 0, "Node %lld", _nodeID.id);

	/*
	Matrix4 m = node->getWorldMatrix();
	ImGui::Text("Node %lld:", _nodeID.id);
	imgui_matrix4(m.data());

	m = m.inverse();
	ImGui::Text("Inv %lld:", _nodeID.id);
	imgui_matrix4(m.data());
	*/

	//Im3d::PushMatrix(node->getLocalMatrix());
	for (NodeID child : node->getChildren())
	{
		drawNode(child);
	}
	//Im3d::PopMatrix();
}

GameApplication::GameApplication()
	: Application("Game", 800, 600)
{

}

GameApplication::~GameApplication()
{

}

void GameApplication::_onStart()
{
	//app().setCameraPosition(Vector3(0.f, 0.f, 3.f));

	//mesh1Transform = Matrix4::FromRotation(Quaternion::FromEuler(PI*.5f, PI*-.25f, 0.f));
	mesh1Transform[3][0] = 0.f;
	mesh1Transform[3][1] = .5f;
	mesh1Transform[3][2] = -1.5f;

	mesh2Transform[3][0] = 1.f;
	mesh2Transform[3][1] = -1.f;

	mesh = resource::findOrCreateFile<MeshFile>("./data/models/viking_room.obj");
	mesh->load();
	YAE_ASSERT(mesh->isLoaded());

	ladybugMesh = resource::findOrCreateFile<MeshFile>("./data/models/ladybug.obj");
	ladybugMesh->load();
	YAE_ASSERT(ladybugMesh->isLoaded());

	pyramidMesh = resource::findOrCreateFile<MeshFile>("./data/models/pyramid.obj");
	pyramidMesh->load();
	// YAE_ASSERT(pyramidMesh->isLoaded());

	texture = resource::findOrCreateFile<TextureFile>("./data/textures/viking_room.png");
	texture->load();
	YAE_ASSERT(texture->isLoaded());

	ladybugTexture = resource::findOrCreateFile<TextureFile>("./data/textures/ladybug_palette.png");
	ladybugTexture->setFilter(TextureFilter::NEAREST);
	ladybugTexture->load();
	YAE_ASSERT(ladybugTexture->isLoaded());

	font = resource::findOrCreateFile<FontFile>("data/fonts/Roboto-Regular.ttf");
	font->setSize(64);
	font->load();
	YAE_ASSERT(font->isLoaded());

	Shader* shaders[] =
	{
		resource::findOrCreateFile<ShaderFile>("./data/shaders/mesh.vert"),
		resource::findOrCreateFile<ShaderFile>("./data/shaders/mesh.frag")
	};
	// NOTE: Several resources can't initialize the same shaders, this is bad. how not to do that ?
	if (!shaders[0]->isLoaded()) shaders[0]->setShaderType(ShaderType::VERTEX);
	if (!shaders[1]->isLoaded()) shaders[1]->setShaderType(ShaderType::FRAGMENT);

	meshShader = resource::findOrCreate<ShaderProgram>("meshShader");
	meshShader->setShaderStages(shaders, countof(shaders));
	meshShader->load();
	YAE_ASSERT(meshShader->isLoaded());

	scene = sceneSystem().createScene("gameScene");
	entity1 = sceneSystem().createEntity("entity1", scene);
	entity2 = sceneSystem().createEntity("entity2", scene);
	YAE_ASSERT(entity1.get());
	YAE_ASSERT(entity2.get());
	entity2->transform().setParent(entity1->transform());
	entity2->transform().setLocalPosition(Vector3::FORWARD() * 10.f);

	RenderCamera* gameCamera = renderer().createCamera("game");
	gameCamera->fov = 45.f;
	gameCamera->nearPlane = .1f;
	gameCamera->farPlane = 100.f;
	gameCamera->position = cameraPosition;
	gameCamera->rotation = Quaternion::FromEuler(D2R * pitch, D2R * yaw, 0.f);
	gameCamera->clearColor = Vector4(.5f, .5f, .5f, 1.f);

	RenderScene* defaultScene = renderer().createScene("game");
	defaultScene->addCamera(gameCamera);
}

void GameApplication::_onStop()
{
	sceneSystem().destroyEntity(entity2);
	sceneSystem().destroyEntity(entity1);
	sceneSystem().destroyScene(scene);

	meshShader->unload();
	texture->unload();
	ladybugTexture->unload();
	font->unload();
	mesh->unload();
	ladybugMesh->unload();
	pyramidMesh->unload();
}

void GameApplication::_onUpdate(float _dt)
{
	RenderCamera* gameCamera = renderer().getCamera("game");
	YAE_ASSERT(gameCamera);

	renderer().pushScene("game");

	// EXIT PROGRAM
	if (input().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		app().requestStop();
	}

	// MOVE CAMERA
	bool currentFpsModeEnabled = input().isMouseButtonDown(MOUSEBUTTON_RIGHT) && !ImGui::GetIO().WantCaptureMouse;
	if (fpsModeEnabled != currentFpsModeEnabled)
	{
		fpsModeEnabled = currentFpsModeEnabled;
		if (fpsModeEnabled)
		{
			input().setCursorMode(CursorMode::LOCKED);
		}
		else
		{
			input().setCursorMode(CursorMode::NORMAL);
		}
	}

	if (fpsModeEnabled)
	{
		// ROTATION
		float rotationSpeed = .2f;
		Vector2 rotationInputRate = -input().getMouseDelta();
		YAE_ASSERT(math::isFinite(rotationInputRate));
		yaw = math::mod(yaw + rotationInputRate.x * rotationSpeed * -1.f, 360.f);
		pitch = math::clamp(pitch + rotationInputRate.y * rotationSpeed * -1.f, -90.f, 90.f);
		gameCamera->rotation = Quaternion::FromEuler(D2R * pitch, D2R * yaw, 0.f);
		YAE_ASSERT(math::isFinite(yaw));
		YAE_ASSERT(math::isFinite(pitch));
		YAE_ASSERT(math::isFinite(gameCamera->rotation));

		// TRANSLATION
		Vector3 forward = math::forward(gameCamera->rotation);
		Vector3 right = math::right(gameCamera->rotation);
		YAE_ASSERT(math::isFinite(forward));
		YAE_ASSERT(math::isFinite(right));
		//Vector3 up = math::up(gameCamera->rotation);

		Vector3 inputRate = Vector3::ZERO();

		if (input().isKeyDown(SDL_SCANCODE_D))
		{
			inputRate += right;
		}
		if (input().isKeyDown(SDL_SCANCODE_A))
		{
			inputRate += -right;
		}
		if (input().isKeyDown(SDL_SCANCODE_W))
		{
			inputRate += forward;
		}
		if (input().isKeyDown(SDL_SCANCODE_S))
		{
			inputRate += -forward;
		}

		float linearSpeed = 2.f;
		inputRate = math::safeNormalize(inputRate);
		gameCamera->position += inputRate * linearSpeed * _dt;
		cameraPosition = gameCamera->position;

		if (!math::isZero(inputRate) || !math::isZero(rotationInputRate))
		{
			program().saveSettings();
		}
	}

	ImGui::Text("camera:");
	if (ImGui::DragVector("position", &gameCamera->position, .05f))
	{
		cameraPosition = gameCamera->position;
	}
	if (ImGui::DragRotation("rotation", &gameCamera->rotation))
	{
		Vector3 euler = math::euler(gameCamera->rotation);
		pitch = euler.x * R2D;
		yaw = euler.y * R2D;
	}

	/*
    ImGui::SetNextWindowSize(ImVec2(600, 512));
    ImGui::Begin("Image");
	ImGui::Image((void*)(intptr_t)font->m_fontTexture, ImVec2(512, 512));
	ImGui::End();
	*/

	Im3d::EnableSorting(true);

	// Axis
	Im3d::SetSize(5.f);
	Im3d::DrawRotation(Quaternion::IDENTITY(), 1.f);

    /*
    Im3d::SetColor(Im3d::Color(1.f, 0.f, 0.f));
    Im3d::SetSize(10.f);

    Im3d::BeginPoints();
    	Im3d::Vertex(-1.f, 0.f, 0.f);
    	Im3d::Vertex(-2.f, 0.f, 0.f);
    	Im3d::Vertex(1.f, 1.f, 1.f);
    	Im3d::Vertex(-1.f, 0.f, 0.f);
    Im3d::End();

    Im3d::SetSize(2.f);
    Im3d::BeginLines();
    	Im3d::Vertex(-1.f, 0.f, 0.f);
    	Im3d::Vertex(-2.f, 0.f, 0.f);
    	Im3d::Vertex(1.f, 1.f, 1.f);
    	Im3d::Vertex(-1.f, 0.f, 0.f);
    Im3d::End();

    Im3d::SetColor(Im3d::Color(1.f, 0.f, 1.f));
    Im3d::BeginTriangles();
    	Im3d::Vertex(1.f, 0.5f, 0.5f);
    	Im3d::Vertex(1.f, 1.f, 0.5f);
    	Im3d::Vertex(0.5f, 0.5f, 0.5f);
    Im3d::End();
	*/

    static int gridSize = 20;
	//ImGui::SliderInt("Grid Size", &gridSize, 1, 50);
	const float gridHalf = (float)gridSize * 0.5f;
	Im3d::SetAlpha(1.0f);
	Im3d::SetSize(2.0f);
	Im3d::BeginLines();
		for (int x = 0; x <= gridSize; ++x)
		{
			Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
			Im3d::Vertex( gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(1.0f, 0.0f, 0.0f));
		}
		for (int z = 0; z <= gridSize; ++z)
		{
			Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
			Im3d::Vertex((float)z - gridHalf, 0.0f,  gridHalf,  Im3d::Color(0.0f, 0.0f, 1.0f));
		}
	Im3d::End();


	Im3d::PushLayerId(IM3D_DRAWONTOP_LAYER);
	Matrix4 m1 = entity1->transform().getLocalMatrix();
	ImGui::DragMatrix("entity1", &m1);
	if (Im3d::Gizmo("entity1", (float*)&m1)) {}
	entity1->transform().setLocalMatrix(m1);
	renderer().drawMesh(
	 	m1,
	 	pyramidMesh,
	 	meshShader,
	 	ladybugTexture
	);

	// Matrix4 m2 = entity2->transform().getLocalMatrix();
	// ImGui::DragMatrix("entity2", &m2);
	// entity2->transform().setLocalMatrix(m2);
	// renderer().drawMesh(
	// 	entity2->transform().getWorldMatrix(),
	// 	ladybugMesh,
	// 	meshShader,
	// 	ladybugTexture
	// );
	Im3d::PopLayerId();

	// Im3d::PushLayerId(IM3D_DRAWONTOP_LAYER);
	// if (Im3d::Gizmo("mesh1", (float*)&mesh1Transform)) {}
	// renderer().drawMesh(
	// 	mesh1Transform,
	// 	ladybugMesh,
	// 	meshShader,
	// 	ladybugTexture
	// );
	// Im3d::PopLayerId();

	float fontScale = 0.005f;
	fontTransform = Matrix4::IDENTITY();
	fontTransform = math::scale(fontTransform, Vector3::ONE() * fontScale);
	fontTransform = math::translate(fontTransform, Vector3(210.f, 20.f, 0.f));
	renderer().drawText(fontTransform, font, "Hello World!");

	renderer().popScene();

	//const char* resourcePath = "./data/textures/ladybug_palette.res";
}

bool GameApplication::_onSerializeSettings(Serializer& _serializer)
{
	bool result = serialization::serializeMirrorType(_serializer, *this, "game");

	RenderCamera* gameCamera = renderer().getCamera("game");
	if (gameCamera)
	{
		gameCamera->position = cameraPosition;
		gameCamera->rotation = Quaternion::FromEuler(D2R * pitch, D2R * yaw, 0.f);
	}

	return result;
}

} // namespace yae

MIRROR_CLASS(yae::GameApplication)
(
	MIRROR_MEMBER(pitch);
	MIRROR_MEMBER(yaw);
	MIRROR_MEMBER(cameraPosition);
);