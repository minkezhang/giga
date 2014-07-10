#ifndef _CLIENT_H
#define _CLIENT_H

#include <atomic>
#include <memory>
#include <vector>

#include "src/exception.h"
#include "src/global.h"
#include "src/info.h"

#include "src/file.h"

namespace giga {
	class File;
	class ClientInfo;
	class BlockInfo;
	/**
	 * client is a linked list
	 */
	class Client : public std::enable_shared_from_this<Client> {
		public:
			Client(const std::shared_ptr<File>& file, const std::shared_ptr<ClientInfo>& client_info, const std::shared_ptr<Client>& next, int id);

			int get_id();

			// get the current file pointer
			giga_size get_pos();

			giga_size seek(giga_size global_pos);

			giga_size read(const std::shared_ptr<std::string>& buffer, giga_size n_bytes);
			giga_size write(const std::shared_ptr<std::string>& buffer);

			void lock_client();
			void unlock_client();

			bool get_is_closed();
			void set_is_closed();

			std::shared_ptr<ClientInfo> get_client_info();

			std::shared_ptr<Client> get_next_unsafe();
			void set_next_unsafe(const std::shared_ptr<Client>& next);

			void insert(const std::shared_ptr<Client>& head);
			void erase(const std::shared_ptr<Client>& target);

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
