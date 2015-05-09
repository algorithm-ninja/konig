#ifndef KONIG_EXCEPTION_HPP
#define KONIG_EXCEPTION_HPP

#include <exception>
#include <iosfwd>

namespace konig {

    class Exception : public std::runtime_error {
    public:
        Exception(const char *message) : std::runtime_error(message) { }
        Exception(const std::string message) : std::runtime_error(message) { }
    };

    class StructureViolation : public Exception {
    public:
        StructureViolation(const char *message) : Exception(message) { }
        StructureViolation(const std::string message) : Exception(message) { }
    };

    class InvalidArgument : public Exception {
    public:
        InvalidArgument(const char *message) : Exception(message) { }
        InvalidArgument(const std::string message) : Exception(message) { }
    };

    #define context_info(s) ("File " + std::string(__FILE__) + " in function " + std::string(__FUNCTION__) + ":" + std::to_string(__LINE__) + ": " + s)

}

#endif //KONIG_EXCEPTION_HPP
