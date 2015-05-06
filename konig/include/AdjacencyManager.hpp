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

        class AdjSplayTree {
        public:
            /**
             * @brief Splay tree node model.
             *
             * A AdjSplayNode contains the fields required by our augmented splay tree to locate
             * edges by rank.
             */
            struct AdjSplayNode {
                // Node pointers
                AdjSplayNode* parent;
                AdjSplayNode* left_child;
                AdjSplayNode* right_child;

                // BBST Augmentation
                size_t subtree_size;
                size_t left_subtree_size;
                adjacency_t adjacency;

                void on_update() {
                    subtree_size = 1;
                    left_subtree_size = 0;

                    if (left_child) {
                        left_subtree_size = left_child->subtree_size;
                        subtree_size += left_child->subtree_size;
                    }
                    if (right_child) {
                        subtree_size += right_child->subtree_size;
                    }
                }
            };

            // Methods
            static bool is_root(AdjSplayNode* node) {
                return !node->parent;
            }

            static bool is_left_child(AdjSplayNode* node) {
                if (is_root(node))
                    return false;
                return node->parent->left_child == node;
            }

            static bool is_right_child(AdjSplayNode* node) {
                if (is_root(node))
                    return false;
                return node->parent->right_child == node;
            }

            static void rotate_right(AdjSplayNode* node) {
                AdjSplayNode* left_child = node->left_child;

                if (left_child) {
                    node->left_child = left_child->right_child;
                    if (left_child->right_child)
                        left_child->right_child->parent = node;
                    left_child->parent = node->parent;
                    left_child->right_child = node;
                }
                if (node->parent) {
                    if (is_left_child(node))
                        node->parent->left_child = left_child;
                    else
                        node->parent->right_child = left_child;
                }
                node->parent = left_child;

                node->on_update();
                if (left_child)
                    left_child->on_update();
            }

            static void rotate_left(AdjSplayNode* node) {
                AdjSplayNode* right_child = node->right_child;

                if (right_child) {
                    node->right_child = right_child->left_child;
                    if (right_child->left_child)
                        right_child->left_child->parent = node;
                    right_child->parent = node->parent;
                    right_child->left_child = node;
                }
                if (node->parent) {
                    if (is_left_child(node))
                        node->parent->left_child = right_child;
                    else
                        node->parent->right_child = right_child;
                }
                node->parent = right_child;

                node->on_update();
                if (right_child)
                    right_child->on_update();
            }

            static void splay(AdjSplayNode* node) {
                while (!is_root(node)) {
                    if (is_root(node->parent)) { // Zig step
                        if (is_left_child(node))
                            rotate_right(node->parent);
                        else
                            rotate_left(node->parent);
                    }
                    else if (is_left_child(node) && is_left_child(node->parent)) { // Zig-zig step (left)
                        rotate_right(node->parent->parent);
                        rotate_right(node->parent);
                    }
                    else if (is_right_child(node) && is_right_child(node->parent)) { // Zig-zig step (right)
                        rotate_left(node->parent->parent);
                        rotate_left(node->parent);
                    }
                    else if (is_left_child(node) && is_right_child(node->parent)) { // Zig-zag step (left-right)
                        rotate_right(node->parent);
                        rotate_left(node->parent);
                    }
                    else { // Zig-zag step (right-left)
                        rotate_left(node->parent);
                        rotate_right(node->parent);
                    }
                }
            }

            static AdjSplayNode* leftmost(AdjSplayNode* node) {
                while (node->left_child)
                    node = node->left_child;
                return node;
            }

            static AdjSplayNode* rightmost(AdjSplayNode* node) {
                while (node->right_child)
                    node = node->right_child;
                return node;
            }

            static AdjSplayNode* root(AdjSplayNode* node) {
                while (!is_root(node))
                    node = node->parent;
                return node;
            }

            static AdjSplayNode* successor(AdjSplayNode* node) {
                splay(node);
                return leftmost(node->right_child);
            }

            static AdjSplayNode* nth_successor(AdjSplayNode* node, size_t increment) {
                //FIXME: Implement this efficiently, using select
                for (; increment; --increment)
                    node = successor(node);
                return node;
            }

            static AdjSplayNode* predecessor(AdjSplayNode* node) {
                splay(node);
                return rightmost(node->left_child);
            }

            static AdjSplayNode* nth_predecessort(AdjSplayNode* node, size_t decrement) {
                //FIXME: Implement this efficiently, using select
                for (; decrement; --decrement)
                    node = predecessor(node);
                return node;
            }

            static size_t rank(AdjSplayNode* node) {
                splay(node);
                return 1 + node->left_subtree_size;
            }

            static size_t select(AdjSplayNode* node, size_t rank) {
                node = root(node);
                while (node && rank) {
                    if (node->left_subtree_size >= rank) {
                        node = node->left_child;
                    } else {
                        rank -= 1 + node->left_subtree_size;
                        node = node->right_child;
                    }
                }
                return node;
            }

            static void erase(AdjSplayNode* node) {
                splay(node);
                if (!node->left_child) {
                    if (node->right_child)
                        node->right_child->parent = NULL;
                }
                else if (!node->right_child) {
                    if (node->left_child)
                        node->left_child->parent = NULL;
                }
                else {
                    node->left_child->parent = NULL;
                    node->right_child->parent = NULL;
                    join(node->left_child, node->right_child);
                    node->left_child = node->right_child = NULL;
                }
            }
        };

        /**
         * @brief Iterator over the (weighted) adjacencies.
         *
         */
        class iterator : std::iterator<std::random_access_iterator_tag, adjacency_t> {
            friend class AdjacencyManager;

            using AdjSplayNode = typename AdjSplayTree::AdjSplayNode;

        private:
            AdjacencyManager& am_instance;
            AdjSplayNode* splay_node;

            iterator(AdjacencyManager& am_instance, AdjSplayNode* splay_node)
                    : am_instance(am_instance), splay_node(splay_node) { }

        public:
            iterator(const iterator& other) : am_instance(other.am_instance) {
                splay_node = other.splay_node;
            }

            adjacency_t& operator*() const {
                return splay_node->adjacency;
            }

            const iterator& operator++() {
                splay_node = AdjSplayTree::successor(splay_node);
                return *this;
            }

            iterator operator++(int) {
                iterator copy(*this);
                ++(*this);
                return copy;
            }

            const iterator& operator--() {
                splay_node = AdjSplayTree::predecessor(splay_node);
                return *this;
            }

            iterator operator--(int) {
                iterator copy(*this);
                --(*this);
                return copy;
            }

            bool operator==(const iterator& other) const {
                return splay_node->adjacency.endpoints == other.splay_node->adjacency.endpoints;
            }

            bool operator!=(const iterator& other) const {
                return splay_node->adjacency.endpoints != other.splay_node->adjacency.endpoints;
            }

            bool operator<(const iterator& other) const {
                return splay_node->adjacency.endpoints < other.splay_node->adjacency.endpoints;
            }

            bool operator>(const iterator& other) const {
                return splay_node->adjacency.endpoints > other.splay_node->adjacency.endpoints;
            }

            bool operator<=(const iterator& other) const {
                return splay_node->adjacency.endpoints <= other.splay_node->adjacency.endpoints;
            }

            bool operator>=(const iterator& other) const {
                return splay_node->adjacency.endpoints >= other.splay_node->adjacency.endpoints;
            }

            iterator& operator+(const size_t& increment) const {
                iterator copy(*this);
                copy += increment;
                return copy;
            }

            iterator& operator+=(const size_t& increment) {
                splay_node = AdjSplayTree::nth_successor(splay_node, increment);
                return *this;
            }

            iterator& operator-(const size_t& decrement) const {
                iterator copy(*this);
                copy -= decrement;
                return copy;
            }

            iterator& operator-=(const size_t& decrement) {
                splay_node = AdjSplayTree::nth_predecessor(splay_node, decrement);
                return *this;
            }
        };

        // Members
    private:
        std::vector<iterator> vertex_lower_bound;

        size_t n_vertices;
        size_t n_adjacencies;

        // Methods
    private:

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
            if (first < n_vertices - 1) {
                return vertex_lower_bound[first + 1];
            } else {
                auto self = const_cast<AdjacencyManager*>(this);
                return iterator(*self, NULL);
            }
        }

        iterator kth_present(const size_t k) noexcept {
            return begin() + k;
        }

        adjacency_t kth_absent(const size_t k) noexcept {

        }
    };
}

#endif //KONIG_ADJACENCYMANAGER_HPP