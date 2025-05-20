#include "Common.h"

#include "Core/Logger.h"

namespace Logger
{

static LogLevel logLevel = Info;

void SetLogLevel(const LogLevel &level)
{
  logLevel = level;
}

void _Info(std::string_view msg)
{
  fmt::print(fg(fmt::color::green), "[Info]: {}\n", msg);
}

void _Warning(std::string_view msg)
{
  fmt::print(fg(fmt::color::yellow), "[Warning]: {}\n", msg);
}

void _Error(std::string_view msg)
{
  fmt::print(fg(fmt::color::red), "[Error]: {}\n", msg);
}

} // namespace Logger
