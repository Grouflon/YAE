#include "logger.h"

#include <core/Program.h>
#include <core/filesystem.h>
#include <core/serialization/serialization.h>


namespace yae {

const char* DEFAULT_CATEGORY_NAME = "Default";

Logger::Logger()
	: m_categories(&toolAllocator())
{
}


Logger::~Logger()
{
}

void Logger::log(const char* _categoryName, yae::LogVerbosity _verbosity, const char* _fileInfo, const char* _msg)
{
	Logger::LogCategory& category = findOrAddCategory(_categoryName);

	OutputColor outputColor = getDefaultOutputColor();
	switch(_verbosity)
	{
		case LogVerbosity::ERROR: outputColor = OutputColor_Red; break;
		case LogVerbosity::WARNING: outputColor = OutputColor_Yellow; break;
		case LogVerbosity::VERBOSE: outputColor = OutputColor_Grey; break;
		default: break;
	}

	if (category.verbosity >= _verbosity)
	{
		String128 fileName = filesystem::getFileName(_fileInfo);

		platform::setOutputColor(outputColor);
		printf("[%s][%s] %s\n", category.name.c_str(), fileName.c_str(), _msg);
		platform::setOutputColor(OutputColor_Default);
	}

	logged.dispatch(category.name.c_str(), _verbosity, _fileInfo, _msg);
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
		category.verbosity = LogVerbosity::LOG;
		categoryPtr = &m_categories.set(hash, category);
	}
	return *categoryPtr;
}

LogVerbosity Logger::getCategoryVerbosity(const char* _categoryName)
{
	return findOrAddCategory(_categoryName).verbosity;
}

void Logger::setDefaultOutputColor(OutputColor _color)
{
	m_defaultOutputColor = _color;
}

OutputColor Logger::getDefaultOutputColor() const
{
	return m_defaultOutputColor;
}

const HashMap<StringHash, Logger::LogCategory>& Logger::getCategories() const
{
	return m_categories;
}

bool Logger::serialize(Serializer& _serializer)
{
	Array<LogCategory> categories(&scratchAllocator());
	if (_serializer.isWriting())
	{
		for(auto pair : m_categories)
		{
			categories.push_back(pair.value);
		}
	}

	//TODO: Make it better than that by doing an object instead of an array, but we need object exploration functions in the serializer to do that
	u32 categoriesSize = categories.size();
	if (_serializer.beginSerializeArray(categoriesSize, "categories"))
	{
		categories.resize(categoriesSize);
		for (u32 i = 0; i < categoriesSize; ++i)
		{
			if (_serializer.beginSerializeObject())
			{
				LogCategory& category = categories[i];
				if (!_serializer.serialize(category.name, "name"))
					continue;

				if (!serialization::serializeMirrorType(_serializer, category.verbosity, "verbosity"))
					continue;

				_serializer.endSerializeObject();
			}
		}

		_serializer.endSerializeArray();
	}

	if (_serializer.isReading())
	{
		for (LogCategory& logCategory : categories)
		{
			setCategoryVerbosity(logCategory.name.c_str(), logCategory.verbosity);
		}
	}

	return true;
}


} // namespace yae
