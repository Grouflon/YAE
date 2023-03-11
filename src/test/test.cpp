#include "test.h"

#include <yae/memory.h>
#include <yae/Module.h>

#include <test/TestSystem.h>
#include <test/serialization.h>
#include <test/math.h>
#include <test/random.h>

using namespace yae;

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{
    TestSystem* testSystem = toolAllocator().create<TestSystem>();
    _module->userData = testSystem;

    testSystem->pushCategory("serialization");
        testSystem->addTest("JsonSerializer", &serialization::testJsonSerializer);
        testSystem->addTest("BinarySerializer", &serialization::testBinarySerializer);
    testSystem->popCategory();

    testSystem->pushCategory("math");
        testSystem->addTest("vectors", &math::testVectors);
        testSystem->addTest("quaternion", &math::testQuaternion);
    testSystem->popCategory();

    testSystem->addTest("random", &random::testRandom);

    testSystem->runAllTests();
}

void onModuleUnloaded(yae::Program* _program, yae::Module* _module)
{
    TestSystem* testSystem = (TestSystem*)_module->userData;

    toolAllocator().destroy(testSystem);
    _module->userData = nullptr;
}

void initModule(yae::Program* _program, yae::Module* _module)
{
    TestSystem* testSystem = (TestSystem*)_module->userData;
}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{

}
