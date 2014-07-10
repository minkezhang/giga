#ifndef _CLIENT_H
#define _CLIENT_H

#include <atomic>
#include <memory>
#include <vector>

#include "src/exception.h"
#include "src/global.h"
#include "src/info.h"

#include "src/file.h"

/**
 * Client::Client is the interface by which all user commands MUST flow throw
 *
 * the provided API will allow a user to concurrently edit the file, initialzed with the File::File API call
 * new Client::Client instances are instantiated by a call to File::open
 */
namespace giga {
	class File;
	class ClientInfo;
	class BlockInfo;
	/**
	 * client is a linked list
	 */
	class Client : public std::enable_shared_from_this<Client> {
		public:
			/**
			 * the API functions are guaranteed to be atomic and MUST be called without explicitly locking the client
			 *	these functions MAY be called by the user
			 */
			int get_id();
			giga_size get_pos();
			giga_size seek(giga_size global_pos);
			giga_size read(const std::shared_ptr<std::string>& buffer, giga_size n_bytes);
			giga_size write(const std::shared_ptr<std::string>& buffer, bool is_insert);
			giga_size erase(size_t len);
			bool get_is_closed();

			/**
			 * these functions MUST NOT be called by the user
			 * Client::get_client_info exposes several internal workings of the library,
			 *	keeps track of client metadata, and is
			 *	called by File::read and File::write
			 * Client::get_next_unsafe is called by File::open and File::close
			 * Client::set_next_unsafe is called by File::open and File::close
			 * Client::insert is called by File::open
			 * Client::remove is called by File::close
			 */
			Client(const std::shared_ptr<File>& file, const std::shared_ptr<ClientInfo>& client_info, const std::shared_ptr<Client>& next, int id);
			std::shared_ptr<ClientInfo> get_client_info();
			std::shared_ptr<Client> get_next_unsafe();
			void set_next_unsafe(const std::shared_ptr<Client>& next);
			void insert(const std::shared_ptr<Client>& head);
			void remove(const std::shared_ptr<Client>& target);
			void lock_client();
			void unlock_client();
			void set_is_closed();

		private:
			std::atomic<int> id;
			std::atomic<bool> is_closed;
			std::shared_ptr<File> file;
			std::shared_ptr<Client> next;
			std::shared_ptr<ClientInfo> client_info;
			std::mutex client_lock;
	};
}

#endif
