#include <core/types.h>
#include "logging.h"

#include <core/logger.h>
#include <core/platform.h>

#include <mirror/mirror.h>

namespace yae {

namespace logging {

void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _msg)
{
	Logger::LogCategory& category = _logger.findOrAddCategory(_categoryName);

	OutputColor outputColor = _logger.getDefaultOutputColor();
	switch(_verbosity)
	{
		case LogVerbosity::ERROR: outputColor = OutputColor_Red; break;
		case LogVerbosity::WARNING: outputColor = OutputColor_Yellow; break;
		case LogVerbosity::VERBOSE: outputColor = OutputColor_Grey; break;
		default: break;
	}
	
	if (category.verbosity >= _verbosity)
	{
		platform::setOutputColor(outputColor);
		printf("[%s]%s\n", category.name.c_str(), _msg);
		platform::setOutputColor(OutputColor_Default);
	}
}

}

} // namespace yae

MIRROR_ENUM(yae::LogVerbosity)
(
	MIRROR_ENUM_VALUE(yae::LogVerbosity::NONE);
	MIRROR_ENUM_VALUE(yae::LogVerbosity::ERROR);
	MIRROR_ENUM_VALUE(yae::LogVerbosity::WARNING);
	MIRROR_ENUM_VALUE(yae::LogVerbosity::LOG);
	MIRROR_ENUM_VALUE(yae::LogVerbosity::VERBOSE);
);
