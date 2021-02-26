#include "Logger.h"

#include <00-Type/GlobalContext.h>

namespace yae {

#define DEFAULT_CATEGORY_NAME "Default"

Logger::Logger(const char* _logFilePath, u32 _outputTargets)
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
  LogCategory& category = _findCategory(_categoryName);
  category.verbosity = _verbosity;
}

LogCategory& Logger::_findCategory(const char* _categoryName)
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
