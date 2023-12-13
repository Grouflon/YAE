#pragma once

#include <core/types.h>
#include <core/containers/HashMap.h>

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

	void setCategoryVerbosity(const char* _categoryName, LogVerbosity _verbosity);
	LogCategory& findOrAddCategory(const char* _categoryName);

	void setDefaultOutputColor(OutputColor _color);
	OutputColor getDefaultOutputColor() const;

	const HashMap<StringHash, LogCategory>& getCategories() const;

	bool serialize(Serializer& _serializer);

// private:
	HashMap<StringHash, LogCategory> m_categories;
	OutputColor m_defaultOutputColor = OutputColor_Default;
};

} // namespace yae
