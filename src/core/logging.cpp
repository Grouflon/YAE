#include <core/types.h>
#include "logging.h"

#include <core/logger.h>
#include <core/platform.h>

#include <mirror/mirror.h>

namespace yae {

namespace logging {

void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _fileInfo, const char* _msg)
{
	_logger.log(_categoryName, _verbosity, _fileInfo, _msg);
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
