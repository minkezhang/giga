#include "catch/include/catch.hpp"

#include <memory>

#include <iostream>

#include "src/file.h"
#include "src/client.h"

TEST_CASE("block|read") {
	std::shared_ptr<std::string> buffer (new std::string);

	std::shared_ptr<giga::File> file_empty (new giga::File("test/files/empty.txt", "r"));
	REQUIRE(file_empty->get_client_list().size() == 0);

	std::shared_ptr<giga::Client> c_empty = file_empty->open();

	giga::giga_size n_bytes = c_empty->read(buffer, 10);
	REQUIRE(n_bytes == 0);

	REQUIRE(buffer->compare("") == 0);

	file_empty->close(c_empty);

	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "r"));
	std::shared_ptr<giga::Client> c_five = file_five->open();
	n_bytes = c_five->read(buffer, 10);

	std::cout << "'" << buffer->c_str() << "'" << std::endl;

	REQUIRE(n_bytes == 5);
	REQUIRE(buffer->compare("abcd\n") == 0);
	file_five->close(c_five);
}
