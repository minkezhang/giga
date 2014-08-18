#include <cstdio>
#include <memory>
#include <mutex>
#include <string>

#include "libs/cachepp/simpleserialcache.h"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/page.h"

giga::Config::Config(size_t i_page_size, size_t m_page_size) : i_page_size(i_page_size), m_page_size(m_page_size) {}

size_t giga::Config::get_i_page_size() { return(this->i_page_size); }
size_t giga::Config::get_m_page_size() { return(this->m_page_size); }

giga::File::File(std::string filename, std::string mode, giga::Config config) : filename(filename), mode(mode), size(0), c_count(0), p_count(0), config(config) {
	this->cache = std::shared_ptr<cachepp::SimpleSerialCache<giga::Page>> (new cachepp::SimpleSerialCache<giga::Page>(2));
	this->l = std::shared_ptr<std::recursive_mutex> (new std::recursive_mutex);

	FILE *fp = fopen(this->get_filename().c_str(), "r");
	if(fp == NULL) {
		throw(exceptionpp::InvalidOperation("giga::File::File", "opening non-existent file in read-only mode"));
	}
	fseek(fp, 0, SEEK_END);
	this->size = ftell(fp);
	fclose(fp);

	for(size_t i = 0; i < this->size; i += this->config.get_i_page_size()) {
		this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, this->filename, i, (this->size - i) > this->config.get_i_page_size() ? this->config.get_i_page_size() : (this->size - i))));
	}
}

std::string giga::File::r(const std::shared_ptr<giga::Client>& client, size_t len) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return("");
}

size_t giga::File::d(const std::shared_ptr<giga::Client>& client, size_t len) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(len);
}

size_t giga::File::w(const std::shared_ptr<giga::Client>& client, std::string val) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(val.length());
}

size_t giga::File::i(const std::shared_ptr<giga::Client>& client, std::string val) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(val.length());
}

std::string giga::File::get_filename() { return(this->filename); }
std::string giga::File::get_mode() { return(this->mode); }

std::shared_ptr<giga::Client> giga::File::open() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	std::shared_ptr<giga::Client> c (new giga::Client(c_count++, this->shared_from_this()));
	this->clients.push_back(c);
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
}

void giga::File::save() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
}
