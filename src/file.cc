#include <memory>
#include <mutex>
#include <string>

#include "libs/cachepp/simpleserialcache.h"

#include "src/file.h"

giga::File::File(std::string filename, std::string mode) : filename(filename), mode(mode) {
	this->cache = std::shared_ptr<cachepp::SimpleSerialCache<giga::Page>> (new cachepp::SimpleSerialCache<giga::Page>(2));
}

std::string giga::File::r(const std::shared_ptr<giga::Client>& client, size_t len) { return(""); }
size_t giga::File::d(const std::shared_ptr<giga::Client>& client, size_t len) { return(len); }
size_t giga::File::w(const std::shared_ptr<giga::Client>& client, std::string val) { return(val.length()); }
size_t giga::File::i(const std::shared_ptr<giga::Client>& client, std::string val) { return(val.length()); }

std::string giga::File::get_filename() { return(this->filename); }
std::string giga::File::get_mode() { return(this->mode); }

std::shared_ptr<giga::Client> giga::File::open() {
	std::shared_ptr<giga::Client> c (new giga::Client());
	this->clients.push_back(c);
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {}

void giga::File::save() {}
