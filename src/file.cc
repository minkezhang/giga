#include <memory>
#include <sys/stat.h>
#include <vector>

#include "src/exception.h"

#include "src/file.h"

giga::File::File(std::string filename, std::string mode, const std::shared_ptr<giga::Config>& config) {
	this->filename = filename;
	if(!mode.compare("ro")) {
		this->mode = "r";
	} else if(!mode.compare("rw")) {
		this->mode = "r+";
	} else if(!mode.compare("wo")) {
		this->mode = "w";
	} else {
		throw(giga::InvalidOperation("giga::File::File", "invalid file mode"));
	}
	this->n_clients = 0;
	this->n_opens = 0;

	this->n_cache_entries = config->get_n_cache_entries();

	for(size_t i = 0; i < this->n_cache_entries; i++) {
		this->cache_entry_locks.push_back(std::shared_ptr<std::mutex> (new std::mutex()));
	}

	this->head_client = NULL;

	FILE *fp = fopen(filename.c_str(), mode.c_str());
	if(fp == NULL && this->mode.compare("r+")) {
		throw(giga::FileNotFound(this->filename));
	// create new file if the mode specifies it
	} else if(!this->mode.compare("r+")) {
		fp = fopen(filename.c_str(), "w");
	}
	fclose(fp);

	// load page table into memory
	struct stat stat_buf;

	int result = stat(filename.c_str(), &stat_buf);
	this->max_page_size = config->get_max_page_size();
	size_t page_size = config->get_page_size();

	giga_size s = (result == 0) ? stat_buf.st_size : -1;

	this->head_block = NULL;
	for(giga_size global_pos = 0; global_pos < s; global_pos += page_size) {
		size_t size = (s - global_pos > (giga_size) page_size) ? page_size : (s - global_pos);
		std::shared_ptr<giga::Block> block (new giga::Block(global_pos, size, this->head_block, NULL));
			if(this->head_block != NULL) {
				this->head_block->set_next_unsafe(block);
			}
		this->head_block = block;
		// release unused references
		block.reset();
	}

	// reverse back up from end of file to the beginning
	if(this->head_block != NULL) {
		while(this->head_block->get_prev_safe() != NULL) {
			this->head_block = this->head_block->get_prev_safe();
		}
	}
}

giga::File::File(std::string filename, std::string mode) : giga::File::File(filename, mode, std::shared_ptr<giga::Config> (new giga::Config())) {}

giga::File::~File() {
	while(this->head_block) {
		if(this->head_block->get_prev_safe()) {
			this->head_block->get_prev_safe()->get_next_safe().reset();
			this->head_block->get_prev_safe().reset();
		}
		this->head_block = this->head_block->get_next_safe();
	}
}

std::shared_ptr<giga::Client> giga::File::get_client_list() { return(this->head_client); }

/**
 * get the client global position
 * lock the client list while doing so
 */
giga::giga_size giga::File::get_client_pos(const std::shared_ptr<giga::Client>& client) {
	// cannot make changes to the document while calculating global position
	client->lock_client();

	if(client->get_is_closed()) {
		client->unlock_client();
		throw(giga::InvalidOperation("giga::File::get_client_pos", "attempting to read from a closed client"));
	}

	this->cache_lock.lock();

	for(size_t i = 0; i < this->n_cache_entries; i++) {
		this->cache_entry_locks.at(i)->lock();
	}

	giga_size result = client->get_client_info()->get_global_position();

	for(size_t i = 0; i < this->n_cache_entries; i++) {
		this->cache_entry_locks.at(i)->unlock();
	}

	this->cache_lock.unlock();
	client->unlock_client();

	return(result);
}

/**
 * set the client block and blockoffset that corresponds to the global pos input
 * lock client list while doing so
 */
giga::giga_size giga::File::seek(const std::shared_ptr<giga::Client>& client, giga_size global_pos) {
	throw(giga::NotImplemented("giga::File::seek"));
	/*
	this->pause();
	giga_size result = 0;
	this->unpause();
	return(result);
	*/
	return(0);
}

/**
 * lock the block(s) for data editing -- no other read/writes can occur at the same time on these blocks
 * ensures that the block pointed to by ClientInfo::get_block can only be edited by the given client
 *
 * TODO -- change to File::acquire_block(client, len), and acquire all data locks given by length
 */
void giga::File::acquire_block(const std::shared_ptr<Client>& client, giga::giga_size n_bytes) {
	bool success = false;
	std::shared_ptr<Block> block;
	while(!success) {
		block = client->get_client_info()->get_block();
		try {
			/**
			 * see if the block reference has changed for this client during the time taken to acquire
			 *	the block cache line lock
			 *
			 * on call to Block::insert, the given block will DELETE from its queue all clients
			 *	whose block reference has changed, and will call ClientInfo::set_block to set the new block reference;
			 *	this tests for that and tries again
			 */
			block->lock_data();
			block->dequeue(client->get_id(), client->get_client_info());
			success = true;
		} catch(const giga::RuntimeError& e) {
			block->unlock_data();
			// put in client request
			block->enqueue(client->get_id(), client->get_client_info());
		}
	}
	giga::giga_size n = 0;
	while((block != NULL) && (n < (n_bytes + client->get_client_info()->get_block_offset()))) {
		if(block->get_id() != client->get_client_info()->get_block()->get_id()) {
			block->lock_data();
		}
		n += (block->get_size() > (n_bytes - n)) ? (n_bytes - n) : block->get_size();
		block = block->get_next_safe();
	}
}


/**
 * set block_offset as if the current block_offset pointer = 0
 *
 * resets buffer to ""
 */
giga::giga_size giga::File::read(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	// client reads sequentially
	client->lock_client();
	if(client->get_is_closed()) {
		client->unlock_client();
		throw(giga::InvalidOperation("giga::File::read", "attempting to read from a closed client"));
	}

	buffer->assign("");
	if((this->head_block == NULL) || (n_bytes == 0)) {
		client->unlock_client();
		return(0);
	}

	std::shared_ptr<giga::ClientInfo> info = client->get_client_info();

	giga::giga_size n = 0;
	giga::giga_size offset = 0;

	this->acquire_block(client, n_bytes);

	while(n < n_bytes) {
		// backup copy of the block being referenced in the cache
		std::shared_ptr<giga::Block> block = info->get_block();

		// lock the approprate cache block -- allocate the block if the block is not in cache
		try {
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->lock();
			this->cache.at(block->get_id());
		} catch(const std::out_of_range& e) {
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
			// side effect of blocking the block cache line here
			this->allocate(block);
		}

		// EOF check
		if(info->get_block_offset() == block->get_size()) {
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
			block->unlock_data();
			break;
		}

		try {
			this->cache.at(block->get_id())->increment();
		} catch(const std::out_of_range& e) {
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
			block->unlock_data();
			client->unlock_client();
			throw(giga::RuntimeError("giga::File::read", "std::out_of_range thrown while incrementing block access count"));
		}

		offset = block->read(info->get_block_offset(), buffer, (n_bytes - n));
		n += offset;

		// a read ended within the same block
		if(info->get_block_offset() + offset < block->get_size()) {
			info->set_block_offset(info->get_block_offset() + offset);
		// a read ended outside the starting block
		} else if(block->get_next_safe() != NULL) {
			info->set_block(block->get_next_safe());
			info->set_block_offset(0);
		// set EOF
		} else {
			info->set_block_offset(info->get_block_offset() + offset);
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
			block->unlock_data();
			break;
		}

		this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
		block->unlock_data();
	}

	client->unlock_client();

	return(n);
}

giga::giga_size giga::File::erase(const std::shared_ptr<giga::Client>& client, size_t len) {
	throw(giga::NotImplemented("giga::File::erase"));
	return(0);
}

giga::giga_size giga::File::write(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<std::string>& buffer, bool is_insert) {
	throw(giga::NotImplemented("giga::File::write"));
	return(0);
}

std::shared_ptr<giga::Client> giga::File::open() {
	std::shared_ptr<giga::ClientInfo> c_info (new giga::ClientInfo(this->head_block));
	std::shared_ptr<giga::Client> c (new giga::Client(this->shared_from_this(), c_info, NULL, this->n_opens.fetch_add(1)));
	// set block reference on the block side
	if(this->head_block != NULL) {
		this->head_block->enqueue(c->get_id(), c->get_client_info());
	}
	if(this->head_client == NULL) {
		// this is clumsy -- fix it
		this->client_list_lock.lock();
		if(this->head_client == NULL) {
			this->head_client = c;
		} else {
			this->head_client->insert(c);
		}
		this->client_list_lock.unlock();
	} else {
		this->head_client->insert(c);
	}

	c_info.reset();

	this->n_clients++;
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	this->head_client->remove(client);
	this->n_clients--;
}

void giga::File::save() {
	throw(giga::NotImplemented("giga::File::save"));
}

/**
 * lock clients in an orderly fashion -- await any clients currently holding the lock
 *
 * prevents arbitrary deadlocking
 *
 * "stop-the-world" updates need to call this (e.g. File::save())
 */
void giga::File::pause() {
	throw(giga::NotImplemented("giga::File::pause"));
}

void giga::File::unpause() {
	throw(giga::NotImplemented("giga::File::unpause"));
}

/**
 * loads required block and acquires the cache line lock
 *
 * caller must manually unlock the cache line
 */
void giga::File::allocate(const std::shared_ptr<giga::Block>& block) {
	this->cache_lock.lock();

	// lock before making any changes -- this way, prevent access by read() and write() until lock is released
	size_t block_cache_pos = block->get_id() % this->n_cache_entries;
	this->cache_entry_locks.at(block_cache_pos)->lock();

	/**
	 * possible that another client has requested to load the same block --
	 *	ensures that the block isn't already loaded before proceeding
	 */
	try {
		this->cache.at(block->get_id());
	} catch(const std::out_of_range& e) {
		if(this->cache.size() > this->n_cache_entries) {
			// select block to unload
			giga::giga_size cur_min = 0;
			giga::giga_size target = 0;
			for(std::map<giga::giga_size, std::shared_ptr<giga::BlockInfo>>::iterator it = this->cache.begin(); it != this->cache.end(); ++it) {
				// finish all pending read and writes on the block
				size_t iterator_id = it->second->get_block()->get_id() % this->n_cache_entries;
				if(iterator_id != block_cache_pos) { this->cache_entry_locks.at(iterator_id)->lock(); }
				if((it->second->get_n_access() < cur_min) || (cur_min == 0)) {
					cur_min = it->second->get_n_access();
					target = it->first;
				} else if(it->second->get_n_access() == 0) {
					target = it->first;
					if(iterator_id != block_cache_pos) { this->cache_entry_locks.at(iterator_id)->unlock(); }
					break;
				/**
				 * prevent run-away accesses
				 *	cf. http://bit.ly/1n6cYZD (second-chance algorithm)
				 */
				} else {
					it->second->decrement();
				}
				if(iterator_id != block_cache_pos) { this->cache_entry_locks.at(iterator_id)->unlock(); }
			}
			if(target % this->n_cache_entries != block_cache_pos) { this->cache_entry_locks.at(target % this->n_cache_entries)->lock(); }
			this->cache.at(target)->get_block()->unload(this->filename);
			this->cache.erase(target);
			if(target % this->n_cache_entries != block_cache_pos) { this->cache_entry_locks.at(target % this->n_cache_entries)->unlock(); }
		}

		block->load(this->filename, this->mode);
		this->cache.insert(std::pair<int, std::shared_ptr<giga::BlockInfo>> (block->get_id(), std::shared_ptr<giga::BlockInfo> (new giga::BlockInfo(block))));
	}

	this->cache_lock.unlock();
}

int giga::File::get_n_clients() { return(this->n_clients); }
