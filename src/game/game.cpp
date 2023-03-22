#include "game.h"

#include <yae/Module.h>
#include <yae/containers/Array.h>
#include <yae/filesystem.h>
#include <yae/program.h>
#include <yae/resources/FileResource.h>
#include <yae/resources/TextureResource.h>
#include <yae/resources/FontResource.h>
#include <yae/math_types.h>
#include <yae/input.h>
#include <yae/math_3d.h>
#include <yae/rendering/Renderer.h>
#include <yae/serialization/BinarySerializer.h>
#include <yae/serialization/JsonSerializer.h>
#include <yae/Application.h>
#include <yae/imgui_extension.h>
#include <yae/Mesh.h>
#include <yae/ResourceManager.h>
#include <yae/serialization/serialization.h>
#include <yae/im3d_extension.h>

#include <game/transform.h>

#include <im3d.h>
#include <imgui.h>
#include <mirror/mirror.h>

#include <stdio.h>
#include <vector>

using namespace yae;

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

class GameInstance
{
public:
	float pitch = 0.f;
	float yaw = 0.f;
	float cameraPosition[3] = {};
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = Matrix4::IDENTITY;
	Matrix4 mesh2Transform = Matrix4::IDENTITY;
	Matrix4 fontTransform = Matrix4::IDENTITY;

	Mesh mesh;
	TextureResource* texture = nullptr;
	FontResource* font = nullptr;

	NodeID node1;
	NodeID node2;
	NodeID node3;
	NodeID node4;

	MIRROR_CLASS_NOVIRTUAL(GameInstance)
	(
		MIRROR_MEMBER(pitch)();
		MIRROR_MEMBER(yaw)();
		MIRROR_MEMBER(cameraPosition)();
	);
};
MIRROR_CLASS_DEFINITION(GameInstance);

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

void beforeInitApplication(Application* _app)
{
	GameInstance* gameInstance = defaultAllocator().create<GameInstance>();
	YAE_ASSERT(gameInstance != nullptr);
	app().setUserData("game", gameInstance);
}

void afterInitApplication(Application* _app)
{
	GameInstance* gameInstance = (GameInstance*)_app->getUserData("game");
	YAE_ASSERT(gameInstance != nullptr);

	//app().setCameraPosition(Vector3(0.f, 0.f, 3.f));

	gameInstance->mesh1Transform = Matrix4::FromRotation(Quaternion::FromEuler(PI*.5f, PI*-.25f, 0.f));
	gameInstance->mesh1Transform[3][0] = 0.f;
	gameInstance->mesh1Transform[3][1] = .5f;

	gameInstance->mesh2Transform[3][0] = 1.f;
	gameInstance->mesh2Transform[3][1] = -1.f;

	YAE_VERIFY(Mesh::LoadFromObjFile(gameInstance->mesh, "./data/models/viking_room.obj"));
	program().resourceManager2().registerMesh("room", &gameInstance->mesh);

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

	RenderCamera* gameCamera = renderer().createCamera("game");
	gameCamera->fov = 45.f;
	gameCamera->nearPlane = .1f;
	gameCamera->farPlane = 100.f;
	gameCamera->position = Vector3(gameInstance->cameraPosition[0], gameInstance->cameraPosition[1], gameInstance->cameraPosition[2]);
	gameCamera->rotation = Quaternion::FromEuler(D2R * gameInstance->pitch, D2R * gameInstance->yaw, 0.f);
	gameCamera->clearColor = Vector4(.5f, .5f, .5f, 1.f);

	RenderScene* defaultScene = renderer().createScene("game");
	defaultScene->addCamera(gameCamera);
}

void beforeShutdownApplication(Application* _app)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");

	program().resourceManager2().unregisterMesh("room");
	gameInstance->texture->releaseUnuse();
	gameInstance->font->releaseUnuse();

	spatialSystem().destroyNode(gameInstance->node2);
	spatialSystem().destroyNode(gameInstance->node1);

	SpatialSystem* spatialSystemPtr = (SpatialSystem*)(app().getUserData("spatialSystem"));
	defaultAllocator().destroy(spatialSystemPtr);
	app().setUserData("spatialSystem", nullptr);
}

void afterShutdownApplication(Application* _app)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");
	defaultAllocator().destroy(gameInstance);
	app().setUserData("game", nullptr);
}

void updateApplication(Application* _app, float _dt)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");
	YAE_ASSERT(gameInstance);
	RenderCamera* gameCamera = renderer().getCamera("game");
	YAE_ASSERT(gameCamera);

	renderer().pushScene("game");

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
		gameInstance->yaw = math::mod(gameInstance->yaw + rotationInputRate.x * rotationSpeed * -1.f, 360.f);
		gameInstance->pitch = math::clamp(gameInstance->pitch + rotationInputRate.y * rotationSpeed * -1.f, -90.f, 90.f);
		gameCamera->rotation = Quaternion::FromEuler(D2R * gameInstance->pitch, D2R * gameInstance->yaw, 0.f);

		// TRANSLATION
		Vector3 forward = math::forward(gameCamera->rotation);
		Vector3 right = math::right(gameCamera->rotation);
		//Vector3 up = math::up(gameCamera->rotation);

		Vector3 inputRate = Vector3::ZERO;

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
		inputRate = math::safeNormalize(inputRate);
		gameCamera->position += inputRate * linearSpeed * _dt;
		gameInstance->cameraPosition[0] = gameCamera->position.x;
		gameInstance->cameraPosition[1] = gameCamera->position.y;
		gameInstance->cameraPosition[2] = gameCamera->position.z;

		if (!math::isZero(inputRate) || !math::isZero(rotationInputRate))
		{
			app().saveSettings();
		}
	}

	ImGui::Text("camera:");
	if (ImGui::DragVector3("position", gameCamera->position, .05f))
	{
		gameInstance->cameraPosition[0] = gameCamera->position.x;
		gameInstance->cameraPosition[1] = gameCamera->position.y;
		gameInstance->cameraPosition[2] = gameCamera->position.z;
	}
	if (ImGui::DragRotation("rotation", gameCamera->rotation))
	{
		Vector3 euler = math::euler(gameCamera->rotation);
		gameInstance->pitch = euler.x * R2D;
		gameInstance->yaw = euler.y * R2D;
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
	Im3d::DrawRotation(Quaternion::IDENTITY, 1.f);

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

	Mesh* mesh = program().resourceManager2().getMesh("room");


	ImGui::Text("mesh1:");
	imgui_matrix4((float*)&gameInstance->mesh1Transform);
	if (Im3d::Gizmo("mesh1", (float*)&gameInstance->mesh1Transform)) {}
	renderer().drawMesh(
		gameInstance->mesh1Transform,
		*mesh, 
		0,
		gameInstance->texture->getTextureHandle()
	);

	if (Im3d::Gizmo("mesh2", (float*)&gameInstance->mesh2Transform)) {}
	renderer().drawMesh(
		gameInstance->mesh2Transform,
		*mesh,
		0,
		gameInstance->texture->getTextureHandle()
	);

	float fontScale = 0.005f;
	gameInstance->fontTransform = Matrix4::IDENTITY;
	gameInstance->fontTransform = math::scale(gameInstance->fontTransform, Vector3::ONE * fontScale);
	gameInstance->fontTransform = math::translate(gameInstance->fontTransform, Vector3(210.f, 20.f, 0.f));
	renderer().drawText(gameInstance->fontTransform, gameInstance->font, "Hello World!");
	//renderer().drawMesh(gameInstance->mesh2Transform, gameInstance->mesh->getMeshHandle());

	ImGui::Text("node %lld:", gameInstance->node2.id);
	imgui_matrix4(math::data(gameInstance->node2->getWorldMatrix()));

	renderer().drawMesh(
		gameInstance->node2->getWorldMatrix(),
		*mesh,
		0,
		gameInstance->texture->getTextureHandle()
	);

	for (NodeID node : spatialSystem().getRoots())
	{
		drawNode(node);
	}

	renderer().popScene();
}

bool onSerializeApplicationSettings(Application* _application, Serializer* _serializer)
{
	GameInstance* gameInstance = (GameInstance*)_application->getUserData("game");
	return serialization::serializeMirrorType(_serializer, *gameInstance, "game");
}
