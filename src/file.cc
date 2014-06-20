#include <memory>
#include <sys/stat.h>
#include <vector>

#include <iostream>

#include "src/exception.h"
#include "src/file.h"

giga::File::File(std::string filename, std::string mode, const std::shared_ptr<giga::Config>& config) {
	this->filename = filename;
	this->mode = mode;
	this->n_clients = 0;

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
			this->head_block->set_next(block);
		}
		this->head_block = block;
		// release unused references
		block.reset();
	}

	// reverse back up from end of file to the beginning
	if(this->head_block != NULL) {
		while(this->head_block->get_prev() != NULL) {
			this->head_block = this->head_block->get_prev();
		}
	}
}

giga::File::File(std::string filename, std::string mode) : giga::File::File(filename, mode, std::shared_ptr<giga::Config> (new giga::Config())) {}

giga::File::~File() {
	while(this->head_block) {
		if(this->head_block->get_prev()) {
			this->head_block->get_prev()->get_next().reset();
			this->head_block->get_prev().reset();
		}
		this->head_block = this->head_block->get_next();
	}
}

std::map<int, std::shared_ptr<giga::ClientInfo>> giga::File::get_client_list() { return(this->client_list); }

/**
 * get the client global position
 * lock the client list while doing so
 */
giga::giga_size giga::File::get_client_pos(const std::shared_ptr<giga::Client>& client) {
	this->lock_clients();
	giga_size result = this->client_list[client->get_id()]->get_global_position();
	this->unlock_clients();
	return(result);
}

/**
 * set the client block and blockoffset that corresponds to the global pos input
 * lock client list while doing so
 */
giga::giga_size giga::File::seek(const std::shared_ptr<giga::Client>& client, giga_size global_pos) {
	throw(giga::NotImplemented("giga::File::seek"));
	this->lock_clients();
	giga_size result = 0;
	this->unlock_clients();
	return(result);
}

/**
 * set block_offset as if the current block_offset pointer = 0
 *
 * resets buffer to ""
 */
giga::giga_size giga::File::read(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	client->lock_client();

	buffer->assign("");
	if(this->head_block == NULL) {
		return(0);
	}

	std::shared_ptr<ClientInfo> info = this->client_list[client->get_id()];

	// EOF check
	if(info->get_block_offset() == info->get_block()->get_size()) {
		return(0);
	}

	giga::giga_size n = 0;
	giga::giga_size offset = 0;

	while(n < n_bytes) {
		if(!info->get_block()->get_is_loaded()) {
			try {
				this->allocate(info->get_block());
			} catch(giga::ToyFunction& e) {
				std::cout << e.what() << std::endl;
			}
		}

		offset = info->get_block()->read(info->get_block_offset(), buffer, (n_bytes - n));
		n += offset;

		// a read ended within the same block
		if(info->get_block_offset() + offset < info->get_block()->get_size()) {
			info->set_block_offset(info->get_block_offset() + offset);
		// a read ended outside the starting block
		} else if(info->get_block()->get_next() != NULL) {
			info->set_block(info->get_block()->get_next());
			info->set_block_offset(0);
		// set EOF
		} else {
			info->set_block_offset(info->get_block_offset() + offset);
			break;
		}
	}

	client->unlock_client();
	return(n);
}

giga::giga_size giga::File::write(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<std::string>& buffer) {
	throw(giga::NotImplemented("giga::File::write"));

	client->lock_client();
	std::shared_ptr<giga::Block> block = this->client_list[client->get_id()]->get_block();

	if(!block->get_is_loaded()) {
		this->allocate(block);
	}

	std::shared_ptr<ClientInfo> info = this->client_list[client->get_id()];
	giga::giga_size n = info->get_block()->write(info->get_block_offset(), buffer);

	info->set_block_offset(info->get_block_offset() + n);
	client->unlock_client();
	return(n);
}

std::shared_ptr<giga::Client> giga::File::open() {
	std::shared_ptr<giga::Client> c (new giga::Client(this->shared_from_this(), this->n_clients++));
	std::shared_ptr<giga::ClientInfo> c_info (new giga::ClientInfo(c, this->head_block));
	this->client_list_lock.lock();
	this->client_list[c->get_id()] = c_info;
	this->client_list_lock.unlock();
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	this->client_list_lock.lock();
	this->client_list.erase(client->get_id());
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
void giga::File::lock_clients() {
	this->client_list_lock.lock();
	for(unsigned int i = 0; i < this->client_list.size(); i++) { this->client_list.at(i)->get_client()->lock_client(); }
}

void giga::File::unlock_clients() {
	for(unsigned int i = 0; i < this->client_list.size(); i++) { this->client_list.at(i)->get_client()->unlock_client(); }
	this->client_list_lock.unlock();
}

void giga::File::allocate(const std::shared_ptr<giga::Block>& block) {
	// lock
	// select block to unload
	block->load(this->filename, this->mode);
	throw(giga::ToyFunction("giga::File::allocate"));
}
