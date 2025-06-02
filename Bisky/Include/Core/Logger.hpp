#pragma once

#include <string>
#include <string_view>

namespace bisky::core
{

/*
 * The various degrees of log messages.
 */
enum LogLevel
{
    Verbose,
    Info,
    Warning,
    Error
};

/*
 * Displays messages at the same level or a higher level than the given level.
 *
 * @param level The minimum level to display.
 */
void setLogLevel(const LogLevel &level);

/*
 * Logs a verbose message.
 * This method should only be called through the macro LOG_VERBOSE.
 *
 * @param msg The message to display.
 * @param filename The filename this was called from.
 * @param line The line that this was called from.
 */
void _verbose(std::string_view msg, const std::string &filename, int line);

/*
 * Logs an info message.
 * This method should only be called through the macro LOG_INFO.
 *
 * @param msg The message to display.
 * @param filename The filename this was called from.
 * @param line The line that this was called from.
 */
void _info(std::string_view msg, const std::string &filename, int line);

/*
 * Logs a warning message.
 * This method should only be called through the macro LOG_WARNING.
 *
 * @param msg The message to display.
 * @param filename The filename this was called from.
 * @param line The line that this was called from.
 */
void _warning(std::string_view msg, const std::string &filename, int line);

/*
 * Logs an error message.
 * This method should only be called through the macro LOG_ERROR.
 *
 * @param msg The message to display.
 * @param filename The filename this was called from.
 * @param line The line that this was called from.
 */
void _error(std::string_view msg, const std::string &filename, int line);

} // namespace bisky::core

/*
 * Preset defines that pass in the file and line automatically.
 * You should be using these instead of the namespace methods.
 */
#define LOG_VERBOSE(msg) bisky::core::_verbose(msg, __FILE__, __LINE__);
#define LOG_INFO(msg) bisky::core::_info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) bisky::core::_warning(msg, __FILE__, __LINE__);
#define LOG_ERROR(msg) bisky::core::_error(msg, __FILE__, __LINE__);