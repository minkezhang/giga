#include "libs/catch/include/catch.hpp"

#include <memory>

#include "src/client.h"
#include "src/config.h"
#include "src/exception.h"
#include "src/file.h"

TEST_CASE("block|seek") {
	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "rw", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));
	std::shared_ptr<giga::Client> c_five = file_five->open();

	REQUIRE(c_five->get_pos() == 0);
	c_five->seek(1);
	REQUIRE(c_five->get_pos() == 1);
	c_five->seek(0);
	REQUIRE(c_five->get_pos() == 1);
	c_five->seek(3);
	REQUIRE(c_five->get_pos() == 4);
	c_five->seek(-3);
	REQUIRE(c_five->get_pos() == 1);
	c_five->seek(10);
	REQUIRE(c_five->get_pos() == 5);
	c_five->seek(-10);
	REQUIRE(c_five->get_pos() == 0);

	file_five->close(c_five);
	REQUIRE_THROWS_AS(c_five->seek(0), giga::InvalidOperation);
	REQUIRE_THROWS_AS(c_five->get_pos(), giga::InvalidOperation);
}

TEST_CASE("block|write-insert") {
	std::shared_ptr<std::string> buffer (new std::string("bla"));

	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "rw", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));
	std::shared_ptr<giga::Client> c_five = file_five->open();
	std::shared_ptr<giga::Client> c_five_read = file_five->open();

	REQUIRE(c_five->write(buffer, true) == 3);
	REQUIRE(c_five->read(buffer, 10) == 5);
	REQUIRE(buffer->compare("abcd\n") == 0);

	REQUIRE(c_five_read->read(buffer, 10) == 8);
	REQUIRE(buffer->compare("blaabcd\n") == 0);

	file_five->close(c_five);
	file_five->close(c_five_read);
}

TEST_CASE("block|write-overwrite") {
	std::shared_ptr<std::string> buffer (new std::string("bla"));

	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "rw", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));
	std::shared_ptr<giga::Client> c_five = file_five->open();
	std::shared_ptr<giga::Client> c_five_read = file_five->open();

	REQUIRE(c_five->write(buffer, false) == 3);
	REQUIRE(c_five->read(buffer, 1) == 1);
	REQUIRE(buffer->compare("d") == 0);

	REQUIRE(c_five_read->read(buffer, 10) == 5);
	REQUIRE(buffer->compare("blad\n") == 0);

	file_five->close(c_five);
	file_five->close(c_five_read);
	REQUIRE_THROWS_AS(c_five->write(buffer, true), giga::InvalidOperation);
}

TEST_CASE("block|read") {
	std::shared_ptr<std::string> buffer (new std::string);

	std::shared_ptr<giga::File> file_empty (new giga::File("test/files/empty.txt", "ro"));
	REQUIRE(file_empty->get_n_clients() == 0);

	std::shared_ptr<giga::Client> c_empty = file_empty->open();

	REQUIRE(c_empty->read(buffer, 10) == 0);
	REQUIRE(buffer->compare("") == 0);

	file_empty->close(c_empty);

	std::shared_ptr<giga::File> file_five (new giga::File("test/files/five.txt", "ro", std::shared_ptr<giga::Config> (new giga::Config(2, 2, 1))));
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
