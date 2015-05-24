#ifndef KONIG_ADJACENCYMANAGER_HPP
#define KONIG_ADJACENCYMANAGER_HPP

#include <unordered_map>
#include "AdjacencyTree.hpp"

namespace konig {

    /**
     * AdjacencyManager (type)
     *
     * This is a handy wrapper for AdjacencyTree.
     * //TODO
     */
    class AdjacencyManager {

        //////////////////////////
        // Subtypes             //
        //////////////////////////
    public:
        typedef AdjacencyTree::iterator iterator;

        //////////////////////////
        // Members              //
        //////////////////////////
    private:
        AdjacencyTree adjacency_tree;

        std::unordered_map<vid_t, AdjacencyTree::iterator> vertex_range_first;
        std::unordered_map<vid_t, AdjacencyTree::iterator> vertex_range_last;


        //////////////////////////
        // Methods              //
        //////////////////////////
    public:
        /**
         * begin (overloaded method)
         *
         * This returns an iterator to the first adjacency stored in the underlying tree.
         */
        iterator begin() {
            return adjacency_tree.begin();
        }

        /**
         * begin (overloaded method)
         *
         * This returns an iterator to the first adjacency having `vertex` as its first endpoint.
         */
        iterator begin(const vid_t vertex) noexcept {
            if (vertex_range_first.count(vertex))
                return vertex_range_first.find(vertex)->second;
            else
                return end();
        }

        /**
         * end (overloaded method)
         *
         * This returns an iterator corresponding to the past-the-end element of the underlying tree.
         */
        iterator end() noexcept {
            return adjacency_tree.end();
        }

        /**
         * end (overloaded method)
         *
         * This returns an iterator corresponding to the past-the-end element of the underlying tree.
         */
        iterator end(const vid_t vertex) noexcept {
            if (vertex_range_last.count(vertex))
                return vertex_range_last.find(vertex)->second + 1;
            else
                return end();
        }

        /**
         * insert (method)
         *
         * This inserts a new adjacency. If the adjacency is already there, it does nothing.
         */
        iterator insert(const adjacency_t adjacency) {
            const auto& u = adjacency.first;

            const auto it = adjacency_tree.insert(adjacency);

            if (!vertex_range_first.count(u))
                vertex_range_first.emplace(u, end());
            if (!vertex_range_last.count(u))
                vertex_range_last.emplace(u, end());

            if (vertex_range_first.find(u)->second.is_past_the_end() || *(vertex_range_first.find(u)->second) > adjacency)
                vertex_range_first.find(u)->second = it;
            if (vertex_range_last.find(u)->second.is_past_the_end() || *(vertex_range_last.find(u)->second) < adjacency)
                vertex_range_last.find(u)->second = it;

            return it;
        }

        /**
         * erase (overloaded method)
         *
         * This deletes an adjacency from the structure. If the adjacency does not exist anymore, nothing happens.
         */
        void erase(const adjacency_t adjacency) {
            const auto it = adjacency_tree.find(adjacency);
            if (!it.is_past_the_end())
                erase(it);
        }

        /**
         * erase (overloaded method)
         *
         * This deletes an adjacency given an iterator.
         */
        void erase(const iterator it) {
#ifdef KONIG_DEBUG
            assert(!it.is_past_the_end());
#endif
            const auto& u = it->first;

#ifdef KONIG_DEBUG
            assert(vertex_range_first.count(u));
            assert(vertex_range_last.count(u));
            assert(!vertex_range_first.find(u)->second.is_past_the_end());
            assert(!vertex_range_last.find(u)->second.is_past_the_end());
#endif
            if (vertex_range_first.find(u)->second == vertex_range_last.find(u)->second) {  // Special case: only one adjacency having u as .first
#ifdef KONIG_DEBUG
                assert(vertex_range_first.find(u)->second == it);
#endif
                vertex_range_first.erase(u);
                vertex_range_last.erase(u);
            } else {
                if (it == vertex_range_first.find(u)->second)
                    ++vertex_range_first.find(u)->second;
                else if (it == vertex_range_last.find(u)->second)
                    --vertex_range_last.find(u)->second;
            }

            adjacency_tree.erase(it);
        }

        /**
         * size (method)
         *
         * This returns the number of adjacencies inserted
         */
        size_t size() const {
            return adjacency_tree.size();
        }
    };
}

#endif //KONIG_ADJACENCYMANAGER_HPP
