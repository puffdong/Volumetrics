#include "UUID.hpp"
#include <random>

static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

std::uint64_t uuid_generate_random64() {
    return s_uniform_distribution(s_engine);
}