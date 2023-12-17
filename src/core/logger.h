#pragma once

#include <core/types.h>
#include <core/containers/HashMap.h>
#include <core/Event.h>

#include <core/platform.h>

namespace yae {

class CORE_API Logger
{
public:
	struct LogCategory
	{
		String128 name;
		LogVerbosity verbosity;
	};

	Logger();
	~Logger();

	void log(const char* _categoryName, LogVerbosity _verbosity, const char* _fileInfo, const char* _msg);

	void setCategoryVerbosity(const char* _categoryName, LogVerbosity _verbosity);
	LogCategory& findOrAddCategory(const char* _categoryName);
	LogVerbosity getCategoryVerbosity(const char* _categoryName);

	void setDefaultOutputColor(OutputColor _color);
	OutputColor getDefaultOutputColor() const;

	const HashMap<StringHash, LogCategory>& getCategories() const;

	bool serialize(Serializer& _serializer);

	// Events
	Event<const char*, LogVerbosity, const char*, const char*> logged;

// private:
	HashMap<StringHash, LogCategory> m_categories;
	OutputColor m_defaultOutputColor = OutputColor_Default;
};

} // namespace yae
