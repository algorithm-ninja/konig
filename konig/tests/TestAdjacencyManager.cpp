#include "Catch/single_include/catch.hpp"
#include "../include/AdjacencyManager.hpp"

namespace TestAdjacencyManager {

    TEST_CASE("AjacencyManager structural updates", "[AM]") {
        konig::AdjacencyManager AM;

        SECTION("Duplicates") {
            AM.insert({0, 1});
            AM.insert({0, 1});
            AM.insert({0, 1});
            AM.insert({0, 1});

            CHECK(AM.size() == 1);
            std::vector<konig::adjacency_t> neig;
            for (auto it = AM.begin(0); it != AM.end(0); ++it) // FIXME (why does the range-based construction fails?)
              neig.push_back(*it);

            CHECK(neig == std::vector<konig::adjacency_t>({
              {0, 1}
            }));
        }
    }
}