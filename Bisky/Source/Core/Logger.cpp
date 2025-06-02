#include "Common.hpp"

#include "Core/Logger.hpp"

namespace bisky::core
{

static LogLevel logLevel = Info;

void setLogLevel(const LogLevel &level)
{
    logLevel = level;
}

void _verbose(std::string_view msg, const std::string &filename, int line)
{
    if (logLevel <= Verbose)
    {
        fmt::print(fg(fmt::color::cyan),
                   "[V] {}.{}: ", std::filesystem::absolute(filename).filename().replace_extension().string(), line);
        fmt::print("{}\n", msg);
    }
}

void _info(std::string_view msg, const std::string &filename, int line)
{
    if (logLevel <= Info)
    {
        fmt::print(fg(fmt::color::light_green),
                   "[I] {}.{}: ", std::filesystem::absolute(filename).filename().replace_extension().string(), line);
        fmt::print("{}\n", msg);
    }
}

void _warning(std::string_view msg, const std::string &filename, int line)
{
    if (logLevel <= Warning)
    {
        fmt::print(fg(fmt::color::lemon_chiffon),
                   "[W] {}.{}: ", std::filesystem::absolute(filename).filename().replace_extension().string(), line);
        fmt::print("{}\n", msg);
    }
}

void _error(std::string_view msg, const std::string &filename, int line)
{
    if (logLevel <= Error)
    {
        fmt::print(fg(fmt::color::indian_red),
                   "[E] {}.{}: ", std::filesystem::absolute(filename).filename().replace_extension().string(), line);
        fmt::print("{}\n", msg);
    }
}

} // namespace bisky::core