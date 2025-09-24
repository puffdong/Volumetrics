#include "uuid.hpp"
#include <random>
#include <unordered_map>

static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

UUID::UUID()
    : uuid(s_uniform_distribution(s_engine))
{
}

UUID::UUID(uint64_t uuid)
    : uuid(uuid)
{
}

