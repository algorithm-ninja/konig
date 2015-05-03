#ifndef KONIG_ADJACENCYMANAGER_HPP
#define KONIG_ADJACENCYMANAGER_HPP

#include <iterator>
#include <memory>
#include <vector>

#include "util.hpp"

namespace konig {

    template<typename weight_t>
    class AdjacencyManager {

        // Defined types
    public:
        typedef std::pair<vid_t, vid_t> adjacency_endpoints_t;

        struct adjacency_t {
            adjacency_endpoints_t endpoints;
            weight_t weight;

            adjacency_t(adjacency_endpoints_t endpoints, weight_t weight)
                    : endpoints(endpoints), weight(weight) { }

            bool operator<(const adjacency_t& other) const {
                return endpoints < other.endpoints;
            }
        };

    private:
        /**
         * @brief Splay tree node.
         *
         * A SplayNode contains the fields required by our augmented splay tree to locate
         * edges by rank.
         */
        class SplayNode {
        private:
            // Node pointers
            SplayNode* parent;
            std::unique_ptr<SplayNode> left_child;
            std::unique_ptr<SplayNode> right_child;

            // BBST Augmentation
            vid_t left_subtree_size;

        public:
            adjacency_t adjacency;
        };

    public:
        /**
         * @brief Iterator over the (weighted) adjacencies.
         *
         */
        class iterator : std::iterator<std::random_access_iterator_tag, adjacency_t> {
        private:
            SplayNode* splay_node;
        public:
            iterator(SplayNode* splay_node)
                    : splay_node(splay_node) { }
        };

        // Members
    private:
        SplayNode* splay_root;
        std::vector<iterator> vertex_lower_bound;

        size_t n_vertices;
        size_t n_adjacencies;

        // Methods
    public:

        /**
         * @brief Inserts adjacency in the AdjacencyManager.
         *
         * @param w_adjacency A weighted adjacency
         */
        void insert(const adjacency_t adjacency) {

        }

        /**
         * @brief Deletes adjacency in the AdjacencyManager, given its endpoints.
         *
         * @param endpoints The endpoints of the adjacency to be deleted
         */
        void erase(const adjacency_endpoints_t endpoints) {

        }

        /**
         * @brief Deletes adjacency, given an adjacency_t object.
         *
         * This is just a handy wrapper for erase(const adjacency_endpoints_t).
         * @warning This function completely ignores the weight field.
         *
         * @param adjacency A weighted adjacency object
         */
        void erase(const adjacency_t adjacency) {
            erase(adjacency.endpoints);
        }

        /**
         * @brief Deletes adjacency in the AdjacencyManager, given a valid iterator.
         *
         * @param iter Iterator to the adjacency to be deleted
         */
        void erase(const iterator iter) {
            erase(iter->endpoints);
        }

        /**
         * @brief Returns an iterator to the first adjacency coming after the provided one, according to
         *        standard increasing lexicographical ordering.
         *
         * @param endpoints The endpoints of the adjacency
         */
        iterator lower_bound(const adjacency_endpoints_t endpoints) noexcept {

        }

        /**
         * @brief Returns an iterator to the first adjacency coming after the provided one, according to
         *        standard increasing lexicographical ordering.
         *
         * This is just a handy wrapper for lower_bound(const adjacency_endpoints_t).
         * @warning This function completely ignores the weight field.
         *
         * @param adjacency A weighted adjacency object
         */
        iterator lower_bound(const adjacency_t adjacency) noexcept {
            return lower_bound(adjacency.endpoints);
        }

        /**
         * @brief Returns an iterator to the adjacency having the provided endpoints.
         *
         * If no stored adjacency matches the provided endpoints, this->end() will be returned.
         *
         * @param iter Iterator to the adjacency to be deleted
         */
        iterator find(const adjacency_endpoints_t endpoints) noexcept {
            auto ans = lower_bound(endpoints);
            if (ans == end() || ans->endpoints != endpoints)
                return end();
            else
                return ans;
        }

        /**
         * @brief Returns an iterator to the adjacency having the provided endpoints.
         *
         * This is just a handy wrapper for find(const adjacency_endpoints_t).
         * @warning This function completely ignores the weight field.
         *
         * @param iter Iterator to the adjacency to be deleted
         */
        iterator find(const adjacency_t adjacency) noexcept {
            return find(adjacency.endpoints);
        }

        /**
         * @brief Returns an iterator to the first adjacency.
         */
        iterator begin() const noexcept {
            return begin(0);
        }

        /**
         * @brief Returns an iterator to the first adjacency having a given first endpoint.
         *
         * @param first The first endpoint
         */
        iterator begin(const vid_t first) const noexcept {
            return vertex_lower_bound[first];
        }

        /**
         * @brief Returns an iterator referring to the past-the-end element.
         */
        iterator end() const noexcept {
            return end(n_vertices - 1);
        }

        /**
         * @brief Returns an iterator referring to the past-the-end element in the range of adjacencies having
         *        a given first endpoint.
         *
         * @param first The first endpoint
         */
        iterator end(const vid_t first) const noexcept {
            if (first < n_vertices - 1)
                return vertex_lower_bound[first + 1];
            else
                return NULL;
        }

        iterator kth_present(const size_t k) noexcept {
            return std::advance(begin(), k);
        }

        adjacency_t kth_absent(const size_t k) noexcept {

        }
    };
}

#endif //KONIG_ADJACENCYMANAGER_HPP