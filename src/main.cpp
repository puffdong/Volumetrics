#include "app/Application.hpp"
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH "C:/Dev/OpenGL/Volumetrics/res/"
#endif 

int main(void)
{
    AppConfig config{600, 400, ASSET_PATH};
    Application application(config);
    return application.run();
}