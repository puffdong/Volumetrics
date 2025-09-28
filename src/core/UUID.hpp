#pragma once
#include <random>
#include <unordered_map>

class UUID
	{
	public:
		UUID();
		explicit UUID(uint64_t v);
		UUID(const UUID&) = default;

		uint64_t value() const { return uuid; }
		operator uint64_t() const { return uuid; }
	private:
		uint64_t uuid{0};
};