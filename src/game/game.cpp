#include "game.h"

#include <yae/Module.h>
#include <yae/containers/Array.h>
#include <yae/filesystem.h>
#include <yae/program.h>
#include <yae/hash.h>
#include <yae/resources/FileResource.h>
#include <yae/resources/MeshResource.h>
#include <yae/resources/TextureResource.h>
#include <yae/resources/FontResource.h>
#include <yae/math_types.h>
#include <yae/input.h>
#include <yae/math_3d.h>
#include <yae/rendering/Renderer.h>
#include <yae/serialization/BinarySerializer.h>
#include <yae/serialization/JsonSerializer.h>
#include <yae/Application.h>

#include <game/transform.h>

#include <im3d.h>
#include <imgui.h>
#include <mirror/mirror.h>

#include <stdio.h>
#include <vector>

using namespace yae;

static void imgui_matrix4(float _matrix[16])
{
	ImGui::Text("%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f\n%.4f %.4f %.4f %.4f",
	_matrix[0], _matrix[1], _matrix[2], _matrix[3],
	_matrix[4], _matrix[5], _matrix[6], _matrix[7],
	_matrix[8], _matrix[9], _matrix[10], _matrix[11],
	_matrix[12], _matrix[13], _matrix[14], _matrix[15]
	);
}

static void drawNode(NodeID _nodeID)
{
	SpatialNode* node = _nodeID.get();
	Vector3 p = node->getWorldPosition();

	Matrix3 r = matrix3::makeRotation(node->getWorldRotation());
	Vector3 s = node->getWorldScale();
	if (Im3d::Gizmo(Im3d::MakeId((void*)(uintptr_t)_nodeID.id), p.data(), r.data(), s.data()))
	{
		Transform t(p, quaternion::makeFromMatrix3(r), s);
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

class GameInstance
{
public:
	float pitch = 0.f;
	float yaw = 0.f;
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = matrix4::IDENTITY;
	Matrix4 mesh2Transform = matrix4::IDENTITY;
	Matrix4 fontTransform = matrix4::IDENTITY;

	MeshResource* mesh = nullptr;
	TextureResource* texture = nullptr;
	FontResource* font = nullptr;

	NodeID node1;
	NodeID node2;
	NodeID node3;
	NodeID node4;
};

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{
}

void onModuleUnloaded(yae::Program* _program, yae::Module* _module)
{
}

void initModule(yae::Program* _program, yae::Module* _module)
{
}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{
}

void afterInitApplication(Application* _app)
{
	GameInstance* gameInstance = defaultAllocator().create<GameInstance>();
	YAE_ASSERT(gameInstance != nullptr);
	app().setUserData("game", gameInstance);

	Vector3 cameraAngles = R2D * quaternion::euler(app().getCameraRotation());
	gameInstance->pitch = cameraAngles.x;
	gameInstance->yaw = cameraAngles.y;

	//app().setCameraPosition(Vector3(0.f, 0.f, 3.f));

	gameInstance->mesh1Transform = matrix4::makeRotation(quaternion::makeFromEuler(PI*.5f, PI*-.25f, 0.f));
	gameInstance->mesh1Transform[3][0] = 0.f;
	gameInstance->mesh1Transform[3][1] = .5f;

	gameInstance->mesh2Transform[3][0] = 1.f;
	gameInstance->mesh2Transform[3][1] = -1.f;

	gameInstance->mesh = findOrCreateResource<MeshResource>("./data/models/viking_room.obj");
	gameInstance->mesh->useLoad();
	YAE_ASSERT(gameInstance->mesh->isLoaded());

	gameInstance->texture = findOrCreateResource<TextureResource>("./data/textures/viking_room.png");
	gameInstance->texture->useLoad();
	YAE_ASSERT(gameInstance->texture->isLoaded());

	gameInstance->font = findOrCreateResource<FontResource>("data/fonts/Roboto-Regular.ttf", 64);
	gameInstance->font->useLoad();
	YAE_ASSERT(gameInstance->font->isLoaded());

	SpatialSystem* spatialSystem = defaultAllocator().create<SpatialSystem>();
	YAE_ASSERT(gameInstance != nullptr);
	app().setUserData("spatialSystem", spatialSystem);

	gameInstance->node1 = spatialSystem->createNode();
	gameInstance->node2 = spatialSystem->createNode();

	gameInstance->node2->setLocalPosition(Vector3(0.f, 0.f, 1.f));

	gameInstance->node2->setParent(gameInstance->node1);
}

void beforeShutdownApplication(Application* _app)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");

	gameInstance->mesh->releaseUnuse();
	gameInstance->texture->releaseUnuse();
	gameInstance->font->releaseUnuse();

	spatialSystem().destroyNode(gameInstance->node2);
	spatialSystem().destroyNode(gameInstance->node1);

	defaultAllocator().destroy(gameInstance);
	app().setUserData("game", nullptr);

	SpatialSystem* spatialSystemPtr = (SpatialSystem*)(app().getUserData("spatialSystem"));
	defaultAllocator().destroy(spatialSystemPtr);
	app().setUserData("spatialSystem", nullptr);
}

void updateApplication(Application* _app, float _dt)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");

	// EXIT PROGRAM
	if (input().isKeyDown(GLFW_KEY_ESCAPE))
	{
		app().requestExit();
	}

	// RELOAD SHADERS
	/*if (input().isCtrlDown() && input().wasKeyJustPressed(GLFW_KEY_Q))
	{
		renderer().reloadIm3dShaders();
	}*/

	// MOVE CAMERA
	bool fpsModeEnabled = input().isMouseButtonDown(1);
	if (fpsModeEnabled != gameInstance->fpsModeEnabled)
	{
		gameInstance->fpsModeEnabled = fpsModeEnabled;
		if (fpsModeEnabled)
		{
			input().setCursorMode(CURSORMODE_DISABLED);
		}
		else
		{
			input().setCursorMode(CURSORMODE_NORMAL);
		}
	}

	if (fpsModeEnabled)
	{
		// ROTATION
		float rotationSpeed = .2f;
		Vector2 rotationInputRate = -input().getMouseDelta();
		gameInstance->yaw += mod(rotationInputRate.x * rotationSpeed, 360.f);
		gameInstance->pitch += clamp(rotationInputRate.y * rotationSpeed, -90.f, 90.f);
		Quaternion cameraRotation = quaternion::makeFromEuler(D2R * gameInstance->pitch, D2R * gameInstance->yaw, 0.f);
		app().setCameraRotation(cameraRotation);


		// TRANSLATION
		Vector3 forward = -quaternion::forward(app().getCameraRotation()); // @NOTE: forward is reversed, we need to figure out why at some point
		Vector3 right = quaternion::right(app().getCameraRotation());
		//Vector3 up = app().getCameraRotation().up();
		Vector3 inputRate = vector3::ZERO;

		if (input().isKeyDown(GLFW_KEY_D))
		{
			inputRate += right;
		}
		if (input().isKeyDown(GLFW_KEY_A))
		{
			inputRate += -right;
		}
		if (input().isKeyDown(GLFW_KEY_W))
		{
			inputRate += forward;
		}
		if (input().isKeyDown(GLFW_KEY_S))
		{
			inputRate += -forward;
		}

		float linearSpeed = 2.f;
		inputRate = vector3::safeNormalize(inputRate);
		Vector3 cameraPosition = app().getCameraPosition();
		cameraPosition += inputRate * linearSpeed * _dt;
		app().setCameraPosition(cameraPosition);
	}

	/*
    ImGui::SetNextWindowSize(ImVec2(600, 512));
    ImGui::Begin("Image");
	ImGui::Image((void*)(intptr_t)gameInstance->font->m_fontTexture, ImVec2(512, 512));
	ImGui::End();
	*/

	Im3d::EnableSorting(true);

	// Axis
    Im3d::SetSize(5.f);
    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Red);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(1.f, 0.f, 0.f);
    Im3d::End();
    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Green);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(0.f, 1.f, 0.f);
    Im3d::End();
    Im3d::BeginLines();
    	Im3d::SetColor(Im3d::Color_Blue);
    	Im3d::Vertex(0.f, 0.f, 0.f);
    	Im3d::Vertex(0.f, 0.f, 1.f);
    Im3d::End();

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

	/*
	glm::vec3 color(1.f, 1.f, 1.f);
	float scale = 1.f;
	Vertex vertices[] =
	{
		Vertex(glm::vec3(-scale, -scale, 0.f), color, glm::vec2(0.f, 0.f)),
		Vertex(glm::vec3(scale, -scale, 0.f), color, glm::vec2(1.f, 0.f)),
		Vertex(glm::vec3(scale, scale, 0.f), color, glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(-scale, scale, 0.f), color, glm::vec2(0.f, 1.f)),
	};
	u32 indices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	renderer().drawMesh(
		gameInstance->mesh1Transform,
		vertices, countof(vertices),
		indices, countof(indices),
		gameInstance->texture->getTextureHandle()
	);

	renderer().drawMesh(
		gameInstance->mesh2Transform,
		vertices, countof(vertices),
		indices, countof(indices),
		gameInstance->texture->getTextureHandle()
	);
	*/

	ImGui::Text("mesh1:");
	imgui_matrix4((float*)&gameInstance->mesh1Transform);
	if (Im3d::Gizmo("mesh1", (float*)&gameInstance->mesh1Transform)) {}
	renderer().drawMesh(
		gameInstance->mesh1Transform,
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(), 
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(), 
		gameInstance->texture->getTextureHandle()
	);

	/*
	if (Im3d::Gizmo("mesh2", (float*)&gameInstance->mesh2Transform)) {}
	renderer().drawMesh(
		gameInstance->mesh2Transform,
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(),
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(),
		gameInstance->texture->getTextureHandle()
	);
	*/

	float fontScale = 0.005f;
	gameInstance->fontTransform = matrix4::IDENTITY;
	gameInstance->fontTransform = matrix4::scale(gameInstance->fontTransform, vector3::ONE * fontScale);
	gameInstance->fontTransform = matrix4::translate(gameInstance->fontTransform, Vector3(210.f, 20.f, 0.f));
	renderer().drawText(gameInstance->fontTransform, gameInstance->font, "Hello World!");
	//renderer().drawMesh(gameInstance->mesh2Transform, gameInstance->mesh->getMeshHandle());

	ImGui::Text("node %lld:", gameInstance->node2.id);
	imgui_matrix4(gameInstance->node2->getWorldMatrix().data());

	renderer().drawMesh(
		gameInstance->node2->getWorldMatrix(),
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(),
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(),
		gameInstance->texture->getTextureHandle()
	);

	for (NodeID node : spatialSystem().getRoots())
	{
		drawNode(node);
	}
}
