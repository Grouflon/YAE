#pragma once

#include <Logger.h>

#ifndef __FILENAME__
#define __FILENAME__ \
	(strrchr(__FILE__,'/') \
	? strrchr(__FILE__,'/')+1 \
	: (strrchr(__FILE__,'\\') \
	? strrchr(__FILE__,'\\')+1 \
	: __FILE__ \
	))
#endif

#define YAE_VERBOSE(_msg) yae::g_logger.log(nullptr, yae::LogVerbosity_Verbose, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_VERBOSE_CAT(_category, _msg) yae::g_logger.log(_category, yae::LogVerbosity_Verbose, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_VERBOSEF(_fmt, ...) yae::g_logger.log(nullptr, yae::LogVerbosity_Verbose, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_VERBOSEF_CAT(_category, _fmt, ...) yae::g_logger.log(_category, yae::LogVerbosity_Verbose, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_LOG(_msg) yae::g_logger.log(nullptr, yae::LogVerbosity_Log, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_LOG_CAT(_category, _msg) yae::g_logger.log(_category, yae::LogVerbosity_Log, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_LOGF(_fmt, ...) yae::g_logger.log(nullptr, yae::LogVerbosity_Log, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_LOGF_CAT(_category, _fmt, ...) yae::g_logger.log(_category, yae::LogVerbosity_Log, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_WARNING(_msg) yae::g_logger.log(nullptr, yae::LogVerbosity_Warning, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_WARNING_CAT(_category, _msg) yae::g_logger.log(_category, yae::LogVerbosity_Warning, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_WARNINGF(_fmt, ...) yae::g_logger.log(nullptr, yae::LogVerbosity_Warning, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_WARNINGF_CAT(_category, _fmt, ...) yae::g_logger.log(_category, yae::LogVerbosity_Warning, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)

#define YAE_ERROR(_msg) yae::g_logger.log(nullptr, yae::LogVerbosity_Error, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_ERROR_CAT(_category, _msg) yae::g_logger.log(_category, yae::LogVerbosity_Error, "[%s:%d] "_msg, __FILENAME__, __LINE__)
#define YAE_ERRORF(_fmt, ...) yae::g_logger.log(nullptr, yae::LogVerbosity_Error, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)
#define YAE_ERRORF_CAT(_category, _fmt, ...) yae::g_logger.log(_category, yae::LogVerbosity_Error, "[%s:%d] "_fmt, __FILENAME__, __LINE__, __VA_ARGS__)
