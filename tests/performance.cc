#include <memory>

#include <iostream>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/performance.h"

TEST_CASE("giga|performance-result") {
	giga::Result r = giga::Result();

	REQUIRE_THROWS_AS(r.to_string(), exceptionpp::InvalidOperation);

	REQUIRE_NOTHROW(r.push_back("EGR", 1000, 1000, 1000, 1, 1, 1, 97, 1000, 1000, 10, 4));
	REQUIRE_NOTHROW(r.to_string());
}


TEST_CASE("giga|performance") {
	auto p = std::shared_ptr<giga::Performance> (new giga::Performance());

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::W}), std::vector<size_t>({100}), 1, 100), exceptionpp::InvalidOperation);

	std::shared_ptr<giga::File> f (new giga::File("tests/files/nonexistent", "rw+", giga::Config(1024, 2048, 100)));

	p->set_file(f);

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 0, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 1, 0), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({100}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERRR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::R}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);

	REQUIRE_NOTHROW(p->run("EXG", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::R}), std::vector<size_t>({0}), 1, 100));
	REQUIRE_NOTHROW(p->get_result().to_string());

	#ifdef _GIGA_PERFORMANCE_TESTS

	// performance testing
	auto c = f->open();
	auto access_pattern_a = std::vector<size_t>();
	auto access_pattern_b = std::vector<size_t>(100, 0);
	auto type_r = std::vector<uint8_t>(100, giga::Performance::R);
	auto type_w = std::vector<uint8_t>(100, giga::Performance::W);
	auto type_i = std::vector<uint8_t>(100, giga::Performance::I);
	auto type_e = std::vector<uint8_t>(100, giga::Performance::E);
	auto size = std::vector<size_t>(100, 1024);

	for(size_t i = 0; i < 100; ++i) {
		auto buf = std::vector<uint8_t> (1024, 0xff);
		c->write(std::string(buf.begin(), buf.end()));
		access_pattern_a.push_back(i * 1024);
	}
	REQUIRE(f->get_size() == 100 * 1024);

	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("R", access_pattern_a, type_r, size, n_clients + 1, 100));
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("W", access_pattern_a, type_w, size, n_clients + 1, 100));
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("I", access_pattern_a, type_i, size, n_clients + 1, 100));
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("E", access_pattern_b, type_e, size, n_clients + 1, 100));
	}

	std::cout << p->get_result().to_string();

	#endif
}
