#ifndef _GIGA_CLIENT_H
#define _GIGA_CLIENT_H

#include <list>
#include <memory>
#include <mutex>

#include "libs/cachepp/globals.h"

#include "src/file.h"
#include "src/page.h"

namespace giga {
	class File;

	class Client : public std::enable_shared_from_this<Client> {
		public:
			Client(cachepp::identifier id, const std::shared_ptr<File>& file);

			cachepp::identifier get_identifier();
			bool get_is_closed();
			void close();
			void open();
			size_t get_pos();
			size_t seek(size_t len, bool is_forward);

		private:
			cachepp::identifier id;
			std::shared_ptr<File> file;
			bool is_closed;
			std::shared_ptr<std::recursive_mutex> l;

			void set_is_closed(bool is_closed);
	};
	class ClientData {
		public:
			ClientData(cachepp::identifier id);

			void set_file_offset(size_t file_offset);
			void set_page(std::list<std::shared_ptr<Page>>::iterator page);
			void set_page_offset(size_t page_offset);

			size_t get_file_offset();
			std::list<std::shared_ptr<Page>>::iterator get_page();
			size_t get_page_offset();

		private:
			cachepp::identifier id;
			size_t file_offset;
			std::list<std::shared_ptr<Page>>::iterator page;
			size_t page_offset;
	};
}

#endif
