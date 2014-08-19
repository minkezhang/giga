#include <iostream>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

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
	REQUIRE(p.get_data() == (std::vector<uint8_t> { 'i', 'i', '\n', '\n' }));
	p.unload();

	REQUIRE(p.get_size() == 4);
	REQUIRE_THROWS_AS(p.probe(10, 0, true), exceptionpp::InvalidOperation);
	REQUIRE(p.probe(0, 2, true) == 2);
	REQUIRE(p.probe(0, 10, true) == 4);
	REQUIRE(p.probe(0, 1, false) == 0);
	REQUIRE(p.probe(1, 2, true) == 2);
	REQUIRE(p.probe(1, 10, true) == 3);
	REQUIRE(p.probe(1, 10, false) == 1);
	REQUIRE(p.probe(3, 2, true) == 1);
	REQUIRE(p.probe(3, 1, false) == 1);
	REQUIRE(p.probe(3, 10, false) == 3);
}
