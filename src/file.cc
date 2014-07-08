#include <memory>
#include <sys/stat.h>
#include <vector>

#include <iostream>
#include <thread>
#include <unistd.h>

#include "src/exception.h"

#include "src/file.h"

giga::File::File(std::string filename, std::string mode, const std::shared_ptr<giga::Config>& config) {
	this->filename = filename;
	this->mode = mode;
	this->n_clients = 0;
	this->n_cache_entries = config->get_n_cache_entries();
	for(size_t i = 0; i < this->n_cache_entries; i++) {
		this->cache_entry_locks.push_back(std::shared_ptr<std::mutex> (new std::mutex()));
	}

	FILE *fp = fopen(filename.c_str(), mode.c_str());
	if(fp == NULL) {
		throw(giga::FileNotFound(this->filename));
	} else {
		fclose(fp);
	}

	// load page table into memory
	struct stat stat_buf;

	int result = stat(filename.c_str(), &stat_buf);

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

std::map<int, std::shared_ptr<giga::ClientInfo>> giga::File::get_client_list() { return(this->client_list); }

/**
 * get the client global position
 * lock the client list while doing so
 */
giga::giga_size giga::File::get_client_pos(const std::shared_ptr<giga::Client>& client) {
	// cannot make changes to the document while calculating global position
	client->lock_client();

	if(client->get_is_closed()) {
		client->unlock_client();
		throw(giga::InvalidOperation("giga::File::read", "attempting to read from a closed client"));
	}

	this->cache_lock.lock();

	for(size_t i = 0; i < this->n_cache_entries; i++) {
		this->cache_entry_locks.at(i)->lock();
	}

	// minke
	if(this->client_list[client->get_id()] == 0x00) {
		throw(giga::InvalidOperation("giga::File::read", "client_list cannot find client with given id"));
	}
	giga_size result = this->client_list[client->get_id()]->get_global_position();

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
	if(this->head_block == NULL) {
		client->unlock_client();
		return(0);
	}

	std::shared_ptr<ClientInfo> info = this->client_list[client->get_id()];

	// EOF check
	if(info->get_block_offset() == info->get_block()->get_size()) {
		client->unlock_client();
		return(0);
	}

	giga::giga_size n = 0;
	giga::giga_size offset = 0;

	while(n < n_bytes) {
		// backup copy of the block being referenced in the cache
		std::shared_ptr<giga::Block> block = info->get_block();

		try {
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->lock();
			this->cache.at(block->get_id());
		} catch(const std::out_of_range& e) {
			// this->allocate() automatically locks the cache line
			this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
			this->allocate(block);
		}

		try {
			this->cache.at(block->get_id())->increment();
		} catch(const std::out_of_range& e) {
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
			break;
		}

		this->cache_entry_locks.at(block->get_id() % this->n_cache_entries)->unlock();
		block.reset();
	}

	client->unlock_client();
	return(n);
}

giga::giga_size giga::File::write(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<std::string>& buffer) {
	throw(giga::NotImplemented("giga::File::write"));
/*
	client->lock_client();
	std::shared_ptr<giga::Block> block = this->client_list[client->get_id()]->get_block();

	if(!block->get_is_loaded()) {
		this->allocate(block);
	}

	std::shared_ptr<ClientInfo> info = this->client_list[client->get_id()];
	giga::giga_size n = info->get_block()->write(info->get_block_offset(), buffer);

	info->set_block_offset(info->get_block_offset() + n);
	client->unlock_client();
*/
	return(0);
}

std::shared_ptr<giga::Client> giga::File::open() {
	this->client_list_lock.lock();

	std::shared_ptr<giga::Client> c;
	c = std::shared_ptr<giga::Client> (new giga::Client(this->shared_from_this(), this->n_clients));
	std::shared_ptr<giga::ClientInfo> c_info (new giga::ClientInfo(c, this->head_block));
	this->client_list[this->n_clients] = c_info;
	c_info.reset();

	this->n_clients++;
	this->client_list_lock.unlock();
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	this->client_list_lock.lock();
	client->lock_client();

	client->set_is_closed();
	// if we use this, we can simplify File::get_n_clients to check for client_list.size()
	// this->client_list.erase(client->get_id());

	client->unlock_client();
	this->client_list_lock.unlock();
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
	this->client_list_lock.lock();
	for(std::map<int, std::shared_ptr<giga::ClientInfo>>::iterator i = this->client_list.begin(); i != this->client_list.end(); ++i) {
		i->second->get_client()->lock_client();
	}
}

void giga::File::unpause() {
	for(std::map<int, std::shared_ptr<giga::ClientInfo>>::iterator i = this->client_list.begin(); i != this->client_list.end(); ++i) {
		i->second->get_client()->unlock_client();
	}
	this->client_list_lock.unlock();
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

int giga::File::get_n_clients() {
	int n = 0;
	this->client_list_lock.lock();

	for(std::map<int, std::shared_ptr<ClientInfo>>::iterator i = this->client_list.begin(); i != this->client_list.end(); ++i) {
		n += !i->second->get_client()->get_is_closed();
	}

	this->client_list_lock.unlock();
	return(n);
}
