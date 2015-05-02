#ifndef KONIG_EXCEPTION_HPP
#define KONIG_EXCEPTION_HPP

#include <exception>

namespace konig {

    class Exception : public std::runtime_error {

    };

    class StructureViolation : public Exception {

    };

    class InvalidArgument : public Exception {

    };

};

#endif //KONIG_EXCEPTION_HPP
