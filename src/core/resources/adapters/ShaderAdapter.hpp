#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

struct ShaderResource;

namespace ShaderAdapter {
    std::optional<ShaderResource> load_shader(const std::string& vertex_path, const std::string& fragment_path);
};