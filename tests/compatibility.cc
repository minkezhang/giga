#include <map>
#include <vector>

#include "libs/catch/catch.hpp"

TEST_CASE("giga|compatibility") {
	{
		INFO("WARNING: system does not support files larger than 4GB");
		CHECK(sizeof(size_t) > 4);
	}

	{
		std::vector<uint8_t> v;
		REQUIRE(v.size() == 0);
		v.push_back(1);
		v.clear();
		REQUIRE(v.size() == 0);

		// cf. http://bit.ly/1oZIz1e
		std::vector<uint8_t>().swap(v);
		INFO("WARNING: vector does not manually resize capacity");
		CHECK(v.capacity() == 0);
	}

	{
		INFO("WARNING: RAND_MAX is below recommended size");
		CHECK(RAND_MAX >= 2147483647);
	}
}
