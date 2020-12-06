#include "Logger.h"

#include <HashTools.h>

namespace yae {

#define DEFAULT_CATEGORY_NAME "Default"

Logger::Logger(const char* _logFilePath, uint32_t _outputTargets)
{

}

Logger::~Logger()
{

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

  uint32_t hash = HashString(_categoryName);
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

Logger g_logger;

} // namespace yae
