#include "TestSystem.h"

#include <core/logger.h>

#include <yae/test/serialization_test.h>
#include <yae/test/math_test.h>
#include <yae/test/random_test.h>

namespace yae {

void TestSystem::_registerAllTests()
{
	pushCategory("serialization");
        addTest("JsonSerializer", &test::testJsonSerializer);
        addTest("BinarySerializer", &test::testBinarySerializer);
    popCategory();

    pushCategory("math");
        addTest("vectors", &test::testVectors);
        addTest("quaternion", &test::testQuaternion);
    popCategory();

    addTest("random", &test::testRandom);
}

TestSystem::TestSystem()
	: m_categoryStack(&toolAllocator())
	, m_tests(&toolAllocator())
	, m_categories(&toolAllocator())
{
}

TestSystem::~TestSystem()
{
	YAE_ASSERT(m_categoryStack.size() == 0);
}

void TestSystem::init()
{
	pushCategory("root");
	_registerAllTests();
}

void TestSystem::shutdown()
{
	YAE_ASSERT(m_categoryStack.size() == 1);
	popCategory();
}

void TestSystem::pushCategory(const char* _name)
{
	YAE_ASSERT(strlen(_name) < 128);
	StringHash nameHash = StringHash(_name);

	TestCategory* categoryPtr = m_categories.get(nameHash);
	if (categoryPtr != nullptr)
	{
		YAE_ASSERT(strcmp(categoryPtr->name, _name) == 0);
	}
	else
	{
		TestCategory category;
		strcpy(category.name, _name);
		categoryPtr = &m_categories.set(nameHash, category);
	}

	if (m_categoryStack.size() > 0)
	{
		TestCategory* parentCategory = m_categories.get(m_categoryStack.back());
		YAE_ASSERT(parentCategory != nullptr);

		if (parentCategory->childCategories.find(nameHash) == nullptr)
		{
			parentCategory->childCategories.push_back(nameHash);
		}
	}
	
	m_categoryStack.push_back(nameHash);
}

void TestSystem::popCategory()
{
	YAE_ASSERT(m_categoryStack.size() > 0);

	m_categoryStack.pop_back();
}

void TestSystem::addTest(const char* _name, void(*_testFunctionPtr)())
{
	YAE_ASSERT(m_categoryStack.size() > 0);
	YAE_ASSERT(strlen(_name) < 128);

	String fullName(&scratchAllocator());
	for (u32 i = 1; i < m_categoryStack.size(); ++i)
	{
		fullName += m_categories.get(m_categoryStack[i])->name;
		fullName += "::";
	}
	fullName += _name;

	Test test;
	strcpy(test.name, _name);
	test.fullName = fullName;
	test.testFunctionPtr = _testFunctionPtr;

	m_tests.push_back(test);

	TestCategory* currentCategory = m_categories.get(m_categoryStack.back());
	YAE_ASSERT(currentCategory != nullptr);
	currentCategory->tests.push_back(m_tests.size());
}

void TestSystem::runAllTests()
{
	YAE_CAPTURE_FUNCTION();
	YAE_VERBOSE_CAT("test", "Running all tests...");

	for (const Test& test : m_tests)
	{
		_runTest(test);
	}

	YAE_VERBOSE_CAT("test", "All tests Done.");
}

void TestSystem::runAllTestsInCategory(char* _name)
{
	YAE_VERBOSEF_CAT("test", "Running all tests in category \"%s\"...", _name);

	StringHash nameHash = StringHash(_name);
	const TestCategory* categoryPtr = m_categories.get(nameHash);
	if (!categoryPtr)
	{
		YAE_ERRORF_CAT("test", "Unknown category \"%s\"", _name);
		return;
	}
	_runAllTestsInCategory(*categoryPtr);

	YAE_VERBOSEF_CAT("test", "All \"%s\" tests Done.", _name);
}

void TestSystem::runTest(u32 _testId)
{
	YAE_ASSERT(_testId < m_tests.size());
	_runTest(m_tests[_testId]);
}

void TestSystem::_runTest(const Test& _test)
{
	try
	{
		_test.testFunctionPtr();
		logger().setDefaultOutputColor(OutputColor_Green);
		YAE_LOGF_CAT("test", "%s: SUCCESS", _test.fullName.c_str());
		logger().setDefaultOutputColor(OutputColor_Default);
	}
	catch(const char* _e)
	{
		YAE_ERRORF_CAT("test", "%s: FAILED (%s)", _test.fullName.c_str(), _e != nullptr ? _e : "");
	}
}

void TestSystem::_runAllTestsInCategory(const TestCategory& _category)
{
	for (StringHash childCategoryId : _category.childCategories)
	{
		const TestCategory* childCategoryPtr = m_categories.get(childCategoryId);
		YAE_ASSERT(childCategoryPtr != nullptr);
		_runAllTestsInCategory(*childCategoryPtr);
	}

	for (u32 testId : _category.tests)
	{
		runTest(testId);
	}
}

} // namespace yae
