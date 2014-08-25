#ifdef _GIGA_PERFORMANCE_TESTS

#include <iostream>
#include <memory>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/performance.h"

TEST_CASE("giga|performance-result") {
	giga::Result r = giga::Result();

	REQUIRE_THROWS_AS(r.to_string(), exceptionpp::InvalidOperation);

	r.push_back("EGR", 1000, 1000, 1000, 1000, 1000, 10, 1024, 4);
	std::cout << r;
}


TEST_CASE("giga|performance") {
	auto p = std::shared_ptr<giga::Performance> (new giga::Performance());

	REQUIRE_THROWS_AS(p->run(std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::W}), std::vector<size_t>({100}), 1), exceptionpp::InvalidOperation);

	p->set_file(std::shared_ptr<giga::File> (new giga::File("tests/files/nonexistent", "rw+")));

	REQUIRE_THROWS_AS(p->run(std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 0), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run(std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1}), 1), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run(std::vector<size_t>({1}), std::vector<uint8_t>({100}), std::vector<size_t>({0}), 1), exceptionpp::InvalidOperation);
}

#endif
