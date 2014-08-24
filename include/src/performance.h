#ifndef _GIGA_PERFORMANCE_H
#define _GIGA_PERFORMANCE_H

#include "src/file.h"

namespace giga {
	class Result {
		public:
			Result();
			void push_back(std::string tag, size_t n_transactions, double total_runtime, size_t total_data, size_t file_size, size_t cache_size, size_t page_size, size_t n_clients);

			size_t get_size();

			std::string get_tag(size_t index);
			double get_latency(size_t index);
			double get_throughput(size_t index);
			size_t get_file_size(size_t index);
			size_t get_cache_size(size_t index);
			size_t get_page_size(size_t index);
			size_t get_n_clients(size_t index);

			std::string to_string(bool is_tsv = false);
			static bool is_dup(char l, char r);
		private:
			size_t size;
			std::vector<std::string> tag;
			std::vector<size_t> n_transactions;
			std::vector<double> total_runtime;
			std::vector<size_t> total_data;
			std::vector<size_t> file_size;
			std::vector<size_t> cache_size;
			std::vector<size_t> page_size;
			std::vector<size_t> n_clients;
	};

	class Performance {
		public:
			Performance(std::shared_ptr<giga::File> file);
	};
}

std::ostream& operator<< (std::ostream& os, giga::Result& obj);

#endif
