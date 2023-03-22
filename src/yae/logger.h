#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>

#include <yae/inline_string.h>
#include <yae/platform.h>

namespace yae {

class YAE_API Logger
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

// private:
	HashMap<StringHash, LogCategory> m_categories;
	OutputColor m_defaultOutputColor = OutputColor_Default;
};

} // namespace yae
