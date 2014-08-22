#include <list>
#include <memory>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/client.h"
#include "src/page.h"

TEST_CASE("giga|client") {
	giga::Client c = giga::Client(0, NULL, "");
	REQUIRE(c.get_is_closed() == false);
	c.close();
	REQUIRE(c.get_is_closed() == true);
	c.open();
	REQUIRE(c.get_is_closed() == false);

	REQUIRE(c.seek(100, true) == 0);

	REQUIRE(c.get_mode().compare("") == 0);

	REQUIRE_THROWS_AS(c.read(10), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(c.write("foo"), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(c.erase(10), exceptionpp::InvalidOperation);
}

TEST_CASE("giga|clientdata") {
	giga::ClientData cd = giga::ClientData(0);
	std::list<std::shared_ptr<giga::Page>> list = std::list<std::shared_ptr<giga::Page>>();

	cd.set_page(list.begin());
	REQUIRE(cd.get_page() == list.begin());
	cd.set_page(list.end());
	REQUIRE(cd.get_page() == list.end());
}
