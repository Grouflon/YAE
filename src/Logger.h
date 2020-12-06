#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <ctime>

#include <Assert.h>
#include <StringHash.h>
#include <StringTools.h>

namespace yae {

enum LogVerbosity
{
  LogVerbosity_None = 0,
  LogVerbosity_Error,
  LogVerbosity_Warning,
  LogVerbosity_Log,
  LogVerbosity_Verbose
};

enum LogOutputTargetFlag
{
  LogOutputTargetFlag_Stdout = 1 << 0,
  LogOutputTargetFlag_File = 1 << 1,
};

/*struct LogLine
{
  std::time_t time;
  std::string message;
  uint32_t category_hash;
};*/

struct LogCategory
{
  std::string name;
  LogVerbosity verbosity;
};

class Logger
{
public:
  Logger(const char* _logFilePath = nullptr, uint32_t _outputTargets = LogOutputTargetFlag_Stdout|LogOutputTargetFlag_File);
  ~Logger();

  template<typename ...Args>
  void log(const char* _categoryName, LogVerbosity _verbosity, const char* _fmt, Args... _args)
  {
    YAE_ASSERT(_fmt);

    LogCategory& category = _findCategory(_categoryName);
    if (category.verbosity >= _verbosity)
    {
      std::string msg = StringFormat(_fmt, _args...);
      printf("[%s]%s\n", category.name.c_str(), msg.c_str());
    }
  }

  void setCategoryVerbosity(const char* _categoryName, LogVerbosity _verbosity);

private:
  LogCategory& _findCategory(const char* _categoryName);

  uint32_t m_outputTargets = 0u;
  std::unordered_map<uint32_t, LogCategory> m_categories;
  //std::vector<LogLine> m_lines;
};

extern Logger g_logger;
};
