#include <core/types.h>
#include "logging.h"

#include <core/logger.h>
#include <core/platform.h>

namespace yae {

namespace logging {

void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _msg)
{
	Logger::LogCategory& category = _logger.findOrAddCategory(_categoryName);

	OutputColor outputColor = _logger.getDefaultOutputColor();
	switch(_verbosity)
	{
		case LogVerbosity_Error: outputColor = OutputColor_Red; break;
		case LogVerbosity_Warning: outputColor = OutputColor_Yellow; break;
		case LogVerbosity_Verbose: outputColor = OutputColor_Grey; break;
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