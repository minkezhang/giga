#ifndef _GIGA_CLIENT_H
#define _GIGA_CLIENT_H

#include "libs/cachepp/globals.h"

#include "src/file.h"
#include "src/page.h"

namespace giga {
	class Client {
		public:
			Client();

		private:
			cachepp::identifier id;
	};
	class ClientData {
		public:
			ClientData();

		private:
			cachepp::identifier id;
			std::shared_ptr<Page> page;
			size_t global_position;
	};
}

#endif
