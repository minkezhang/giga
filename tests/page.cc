#include "libs/catch/catch.hpp"

#include "src/page.h"

TEST_CASE("giga|page") {
	giga::Page p = giga::Page(1, "files/foo", 0, 0);

	REQUIRE(p.get_identifier() == 1);
	REQUIRE(p.get_is_dirty() == false);
}
