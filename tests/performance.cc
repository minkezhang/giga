#include <ctime>
#include <memory>
#include <random>

#include <iostream>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/performance.h"

TEST_CASE("giga|performance-result") {
	giga::Result r = giga::Result();

	REQUIRE_THROWS_AS(r.to_string(), exceptionpp::InvalidOperation);

	REQUIRE_NOTHROW(r.push_back("EGR", 1000, 1000, 1000, 1, 1, 1, 97, 1000, 1000, 1000, 10, .94, 4));
	REQUIRE_NOTHROW(r.pop_front(true, true));

	REQUIRE_THROWS_AS(r.to_string(), exceptionpp::InvalidOperation);
}


TEST_CASE("giga|performance") {
	auto p = std::shared_ptr<giga::Performance> (new giga::Performance());

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::W}), std::vector<size_t>({100}), 1, 100), exceptionpp::InvalidOperation);

	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-performance", "rw+", giga::Config(16 * 1024, 24 * 1024, 10)));

	p->set_file(f);

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 0, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 1, 0), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({100}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERRR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::R}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);

	#ifdef _GIGA_PERFORMANCE_TESTS

	srand(time(NULL));

	size_t pattern_size = 1000;
	size_t n_attempts = 100;

	// performance testing
	auto c = f->open();
	auto access_pattern = std::vector<size_t>();
	auto type_r = std::vector<uint8_t>(pattern_size, giga::Performance::R);
	auto type_w = std::vector<uint8_t>(pattern_size, giga::Performance::W);
	auto type_i = std::vector<uint8_t>(pattern_size, giga::Performance::I);
	auto type_e = std::vector<uint8_t>(pattern_size, giga::Performance::E);
	auto size = std::vector<size_t>();

	auto buf = std::vector<uint8_t> (1024, 0xff);
	for(size_t i = 0; i < pattern_size; ++i) {
		c->write(std::string(buf.begin(), buf.end()));
		access_pattern.push_back(i * 1024 + (rand() % 1024));
		size.push_back(rand() % 1024);
	}
	c->save();
	REQUIRE(f->get_size() == pattern_size * 1024);

	c->close();

	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("R", access_pattern, type_r, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, n_clients == 0) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("W", access_pattern, type_w, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("I", access_pattern, type_i, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("E", access_pattern, type_e, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}

	#endif
}
