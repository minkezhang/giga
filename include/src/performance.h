#ifndef _GIGA_PERFORMANCE_H
#define _GIGA_PERFORMANCE_H

#include <atomic>
#include <memory>

#include "src/client.h"
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
			double get_data_size(size_t index);
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
			Performance();

			void set_file(std::shared_ptr<giga::File> file);

			void run(std::string tag, std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size, size_t n_clients, size_t n_attempts);
			Result get_result();

			const static uint8_t R = 0;
			const static uint8_t W = 1;
			const static uint8_t I = 2;
			const static uint8_t E = 3;

		private:
			std::weak_ptr<File> file;
			Result result;

			void aux_run(const std::shared_ptr<std::atomic<double>>& runtime, const std::shared_ptr<std::atomic<size_t>>& data, const std::shared_ptr<Client>& client, std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size);
	};
}

std::ostream& operator<< (std::ostream& os, giga::Result& obj);

#endif
