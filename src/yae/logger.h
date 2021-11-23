#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>
#include <yae/hash.h>
#include <yae/inline_string.h>

namespace yae {

class YAELIB_API Logger
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

// private:
	HashMap<StringHash, LogCategory> m_categories;
};

} // namespace yae
