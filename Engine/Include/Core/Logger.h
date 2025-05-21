#pragma once

namespace Logger
{

enum LogLevel
{
  Info,
  Warning,
  Error
};

void SetLogLevel(const LogLevel &level);

void _Info(std::string_view msg, const std::string &filename, int line);
void _Warning(std::string_view msg, const std::string &filename, int line);
void _Error(std::string_view msg, const std::string &filename, int line);

} // namespace Logger

#define LOG_INFO(msg) Logger::_Info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::_Warning(msg, __FILE__, __LINE__);
#define LOG_ERROR(msg) Logger::_Error(msg, __FILE__, __LINE__);
