#ifndef KONIG_UTIL_HPP
#define KONIG_UTIL_HPP

#include <exception>
#include <random>
#include <cstdint>

namespace konig {

    typedef uint32_t vid_t;

    namespace random {
        std::random_device random_device;
        std::mt19937_64 random_generator(random_device());

        template<typename T1, typename T2>
        auto randrange(T1 bottom, T2 top)
                -> typename std::enable_if<!std::is_integral<decltype(bottom + top)>::value, decltype(bottom + top)>::type
        {
            return std::uniform_real_distribution<decltype(bottom + top)>(bottom, top)();
        }

        template<typename T1, typename T2>
        auto randrange(T1 bottom, T2 top)
                -> typename std::enable_if<std::is_integral<decltype(bottom + top)>::value, decltype(bottom + top)>::type
        {
            return std::uniform_int_distribution<decltype(bottom + top)>(bottom, top)();
        }
    }
}

#endif //KONIG_UTIL_HPP
