#include <cstdio>
#include <errno.h>
#include <sstream>

#include <iostream>

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

giga::giga_size giga::Block::write(giga::giga_size start, const std::shared_ptr<std::string>& buffer) {
	throw(giga::NotImplemented("giga::Block::write"));

	this->is_dirty = 1;
	this->global_offset = 0;
	return(buffer->length());
}

giga::giga_size giga::Block::get_id() { return(this->id); }
int giga::Block::get_is_loaded() { return(this->is_loaded); }
int giga::Block::get_is_dirty() { return(this->is_dirty); }

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

	head->set_prev_unsafe(shared_from_this());
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
void giga::Block::unlock_prev() { this->prev_lock = false; }
void giga::Block::unlock_next() { this->next_lock = false; }

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
