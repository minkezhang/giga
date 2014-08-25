#include <cstdio>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "libs/cachepp/simpleserialcache.h"
#include "libs/exceptionpp/exception.h"
#include "libs/md5/md5.h"

#include "src/file.h"
#include "src/page.h"

giga::Config::Config(size_t i_page_size, size_t m_page_size, size_t cache_size) : i_page_size(i_page_size), m_page_size(m_page_size), cache_size(cache_size) {}

size_t giga::Config::get_i_page_size() { return(this->i_page_size); }
size_t giga::Config::get_m_page_size() { return(this->m_page_size); }
size_t giga::Config::get_cache_size() { return(this->cache_size); }

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

giga::File::File(std::string filename, std::string mode, giga::Config config) : filename(filename), size(0), c_count(0), p_count(0), config(config) {
	this->l = std::unique_ptr<std::recursive_mutex> (new std::recursive_mutex);

	this->set_mode(mode);

	this->cache = std::unique_ptr<cachepp::SimpleSerialCache<giga::Page>> (new cachepp::SimpleSerialCache<giga::Page>(this->config.get_cache_size()));
	this->load();
}

giga::File::~File() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	for(std::list<std::shared_ptr<Client>>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
		(*it)->close();
		(*it).reset();
	}
}

void giga::File::load() {
	this->cache->clear();
	this->pages.clear();

	FILE *fp = fopen(this->get_filename().c_str(), "r");
	if(fp == NULL) {
		if(!(this->mode & giga::File::dne_create)) {
			throw(exceptionpp::InvalidOperation("giga::File::File", "opening non-existent file without explicitly specifying '+' mode"));
		} else {
			// create the file
			fp = fopen(filename.c_str(), "w+");
		}
	}
	fseek(fp, 0, SEEK_END);
	this->set_size(ftell(fp));
	fclose(fp);
	fp = NULL;

	this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, this->filename, 0, 0)));
	if(this->get_size() == 0) {
		this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, "", 0, 0, true)));
	}
	for(size_t i = 0; i < this->get_size(); i += this->config.get_i_page_size()) {
		std::shared_ptr<giga::Page> p (new giga::Page(this->p_count++, this->filename, i, (this->get_size() - i) > this->config.get_i_page_size() ? this->config.get_i_page_size() : (this->get_size() - i)));
		this->pages.push_back(p);
	}
	this->pages.push_back(std::shared_ptr<giga::Page> (new giga::Page(this->p_count++, this->filename, 0, 0)));


	/**
	 * we are ordering the clients by file_offset -- the map iterator starts at the smallest offset and increments accordingly, giving us a natural
	 *	way to express an efficienty comparing of file offsets to page offsets
	 *
	 * this is essentially to set up the final step of a merge sort
	 *
	 * cf. http://bit.ly/1q8rpN5
	 */
	std::map<size_t, std::list<std::shared_ptr<giga::ClientData>>> lookaside;
	for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
		lookaside[it->second->get_file_offset()].push_back(it->second);
	}

	size_t file_offset = 0;
	std::map<size_t, std::list<std::shared_ptr<giga::ClientData>>>::iterator it = lookaside.begin();
	for(std::list<std::shared_ptr<giga::Page>>::iterator jt = this->pages.begin(); jt != this->pages.end(); ++jt) {
		if(it == lookaside.end()) {
			break;
		}

		file_offset += (*jt)->get_size();

		// it->first contained within this page
		if(it->first < file_offset) {
			for(std::list<std::shared_ptr<giga::ClientData>>::iterator kt = it->second.begin(); kt != it->second.end(); ++kt) {
				(*kt)->set_page(jt);
				(*kt)->set_page_offset(it->first - (file_offset - (*jt)->get_size()));
			}
			it = std::next(it, 1);
		}
	}

	// set EOF pointers
	while(it != lookaside.end()) {
		for(std::list<std::shared_ptr<giga::ClientData>>::iterator kt = it->second.begin(); kt != it->second.end(); ++kt) {
			(*kt)->set_page(std::prev(this->pages.end(), 1));
			(*kt)->set_page_offset(0);
		}
		it = std::next(it, 1);
	}
}

giga::Config giga::File::get_config() { return(this->config); }

size_t giga::File::get_size() { return(this->size); }
void giga::File::set_size(size_t size) { this->size = size; }

void giga::File::align(const std::shared_ptr<giga::Client>& client) {
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	if(info->get_page() == this->pages.begin()) {
		info->set_page(std::next(info->get_page(), 1));
		info->set_page_offset(0);
	}
	if(info->get_page() == this->pages.end()) {
		info->set_page(std::prev(info->get_page(), 1));
		info->set_page_offset((*(info->get_page()))->get_size());
	}
}

size_t giga::File::s(const std::shared_ptr<giga::Client>& client, size_t len, bool is_forward, bool is_absolute) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	len = (this->get_size() > len) ? len : this->get_size();

	if(is_absolute) {
		if(is_forward) {
			if(len > info->get_file_offset()) {
				len -= info->get_file_offset();
			} else {
				len = info->get_file_offset() - len;
				is_forward = false;
			}
		} else {
			if((this->get_size() - len) > info->get_file_offset()) {
				len = (this->get_size() - len) - info->get_file_offset();
				is_forward = true;
			} else {
				len = info->get_file_offset() - (this->get_size() - len);
			}
		}
	}

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

	if(!(this->mode & giga::File::read_only)) {
		throw(exceptionpp::InvalidOperation("giga::File::r", "permission denied"));
	}

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

	if(!(this->mode & giga::File::write_only)) {
		throw(exceptionpp::InvalidOperation("giga::File::w", "permission denied"));
	}

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

	if(!(this->mode & giga::File::write_only)) {
		throw(exceptionpp::InvalidOperation("giga::File::d", "permission denied"));
	}

	this->align(client);
	std::shared_ptr<giga::ClientData> info = this->lookaside[client->get_identifier()];

	size_t expected = len;

	while(len > 0) {
		size_t n_bytes = (*(info->get_page()))->probe(info->get_page_offset(), len, true);
		if(n_bytes == 0) {
			break;
		}
		std::vector<uint8_t> buf = this->cache->r((*(info->get_page())));

		// adjust client -> page pointers
		for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
			std::shared_ptr<giga::ClientData> tmp_info = it->second;
			if(tmp_info->get_file_offset() > info->get_file_offset()) {
				// different pages
				if(tmp_info->get_page() != info->get_page()) {
					tmp_info->set_file_offset(tmp_info->get_file_offset() - n_bytes);
				} else if((tmp_info->get_page() == info->get_page()) && (tmp_info->get_page_offset() > info->get_page_offset())) {
					size_t tmp_n_bytes = (n_bytes > (tmp_info->get_page_offset() - info->get_page_offset())) ? (tmp_info->get_page_offset() - info->get_page_offset()) : n_bytes;
					tmp_info->set_page_offset(tmp_info->get_page_offset() - tmp_n_bytes);
					tmp_info->set_file_offset(tmp_info->get_file_offset() - tmp_n_bytes);
				}
			}
		}

		// deleting pages
		if(n_bytes == (*(info->get_page()))->get_size()) {
			for(std::map<cachepp::identifier, std::shared_ptr<giga::ClientData>>::iterator it = this->lookaside.begin(); it != this->lookaside.end(); ++it) {
				std::shared_ptr<giga::ClientData> tmp_info = it->second;
				if((tmp_info != info) && (tmp_info->get_page() == info->get_page())) {
					tmp_info->set_page(std::next(tmp_info->get_page(), 1));
				}
			}
			info->set_page(std::next(info->get_page(), 1));
			this->pages.erase(std::prev(info->get_page(), 1));
		} else {
			buf.erase(buf.begin() + info->get_page_offset(), buf.begin() + info->get_page_offset() + n_bytes);
			this->cache->w((*(info->get_page())), buf);
			(*(info->get_page()))->set_size(buf.size());
		}

		len -= n_bytes;
		this->set_size(this->get_size() - n_bytes);
	}

	return(expected - len);
}

size_t giga::File::i(const std::shared_ptr<giga::Client>& client, std::string val) {
	std::lock_guard<std::recursive_mutex> l(*this->l);

	if(!(this->mode & giga::File::write_only)) {
		throw(exceptionpp::InvalidOperation("giga::File::i", "permission denied"));
	}

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
			if(tmp_info->get_file_offset() > info->get_file_offset()) {
				tmp_info->set_file_offset(tmp_info->get_file_offset() + n_bytes);
				// adjust same page
				if((tmp_info->get_page() == info->get_page()) && (tmp_info->get_page_offset() > info->get_page_offset())) {
					tmp_info->set_page(std::next(tmp_info->get_page(), 1));
					tmp_info->set_page_offset(tmp_info->get_page_offset() - info->get_page_offset());
				}
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

void giga::File::set_mode(std::string mode) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->mode = 0;
	if(mode.find('r') != std::string::npos) {
		this->mode |= giga::File::read_only;
	}
	if(mode.find('w') != std::string::npos) {
		this->mode |= giga::File::write_only;
	}
	if(mode.find('+') != std::string::npos) {
		this->mode |= giga::File::dne_create;
	}
}

std::string giga::File::get_mode() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	std::stringstream buf;
	if(this->mode & giga::File::read_only) {
		buf << 'r';
	}
	if(this->mode & giga::File::write_only) {
		buf << 'w';
	}
	if(this->mode & giga::File::dne_create) {
		buf << '+';
	}
	return(buf.str());
}

std::shared_ptr<giga::Client> giga::File::open(const std::shared_ptr<giga::Client>& client, std::string mode) {
	std::lock_guard<std::recursive_mutex> l(*this->l);

	std::shared_ptr<giga::Client> c;
	if(client == NULL) {
		if(mode.compare("") == 0) {
			mode = this->get_mode();
		}
		c = std::shared_ptr<giga::Client> (new giga::Client(c_count++, this->shared_from_this(), mode));
	} else {
		if(mode.compare("") != 0) {
			throw(exceptionpp::InvalidOperation("giga::File::open", "cannot set mode on an existing client"));
		}
		c = client;
	}

	std::shared_ptr<giga::ClientData> cd (new giga::ClientData(c->get_identifier()));

	cd->set_page(this->pages.begin());

	cd->set_file_offset(0);
	cd->set_page_offset(0);

	this->clients.push_back(c);
	this->lookaside[c->get_identifier()] = cd;

	this->align(c);

	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->lookaside.erase(client->get_identifier());
}

void giga::File::save() {
	std::lock_guard<std::recursive_mutex> l(*this->l);

	this->cache->clear();

	std::stringstream name;
	std::stringstream path;

	name << this->filename << "_" << rand();
	path << "/tmp/" << md5(name.str());

	FILE *fp = fopen(path.str().c_str(), "w");
	for(std::list<std::shared_ptr<giga::Page>>::iterator it = this->pages.begin(); it != this->pages.end(); ++it) {
		std::vector<uint8_t> buf = this->cache->r(*it);
		fputs(std::string(buf.begin(), buf.end()).c_str(), fp);
	}
	fclose(fp);
	fp = NULL;

	rename(path.str().c_str(), this->filename.c_str());
	remove(path.str().c_str());

	// populate new pages
	this->load();
}
