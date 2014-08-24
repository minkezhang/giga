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
			Client(cachepp::identifier id, const std::shared_ptr<File>& file, std::string mode);

			cachepp::identifier get_identifier();
			bool get_is_closed();
			void close();
			void open();
			size_t get_pos();
			size_t seek(size_t len, bool is_forward, bool is_absolute = false);
			std::string read(size_t len);
			size_t write(std::string buffer, bool is_insert = false);
			size_t erase(size_t len);
			void save();

			std::string get_mode();

			const static uint8_t read_only = 1;
			const static uint8_t write_only = 2;

		private:
			cachepp::identifier id;
			std::shared_ptr<File> file;
			bool is_closed;
			std::shared_ptr<std::recursive_mutex> l;
			uint8_t mode;

			void set_is_closed(bool is_closed);
			void set_mode(std::string mode);
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
