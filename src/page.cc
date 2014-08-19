#include <sstream>
#include <string>
#include <vector>

#include "libs/cachepp/globals.h"
#include "libs/exceptionpp/exception.h"
#include "libs/md5/md5.h"

#include "src/page.h"

giga::Page::Page(cachepp::identifier id, std::string filename, size_t file_offset, size_t size, bool is_dirty) : cachepp::LineInterface<std::string>::LineInterface(id), filename(filename), file_offset(file_offset), size(size) {
	this->is_dirty = is_dirty;
}

size_t giga::Page::get_size() { return(this->size); }
void giga::Page::set_size(size_t size) { this->size = size; }

size_t giga::Page::probe(size_t offset, size_t len, bool is_forward) {
	if(offset > this->get_size()) {
		throw(exceptionpp::InvalidOperation("giga::Page::probe", "offset is invalid"));
	}
	if(is_forward) {
		return((this->get_size() - offset) > len ? len : (this->get_size() - offset));
	} else {
		return(offset > len ? len : offset);
	}
}

void giga::Page::set_filename(std::string filename) { this->filename = filename; }

std::string giga::Page::get_filename() {
	if(this->is_dirty) {
		std::stringstream path;
		std::stringstream buf;
		buf << filename << "_" << this->get_identifier();
		path << "/tmp/" << md5(buf.str());
		return(path.str());
	} else {
		return(this->filename);
	}
}

void giga::Page::set_file_offset(size_t file_offset) { this->file_offset = file_offset; }

void giga::Page::aux_load() {
	this->data.clear();

	// load data into the page
	FILE *fp = fopen(this->get_filename().c_str(), "r");
	if(fseek(fp, this->file_offset, SEEK_SET) == -1) {
		throw(exceptionpp::RuntimeError("giga::Page::aux_load", "invalid result returned from fseek"));
	}

	// data buffer is NOT null-terminated -- note that we could have made it so, by setting size to this->size + 1
	// because data buffer is not null-terminated, we must manually pass in the size of the buffer to this->data.assign()
	char *data = (char *) calloc(this->get_size(), sizeof(char));

	if(fread((void *) data, sizeof(char), this->get_size(), fp) < this->get_size()) {
		throw(exceptionpp::RuntimeError("giga::Page::aux_load", "invalid result returned from fread"));
	}
	fclose(fp);

	this->data.insert(this->data.end(), data, data + this->get_size());

	free((void *) data);
}

void giga::Page::aux_unload() {
	this->set_size(this->data.size());

	FILE *fp = fopen(this->get_filename().c_str(), "w");
	fputs(std::string(this->data.begin(), this->data.end()).c_str(), fp);
	fclose(fp);

	this->data.clear();
	this->data.resize(0);

	// we are now reading a "clean" file again next time
	this->set_file_offset(0);
	this->set_filename(this->get_filename());
	this->set_is_dirty(false);
}

std::string giga::Page::hash() {
	if(!this->get_is_loaded()) {
		throw(exceptionpp::RuntimeError("giga::Page::hash", "attempting to hash an unloaded page"));
	}
	return(md5(std::string(this->data.begin(), this->data.end())));
}
