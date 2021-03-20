#include "Game.h"

#include <stdio.h>

#include <00-Type/GlobalContext.h>
#include <01-Hash/HashTools.h>
#include <01-Serialization/JsonSerializer.h>
#include <03-Resource/ResourceManager.h>
#include <03-Resource/FileResource.h>


using namespace yae;

void OnLibraryLoaded()
{
	//
	{
		FileResource* configFile = FindOrCreateResource<FileResource>("./config.json");
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
}

void OnLibraryUnloaded()
{
}

void InitGame()
{
}

void UpdateGame()
{
	//printf("Hello again World!\n");
}

void ShutdownGame()
{

}
