#include "logger.h"

#include <yae/program.h>
#include <yae/hash.h>

namespace yae {

const char* DEFAULT_CATEGORY_NAME = "Default";

Logger::Logger()
	: m_categories(&toolAllocator())
{
}


Logger::~Logger()
{
}


void Logger::setCategoryVerbosity(const char* _categoryName, LogVerbosity _verbosity)
{
	LogCategory& category = findOrAddCategory(_categoryName);
	category.verbosity = _verbosity;
}


Logger::LogCategory& Logger::findOrAddCategory(const char* _categoryName)
{
	if (_categoryName == nullptr)
	{
		_categoryName = DEFAULT_CATEGORY_NAME;
	}

	StringHash hash = _categoryName;
	LogCategory* categoryPtr = m_categories.get(hash);
	if (categoryPtr == nullptr)
	{
		LogCategory category;
		category.name = _categoryName;
		category.verbosity = LogVerbosity_Log;
		categoryPtr = &m_categories.set(hash, category);
	}
	return *categoryPtr;
}

void Logger::setDefaultOutputColor(OutputColor _color)
{
	m_defaultOutputColor = _color;
}

OutputColor Logger::getDefaultOutputColor() const
{
	return m_defaultOutputColor;
}

} // namespace yae
