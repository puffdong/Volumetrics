#include "app/Application.hpp"

int main(void)
{
    AppConfig config{1920, 1080, ""};
    Application application(config);
    return application.run();
}