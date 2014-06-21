#define CATCH_CONFIG_RUNNER
#include "libs/catch/include/catch.hpp"

int main(int argc, char **argv) {
	int result = Catch::Session().run(argc, argv);
	return(result);
}
