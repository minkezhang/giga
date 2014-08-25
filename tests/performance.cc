#ifdef _GIGA_PERFORMANCE_TESTS

#include <iostream>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/performance.h"

TEST_CASE("giga|performance-result") {
	giga::Result r = giga::Result();

	REQUIRE_THROWS_AS(r.to_string(), exceptionpp::InvalidOperation);
	r.push_back("EGR", 1000, 1000, 1000, 1000, 1000, 10, 1024, 4);
	std::cout << r;
}

#endif
