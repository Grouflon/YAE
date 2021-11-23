#include <yae/types.h>
#include "logging.h"

#include <yae/logger.h>

namespace yae {

namespace logging {

void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _msg)
{
	Logger::LogCategory& category = _logger.findOrAddCategory(_categoryName);
	if (category.verbosity >= _verbosity)
	{
		printf("[%s]%s\n", category.name.c_str(), _msg);
	}
}

}

} // namespace yae