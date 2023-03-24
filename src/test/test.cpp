#include "test.h"

#include <yae/memory.h>
#include <yae/Module.h>

#include <test/TestSystem.h>
#include <test/serialization_test.h>
#include <test/math_test.h>
#include <test/random_test.h>

using namespace yae;

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{
    TestSystem* testSystem = toolAllocator().create<TestSystem>();
    _module->userData = testSystem;

    testSystem->pushCategory("serialization");
        testSystem->addTest("JsonSerializer", &serialization_test::testJsonSerializer);
        testSystem->addTest("BinarySerializer", &serialization_test::testBinarySerializer);
    testSystem->popCategory();

    testSystem->pushCategory("math");
        testSystem->addTest("vectors", &::math_test::testVectors);
        testSystem->addTest("quaternion", &::math_test::testQuaternion);
    testSystem->popCategory();

    testSystem->addTest("random", &random_test::testRandom);

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
