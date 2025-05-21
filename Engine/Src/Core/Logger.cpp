#include "Common.h"

#include "Core/Logger.h"

namespace Logger
{

static LogLevel logLevel = Info;

void SetLogLevel(const LogLevel &level)
{
  logLevel = level;
}

void _Info(std::string_view msg, const std::string &filename, int line)
{
  fmt::print(fg(fmt::color::green), "[I] {}.{}: {}\n",
             std::filesystem::absolute(filename).filename().replace_extension().string(), line, msg);
}

void _Warning(std::string_view msg, const std::string &filename, int line)
{
  fmt::print(fg(fmt::color::yellow), "[W] {}.{}: {}\n",
             std::filesystem::absolute(filename).filename().replace_extension().string(), line, msg);
}

void _Error(std::string_view msg, const std::string &filename, int line)
{
  fmt::print(fg(fmt::color::red), "[E] {}.{}: {}\n",
             std::filesystem::absolute(filename).filename().replace_extension().string(), line, msg);
}

} // namespace Logger