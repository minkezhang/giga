#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>

namespace giga {
	class FileNotFound : public std::exception {
		public:
			FileNotFound(std::string filename);
			virtual const char *what() const throw();
		private:
			std::string filename;
	};

	class InvalidFileOperation : public std::exception {
		public:
			InvalidFileOperation(std::string filename, std::string mode, int error_type);
			virtual const char *what() const throw();
		private:
			std::string filename;
			std::string mode;
			int error_type;
	};
}

#endif
