#ifndef _GIGA_FILE_H
#define _GIGA_FILE_H

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "libs/cachepp/simpleserialcache.h"

#include "src/client.h"
#include "src/page.h"

namespace giga {
	typedef size_t size;

	class Client;
	class ClientData;

	class Config {
		public:
			Config(size_t i_page_size, size_t m_page_size);
			size_t get_i_page_size();
			size_t get_m_page_size();

		private:
			size_t i_page_size;
			size_t m_page_size;
	};

	class File : public std::enable_shared_from_this<File> {
		public:
			File(std::string filename, std::string mode, Config config = Config(1024, 1024));

			/**
			 * returns the string read from the file
			 */
			std::string r(const std::shared_ptr<Client>& client, size_t len);

			/**
			 * deletes next len-bytes
			 */
			size_t d(const std::shared_ptr<Client>& client, size_t len);

			/**
			 * writes the buffer to the file
			 */
			size_t w(const std::shared_ptr<Client>& client, std::string val);

			/**
			 * insert buffer to the file
			 */
			size_t i(const std::shared_ptr<Client>& client, std::string val);

			/**
			 * seek op -- returns global position
			 */
			size_t s(const std::shared_ptr<Client>& client, size_t len, bool is_forward);

			std::shared_ptr<Client> open(const std::shared_ptr<giga::Client>& client = NULL);
			void close(const std::shared_ptr<Client>& client);
			void save();

			/**
			 * read-only properties
			 */
			std::string get_filename();
			std::string get_mode();

		private:
			std::string filename;
			std::string mode;

			size_t size;

			// increment every time a client or page is created -- guarantees unique client / page ids
			size_t c_count;
			size_t p_count;

			Config config;

			std::list<std::shared_ptr<Page>> pages;
			std::list<std::shared_ptr<Client>> clients;
			std::map<cachepp::identifier, std::shared_ptr<ClientData>> lookaside;

			std::shared_ptr<cachepp::SimpleSerialCache<Page>> cache;

			// because I'm stupid and can't figure out fine-grained locking on doubly-linked lists
			std::shared_ptr<std::recursive_mutex> l;
	};
}

#endif
