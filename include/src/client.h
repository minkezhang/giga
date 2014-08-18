#ifndef _GIGA_CLIENT_H
#define _GIGA_CLIENT_H

#include <memory>

#include "libs/cachepp/globals.h"

#include "src/file.h"
#include "src/page.h"

namespace giga {
	class File;

	class Client : public std::enable_shared_from_this<Client> {
		public:
			Client(cachepp::identifier id, const std::shared_ptr<File>& file);

			bool get_is_closed();
			void close();
			void open();

		private:
			cachepp::identifier id;
			std::shared_ptr<File> file;
			bool is_closed;

			void set_is_closed(bool is_closed);
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
