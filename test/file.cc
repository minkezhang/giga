#include "catch/include/catch.hpp"

#include <memory>

#include "src/file.h"

TEST_CASE("file|initialization") {
	std::shared_ptr<giga::File> file (new giga::File("file_open"));
	std::shared_ptr<giga::Client> c = file->open();
	REQUIRE(c->get_pos() == 0);
}
