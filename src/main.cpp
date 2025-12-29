#include "app/Application.hpp"
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH "C:/Dev/OpenGL/Volumetrics/res/"
#endif 

int main(void) {
#ifdef __APPLE__
    AppConfig config{750, 500, ASSET_PATH, "macOS"};
#endif 
#if defined _WIN32 || defined _WIN64
    AppConfig config{1920, 1080, ASSET_PATH, "Windows"};
#endif
    Application application(config);
    return application.run();
}