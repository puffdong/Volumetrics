#include "app/Application.hpp"
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH ""
#endif 

int main(void)
{
    AppConfig config{1920, 1080, ASSET_PATH};
    Application application(config);
    return application.run();
}