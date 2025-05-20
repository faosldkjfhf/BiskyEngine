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

void _Info(std::string_view msg);
void _Warning(std::string_view msg);
void _Error(std::string_view msg);

} // namespace Logger

#define LOG_INFO(msg) Logger::_Info(msg)
#define LOG_WARNING(msg) Logger::_Warning(msg);
#define LOG_ERROR(msg) Logger::_Error(msg);
