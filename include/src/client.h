#ifndef _CLIENT_H
#define _CLIENT_H

#include <memory>
#include <vector>

#include "src/global.h"

#include "src/file.h"

namespace giga {
	class File;
	class Client : public std::enable_shared_from_this<Client> {
		public:
			Client(const std::shared_ptr<File>& file, int id);

			int get_id();

			// get the current file pointer
			giga_size get_pos();

			giga_size seek(giga_size global_pos);

			giga_size read(std::string buffer, giga_size n_bytes);
			giga_size write(std::string buffer);

			void lock_client();
			void unlock_client();

		private:
			int id;
			std::shared_ptr<File> file;
			std::mutex client_lock;
	};
}

#endif
