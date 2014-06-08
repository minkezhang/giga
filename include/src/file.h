#ifndef _FILE_H
#define _FILE_H

/**
 * File abstraction for Giga -- main thread will call this to load the file instance giga_sizeo memory
 */

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "src/block.h"
#include "src/info.h"
#include "src/client.h"
#include "src/global.h"

namespace giga {
	class Client;
	class File : public std::enable_shared_from_this<File> {
		public:
			File(std::string filename, std::string mode);

			std::map<int, std::shared_ptr<giga::ClientInfo>> get_client_list();

			giga_size get_client_pos(const std::shared_ptr<Client>& client);

			giga_size seek(const std::shared_ptr<Client>& client, giga_size global_pos);

			giga_size read(const std::shared_ptr<Client>& client, const std::shared_ptr<std::string>& buffer, giga_size n_bytes);
			giga_size write(const std::shared_ptr<Client>& client, const std::shared_ptr<std::string>& buffer);

			std::shared_ptr<Client> open();
			void close(const std::shared_ptr<Client>& client);

			void save();

		private:
			int n_clients;

			std::string filename;
			std::string mode;

			std::mutex client_list_lock;

			std::map<int, std::shared_ptr<ClientInfo>> client_list;

			// data is represented by a linked list -- gives location of the first block of data
			std::shared_ptr<Block> head_block;

			void lock_clients();
			void unlock_clients();

			// list of blocks whose data is loaded
			// std::vector<std::shared_ptr<BlockInfo>> cache;
	};

}

#endif
