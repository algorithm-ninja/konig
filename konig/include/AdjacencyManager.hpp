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
        struct adjacency_t {
            vid_t first, second;
            weight_t weight;

            adjacency_t(vid_t first, vid_t second, weight_t weight = weight_t())
                    : first(first), second(second), weight(weight) { };

            bool operator<(const adjacency_t& other) const {
                return (first < other.first) || (first == other.first && second < other.second);
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
         * @brief Iterator object over the adjacencies.
         *
         */
        class iterator : std::iterator<std::random_access_iterator_tag, adjacency_t> {
        private:
            SplayNode* splay_node;
        };

        // Members
    private:
        SplayNode* splay_root;
        std::vector<iterator> vertex_lower_bound;

        size_t n_vertices;
        size_t n_adjacencies;

        // Methods
    public:
        void insert(const vid_t first, const vid_t second, const weight_t weight) {

        }

        void erase(const vid_t first, const vid_t second) {

        }

        // NOTE: find, lower_bound, ... cannot be declared as const, since the potentially
        // update the splay tree root.
        iterator find(const vid_t first, const vid_t second) noexcept {

        }

        iterator lower_bound(const vid_t first, const vid_t second) noexcept {

        }

        iterator begin() const noexcept {

        }

        iterator begin(const vid_t first) const noexcept {

        }

        iterator end() const noexcept {

        }

        iterator end(const vid_t first) const noexcept {

        }

        iterator kth_present(const size_t k) noexcept {
            return std::advance(begin(), k);
        }

        adjacency_t kth_absent(const size_t k) noexcept {

        }
    };
}

#endif //KONIG_ADJACENCYMANAGER_HPP