#include <yae/memory.h>
#include <yae/program.h>
#include <yae/application.h>
#include <yae/application.h>
#include <yae/logger.h>

#include <mirror.h>
#include <stdio.h>
#include <cstdlib>

#include <yae/containers/Array.h>
#include <yae/serialization/Serializer.h>

#define APPLICATION_NAME "yae | " YAE_CONFIG

int main(int _argc, char** _argv)
{
    mirror::GetTypeSet().resolveTypes();

    yae::FixedSizeAllocator allocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator scratchAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::FixedSizeAllocator toolAllocator = yae::FixedSizeAllocator(1024*1024*32);
    yae::setAllocators(&allocator, &scratchAllocator, &toolAllocator);

    {
        yae::Program program;
        program.logger().setCategoryVerbosity("OpenGL", yae::LogVerbosity_Verbose);
        //program.logger().setCategoryVerbosity("input", yae::LogVerbosity_Verbose);
        program.init(_argv, _argc);

        struct Sfouf
        {
            u32 a;
            float b;
            yae::Array<yae::Array<u8>> array;
            bool c;
        };
        auto doSerialize = [](yae::Serializer& _serializer, Sfouf& _sfouf)
        {
            // Si l'erreur peut venir de la data, error. Si l'erreur vient du code, assert
            /*
            YAE_VERIFY(_serializer.beginSerializeObject());
            YAE_VERIFY(_serializer.serialize(_sfouf.a, "a"));
            YAE_VERIFY(_serializer.serialize(_sfouf.b, "b"));
            u32 arraySize = _sfouf.array.size();
            YAE_VERIFY(_serializer.beginSerializeArray(arraySize, "array"));
            _sfouf.array.resize(arraySize);
            for (auto element : _sfouf.array)
            {
                YAE_VERIFY(_serializer.serialize(element));
            }
            YAE_VERIFY(_serializer.endSerializeArray());
            YAE_VERIFY(_serializer.serialize(_sfouf.c, "c"));
            YAE_VERIFY(_serializer.endSerializeObject());
            */

            YAE_VERIFY(_serializer.serialize(_sfouf.a));
            YAE_VERIFY(_serializer.serialize(_sfouf.b));

            u32 arraySize = _sfouf.array.size();
            YAE_VERIFY(_serializer.beginSerializeArray(arraySize));
            _sfouf.array.resize(arraySize);
            for (auto& array : _sfouf.array)
            {
                u32 arraySize2 = array.size();
                YAE_VERIFY(_serializer.beginSerializeArray(arraySize2));
                array.resize(arraySize2);
                for (auto& element : array)
                {
                    YAE_VERIFY(_serializer.serialize(element));
                }
                YAE_VERIFY(_serializer.endSerializeArray());
            }
            
            YAE_VERIFY(_serializer.endSerializeArray());

            YAE_VERIFY(_serializer.serialize(_sfouf.c));
        };

        yae::BinarySerializer serializer(&allocator);

        Sfouf sfoufWrite;
        sfoufWrite.a = 2;
        sfoufWrite.b = 2.f;
        sfoufWrite.array.resize(3);
        sfoufWrite.array[0].resize(3);
        sfoufWrite.array[1].resize(4);
        sfoufWrite.array[2].resize(5);
        sfoufWrite.array[2][0] = 5;
        sfoufWrite.array[2][1] = 4;
        sfoufWrite.array[2][2] = 3;
        sfoufWrite.array[2][3] = 2;
        sfoufWrite.array[2][4] = 1;
        sfoufWrite.c = true;
        serializer.beginWrite();
        doSerialize(serializer, sfoufWrite);

        size_t dataSize = serializer.getDataSize();
        void* data = allocator.allocate(dataSize);
        memcpy(data, serializer.getData(), dataSize);

        serializer.endWrite();

        Sfouf sfoufRead;
        serializer.beginRead(data, dataSize);
        doSerialize(serializer, sfoufRead);
        serializer.endRead();

        allocator.deallocate(data);

        program.shutdown();
        return EXIT_SUCCESS;


        yae::Application app = yae::Application(APPLICATION_NAME, 800u, 600u);
        program.registerApplication(&app);

        program.run();

        program.unregisterApplication(&app);

        program.shutdown();
    }

    return EXIT_SUCCESS;
}
