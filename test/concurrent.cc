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

	throw(giga::RuntimeError("blah", "hi"));

	std::shared_ptr<std::atomic<int>> result (new std::atomic<int>());

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

		if(((attempt + 1) % 100) == 0) {
			std::cout << "attempt " << attempt + 1 << "/" << n_attempts << std::endl;
		}
	}

	REQUIRE(*result == n_attempts * n_threads);
}
