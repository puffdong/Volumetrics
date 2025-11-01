#include "app/Application.hpp"
#ifdef __APPLE__
    #define ASSET_PATH "/Users/puff/Developer/graphics/Volumetrics/res/"
    #endif
#if defined _WIN32 || defined _WIN64
    #define ASSET_PATH "C:/Dev/OpenGL/Volumetrics/res/"
#endif 

void print_cpp_version() {
    std::cout << "C++ Version: "; // __cplusplus
#if   __cplusplus >= 202302L
    std::cout << "C++23 or later" << std::endl;
#elif __cplusplus >= 202002L
    std::cout << "C++20" << std::endl;
#elif __cplusplus >= 201703L
    std::cout << "C++17" << std::endl;
#elif __cplusplus >= 201402L
    std::cout << "C++14" << std::endl;
#elif __cplusplus >= 201103L
    std::cout << "C++11" << std::endl;
#else
    std::cout << "Pre-C++11 (C++98/03)" << std::endl;
#endif
}

int main(void)
{
    // print_cpp_version();
#ifdef __APPLE__
    AppConfig config{750, 500, ASSET_PATH, "macOS"};
#endif 
#if defined _WIN32 || defined _WIN64
    AppConfig config{1920, 1080, ASSET_PATH, "Windows"};
#endif
    Application application(config);
    return application.run();
}