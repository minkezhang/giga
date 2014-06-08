#include <cstdio>
#include <errno.h>
#include <sstream>
#include <stdio.h>

#include "src/exception.h"

#include "src/block.h"

giga::Block::Block(giga::giga_size global_offset, size_t size, const std::shared_ptr<giga::Block>& prev, const std::shared_ptr<giga::Block>& next) {
	this->id = rand(); // cf. tmpnam
	this->global_offset = global_offset;
	this->size = size;
	this->prev = prev;
	this->next = next;
	this->is_dirty = 0;
	this->is_loaded = 0;
}

void giga::Block::load(std::string filename, std::string mode) {
	this->is_loaded = 1;
	std::ostringstream path;
	if(this->is_dirty) {
		path << "/tmp/.giga_" << filename << "_" << this->id;
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

void giga::Block::unload() {
	this->is_loaded = 0;
}

giga::giga_size giga::Block::read(giga::giga_size start, const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	this->block_lock.lock();
	std::shared_ptr<giga::Block> block = this->shared_from_this();
	while(n_bytes != 0) {
		size_t bytes_read = ((this->size - start) < (size_t) n_bytes) ? this->size - start : (size_t) n_bytes;
		buffer->append(this->data.substr(start, bytes_read));
		n_bytes -= bytes_read;
		if(block->next == NULL) { break; }
		block = block->next;
	}
	this->block_lock.unlock();
	return(buffer->length());
}

giga::giga_size giga::Block::write(giga::giga_size start, const std::shared_ptr<std::string>& buffer) {
	this->block_lock.lock();
	this->is_dirty = 1;
	this->global_offset = 0;
	this->block_lock.unlock();
	return(buffer->length());
}

int giga::Block::get_is_loaded() { return(this->is_loaded); }
int giga::Block::get_is_dirty() { return(this->is_dirty); }

giga::giga_size giga::Block::get_size() { return(this->size); }

std::shared_ptr<giga::Block> giga::Block::get_prev() { return(this->prev); }
std::shared_ptr<giga::Block> giga::Block::get_next() { return(this->next); }

void giga::Block::set_prev(const std::shared_ptr<giga::Block>& prev) { this->prev = prev; }
void giga::Block::set_next(const std::shared_ptr<giga::Block>& next) { this->next = next; }
