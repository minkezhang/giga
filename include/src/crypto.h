/**
 * the crypto internal library MAY be called by the user, but the author questions why
 */
namespace giga {
	class Crypto {
		public:
			static std::string hash(std::string s);
	};
}
