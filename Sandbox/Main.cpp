#include "Bisky.hpp"

int main()
{
    bisky::core::setLogLevel(bisky::core::Verbose);
    SET_DEFAULT_WORKING_DIRECTORY();

    std::unique_ptr<bisky::core::Application> app = std::make_unique<bisky::core::Application>(1280, 960, "Sandbox");
    app->run();

    return 0;
}