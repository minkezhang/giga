#include "libs/catch/include/catch.hpp"

#include <memory>

#include <iostream>

#include "src/client.h"
#include "src/config.h"
#include "src/exception.h"
#include "src/file.h"

TEST_CASE("block|read") {
	std::shared_ptr<std::string> buffer (new std::string);

	std::shared_ptr<giga::File> file_empty (new giga::File("test/files/empty.txt", "r"));
	REQUIRE(file_empty->get_client_list().size() == 0);

	std::shared_ptr<giga::Client> c_empty = file_empty->open();

	REQUIRE(c_empty->read(buffer, 10) == 0);
	REQUIRE(buffer->compare("") == 0);

	file_empty->close(c_empty);

	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "r", std::shared_ptr<giga::Config> (new giga::Config(2, 1))));
	std::shared_ptr<giga::Client> c_five = file_five->open();

	REQUIRE(c_five->read(buffer, 0) == 0);
	REQUIRE(buffer->compare("") == 0);

	REQUIRE(c_five->read(buffer, 1) == 1);
	REQUIRE(buffer->compare("a") == 0);

	REQUIRE(c_five->read(buffer, 10) == 4);
	REQUIRE(buffer->compare("bcd\n") == 0);

	REQUIRE(c_five->read(buffer, 1) == 0);
	REQUIRE(buffer->compare("") == 0);

	file_five->close(c_five);
	REQUIRE_THROWS_AS(c_five->read(buffer, 1), giga::InvalidOperation);
}
