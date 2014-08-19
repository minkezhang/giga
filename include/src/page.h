#ifndef _GIGA_PAGE_H
#define _GIGA_PAGE_H

#include <string>

#include "libs/cachepp/globals.h"
#include "libs/cachepp/lineinterface.h"

namespace giga {
	class Page : public cachepp::LineInterface<std::string> {
		public:
			Page(cachepp::identifier id, std::string filename, size_t file_offset, size_t size, bool is_dirty = false);

			size_t get_size();
			/**
			 * returns the number of bytes to END or BEGINNING if traveling LEN bytes from the offset
			 *    v -- offset = 3
			 * abc|defg
			 * max length backward == 3
			 * max length forward == 4
			 */
			size_t probe(size_t offset, size_t len, bool is_forward);

		private:
			cachepp::identifier id;
			std::string filename;
			// file offset from the filename
			size_t file_offset;
			size_t size;

			/**
			 * attribute setters
			 */
			void set_filename(std::string filename);
			void set_size(size_t size);
			void set_file_offset(size_t file_offset);

			/**
			 * attribute getters
			 */
			std::string get_filename();

			/**
			 * cachepp interface function definitions
			 */
			virtual void aux_load() final;
			virtual void aux_unload() final;
			virtual std::string hash() final;
	};
}

#endif
