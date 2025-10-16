#include "app/Application.hpp"
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH "C:/Dev/OpenGL/Volumetrics/res/"
#endif 

int main(void)
{
#ifdef __APPLE__ // this looks like doodoo, but who else is using this except me right now?
    AppConfig config{650, 400, ASSET_PATH};
#endif 
#if defined _WIN32 || defined _WIN64
    AppConfig config{1920, 1080, ASSET_PATH};
#endif 
    Application application(config);
    return application.run();
}