#include "libs/catch/include/catch.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <iostream>

#include "src/client.h"
#include "src/config.h"
#include "src/file.h"

void aux_read_test_worker(std::shared_ptr<giga::File> file, std::shared_ptr<std::atomic<int>> result) {
	std::shared_ptr<giga::Client> c = file->open();
	std::shared_ptr<std::string> buffer (new std::string);

	int res = 0;

	res += (c->get_pos() == 0);
	res += (c->read(buffer, 0) == 0);
	res += (buffer->compare("") == 0);

	res += (c->read(buffer, 1) == 1);
	res += (buffer->compare("a") == 0);

	res += (c->read(buffer, 10) == 4);
	res += (buffer->compare("bcd\n") == 0);

	res += (c->read(buffer, 1) == 0);
	res += (buffer->compare("") == 0);

	file->close(c);

	int expected = 9;
	*result += (int) (res == expected);
}

TEST_CASE("concurrent|read") {
	int n_threads = 5;
	std::shared_ptr<std::string> buffer (new std::string);
	std::vector<std::thread> threads;
	std::shared_ptr<giga::File> file (new giga::File("test/files/five.txt", "r", std::shared_ptr<giga::Config> (new giga::Config(2, 0))));

	std::shared_ptr<std::atomic<int>> result (new std::atomic<int>());

	for(int i = 0; i < n_threads; i++) {
		threads.push_back(std::thread (aux_read_test_worker, file, result));
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	while(file->get_n_clients()) {
		// cf. http://bit.ly/1pLvXct
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}


	for(int i = 0; i < n_threads; i++) {
		threads.at(i).join();
	}
	REQUIRE(*result == n_threads);
}
