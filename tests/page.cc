#include <iostream>

#include "libs/catch/catch.hpp"

#include "src/page.h"

TEST_CASE("giga|page") {
	giga::Page p = giga::Page(1, "tests/files/foo", 1, 3, false);

	REQUIRE(p.get_identifier() == 1);
	REQUIRE(p.get_is_dirty() == false);
	REQUIRE(p.get_is_loaded() == false);

	p.load();

	REQUIRE(p.get_is_loaded() == true);
	REQUIRE(p.get_data() == (std::vector<uint8_t> { 'o', 'o', '\n' }));
	p.set_data(std::vector<uint8_t> { 'i', 'i', '\n', '\n' });
	REQUIRE(p.get_is_dirty() == true);

	p.unload();
	p.load();
	REQUIRE(p.get_is_dirty() == false);
}
