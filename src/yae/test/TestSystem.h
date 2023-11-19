#pragma once

#include <yae/types.h>
#include <core/containers/Array.h>
#include <core/containers/HashMap.h>

namespace yae {

struct Test
{
	char name[128];
	void(*testFunctionPtr)();
	String fullName;
};

struct TestCategory
{
	char name[128];
	DataArray<StringHash> childCategories;
	DataArray<u32> tests;
};

class TestSystem
{
public:
	TestSystem();
	~TestSystem();

	void init();
	void shutdown();

	void pushCategory(const char* _name);
	void popCategory();

	void addTest(const char* _name, void(*_testFunctionPtr)());

	void runAllTests();
	void runAllTestsInCategory(char* _name);
	void runTest(u32 _testId);

	const TestCategory& getRootCategory() const;

//private:
	void _registerAllTests();
	void _runTest(const Test& _test);
	void _runAllTestsInCategory(const TestCategory& _category);

	DataArray<StringHash> m_categoryStack;

	Array<Test> m_tests;
	HashMap<StringHash, TestCategory> m_categories;
};

} // namespace yae
