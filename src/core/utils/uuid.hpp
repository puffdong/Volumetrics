#pragma once
#include <random>
#include <unordered_map>

class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return uuid; }
	private:
		uint64_t uuid;
	};