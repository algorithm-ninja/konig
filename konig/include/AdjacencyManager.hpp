#ifndef KONIG_ADJACENCYMANAGER_HPP
#define KONIG_ADJACENCYMANAGER_HPP

#include <iterator>
#include <memory>
#include <vector>
#include <cassert>
#include "util.hpp"
#include "Exception.hpp"

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
        };

        class AdjSplayTree {
            // Defined types
        public:
            /**
             * @brief Splay tree vertex model.
             *
             * A AdjSplayVertex contains the fields required by our augmented splay tree to locate
             * edges by rank.
             */
            struct AdjSplayVertex {
                friend class AdjSplayTree;

                // Members
            private:
                // Node pointers
                AdjSplayVertex* parent = NULL;
                AdjSplayVertex* left_child = NULL;
                AdjSplayVertex* right_child = NULL;

                // BBST Augmentation
                size_t subtree_size = 1;
                size_t left_subtree_size = 0;

            public:
                adjacency_t adjacency;

                // Methods
            private:
                void update() noexcept {
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

            public:
                AdjSplayVertex(adjacency_t adjacency) : adjacency(adjacency) { }

            };

            // Members
        private:
            AdjSplayVertex* tree_root = NULL;

            // Methods
        private:
            /**
             * @brief Checks whether `vertex` is the current root of the splay tree.
             *
             * @pre `vertex` in *not* NULL
             */
            bool is_root(AdjSplayVertex* const vertex) const noexcept {
                assert(vertex);
                return !vertex->parent;
            }

            AdjSplayVertex* root() const noexcept {
                return tree_root;
            }

            /**
             * @brief Checks whether `vertex` is a left child.
             *
             * @pre `vertex` in *not* NULL
             */
            bool is_left_child(AdjSplayVertex* const vertex) const noexcept {
                assert(vertex);
                if (is_root(vertex))
                    return false;
                return vertex->parent->left_child == vertex;
            }

            /**
             * @brief Checks whether `vertex` is a right child.
             *
             * @pre `vertex` in *not* NULL
             */
            bool is_right_child(AdjSplayVertex* const vertex) const noexcept {
                assert(vertex);
                if (is_root(vertex))
                    return false;
                return vertex->parent->right_child == vertex;
            }

            /**
             * @brief Returns a pointer to the minimum-key vertex in the subtree rooted in `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            AdjSplayVertex* minimum(AdjSplayVertex* const vertex) const noexcept {
                assert(vertex);
                auto ans = vertex;
                while (ans->left_child)
                    ans = ans->left_child;
                return ans;
            }

            /**
             * @brief Returns a pointer to the maximum-key vertex in the subtree rooted in `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            AdjSplayVertex* maximum(AdjSplayVertex* const vertex) const noexcept {
                assert(vertex);
                auto ans = vertex;
                while (ans->right_child)
                    ans = ans->right_child;
                return ans;
            }

            /**
             * @brief Performs a right (clockwise) tree rotation around `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            void rotate_right(AdjSplayVertex* const vertex) noexcept {
                assert(vertex);
                AdjSplayVertex* left_child = vertex->left_child;

                if (left_child) {
                    vertex->left_child = left_child->right_child;
                    if (left_child->right_child)
                        left_child->right_child->parent = vertex;
                    left_child->parent = vertex->parent;
                    left_child->right_child = vertex;
                }
                if (vertex->parent) {
                    if (is_left_child(vertex))
                        vertex->parent->left_child = left_child;
                    else
                        vertex->parent->right_child = left_child;
                }
                vertex->parent = left_child;

                vertex->update();
                if (left_child)
                    left_child->update();

                if (is_root(left_child))
                    tree_root = left_child;
            }

            /**
             * @brief Performs a left (counterclockwise) tree rotation around `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            void rotate_left(AdjSplayVertex* const vertex) noexcept {
                assert(vertex);
                AdjSplayVertex* right_child = vertex->right_child;

                if (right_child) {
                    vertex->right_child = right_child->left_child;
                    if (right_child->left_child)
                        right_child->left_child->parent = vertex;
                    right_child->parent = vertex->parent;
                    right_child->left_child = vertex;
                }
                if (vertex->parent) {
                    if (is_left_child(vertex))
                        vertex->parent->left_child = right_child;
                    else
                        vertex->parent->right_child = right_child;
                }
                vertex->parent = right_child;

                vertex->update();
                if (right_child)
                    right_child->update();

                if (is_root(right_child))
                    tree_root = right_child;
            }

            /**
             * @brief Reroots the splay tree in `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            void splay(AdjSplayVertex* const vertex) noexcept {
                assert(vertex);
                while (!is_root(vertex)) {
                    if (is_root(vertex->parent)) { // Zig step
                        if (is_left_child(vertex))
                            rotate_right(vertex->parent);
                        else
                            rotate_left(vertex->parent);
                    }
                    else if (is_left_child(vertex) && is_left_child(vertex->parent)) { // Zig-zig step (left)
                        rotate_right(vertex->parent->parent);
                        rotate_right(vertex->parent);
                    }
                    else if (is_right_child(vertex) && is_right_child(vertex->parent)) { // Zig-zig step (right)
                        rotate_left(vertex->parent->parent);
                        rotate_left(vertex->parent);
                    }
                    else if (is_left_child(vertex) && is_right_child(vertex->parent)) { // Zig-zag step (left-right)
                        rotate_right(vertex->parent);
                        rotate_left(vertex->parent);
                    }
                    else { // Zig-zag step (right-left)
                        rotate_left(vertex->parent);
                        rotate_right(vertex->parent);
                    }
                }
                tree_root = vertex;
            }

            /**
             * @brief Joins two disjoint trees.
             *
             * Keys belonging to the tree containing `v` *must* be greater than or equal to all the keys belonging to the
             * tree containing `u`.
             */
            void join(AdjSplayVertex* const u, AdjSplayVertex* const v) noexcept {
                assert(is_root(u) && is_root(v));
                auto max_u = maximum(u);

                splay(max_u);
                max_u->right_child = v;
                v->parent = max_u;
                max_u->update();

                tree_root = max_u;
            }

            /**
            * @brief Splits the tree at `vertex`.
            *
            * This creates two trees, one containing all vertices up to `vertex` (inclusive), and the other
            * containing vertices from `successor(vertex)` onwards.
            */
            void split(AdjSplayVertex* vertex) noexcept {
                splay(vertex);
                if (vertex->right_child)
                    vertex->right_child->parent = NULL;
                vertex->right_child = NULL;
                vertex->update();
            }

        public:
            /**
             * @brief Returns a pointer to the minimum (leftmost) element in the splay tree.
             */
            AdjSplayVertex* minimum() const noexcept {
                if (root())
                    return minimum(root());
                else
                    return NULL;
            }

            /**
             * @brief Returns a pointer to the maximum (rightmost) element in the splay tree.
             */
            AdjSplayVertex* maximum() const noexcept {
                if (root())
                    return maximum(root());
                else
                    return NULL;
            }

            /**
             * @brief Returns the rank of `vertex`.
             *
             * @pre `vertex` in *not* NULL
             */
            size_t rank(AdjSplayVertex* vertex) noexcept {
                assert(vertex);
                splay(vertex);
                return 1 + vertex->left_subtree_size;
            }

            /**
             * @brief Returns the vertex whose rank is `rank`.
             */
            AdjSplayVertex* select(size_t rank) noexcept {
                auto vertex = root();
                while (vertex && rank) {
                    if (vertex->left_subtree_size >= rank) {
                        vertex = vertex->left_child;
                    } else if (vertex->left_subtree_size < rank - 1) {
                        rank -= 1 + vertex->left_subtree_size;
                        vertex = vertex->right_child;
                    } else {
                        rank = 0;
                    }
                }
                return vertex;
            }

            /**
             * @brief Returns the pointer to the vertex whose rank is `rank(vertex) + delta`.
             *
             * @pre `vertex` in *not* NULL
             */
            AdjSplayVertex* advance(AdjSplayVertex* vertex, std::ptrdiff_t delta) noexcept {
                assert(vertex);
                return select(rank(vertex) + delta);
            }

            /**
             * @brief Returns the leftmost node that evaluates as >= endpoints
             */
            AdjSplayVertex* lower_bound(adjacency_endpoints_t endpoints) noexcept {
                AdjSplayVertex* vertex = root();
                AdjSplayVertex* cut_point = NULL;

                while (vertex) {
                    if (vertex->adjacency.endpoints >= endpoints) {
                        cut_point = vertex;
                        vertex = vertex->left_child;

                        if (vertex->adjacency.endpoints == endpoints) // There are no duplicates in this structure
                            break;
                    } else {
                        vertex = vertex->right_child;
                    }
                }

                if (cut_point)
                    splay(cut_point);
                return cut_point;
            }

            /**
             * @brief Returns the leftmost node that evaluates as > endpoints
             */
            AdjSplayVertex* upper_bound(adjacency_endpoints_t endpoints) noexcept {
                AdjSplayVertex* vertex = root();
                AdjSplayVertex* cut_point = NULL;

                while (vertex) {
                    if (vertex->adjacency.endpoints > endpoints) {
                        cut_point = vertex;
                        vertex = vertex->left_child;
                    } else {
                        vertex = vertex->right_child;
                    }
                }

                if (cut_point)
                    splay(cut_point);
                return cut_point;
            }

            /**
             * @brief Checks whether the tree contains a vertex having key `endpoints`.
             */
            bool has(adjacency_endpoints_t endpoints) noexcept {
                auto key_lower_bound = lower_bound(endpoints);

                return (key_lower_bound && key_lower_bound->adjacency.endpoints == endpoints);
            }

            AdjSplayVertex* find(AdjacencyManager* vertex, adjacency_endpoints_t endpoints) noexcept {
                return lower_bound(vertex, endpoints);
            }

            /**
             * @brief Creates a new vertex corresponding to the given adjacency and returns the pointer
             * to the newly created vertex.
             */
            AdjSplayVertex* insert(adjacency_t adjacency) noexcept {
                AdjSplayVertex* new_vertex = new AdjSplayVertex(adjacency);

                if (root()) {
                    auto cut_point = lower_bound(adjacency.endpoints);

                    if (cut_point) {
                        auto prev = advance(cut_point, -1);
                        if (prev) {
                            split(prev);
                            join(new_vertex, cut_point);
                            join(prev, new_vertex);
                        } else {
                            join(new_vertex, cut_point);
                        }
                    } else {
                        join(root(), new_vertex);
                    }

                    assert(is_root(root()));
                }

                splay(new_vertex);
                return new_vertex;
            }

            /**
             * @brief Deletes `vertex` from the tree.
             */
            void erase(AdjSplayVertex* vertex) noexcept {
                splay(vertex);
                if (!vertex->left_child) {
                    if (vertex->right_child)
                        vertex->right_child->parent = NULL;

                    tree_root = vertex->right_child;
                }
                else if (!vertex->right_child) {
                    if (vertex->left_child)
                        vertex->left_child->parent = NULL;

                    tree_root = vertex->left_child;
                }
                else {
                    vertex->left_child->parent = NULL;
                    vertex->right_child->parent = NULL;

                    join(vertex->left_child, vertex->right_child);
                    vertex->left_child = vertex->right_child = NULL;
                }

                assert(is_root(root()));
                delete vertex;
            }
        } adj_splay_tree;

        /**
         * @brief Iterator over the (weighted) adjacencies.
         *
         */
        class iterator : std::iterator<std::random_access_iterator_tag, adjacency_t> {
            friend class AdjacencyManager;

            using AdjSplayVertex = typename AdjSplayTree::AdjSplayVertex;

        private:
            AdjacencyManager& am_instance;
            AdjSplayVertex* splay_vertex;

            iterator(AdjacencyManager& am_instance, AdjSplayVertex* splay_vertex)
                    : am_instance(am_instance), splay_vertex(splay_vertex) { }

        public:
            iterator(const iterator& other) : am_instance(other.am_instance) {
                splay_vertex = other.splay_vertex;
            }

            adjacency_t& operator*() const noexcept {
                return splay_vertex->adjacency;
            }

            adjacency_t* operator->() const noexcept {
                return &(splay_vertex->adjacency);
            }

            const iterator& operator++() noexcept {
                *(this) += 1;
                return *this;
            }

            iterator operator++(int) noexcept {
                iterator copy(*this);
                ++(*this);
                return copy;
            }

            const iterator& operator--() noexcept {
                *(this) -= 1;
                return *this;
            }

            iterator operator--(int) noexcept {
                iterator copy(*this);
                --(*this);
                return copy;
            }

            bool operator==(const iterator& other) const noexcept {
                if (splay_vertex == NULL || other.splay_vertex == NULL)
                    return splay_vertex == other.splay_vertex;
                else
                    return splay_vertex->adjacency.endpoints == other.splay_vertex->adjacency.endpoints;
            }

            bool operator!=(const iterator& other) const noexcept {
                if (splay_vertex == NULL || other.splay_vertex == NULL)
                    return splay_vertex != other.splay_vertex;
                else
                    return splay_vertex->adjacency.endpoints != other.splay_vertex->adjacency.endpoints;
            }

            iterator& operator+(const size_t increment) const noexcept {
                iterator copy(*this);
                copy += increment;
                return copy;
            }

            iterator& operator+=(const size_t increment) noexcept {
                splay_vertex = am_instance.adj_splay_tree.advance(splay_vertex, increment);
                return *this;
            }

            iterator& operator-(const size_t decrement) const noexcept {
                iterator copy(*this);
                copy -= decrement;
                return copy;
            }

            iterator& operator-=(const size_t decrement) noexcept {
                splay_vertex = am_instance.adj_splay_tree.advance(splay_vertex, -decrement);
                return *this;
            }
        };

        // Members
    private:
        std::vector<typename AdjSplayTree::AdjSplayVertex*> vertex_first_adj;
        std::vector<typename AdjSplayTree::AdjSplayVertex*> vertex_last_adj;

        size_t n_vertices = 0;
        size_t n_adjacencies = 0;

        // Methods
    private:
        bool is_valid_adjacency_endpoints(const adjacency_endpoints_t endpoints) const noexcept {
            return (endpoints.first < n_vertices &&
                    endpoints.second < n_vertices &&
                    endpoints.first != endpoints.second);
        }

        bool is_valid_adjacency(const adjacency_t adjacency) const noexcept {
            return is_valid_adjacency_endpoints(adjacency.endpoints);
        }

        iterator make_iterator(typename AdjSplayTree::AdjSplayVertex* ptr) const noexcept {
            auto self = const_cast<AdjacencyManager*>(this);
            return iterator(*self, ptr);
        }

    public:
        /**
         * @brief Adds a vertex to the structure.
         */
        void push_vertex() {
            ++n_vertices;
            vertex_first_adj.resize(n_vertices, NULL);
            vertex_last_adj.resize(n_vertices, NULL);
        }

        /*
         * @brief Deletes the last vertex.
         */
        void pop_vertex() {
            for (auto it = begin(n_vertices - 1); it != end(n_vertices - 1); ++it)
                erase_adjacency(it);

            --n_vertices;
            vertex_first_adj.resize(n_vertices, NULL);
            vertex_last_adj.resize(n_vertices, NULL);
        }

        /**
         * @brief Inserts a new adjacency.
         *
         * @param adjacency A weighted adjacency
         */
        void insert_adjacency(const adjacency_t adjacency) {
            if (!is_valid_adjacency(adjacency))
                throw InvalidArgument(context_info("invalid adjacency"));
            if (adj_splay_tree.has(adjacency.endpoints))
                throw InvalidArgument(context_info("duplicate adjacency"));

            typename AdjSplayTree::AdjSplayVertex* new_adj = adj_splay_tree.insert(adjacency);

            const vid_t u = adjacency.endpoints.first;
            const vid_t v = adjacency.endpoints.second;

            if (!vertex_first_adj[u] || vertex_first_adj[u]->adjacency.endpoints.first > v)
                vertex_first_adj[u] = new_adj;
            if (!vertex_last_adj[u] || vertex_last_adj[u]->adjacency.endpoints.second < v)
                vertex_last_adj[u] = new_adj;
            ++n_adjacencies;
        }

        /**
         * @brief Deletes adjacency in the AdjacencyManager, given its endpoints.
         *
         * @param endpoints The endpoints of the adjacency to be deleted
         */
        void erase_adjacency(const adjacency_endpoints_t endpoints) {
            if (!is_valid_adjacency_endpoints(endpoints))
                throw InvalidArgument(context_info("invalid adjacency"));
            if (!adj_splay_tree.has(endpoints))
                throw InvalidArgument(context_info("missing adjacency"));

            --n_adjacencies;
            adj_splay_tree.erase(adj_splay_tree.find(endpoints));
        }

        /**
         * @brief Deletes adjacency, given an adjacency_t object.
         *
         * This is just a handy wrapper for erase(const adjacency_endpoints_t).
         * @warning This function completely ignores the weight field.
         *
         * @param adjacency A weighted adjacency object
         */
        void erase_adjacency(const adjacency_t adjacency) {
            erase(adjacency.endpoints);
        }

        /**
         * @brief Deletes adjacency in the AdjacencyManager, given a valid iterator.
         *
         * @param iter Iterator to the adjacency to be deleted
         */
        void erase_adjacency(const iterator iter) {
            erase(iter->endpoints);
        }

        /**
         * @brief Returns an iterator to the first adjacency coming after the provided one, according to
         *        standard increasing lexicographical ordering.
         *
         * @param endpoints The endpoints of the adjacency
         */
        iterator lower_bound(const adjacency_endpoints_t endpoints) {
            auto vertex_lower_bound = adj_splay_tree.lower_bound(endpoints);

            return make_iterator(vertex_lower_bound);
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
        iterator lower_bound(const adjacency_t adjacency) {
            return lower_bound(adjacency.endpoints);
        }

        /**
         * @brief Returns an iterator to the adjacency having the provided endpoints.
         *
         * If no stored adjacency matches the provided endpoints, this->end() will be returned.
         *
         * @param iter Iterator to the adjacency to be deleted
         */
        iterator find(const adjacency_endpoints_t endpoints) {
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
        iterator find(const adjacency_t adjacency) {
            return find(adjacency.endpoints);
        }

        /**
         * @brief Returns an iterator to the first adjacency.
         */
        iterator begin() const {
            return make_iterator(adj_splay_tree.minimum());
        }

        /**
         * @brief Returns an iterator to the first adjacency having a given first endpoint.
         *
         * @param first The first endpoint
         */
        iterator begin(const vid_t first) const {
            if (first >= n_vertices)
                throw InvalidArgument(context_info("the vertex is missing"));

            return make_iterator(vertex_first_adj[first]);
        }

        /**
         * @brief Returns an iterator referring to the past-the-end adjacency.
         */
        iterator end() const {
            return ++make_iterator(adj_splay_tree.maximum());
        }

        /**
         * @brief Returns an iterator referring to the past-the-end element in the range of adjacencies having
         *        a given first endpoint.
         *
         * @param first The first endpoint
         */
        iterator end(const vid_t first) const {
            if (first >= n_vertices)
                throw InvalidArgument(context_info("the vertex is missing"));

            if (first < n_vertices - 1) {
                return ++make_iterator(vertex_last_adj[first + 1]);
            } else {
                return make_iterator(NULL);
            }
        }

        iterator kth_present(const size_t k) {
            if (k == 0 || k > n_adjacencies)
                throw InvalidArgument(context_info("not enough adjacencies"));

            return begin() + (k - 1);
        }

        adjacency_t kth_absent(const size_t k) {
            if (k == 0 || k > (n_vertices * (n_vertices - 1)) / 2)
                throw InvalidArgument(context_info("not enough absent adjacencies"));

            //TODO
        }
    };
}

#endif //KONIG_ADJACENCYMANAGER_HPP