#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>

namespace giga {
	/**
	 * functions which throw this signal their functionality is not yet complete
	 *
	 * functions which call such functions will need a try / catch to handle these functions
	 */
	class ToyFunction : public std::exception {
		public:
			ToyFunction(std::string func_name);
			virtual const char *what() const throw();
		private:
			std::string func_name;
	};

	class NotImplemented : public std::exception {
		public:
			NotImplemented(std::string func_name);
			virtual const char *what() const throw();
		private:
			std::string func_name;
	};

	/**
	 * general error class
	 */
	class InvalidOperation : public std::exception {
		public:
			InvalidOperation(std::string func_name, std::string msg);
			virtual const char *what() const throw();
		private:
			std::string func_name;
			std::string msg;
	};

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
