#include <chrono>
#include <cstdio>
#include <errno.h>
#include <sstream>
#include <thread>

#include "libs/md5/md5.h"
#include "src/crypto.h"
#include "src/exception.h"

#include "src/block.h"

giga::Block::Block(giga::giga_size global_offset, size_t size, const std::shared_ptr<giga::Block>& prev, const std::shared_ptr<giga::Block>& next) {
	this->id = rand(); // cf. tmpnam()
	this->global_offset = global_offset;
	this->size = size;
	this->prev = prev;
	this->next = next;
	this->is_dirty = 0;
	this->is_loaded = 0;
	this->prev_lock = false;
	this->next_lock = false;
	this->data_lock = false;
	this->queue_lock = false;
	this->checksum = "";
}

/**
 * a client is requesting a read-write on the block
 */
void giga::Block::enqueue(int client_id, const std::shared_ptr<giga::ClientInfo>& client_info) {
	this->lock_queue();
	try {
		this->queue.at(client_id);
		this->unlock_queue();
		throw(giga::RuntimeError("giga::Block::enqueue", "double enqueue detected"));
	} catch (const std::out_of_range& e) {
		this->queue.insert(std::pair<int, std::shared_ptr<giga::ClientInfo>> (client_id, client_info));
	}
	this->unlock_queue();
}

void giga::Block::dequeue(int client_id, const std::shared_ptr<giga::ClientInfo>& client_info) {
	this->lock_queue();
	if(!this->queue.erase(client_id)) {
		this->unlock_queue();
		throw(giga::RuntimeError("giga::Block::dequeue", "client_id not found"));
	}
	this->unlock_queue();
}

void giga::Block::load(std::string filename, std::string mode) {
	this->is_loaded = 1;

	std::ostringstream path;
	if(this->is_dirty) {
		std::ostringstream buffer;
		buffer << filename << "_" << this->id;
		path << "/tmp/" << giga::Crypto::hash(buffer.str());
	} else {
		path << filename;
	}

	// load data into file
	FILE *fp = fopen(path.str().c_str(), mode.c_str());
	if(fseek(fp, this->global_offset, SEEK_SET) == -1) {
		throw(InvalidFileOperation(filename, "seek", ferror(fp)));
	}

	// data buffer is NOT null-terminated -- note that we could have made it so, by setting size to this->size + 1
	// because data buffer is not null-terminated, we must manually pass in the size of the buffer to this->data.assign()
	char *data = (char *) calloc(this->size, sizeof(char));

	if(fread((void *) data, sizeof(char), this->size, fp) < this->size) {
		throw(InvalidFileOperation(filename, "read", ferror(fp)));
	}
	fclose(fp);

	this->data.assign(std::string(data, this->size));

	if(this->is_dirty) {
		if(giga::Crypto::hash(this->data).compare(this->checksum)) {
			std::cout << this->data << std::endl;
			throw(RuntimeError("giga::Block::load", "data was corrupted between save and load"));
		}
	}

	free((void *) data);
}

void giga::Block::unload(std::string filename) {
	if(this->is_dirty) {
		std::ostringstream buffer;
		std::ostringstream path;
		buffer << filename << "_" << this->id;
		path << "/tmp/" << giga::Crypto::hash(buffer.str());

		// write to swap file
		FILE *fp = fopen(path.str().c_str(), "w");
		fputs(this->data.c_str(), fp);
		fclose(fp);

		this->checksum = giga::Crypto::hash(this->data);
	}

	this->is_loaded = 0;
	this->data.assign("");
	// free memory
	//	cf. http://bit.ly/1spckXq
	this->data.reserve(0);
}

giga::giga_size giga::Block::read(giga::giga_size start, const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	if(!this->is_loaded) {
		throw(giga::InvalidOperation("giga::Block::read", "attempted to read an unloaded block"));
	}

	size_t bytes_read = ((this->size - start) < (size_t) n_bytes) ? this->size - start : (size_t) n_bytes;
	buffer->append(this->data.substr(start, bytes_read));

	return(bytes_read);
}

giga::giga_size giga::Block::erase(giga::giga_size start, size_t len) {
	this->is_dirty = 1;
	this->global_offset = 0;
	this->data.erase(start, len);
	this->size -= len;
	return(len);
}

giga::giga_size giga::Block::append(const std::shared_ptr<std::string>& buffer) {
	this->is_dirty = 1;
	this->global_offset = 0;
	this->data.append(*buffer);
	this->size += buffer->length();
	return(buffer->length());
}

giga::giga_size giga::Block::write(giga::giga_size start, const std::shared_ptr<std::string>& buffer, bool is_insert) {
	this->is_dirty = 1;
	this->global_offset = 0;
	if(!is_insert) {
		this->data.replace(start, ((this->size - start) > buffer->length()) ? buffer->length() : (this->size - start), *buffer);
	} else {
		// no one can enqueue or dequeue at this time -- we have sole access to the client list (and by extension, the client info)
		this->lock_queue();
		this->data.insert(start, *buffer);
		this->size += buffer->length();
		// walk through queue and increment pointers
		for(std::map<int, std::shared_ptr<giga::ClientInfo>>::iterator i = this->queue.begin(); i != this->queue.end(); ++i) {
			if(i->second->get_block_offset() > start) {
				i->second->set_block_offset(i->second->get_block_offset() + buffer->length());
			}
		}
		this->unlock_queue();
	}
	return(buffer->length());
}

giga::giga_size giga::Block::get_id() { return(this->id); }
bool giga::Block::get_is_loaded() { return(this->is_loaded); }
bool giga::Block::get_is_dirty() { return(this->is_dirty); }

giga::giga_size giga::Block::get_size() { return(this->size); }

// linked list maintenance functions

/**
 * inserts a linked list into the current list
 */
void giga::Block::insert(const std::shared_ptr<giga::Block>& head, const std::shared_ptr<giga::Block>& tail) {
	this->lock_next();
	if(this->get_next_unsafe() != NULL) {
		this->get_next_unsafe()->lock_prev();
	}

	head->set_prev_unsafe(this->shared_from_this());
	tail->set_next_unsafe(this->get_next_unsafe());

	if(this->get_next_unsafe() != NULL) {
		this->get_next_unsafe()->set_prev_unsafe(tail);
	}
	this->set_next_unsafe(head);

	if(this->get_next_unsafe() != NULL) {
		this->get_next_unsafe()->unlock_prev();
	}
	this->unlock_next();
}

void giga::Block::lock_prev() { while(this->prev_lock.exchange(true)) {} }
void giga::Block::lock_next() { while(this->next_lock.exchange(true)) {} }
void giga::Block::lock_data() {
	while(this->data_lock.exchange(true)) {
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}
void giga::Block::lock_queue() { while(this->queue_lock.exchange(true)) {} }

void giga::Block::unlock_prev() {
	if(!this->prev_lock) {
		throw(giga::RuntimeError("giga::Block::unlock_prev", "double unlock detected"));
	}
	this->prev_lock = false;
}
void giga::Block::unlock_next() {
	if(!this->next_lock) {
		throw(giga::RuntimeError("giga::Block::unlock_next", "double unlock detected"));
	}
	this->next_lock = false;
}
void giga::Block::unlock_data() {
	if(!this->data_lock) {
		throw(giga::RuntimeError("giga::Block::unlock_data", "double unlock detected"));
	}
	this->data_lock = false;
}
void giga::Block::unlock_queue() {
	if(!this->queue_lock) {
		throw(giga::RuntimeError("giga::Block::unlock_queue", "double unlock detected"));
	}
	this->queue_lock = false;
}

std::shared_ptr<giga::Block> giga::Block::get_prev_unsafe() { return(this->prev); }
std::shared_ptr<giga::Block> giga::Block::get_next_unsafe() { return(this->next); }

std::shared_ptr<giga::Block> giga::Block::get_prev_safe() {
	this->lock_prev();
	std::shared_ptr<giga::Block> val = this->get_prev_unsafe();
	this->unlock_prev();
	return(val);
}

std::shared_ptr<giga::Block> giga::Block::get_next_safe() {
	this->lock_next();
	std::shared_ptr<giga::Block> val = this->get_next_unsafe();
	this->unlock_next();
	return(val);
}

void giga::Block::set_prev_unsafe(const std::shared_ptr<giga::Block>& prev) { this->prev = prev; }
void giga::Block::set_next_unsafe(const std::shared_ptr<giga::Block>& next) { this->next = next; }
