#include "libs/catch/include/catch.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <random>
#include <thread>

#include <iostream>

#include "src/client.h"
#include "src/config.h"
#include "src/file.h"

void aux_write_overwrite_test_worker(std::shared_ptr<giga::File> file, std::shared_ptr<std::atomic<int>> result) {
	int res = 0;

	std::shared_ptr<giga::Client> c = file->open();
	res += (file->get_n_clients() > 0);

	std::shared_ptr<std::string> buffer (new std::string("bla"));

	res += (c->get_pos() == 0);
	res += (c->write(buffer, false) == 3);
	res += (c->read(buffer, 10) == 2);
	res += (buffer->compare("d\n") == 0);

	c->seek(-10);
	res += (c->get_pos() == 0);
	res += (c->read(buffer, 10) == 5);
	res += (buffer->compare("blad\n") == 0);

	file->close(c);

	int expected = 8;
	*result += (int) (res == expected);
}

void aux_read_test_worker(std::shared_ptr<giga::File> file, std::shared_ptr<std::atomic<int>> result) {
	int res = 0;

	std::shared_ptr<giga::Client> c = file->open();
	res += (file->get_n_clients() > 0);

	std::shared_ptr<std::string> buffer (new std::string);

	res += (c->get_pos() == 0);
	res += (c->read(buffer, 0) == 0);
	res += (buffer->compare("") == 0);

	res += (c->read(buffer, 1) == 1);
	res += (buffer->compare("a") == 0);

	res += (c->read(buffer, 10) == 4);
	res += (buffer->compare("bcd\n") == 0);

	res += (c->read(buffer, 1) == 0);
	res += (buffer->compare("") == 0);

	res += (file->get_n_clients() > 0);
	file->close(c);

	try {
		c->read(buffer, 1);
	} catch(const giga::InvalidOperation& e) {
		res++;
	}

	int expected = 12;
	*result += (int) (res == expected);
}

TEST_CASE("concurrent|read") {
	int n_threads = 16;
	int n_attempts = 1000;

	std::shared_ptr<std::atomic<int>> result (new std::atomic<int>());

	std::cout << "RD: ";
	std::cout.flush();

	for(int attempt = 0; attempt < n_attempts; attempt++) {
		std::shared_ptr<std::string> buffer (new std::string);
		std::vector<std::thread> threads;
		std::shared_ptr<giga::File> file (new giga::File("test/files/five.txt", "ro", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));

		for(int i = 0; i < n_threads; i++) {
			threads.push_back(std::thread (aux_read_test_worker, file, result));
		}

		while(file->get_n_clients()) {
			// cf. http://bit.ly/1pLvXct
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}


		for(int i = 0; i < n_threads; i++) {
			threads.at(i).join();
		}

		if(((attempt + 1) % 25) == 0) {
			std::cout << ".";
			std::cout.flush();
		}
	}

	std::cout << std::endl;

	REQUIRE(*result == n_attempts * n_threads);
}

TEST_CASE("concurrent|write-insert") {
	std::cout << "WI: ." << std::endl;
}

TEST_CASE("concurrent|write-erase") {
	std::cout << "WE: ." << std::endl;
}

TEST_CASE("concurrent|write-overwrite") {
	int n_threads = 16;
	int n_attempts = 1000;

	std::shared_ptr<std::atomic<int>> result (new std::atomic<int>());

	std::cout << "WO: ";
	std::cout.flush();

	for(int attempt = 0; attempt < n_attempts; attempt++) {
		std::shared_ptr<std::string> buffer (new std::string);
		std::vector<std::thread> threads;
		std::shared_ptr<giga::File> file (new giga::File("test/files/five.txt", "rw", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));

		for(int i = 0; i < n_threads; i++) {
			threads.push_back(std::thread (aux_write_overwrite_test_worker, file, result));
		}

		while(file->get_n_clients()) {
			// cf. http://bit.ly/1pLvXct
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}


		for(int i = 0; i < n_threads; i++) {
			threads.at(i).join();
		}

		if(((attempt + 1) % 25) == 0) {
			std::cout << ".";
			std::cout.flush();
		}
	}

	std::cout << std::endl;

	REQUIRE(*result == n_attempts * n_threads);
}
