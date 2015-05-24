#include "Catch/single_include/catch.hpp"
#include "../include/AdjacencyTree.hpp"

namespace TestAdjacencyTree {

    TEST_CASE("AjacencyTree structural updates", "[AT]") {
        konig::AdjacencyTree AT;

        SECTION("Duplicates") {
            AT.insert({0, 1});
            AT.insert({0, 1});
            AT.insert({0, 1});
            AT.insert({0, 1});

            CHECK(AT.size() == 1);
        }

        SECTION("Mixed insert") {
            AT.insert({0, 1});
            AT.insert({1, 2});
            CHECK(AT.size() == 2);

            AT.insert({0, 2});
            AT.insert({0, 3});
            AT.insert({1, 2});
            CHECK(AT.size() == 4);
        }

        SECTION("Deletion") {
            AT.insert({0, 1});
            AT.insert({1, 2});
            AT.insert({0, 2});
            AT.insert({0, 3});
            AT.insert({1, 2});

            AT.erase(AT.find({0, 2}));
            CHECK(AT.size() == 3);

            AT.erase(AT.find({0, 3}));
            CHECK(AT.size() == 2);

            AT.erase(AT.find({0, 2}));
            CHECK(AT.size() == 2);
        }

        SECTION("Value") {
            AT.insert({1, 2});
            AT.insert({1 << 30, 1 << 29});
            CHECK(AT.size() == 2);
        }
    }

}