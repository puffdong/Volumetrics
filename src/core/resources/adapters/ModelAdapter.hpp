#pragma once

#include <string>
#include "core/rendering/Model.hpp"

namespace ModelAdapter {

    ModelGpuData load_obj(const std::string& filepath); // load model and upload to gpu, gives back model struct with info!
}