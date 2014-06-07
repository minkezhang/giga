#include <memory>
#include <sys/stat.h>
#include <vector>

#include "src/file.h"

giga::File::File(std::string filename, std::string mode) {
	this->filename = filename;
	this->mode = mode;
	this->n_clients = 0;

	// load page table into memory
	struct stat stat_buf;
	int result = stat(filename.c_str(), &stat_buf);

	giga_size page_size = 16 * 1024;
	giga_size s = (result == 0) ? stat_buf.st_size : -1;
	for(giga_size global_pos = 0; global_pos < s; global_pos += page_size) {
		giga_size size = (s - global_pos > page_size) ? page_size : (s - global_pos);
		std::shared_ptr<giga::Block> block (new giga::Block(global_pos, size, this->head_block, NULL));
		this->head_block = block;
	}

	// reverse back up from end of file to the beginning
	if(this->head_block != NULL) {
		while(this->head_block->get_prev() != NULL) {
			this->head_block = this->head_block->get_prev();
		}
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
	this->lock_clients();
	giga_size result = 0;
	this->unlock_clients();
	return(result);
}

/**
 * set block_offset as if the current block_offset pointer = 0
 */
giga::giga_size giga::File::read(const std::shared_ptr<giga::Client>& client, std::string buffer, giga::giga_size n_bytes) {
	client->lock_client();
	std::shared_ptr<ClientInfo> info = this->client_list[client->get_id()];
	giga::giga_size n = info->get_block()->read(info->get_block_offset(), buffer, n_bytes);

	info->set_block_offset(info->get_block_offset() + n);
	client->unlock_client();
	return(n);
}

giga::giga_size giga::File::write(const std::shared_ptr<giga::Client>& client, std::string buffer) {
	client->lock_client();
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

void giga::File::save() {}

void giga::File::lock_clients() {
	this->client_list_lock.lock();
	for(unsigned int i = 0; i < this->client_list.size(); i++) { this->client_list.at(i)->get_client()->lock_client(); }
}

void giga::File::unlock_clients() {
	for(unsigned int i = 0; i < this->client_list.size(); i++) { this->client_list.at(i)->get_client()->unlock_client(); }
	this->client_list_lock.unlock();
}
