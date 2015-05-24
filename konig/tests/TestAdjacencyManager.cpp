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
            std::vector<konig::adjacency_t> neig(AM.begin(0), AM.end(0));

            CHECK(neig == std::vector<konig::adjacency_t>({
              {0, 1}
            }));
        }
    }
}