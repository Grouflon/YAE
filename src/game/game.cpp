#include "game.h"

#include <yae/hash.h>
#include <yae/serialization.h>
#include <yae/resources/FileResource.h>
#include <yae/resources/MeshResource.h>
#include <yae/resources/TextureResource.h>
#include <yae/resources/FontResource.h>
#include <yae/math_types.h>
#include <yae/input.h>
#include <yae/application.h>
#include <yae/math.h>
#include <yae/Renderer.h>

#include <im3d.h>
#include <imgui.h>

#include <stdio.h>
#include <vector>
#include <glm/gtx/quaternion.hpp>
MIRROR_CLASS_DEFINITION(ConfigData);

using namespace yae;


void onLibraryLoaded()
{
	/*
	printf("\x1b[31mBA.\r\n");
	YAE_LOG("Bonjour");
	{
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

class GameInstance
{
public:
	float pitch = 0.f;
	float yaw = 0.f;
	bool fpsModeEnabled = false;
	Matrix4 mesh1Transform = Matrix4::IDENTITY;
	Matrix4 mesh2Transform = Matrix4::IDENTITY;

	MeshResource* mesh = nullptr;
	TextureResource* texture = nullptr;
	FontResource* font = nullptr;
};

void onLibraryUnloaded()
{
}

void initGame()
{
	GameInstance* gameInstance = defaultAllocator().create<GameInstance>();
	YAE_ASSERT(gameInstance != nullptr);
	app().setUserData(gameInstance);

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

	gameInstance->font = findOrCreateResource<FontResource>("data/fonts/Roboto-Regular.ttf", 32);
	gameInstance->font->useLoad();
	YAE_ASSERT(gameInstance->font->isLoaded());
}

void shutdownGame()
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData();

	gameInstance->mesh->releaseUnuse();
	gameInstance->texture->releaseUnuse();
	gameInstance->font->releaseUnuse();

	defaultAllocator().destroy(gameInstance);
	app().setUserData(nullptr);
}

void updateGame(float _dt)
{
	GameInstance* gameInstance = (GameInstance*)app().getUserData();

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

    ImGui::SetNextWindowSize(ImVec2(600, 512));
    ImGui::Begin("Image");
	ImGui::Image((void*)(intptr_t)gameInstance->font->m_fontTexture, ImVec2(512, 512));
	ImGui::End();

	/*Im3d::EnableSorting(true);
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
	*/

	/*
	glm::vec3 color(1.f, 1.f, 1.f);
	float scale = 1.f;
	Vertex vertices[] =
	{
		Vertex(glm::vec3(-scale, -scale, 0.f), color, glm::vec2(0.f, 0.f)),
		Vertex(glm::vec3(scale, -scale, 0.f), color, glm::vec2(1.f, 0.f)),
		Vertex(glm::vec3(scale, scale, 0.f), color, glm::vec2(1.f, 1.f)),
		Vertex(glm::vec3(-scale, scale, 0.f), color, glm::vec2(0.f, 0.f)),
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

	renderer().drawMesh(
		gameInstance->mesh1Transform,
		gameInstance->mesh->m_vertices.data(), gameInstance->mesh->m_vertices.size(), 
		gameInstance->mesh->m_indices.data(), gameInstance->mesh->m_indices.size(), 
		gameInstance->texture->getTextureHandle()
	);

	renderer().drawText(gameInstance->mesh2Transform, gameInstance->font, "Hello World!");
	//renderer().drawMesh(gameInstance->mesh2Transform, gameInstance->mesh->getMeshHandle());

}
