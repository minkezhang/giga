#include <string>

#include "libs/cachepp/globals.h"
#include "libs/md5/md5.h"

#include "src/page.h"

giga::Page::Page(cachepp::identifier id, std::string filename, size_t file_offset, bool is_dirty) : cachepp::LineInterface<std::string>::LineInterface(id), filename(filename), file_offset(file_offset) {
	this->is_dirty = is_dirty;
}

cachepp::identifier giga::Page::get_identifier() { return(this->id); }

void giga::Page::set_filename(std::string filename) { this->filename = filename; }

void giga::Page::set_file_offset(size_t file_offset) { this->file_offset = file_offset; }

void giga::Page::aux_load() {
}

void giga::Page::aux_unload() {
}

std::string giga::Page::hash() {
	if(!this->get_is_loaded()) {
		throw(exceptionpp::RuntimeError("giga::Page::hash", "attempting to hash an unloaded page"));
	}
	return(md5(std::string(this->data.begin(), this->data.end())));
}
