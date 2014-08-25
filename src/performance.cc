#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "libs/exceptionpp/exception.h"

#include "src/performance.h"

bool giga::Result::is_dup(char l, char r) { return(((l == ' ') || (l == '\n')) && (l == r)); }

giga::Result::Result() : size(0) {}

void giga::Result::push_back(std::string tag, size_t n_transactions, double total_runtime, size_t total_data, size_t file_size, double data_size, size_t cache_size, size_t page_size, size_t n_clients) {
	this->tag.push_back(tag);
	this->n_transactions.push_back(n_transactions);
	this->total_runtime.push_back(total_runtime);
	this->total_data.push_back(total_data);
	this->file_size.push_back(file_size);
	this->data_size.push_back(data_size);
	this->cache_size.push_back(cache_size);
	this->page_size.push_back(page_size);
	this->n_clients.push_back(n_clients);
	this->size++;
}

size_t giga::Result::get_size() { return(this->size); }

std::string giga::Result::get_tag(size_t index) { return(this->tag.at(index)); }
double giga::Result::get_latency(size_t index) { return(this->total_runtime.at(index) / this->n_transactions.at(index)); }
double giga::Result::get_throughput(size_t index) { return(this->total_data.at(index) / this->total_runtime.at(index)); }
size_t giga::Result::get_file_size(size_t index) { return(this->file_size.at(index)); }
double giga::Result::get_data_size(size_t index) { return(this->data_size.at(index)); }
size_t giga::Result::get_cache_size(size_t index) { return(this->cache_size.at(index)); }
size_t giga::Result::get_page_size(size_t index) { return(this->page_size.at(index)); }
size_t giga::Result::get_n_clients(size_t index) { return(this->n_clients.at(index)); }

std::string giga::Result::to_string(bool is_tsv) {
	size_t pad = 11;
	std::string sep = " | ";

	if(this->get_size() == 0) {
		throw(exceptionpp::InvalidOperation("giga::Result::to_string", "reporting zero trials"));
	}

	std::stringstream buffer;
	buffer << std::setw(pad) << "trial" << sep << std::setw(3) << "tag" << sep << std::setw(pad) << "tput (B/us)" << sep << std::setw(pad) << "lat (B)" << sep << std::setw(pad) << "file (B)" << sep << std::setw(pad) << "data (B)" << sep << std::setw(pad) << "cache" << sep << std::setw(pad) << "page (B)" << sep << std::setw(pad) << "n_clients" << std::endl;
	buffer << std::string(buffer.str().length(), '=') << std::endl;
	for(size_t index = 0; index < this->get_size(); ++index) {
		buffer << std::setprecision(2) << std::fixed;
		buffer << std::setw(pad) << index + 1 << sep << std::setw(3) << this->get_tag(index) << sep << std::setw(pad) << this->get_throughput(index) << sep << std::setw(pad) << this->get_latency(index) << sep << std::setw(pad) << this->get_file_size(index) << sep << std::setw(pad) << this->get_data_size(index) << sep << std::setw(pad) << this->get_cache_size(index) << sep << std::setw(pad) << this->get_page_size(index) << sep << std::setw(pad) << this->get_n_clients(index) << std::endl;
	}

	std::string ret = buffer.str();
	// format to tabs
	if(is_tsv) {
		std::replace(ret.begin(), ret.end(), '|', '\t');
		// cf. http://bit.ly/1p6ATc7
		ret.erase(std::remove(ret.begin(), ret.end(), '='), ret.end());

		// cf. http://bit.ly/1ywrQSZ
		std::string::iterator end = std::unique(ret.begin(), ret.end(), giga::Result::is_dup);
		ret.erase(end, ret.end());

		// cf. http://bit.ly/1oN9U5R
		std::stringstream s;
		s << std::endl << "\t";
		size_t pos = 0;
		while((pos = ret.find(s.str())) != std::string::npos) {
			ret.erase(pos + 1, 1);
		}
		pos = 0;
		while((pos = ret.find(" \t ")) != std::string::npos) {
			ret.erase(pos + 2, 1);
			ret.erase(pos, 1);
		}

		ret = ret.substr(1);
	}
	return(ret);
}

std::ostream& operator<< (std::ostream& os, giga::Result& obj) {
	os << obj.to_string(false);
	return(os);
}

