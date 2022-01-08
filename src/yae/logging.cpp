#include <yae/types.h>
#include "logging.h"

#include <yae/logger.h>

namespace yae {

namespace logging {

void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _msg)
{
	// @NOTE: see this thread for help on colors: https://stackoverflow.com/questions/8765938/colorful-text-using-printf-in-c
	// We might want to go for the selected answer with platform specific code, in order to avoid having to do any modification on the user's machine
	Logger::LogCategory& category = _logger.findOrAddCategory(_categoryName);
	const char* beginTag = "\033[0m";
	const char* endTag = "\033[0m";

	switch(_verbosity)
	{
		case LogVerbosity_Error: beginTag = "\033[31m"; break;
		case LogVerbosity_Warning: beginTag = "\033[33m"; break;
		case LogVerbosity_Verbose: beginTag = "\033[90m"; break;
		default: break;
	}

	if (category.verbosity >= _verbosity)
	{
		printf("%s[%s]%s%s\n", beginTag, category.name.c_str(), _msg, endTag);
	}
}

}

} // namespace yae