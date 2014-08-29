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

	size_t n_attempts = 1;
	size_t file_size = 1024 * 1024;
	size_t page_size = 1024;
	size_t edit_size = 512;
	size_t pattern_size = file_size / edit_size;

	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-performance", "rw+", giga::Config(2 * page_size, 3 * page_size, 100)));

	p->set_file(f);

	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 0, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1, 1}), 1, 0), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1, 2}), std::vector<uint8_t>({giga::Performance::R, giga::Performance::W}), std::vector<size_t>({1}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERR", std::vector<size_t>({1}), std::vector<uint8_t>({100}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(p->run("ERRR", std::vector<size_t>({1}), std::vector<uint8_t>({giga::Performance::R}), std::vector<size_t>({0}), 1, 100), exceptionpp::InvalidOperation);

	#ifdef _GIGA_PERFORMANCE_TESTS

	srand(time(NULL));

	// performance testing
	auto c = f->open();
	auto access_pattern_seq = std::vector<size_t>();
	auto access_pattern_ran = std::vector<size_t>();
	auto type_r = std::vector<uint8_t>(pattern_size, giga::Performance::R);
	auto type_w = std::vector<uint8_t>(pattern_size, giga::Performance::W);
	auto type_i = std::vector<uint8_t>(pattern_size, giga::Performance::I);
	auto type_e = std::vector<uint8_t>(pattern_size, giga::Performance::E);
	auto size = std::vector<size_t>(pattern_size, edit_size);

	auto buf = std::vector<uint8_t> (file_size / pattern_size, 0xff);
	for(size_t i = 0; i < pattern_size; ++i) {
		c->write(std::string(buf.begin(), buf.end()));
		access_pattern_seq.push_back(i * (file_size / pattern_size));
		access_pattern_ran.push_back(rand() % file_size);
		// save incrementally
		if((f->get_size() % 1000000) == 0) {
			c->save();
		}
	}
	// write to disk
	c->save();
	REQUIRE(f->get_size() == file_size);

	c->close();

	REQUIRE_NOTHROW(p->run("Isq", access_pattern_seq, type_i, size, 10, n_attempts));

/*
	// sequential sequence writes
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Rsq", access_pattern_seq, type_r, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, n_clients == 0) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Wsq", access_pattern_seq, type_w, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	std::cout << "STARTING PERFORMANCE" << std::endl;
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Isq", access_pattern_seq, type_i, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}

	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Esq", access_pattern_seq, type_e, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}

	// random sequence writes
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Rrn", access_pattern_ran, type_r, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Wrn", access_pattern_ran, type_w, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Irn", access_pattern_ran, type_i, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
	for(size_t n_clients = 0; n_clients < 4; ++n_clients) {
		REQUIRE_NOTHROW(p->run("Ern", access_pattern_ran, type_e, size, n_clients + 1, n_attempts));
		std::cout << p->get_result()->pop_front(false, false) << std::flush;
	}
*/
	#endif
}
