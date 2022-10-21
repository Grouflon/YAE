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
#include <yae/math.h>
#include <yae/rendering/Renderer.h>
#include <yae/serialization/BinarySerializer.h>
#include <yae/serialization/JsonSerializer.h>
#include <yae/Application.h>

#include <im3d.h>
#include <imgui.h>
#include <mirror/mirror.h>

#include <stdio.h>
#include <vector>
#include <glm/gtx/quaternion.hpp>

using namespace yae;

class GameInstance
{
public:
	float pitch = 0.f;
	float yaw = 0.f;
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = Matrix4::IDENTITY;
	Matrix4 mesh2Transform = Matrix4::IDENTITY;
	Matrix4 fontTransform = Matrix4::IDENTITY;

	MeshResource* mesh = nullptr;
	TextureResource* texture = nullptr;
	FontResource* font = nullptr;
};

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{
	// Reflection
#if false
	mirror::TypeDesc* type = pointerType->getSubType();

	mirror::TypeSet& typeSet = mirror::GetTypeSet();
	std::vector<mirror::TypeDesc*> types;
	for (auto& type : typeSet.getTypes())
	{
		mirror::Class* clss = type->asClass();
		YAE_LOG(type->getName());
		if (clss)
		{
			YAE_LOGF("%d", clss->getChildren().size());
		}
		types.push_back(type);
	}
#endif
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

	app().setCameraPosition(Vector3(0.f, 0.f, 3.f));

	gameInstance->mesh1Transform = glm::toMat4(Quaternion(PI*.5f, PI*-.25f, 0.f));
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
}

void beforeShutdownApplication(Application* _app)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData("game");

	gameInstance->mesh->releaseUnuse();
	gameInstance->texture->releaseUnuse();
	gameInstance->font->releaseUnuse();

	defaultAllocator().destroy(gameInstance);
	app().setUserData("game", nullptr);
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
		gameInstance->pitch -= clamp(rotationInputRate.y * rotationSpeed, -90.f, 90.f);
		Quaternion cameraRotation = Quaternion(D2R * gameInstance->pitch, D2R * gameInstance->yaw, 0.f);
		app().setCameraRotation(cameraRotation);


		// TRANSLATION
		Vector3 forward = -app().getCameraRotation().forward(); // @NOTE: forward is reversed, we need to figure out why at some point
		Vector3 right = app().getCameraRotation().right();
		//Vector3 up = app().getCameraRotation().up();
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
		inputRate = safeNormalize(inputRate);
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

	
	if (Im3d::Gizmo("mesh1", (float*)&gameInstance->mesh1Transform)) {}
	if (Im3d::Gizmo("mesh2", (float*)&gameInstance->mesh2Transform)) {}

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

	renderer().drawMesh(
		gameInstance->mesh1Transform,
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(), 
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(), 
		gameInstance->texture->getTextureHandle()
	);

	renderer().drawMesh(
		gameInstance->mesh2Transform,
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(),
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(),
		gameInstance->texture->getTextureHandle()
	);

	float fontScale = 0.005f;
	gameInstance->fontTransform = Matrix4::IDENTITY;
	gameInstance->fontTransform = glm::scale(gameInstance->fontTransform, glm::vec3(fontScale, fontScale, fontScale));
	gameInstance->fontTransform = glm::translate(gameInstance->fontTransform, glm::vec3(210.f, 20.f, 0.f));
	renderer().drawText(gameInstance->fontTransform, gameInstance->font, "Hello World!");
	//renderer().drawMesh(gameInstance->mesh2Transform, gameInstance->mesh->getMeshHandle());

}
