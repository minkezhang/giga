#define CATCH_CONFIG_RUNNER

#include <cstdlib>

#include "libs/catch/catch.hpp"

int main(int argc, char **argv) {
	int i = system("make prep");
	int result = Catch::Session().run(argc, argv);
	return(result);
}
