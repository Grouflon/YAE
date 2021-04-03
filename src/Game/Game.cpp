#include "Game.h"

#include <stdio.h>
#include <vector>

#include <context.h>
#include <hash.h>
#include <serialization.h>
#include <log.h>

#include <resources/FileResource.h>


MIRROR_CLASS_DEFINITION(ConfigData);

using namespace yae;

bool dummyFunction(float _a, const char* _b)
{
	return true;
}

void onLibraryLoaded()
{
	//
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
	}

	/*auto functionPtr = &dummyFunction;
	auto functionType = mirror::GetTypeDesc(functionPtr);*/
	int a = 0;
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
