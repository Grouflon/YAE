#pragma once

#ifdef YAELIB_EXPORT
#define YAELIB_API __declspec(dllexport)
#else
#define YAELIB_API __declspec(dllimport)
#endif
