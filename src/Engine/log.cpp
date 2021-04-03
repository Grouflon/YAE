#include "log.h"

namespace yae {

const char* DEFAULT_CATEGORY_NAME = "Default";



Logger::Logger()
{
    if (g_context.logger == nullptr)
    {
        g_context.logger = this;
    }
}



Logger::~Logger()
{
	if (g_context.logger == this)
	{
		g_context.logger = nullptr;
	}
}



void Logger::setCategoryVerbosity(const char* _categoryName, LogVerbosity _verbosity)
{
	LogCategory& category = findCategory(_categoryName);
	category.verbosity = _verbosity;
}



Logger::LogCategory& Logger::findCategory(const char* _categoryName)
{
	if (_categoryName == nullptr)
	{
		_categoryName = DEFAULT_CATEGORY_NAME;
	}

	StringHash hash = _categoryName;
	auto it = m_categories.find(hash);
	if (it == m_categories.end())
	{
		LogCategory category;
		category.name = _categoryName;
		category.verbosity = LogVerbosity_Log;
		auto result = m_categories.insert(std::make_pair(hash, category));
		YAE_ASSERT(result.second);
		it = result.first;
	}

	return it->second;
}

} // namespace yae
