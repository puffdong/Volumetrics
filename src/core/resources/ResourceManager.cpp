#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const std::string& assets_root_path, const std::string& assets_handle)
: root_path(assets_root_path), asset_handle(assets_handle)
{

}

std::string ResourceManager::get_full_path(const std::string& asset_path) {
    return std::string(root_path) + std::string(asset_path.substr(asset_handle.size(), asset_path.size()));
}