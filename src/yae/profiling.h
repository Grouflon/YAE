#pragma once

#define YAE_PROFILING_ENABLED 1

namespace yae {
namespace profiling {

struct YAELIB_API CaptureScope
{
	CaptureScope(const char* _name);
	~CaptureScope();

	const char* m_name;
};

YAELIB_API void startCapture(const char* _captureName);
YAELIB_API void stopCapture(const char* _captureName);

} // namespace profiling
} // namespace yae

#if YAE_PROFILING_ENABLED
#define YAE_CAPTURE_START(_captureName) yae::profiling::startCapture(_captureName)
#define YAE_CAPTURE_STOP(_captureName) yae::profiling::stopCapture(_captureName)
#define YAE_CAPTURE_SCOPE(_scopeName) yae::profiling::CaptureScope __scope##__LINE__(_scopeName)
#define YAE_CAPTURE_FUNCTION() YAE_CAPTURE_SCOPE(__FUNCTION__)
#else
#define YAE_CAPTURE_START(_captureName)
#define YAE_CAPTURE_STOP(_captureName)
#define YAE_CAPTURE_SCOPE(_scopeName)
#define YAE_CAPTURE_FUNCTION()
#endif
