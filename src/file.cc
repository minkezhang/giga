#include <cstdio>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>

#include <iostream>

#include "libs/cachepp/simpleserialcache.h"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"
#include "src/page.h"

giga::Config::Config(size_t i_page_size, size_t m_page_size) : i_page_size(i_page_size), m_page_size(m_page_size) {}

size_t giga::Config::get_i_page_size() { return(this->i_page_size); }
size_t giga::Config::get_m_page_size() { return(this->m_page_size); }

size_t giga::Config::probe(const std::shared_ptr<giga::Page>& page, size_t offset, size_t len) {
	size_t n_bytes = page->probe(offset, len, true);
	if(n_bytes >= len) {
		return(len);
	}
	if(len > (this->get_m_page_size() - offset)) {
		return(this->get_m_page_size() - offset);
	}
	return(len);
}

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

	this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, this->filename, 0, 0)));
	for(size_t i = 0; i < this->get_size(); i += this->config.get_i_page_size()) {
		std::shared_ptr<giga::Page> p (new giga::Page(this->p_count++, this->filename, i, (this->get_size() - i) > this->config.get_i_page_size() ? this->config.get_i_page_size() : (this->get_size() - i)));
		this->pages.push_back(p);
	}
	this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, this->filename, 0, 0)));
}

giga::File::~File() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	for(std::list<std::shared_ptr<Client>>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		(*it)->close();
		(*it).reset();
	}
}

size_t giga::File::get_size() { return(this->size); }
void giga::File::set_size(size_t size) { this->size = size; }

void giga::File::align(const std::shared_ptr<giga::Client>& client) {
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];
	if((*(info->get_page())) == this->pages.front()) {
		info->set_page(std::next(info->get_page(), 1));
		info->set_page_offset(0);
	}
	if((*(info->get_page())) == this->pages.back()) {
		info->set_page(std::prev(info->get_page(), 1));
		info->set_page_offset((*(info->get_page()))->get_size());
	}
}

size_t giga::File::s(const std::shared_ptr<giga::Client>& client, size_t len, bool is_forward) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];
	if(len != 0) {
		this->align(client);
		if(is_forward) {
			while(len > 0 && (*(info->get_page())) != this->pages.back()) {
				size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, is_forward);
				info->set_file_offset(info->get_file_offset() + n_bytes);

				if(info->get_page_offset() + n_bytes < (*(info->get_page()))->get_size()) {
					info->set_page_offset(info->get_page_offset() + n_bytes);
				} else {
					info->set_page_offset(0);
					info->set_page(std::next(info->get_page(), 1));
				}
				len -= n_bytes;
			}
		} else {
			while(len > 0 && (*(info->get_page())) != this->pages.front()) {
				size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, is_forward);
				info->set_file_offset(info->get_file_offset() - n_bytes);

				if(info->get_page_offset() > n_bytes) {
					info->set_page_offset(info->get_page_offset() - n_bytes);
				} else {
					info->set_page(std::prev(info->get_page(), 1));
					info->set_page_offset((*(info->get_page()))->get_size());
				}
				len -= n_bytes;
			}
		}
	}
	return(info->get_file_offset());
}

std::string giga::File::r(const std::shared_ptr<giga::Client>& client, size_t len) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->align(client);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	std::string val = "";
	while(len > 0 && (*(info->get_page())) != this->pages.back()) {
		size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, true);
		std::vector<uint8_t> buf = this->cache->r((*(info->get_page())));
		val.append(buf.begin() + info->get_page_offset(), buf.begin() + info->get_page_offset() + n_bytes);
		info->set_file_offset(info->get_file_offset() + n_bytes);

		if(info->get_page_offset() + n_bytes < (*(info->get_page()))->get_size()) {
			info->set_page_offset(info->get_page_offset() + n_bytes);
		} else {
			info->set_page_offset(0);
			info->set_page(std::next(info->get_page(), 1));
		}
		len -= n_bytes;
	}

	return(val);
}

size_t giga::File::w(const std::shared_ptr<giga::Client>& client, std::string val) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->align(client);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	size_t len = val.length();
	while(len > 0 && (*(info->get_page())) != this->pages.back()) {
		size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, true);
		std::vector<uint8_t> buf = this->cache->r((*(info->get_page())));

		// copy into the buffer
		std::copy(val.begin() + (val.length() - len), val.begin() + (val.length() - len) + n_bytes, buf.begin() + info->get_page_offset());
		this->cache->w((*(info->get_page())), buf);

		info->set_file_offset(info->get_file_offset() + n_bytes);

		if(info->get_page_offset() + n_bytes < (*(info->get_page()))->get_size()) {
			info->set_page_offset(info->get_page_offset() + n_bytes);
		} else {
			info->set_page_offset(0);
			info->set_page(std::next(info->get_page(), 1));
		}
		len -= n_bytes;
	}

	return((val.length() - len) + this->i(client, val.substr(val.length() - len)));
}

size_t giga::File::d(const std::shared_ptr<giga::Client>& client, size_t len) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->align(client);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	while(len > 0 && (*(info->get_page())) != this->pages.back()) {
		size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, false);
		std::vector<uint8_t> buf = this->cache->r((*(info->get_page())));

		// adjust client -> page pointers
		for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
			std::shared_ptr<giga::ClientData> tmp_info = it->second;
			std::cout << "    giga::File::d -- tmp_info: " << tmp_info << ", info: " << info << std::endl;
			std::cout << "    giga::File::d -- tmp_info offset: " << (*(tmp_info->get_page()))->get_size() << ", info: " << info << std::endl;
			if(std::distance(tmp_info->get_page(), info->get_page()) < 0) {
				if(n_bytes == (*(info->get_page()))->get_size()) {
					tmp_info->set_page(std::prev(tmp_info->get_page(), 1));
				}
				tmp_info->set_file_offset(tmp_info->get_file_offset() - n_bytes);
			}
			if((std::distance(tmp_info->get_page(), info->get_page()) == 0) && (tmp_info->get_page_offset() > info->get_page_offset())) {
				size_t tmp_n_bytes = (*(tmp_info->get_page()))->probe(tmp_info->get_page_offset(), n_bytes, false);
				tmp_info->set_page_offset(tmp_info->get_file_offset() - tmp_n_bytes);
				tmp_info->set_file_offset(tmp_info->get_file_offset() - tmp_n_bytes);
			}
		}

		if(n_bytes == (*(info->get_page()))->get_size()) {
			for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
				std::shared_ptr<giga::ClientData> tmp_info = it->second;
				if(std::distance(tmp_info->get_page(), info->get_page()) == 0) {
					tmp_info->set_page(std::prev(tmp_info->get_page(), 1));
					tmp_info->set_page_offset((*(tmp_info->get_page()))->get_size());
				}
			}
			std::cout << "    giga::File::d -- erasing page" << std::endl;
			this->pages.erase(std::next(info->get_page(), 1));
		} else {
			// write to page
			buf.erase(buf.begin() + info->get_page_offset(), buf.end());
			this->cache->w((*(info->get_page())), buf);
			(*(info->get_page()))->set_size(buf.size());
		}

		this->set_size(this->get_size() - n_bytes);
		std::cout << this->get_size() << std::endl;
	}
	return(len);
}

size_t giga::File::i(const std::shared_ptr<giga::Client>& client, std::string val) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->align(client);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	size_t len = val.length();
	while(len > 0) {
		size_t n_bytes = this->config.probe((*(info->get_page())), info->get_page_offset(), len);
		std::vector<uint8_t> buf = this->cache->r((*(info->get_page())));

		// split page
		std::shared_ptr<giga::Page> p (new giga::Page(this->p_count++, "", 0, buf.size() - info->get_page_offset(), true));
		this->pages.insert(std::next(info->get_page(), 1), p);
		this->cache->w(p, std::vector<uint8_t>(buf.begin() + info->get_page_offset(), buf.end()));

		// adjust client -> page pointers
		for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
			std::shared_ptr<giga::ClientData> tmp_info = it->second;
			if((std::distance(tmp_info->get_page(), info->get_page()) < 0) || ((std::distance(tmp_info->get_page(), info->get_page()) == 0) && (tmp_info->get_page_offset() > info->get_page_offset()))) {
				tmp_info->set_page(std::next(tmp_info->get_page(), 1));
				tmp_info->set_file_offset(tmp_info->get_file_offset() + n_bytes);
			}
		}

		// write to page
		buf.erase(buf.begin() + info->get_page_offset(), buf.end());
		buf.insert(buf.end(), val.begin() + (val.length() - len), val.begin() + (val.length() - len) + n_bytes);
		this->cache->w((*(info->get_page())), buf);
		(*(info->get_page()))->set_size(buf.size());

		info->set_file_offset(info->get_file_offset() + n_bytes);

		if(info->get_page_offset() + n_bytes < this->config.get_m_page_size()) {
			info->set_page_offset(info->get_page_offset() + n_bytes);
		} else {
			info->set_page_offset(0);
			info->set_page(std::next(info->get_page(), 1));
		}
		len -= n_bytes;
		this->set_size(this->get_size() + n_bytes);
	}

	return(val.length() - len);
}

std::string giga::File::get_filename() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(this->filename);
}

std::string giga::File::get_mode() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(this->mode);
}

std::shared_ptr<giga::Client> giga::File::open(const std::shared_ptr<giga::Client>& client) {
	std::lock_guard<std::recursive_mutex> l(*this->l);

	std::shared_ptr<giga::Client> c;
	if(client == NULL) {
		c = std::shared_ptr<giga::Client> (new giga::Client(c_count++, this->shared_from_this()));
	} else {
		c = client;
	}

	std::shared_ptr<giga::ClientData> cd (new giga::ClientData(c->get_identifier()));

	cd->set_page(this->pages.begin());
	while((*(cd->get_page()))->get_size() == 0) {
		cd->set_page(std::next(cd->get_page(), 1));
	}
	cd->set_file_offset(0);
	cd->set_page_offset(0);

	this->clients.push_back(c);
	this->lookaside[c->get_identifier()] = cd;

	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->lookaside.erase(client->get_identifier());
}

void giga::File::save() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
}
