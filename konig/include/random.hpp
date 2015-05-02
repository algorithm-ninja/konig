#ifndef KONIG_UTILS_HPP
#define KONIG_UTILS_HPP

#include <exception>

namespace konig {
    
    namespace random {
        std::random_device random_device;
        std::mt19937_64 random_generator(random_device());

        template<typename T1, typename T2, std::enable_if<!std::is_integral<decltype(bottom + top)>::value>
        auto randrange(T1 bottom, T2 top) {
            return double(random_generator()) / rand_max * (top - bottom) + bottom;
        }

        template<typename T1, typename T2, std::enable_if<std::is_integral<decltype(bottom + top)>::value>
        auto randrange(T1 bottom, T2 top) {
            return random_generator() % (top - bottom) + bottom;
        }

        class NotImplementedException : public std::exception { // FIXME: Use std::runtime_error
            virtual const char* what() const noexcept {
                return "This function is not implemented yet!";
            }
        };
    }
}

#endif //KONIG_UTILS_HPP
