#pragma once
#include <functional>

//forward decl
uint64_t uuid_generate_random64();
struct Resource;
class Shader;
class Texture;
class Model; // yuh, I know, its a thing that just needs to be fucking done at some point

template <class T = void>
class UUID {
public:
    UUID() : _uuid(uuid_generate_random64()) {};

    explicit UUID(uint64_t v) : _uuid(v) {};

    uint64_t value() const { return _uuid; };
    operator uint64_t() const { return _uuid; }; // to be or not to be explicit

private:
    uint64_t _uuid{0}; // hmm 0 should be reserved for uninitialized uuids
					   // 0 should invoke an error cuz it just can't be
					   // eh, look into sometime yuhyuh
};

template <class T>
constexpr bool operator==(const UUID<T>& a, const UUID<T>& b) noexcept {
    return a.value() == b.value();
}
template <class T>
constexpr bool operator!=(const UUID<T>& a, const UUID<T>& b) noexcept {
    return !(a == b);
}

template<class T>
struct uuid_hash {
    size_t operator()(const UUID<T>& id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value());
    }
};

namespace Res { 
    struct Shader; 
    struct Texture; 
    struct Model; 
}

using ShaderID = UUID<Res::Shader>;
using TextureID = UUID<Res::Texture>;
using ModelID = UUID<Res::Model>;