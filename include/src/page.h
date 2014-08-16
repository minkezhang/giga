#ifndef _GIGA_PAGE_H
#define _GIGA_PAGE_H

#include <string>

#include "libs/cachepp/globals.h"
#include "libs/cachepp/lineinterface.h"

namespace giga {
	class Page : public cachepp::LineInterface<std::string> {
		public:
			Page(cachepp::identifier id, std::string filename, size_t file_offset, bool is_dirty = false);

		private:
			cachepp::identifier id;
			std::string filename;

			// file offset from the filename
			size_t file_offset;

			/**
			 * attribute setters
			 */
			void set_filename(std::string filename);
			void set_file_offset(size_t file_offset);

			/**
			 * cachepp interface function definitions
			 */
			virtual void aux_load() final;
			virtual void aux_unload() final;
			virtual std::string hash() final;
	};
}

#endif
