#include <atomic>
#include <memory>
#include <random>
#include <thread>
#include <vector>

#include <iostream>

#include "libs/catch/catch.hpp"

#include "src/client.h"
#include "src/file.h"

#define N_ATTEMPTS 100
#define N_THREADS 4
#define N_THREAD_ATTEMPTS 100
#define FILE_SIZE 9999

void read_aux(std::shared_ptr<std::atomic<uint16_t>> r, std::shared_ptr<giga::Client> c) {
	size_t local_r = 0;
	size_t n_tests = 3;
	if(c->get_pos() != 0) {
		throw(exceptionpp::RuntimeError("read_aux", "client not initialized properly"));
	}
	for(size_t i = 0; i < N_THREAD_ATTEMPTS; ++i) {
		size_t pos = 3 * (rand() % (FILE_SIZE - 1));
		local_r += (c->seek(pos, true) == pos);
		local_r += (c->read(3).compare("abc") == 0);
		local_r += (c->seek(pos + 3, false) == 0);
	}
	c->close();
	*r += (local_r == n_tests * N_THREAD_ATTEMPTS);
}

void write_aux(std::shared_ptr<std::atomic<uint16_t>> r, std::shared_ptr<giga::Client> c) {
	size_t local_r = 0;
	size_t n_tests = 1;
	for(size_t j = 0; j < FILE_SIZE; ++j) {
		local_r += (c->write("f") == 1);
	}
	c->close();
	*r += (local_r == n_tests * FILE_SIZE);
}

void insert_aux(std::shared_ptr<std::atomic<uint16_t>> r, std::shared_ptr<giga::Client> c) {
}

void erase_aux(std::shared_ptr<std::atomic<uint16_t>> r, std::shared_ptr<giga::Client> c) {
}

void seek_aux(std::shared_ptr<std::atomic<uint16_t>> r, std::shared_ptr<giga::Client> c) {
	size_t local_r = 0;
	size_t n_tests = 2;
	for(size_t i = 0; i < N_THREAD_ATTEMPTS; ++i) {
		size_t pos = rand() % FILE_SIZE;
		local_r += (c->seek(pos, true) == pos);
		local_r += (c->seek(pos, false) == 0);
	}
	c->close();
	*r += (local_r == n_tests * N_THREAD_ATTEMPTS);
}

TEST_CASE("giga|read-concurrent") {
	std::shared_ptr<std::atomic<uint16_t>> r (new std::atomic<uint16_t>(0));
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-read-concurrent", "rw+"));
	std::shared_ptr<giga::Client> c = f->open();
	for(size_t i = 0; i < FILE_SIZE; ++i) {
		c->write("abc");
	}
	c->close();

	for(size_t i = 0; i < N_ATTEMPTS; ++i) {
		std::vector<std::thread> t;
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.push_back(std::thread(read_aux, r, f->open(NULL, "r")));
		}
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.at(j).join();
		}
	}

	REQUIRE(*r == N_ATTEMPTS * N_THREADS);
	remove("tests/files/giga-read-concurrent");
}

TEST_CASE("giga|write-concurrent") {
	std::shared_ptr<std::atomic<uint16_t>> r (new std::atomic<uint16_t>(0));
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-write-concurrent", "rw+"));
	std::shared_ptr<giga::Client> c = f->open();
	for(size_t i = 0; i < FILE_SIZE; ++i) {
		c->write("a");
	}
	c->close();

	for(size_t i = 0; i < N_ATTEMPTS; ++i) {
		std::vector<std::thread> t;
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.push_back(std::thread(write_aux, r, f->open(NULL, "rw")));
		}
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.at(j).join();
		}
	}

	REQUIRE(*r == N_ATTEMPTS * N_THREADS);

	c->open();
	for(size_t i = 0; i < FILE_SIZE; ++i) {
		REQUIRE(c->read(1).compare("f") == 0);
	}
	c->close();

	remove("tests/files/giga-write-concurrent");
}

TEST_CASE("giga|insert-concurrent") {
}

TEST_CASE("giga|erase-concurrent") {
}

TEST_CASE("giga|seek-concurrent") {
	std::shared_ptr<std::atomic<uint16_t>> r (new std::atomic<uint16_t>(0));
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-seek-concurrent", "rw+"));
	std::shared_ptr<giga::Client> c = f->open();
	for(size_t i = 0; i < FILE_SIZE; ++i) {
		c->write("a");
	}
	c->close();

	for(size_t i = 0; i < N_ATTEMPTS; ++i) {
		std::vector<std::thread> t;
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.push_back(std::thread(seek_aux, r, f->open(NULL, "r")));
		}
		for(size_t j = 0; j < N_THREADS; ++j) {
			t.at(j).join();
		}
	}

	REQUIRE(*r == N_ATTEMPTS * N_THREADS);
	remove("tests/files/giga-seek-concurrent");
}
