#include <list>
#include <memory>

#include "libs/catch/catch.hpp"

#include "src/client.h"
#include "src/page.h"

TEST_CASE("giga|client") {
	giga::Client c = giga::Client(0, NULL);
	REQUIRE(c.get_is_closed() == false);
	c.close();
	REQUIRE(c.get_is_closed() == true);
	c.open();
	REQUIRE(c.get_is_closed() == false);

	REQUIRE(c.seek(100, true) == 0);
}

TEST_CASE("giga|clientdata") {
	giga::ClientData cd = giga::ClientData(0);
	std::list<std::shared_ptr<giga::Page>> list = std::list<std::shared_ptr<giga::Page>>();

	cd.set_page(list.begin());
	REQUIRE(cd.get_page() == list.begin());
	cd.set_page(list.end());
	REQUIRE(cd.get_page() == list.end());
}
