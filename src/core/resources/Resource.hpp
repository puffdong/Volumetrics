#pragma once
#include <string>
#include "core/UUID.hpp"
#include "core/rendering/Model.hpp"

namespace Res {
    struct Model {
        ModelID id;
        std::string name;
        std::string file_extension;
    };
}    

struct ModelResource {
    ModelID id;
    std::string name;
    std::string asset_path;
    std::string file_path;
    std::string file_extension;

    ModelGpuData gpu_data;

    int ref_count = 0; // for future use when we implement resource unloading, currently unused
};