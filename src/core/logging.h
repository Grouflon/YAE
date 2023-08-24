#pragma once

#include <cstdlib>

namespace yae {

class Logger;

enum LogVerbosity
{
	LogVerbosity_None = 0,
	LogVerbosity_Error,
	LogVerbosity_Warning,
	LogVerbosity_Log,
	LogVerbosity_Verbose
};

namespace logging {

CORE_API void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _msg);

template<typename ...Args>
void log(::yae::Logger& _logger, const char* _categoryName, yae::LogVerbosity _verbosity, const char* _fmt, Args... _args)
{
	const int BUFFER_SIZE = 512;
	int logSize = snprintf(nullptr, 0, _fmt, _args...);
	if (logSize < BUFFER_SIZE)
	{
		char buffer[BUFFER_SIZE];
		snprintf(buffer, BUFFER_SIZE, _fmt, _args...);
		log(_logger, _categoryName, _verbosity, buffer);
	}
	else
	{
		char* buffer = (char*)malloc(logSize + 1);
		snprintf(buffer, logSize + 1, _fmt, _args...);
		log(_logger, _categoryName, _verbosity, buffer);
		free(buffer);
	}
}

} // namespace logging

} // namespace yae

#define YAE_VERBOSE(_msg)						::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Verbose, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_VERBOSE_CAT(_category, _msg)		::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Verbose, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_VERBOSEF(_fmt, ...)					::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Verbose, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_VERBOSEF_CAT(_category, _fmt, ...)	::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Verbose, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_LOG(_msg)							::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Log, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_LOG_CAT(_category, _msg)			::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Log, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_LOGF(_fmt, ...)						::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Log, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_LOGF_CAT(_category, _fmt, ...)		::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Log, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_WARNING(_msg)						::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Warning, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_WARNING_CAT(_category, _msg)		::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Warning, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_WARNINGF(_fmt, ...)					::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Warning, "[%s:%d] " _fmt "", __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_WARNINGF_CAT(_category, _fmt, ...)	::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Warning, "[%s:%d] " _fmt "", __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_ERROR(_msg)							::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Error, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_ERROR_CAT(_category, _msg)			::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Error, "[%s:%d] %s", __FILENAME__, __LINE__, _msg)
#define YAE_ERRORF(_fmt, ...)					::yae::logging::log(::yae::logger(), nullptr, ::yae::LogVerbosity_Error, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_ERRORF_CAT(_category, _fmt, ...)	::yae::logging::log(::yae::logger(), _category, ::yae::LogVerbosity_Error, "[%s:%d] " _fmt, __FILENAME__, __LINE__, __VA_ARGS__)
