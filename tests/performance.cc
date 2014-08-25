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

	REQUIRE_NOTHROW(r.push_back("EGR", 1000, 1000, 1000, 1000, 1000, 10, 4));
	REQUIRE_NOTHROW(r.to_string());
}


TEST_CASE("giga|performance") {
	auto p = std::shared_ptr<giga::Performance> (new giga::Performance());

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::W}), std::vector<size_t>({100}), 1), exceptionpp::InvalidOperation);

	std::shared_ptr<giga::File> f (new giga::File("tests/files/nonexistent", "rw+"));

	p->set_file(f);

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 0), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1}), 1), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({100}), std::vector<size_t>({0}), 1), exceptionpp::InvalidOperation);

	p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::R}), std::vector<size_t>({0}), 1);
	std::cout << p->get_result().to_string();
}

#endif
