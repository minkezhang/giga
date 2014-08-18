#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"

TEST_CASE("giga|file") {
	REQUIRE_THROWS_AS(giga::File("tests/files/nonexistent", "r"), exceptionpp::InvalidOperation);

	giga::File f = giga::File("tests/files/foo", "r", giga::Config(3, 4));
	REQUIRE(f.get_filename().compare("tests/files/foo") == 0);
	REQUIRE(f.get_mode().compare("r") == 0);
}

TEST_CASE("giga|file-read") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r"));
	std::shared_ptr<giga::Client> c = f->open();
}