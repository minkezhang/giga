#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>

#include "libs/cachepp/globals.h"
#include "libs/exceptionpp/exception.h"
#include "libs/md5/md5.h"

#include "src/page.h"

giga::Page::Page(cachepp::identifier id, std::string filename, size_t file_offset, size_t size, bool is_dirty) : cachepp::LineInterface<std::string>::LineInterface(id), filename(filename), file_offset(file_offset), size(size) {
	this->is_dirty = is_dirty;
	this->set_filename(this->filename);
	this->data.clear();
	this->set_checksum(this->hash());
	this->r = rand();
}
giga::Page::~Page() {
	// remove tmp file to lessen bloat
	if(this->cached.compare(this->filename) != 0) {
		remove(this->get_filename().c_str());
	}
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

void giga::Page::set_filename(std::string filename) { this->cached = filename; }

std::string giga::Page::get_filename() {
	if(this->get_is_dirty()) {
		std::stringstream path;
		std::stringstream buf;
		buf << this->cached << "_" << this->get_identifier();
		path << "/tmp/giga/" << md5(buf.str()) << this->r;
		return(path.str());
	} else {
		return(this->cached);
	}
}

void giga::Page::set_file_offset(size_t file_offset) { this->file_offset = file_offset; }

void giga::Page::aux_load() {
	// deallocate the data vector
	this->data.clear();
	std::vector<uint8_t>().swap(this->data);

	if(this->get_size() == 0) {
		return;
	}

	std::vector<uint8_t> buf(this->get_size(), 0);

	std::cout << "loading page " << this->get_identifier() << " from file " << this->get_filename()  << std::endl;
	std::cout << " -- page dump: " << this->get_size() << std::endl;

	// load data into the page
	if(!this->get_is_dirty()) {
		FILE *fp = fopen(this->get_filename().c_str(), "r");
		if(fp == NULL) {
			std::stringstream buf;
			buf << "cannot open working file '" << this->get_filename() << "'";
			throw(exceptionpp::RuntimeError("giga::Page::aux_load", buf.str()));
		}
		if(fseek(fp, this->file_offset, SEEK_SET) == -1) {
			fclose(fp);
			fp = NULL;
			throw(exceptionpp::RuntimeError("giga::Page::aux_load", "invalid result returned from fseek"));
		}

		if(fread(buf.data(), sizeof(char), this->get_size(), fp) < this->get_size()) {
			fclose(fp);
			fp = NULL;
			std::cout << "cached: " << this->cached << ", filename: " << this->get_filename() << std::endl;
			throw(exceptionpp::RuntimeError("giga::Page::aux_load", "invalid result returned from fread"));
		}
		fclose(fp);
		fp = NULL;

		buf.swap(this->data);
	}
}

void giga::Page::aux_unload() {
	this->set_size(this->data.size());

	std::cout << "unloading page " << this->get_identifier() << " to file " << this->get_filename()  << std::endl;
	std::cout << " -- page dump: " << this->get_size() << std::endl;

	FILE *fp = fopen(this->get_filename().c_str(), "w");
	if(fp == NULL) {
		std::stringstream buf;
		buf << "cannot open working file '" << this->get_filename() << "'";
		throw(exceptionpp::RuntimeError("giga::Page::aux_unload", buf.str()));
	}

	fputs(std::string(this->data.begin(), this->data.end()).c_str(), fp);
	fclose(fp);
	fp = NULL;

	this->data.clear();

	std::vector<uint8_t>().swap(this->data);

	// we are now reading a "clean" file again next time
	this->set_file_offset(0);

	// remove tmp file to lessen bloat
	if(this->cached.compare(this->filename) != 0) {
		remove(this->cached.c_str());
	}
	this->set_filename(this->get_filename());
	this->set_is_dirty(false);
}

std::string giga::Page::hash() { return(md5(std::string(this->data.begin(), this->data.end()))); }
