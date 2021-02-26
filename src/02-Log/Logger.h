#pragma once

#include <string>
#include <vector>
#include <ctime>

#include <00-Macro/Assert.h>
#include <00-Type/IntTypes.h>
#include <01-Hash/StringHash.h>
#include <01-String/StringTools.h>

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
  u32 category_hash;
};*/

struct LogCategory
{
  std::string name;
  LogVerbosity verbosity;
};

class Logger
{
public:
  Logger(const char* _logFilePath = nullptr, u32 _outputTargets = LogOutputTargetFlag_Stdout|LogOutputTargetFlag_File);
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

  u32 m_outputTargets = 0u;
  std::unordered_map<StringHash, LogCategory> m_categories;
  //std::vector<LogLine> m_lines;
};

};
