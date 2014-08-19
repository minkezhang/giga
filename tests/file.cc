#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include <iostream>

#include "src/file.h"

TEST_CASE("giga|file") {
	REQUIRE_THROWS_AS(giga::File("tests/files/nonexistent", "r"), exceptionpp::InvalidOperation);

	giga::File f = giga::File("tests/files/foo", "r", giga::Config(3, 4));
	REQUIRE(f.get_filename().compare("tests/files/foo") == 0);
	REQUIRE(f.get_mode().compare("r") == 0);
}

TEST_CASE("giga|file-open") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r"));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->get_pos() == 0);
	c->close();
}

TEST_CASE("giga|file-seek") {
	std::cout << "file-seek" << std::endl;
	std::shared_ptr<giga::File> f (new giga::File("tests/files/foo", "r"));
	std::shared_ptr<giga::Client> c = f->open();
	std::cout << "seek == 2:" << std::endl;
	REQUIRE(c->seek(2, true) == 2);
	std::cout << "pos == 2:" << std::endl;
	REQUIRE(c->get_pos() == 2);
	std::cout << "seek == 2:" << std::endl;
	REQUIRE(c->seek(2, false) == 2);
	std::cout << "pos == 0:" << std::endl;
	REQUIRE(c->get_pos() == 0);
	std::cout << "seek == 100:" << std::endl;
	REQUIRE(c->seek(100, true) == 4);
	REQUIRE(c->get_pos() == 4);
	REQUIRE(c->seek(100, false) == 4);
	REQUIRE(c->get_pos() == 0);
	c->close();
}

TEST_CASE("giga|file-read") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r"));
	std::shared_ptr<giga::Client> c = f->open();
	c->close();
}
