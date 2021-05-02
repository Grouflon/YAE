#pragma once

#include <yae/types.h>
#include <yae/context.h>
#include <yae/hash.h>
#include <yae/containers.h>

namespace yae {

enum YAELIB_API LogVerbosity
{
	LogVerbosity_None = 0,
	LogVerbosity_Error,
	LogVerbosity_Warning,
	LogVerbosity_Log,
	LogVerbosity_Verbose
};

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
	LogCategory& findCategory(const char* _categoryName);

private:
	HashMap<StringHash, LogCategory> m_categories;
};



namespace logging {

template<typename ...Args>
void log(yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _fmt, Args... _args)
{
	YAE_ASSERT(_fmt);

	Logger::LogCategory& category = _logger.findCategory(_categoryName);
	if (category.verbosity >= _verbosity)
	{
		String msg = string::format(_fmt, _args...);
		printf("[%s]%s\n", category.name.c_str(), msg.c_str());
	}
}

} // namespace logging

} // namespace yae

#define YAE_VERBOSE(_msg)						yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Verbose, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_VERBOSE_CAT(_category, _msg)		yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Verbose, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_VERBOSEF(_fmt, ...)					yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Verbose, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_VERBOSEF_CAT(_category, _fmt, ...)	yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Verbose, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_LOG(_msg)							yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Log, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_LOG_CAT(_category, _msg)			yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Log, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_LOGF(_fmt, ...)						yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Log, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_LOGF_CAT(_category, _fmt, ...)		yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Log, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_WARNING(_msg)						yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Warning, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_WARNING_CAT(_category, _msg)		yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Warning, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_WARNINGF(_fmt, ...)					yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Warning, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_WARNINGF_CAT(_category, _fmt, ...)	yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Warning, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_ERROR(_msg)							yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Error, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_ERROR_CAT(_category, _msg)			yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Error, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_ERRORF(_fmt, ...)					yae::logging::log(*yae::context().logger, nullptr, yae::LogVerbosity_Error, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_ERRORF_CAT(_category, _fmt, ...)	yae::logging::log(*yae::context().logger, _category, yae::LogVerbosity_Error, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
