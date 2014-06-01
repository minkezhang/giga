#define CATCH_CONFIG_RUNNER
#include "catch/include/catch.hpp"

unsigned int Factorial(unsigned int number) {
	return number < 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("test/main_test_1", "[factorial]") {
	REQUIRE(Factorial(1) == 1);
}
TEST_CASE("test/main_test_2", "[factorial]") {
	REQUIRE(Factorial(1) == 1);
}

int main(int argc, char **argv) {
	int result = Catch::Session().run(argc, argv);
	return(result);
}
