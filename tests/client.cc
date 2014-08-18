#include "libs/catch/catch.hpp"

#include "src/client.h"

TEST_CASE("giga|client") {
	giga::Client c = giga::Client(0, NULL);
	REQUIRE(c.get_is_closed() == false);
	c.close();
	REQUIRE(c.get_is_closed() == true);
	c.open();
	REQUIRE(c.get_is_closed() == false);
}
