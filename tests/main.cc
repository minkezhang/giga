#define CATCH_CONFIG_RUNNER

#include <cstdlib>

#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

int main(int argc, char **argv) {
	if(system("make prep")) {
		throw(exceptionpp::RuntimeError("main", "could not prep the tests"));
	}

	int result = Catch::Session().run(argc, argv);
	return(result);
}
