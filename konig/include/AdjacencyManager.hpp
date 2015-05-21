#ifndef KONIG_ADJACENCYMANAGER_HPP
#define KONIG_ADJACENCYMANAGER_HPP

#include <iterator>
#include <memory>
#include <vector>
#include <cassert>
#include "util.hpp"
#include "Exception.hpp"

namespace konig {
    /**
     * adjacency_t (type)
     *
     * This type represents a ``graph adjacency'', i.e. an ordered pair of vertices. Notice that the concept of
     * graph adjacency differs slightly from that of graph edge, as the former is always an ordered pair, while the
     * latter may be an *unordered* pair (think of an undirected graph, for instance).
     */
    typedef std::pair<vid_t, vid_t> adjacency_t;

    /**
     * AdjacencyManager (type)
     *
     * This is the heart of Konig. It provides a way to manage graph *adjacencies* (as opposed to edges),
     * allowing low-level operations such as:
     *  - adjacency insertion
     *  - adjacency deletions
     *  - adjacency lookup
     *  - random access iteration
     *
     * It internally uses an augmented Splay Tree data structure, to efficiently support all the above mentioned
     * operations. The vertices are sorted in increasing lexicographical order with respect to the adjacencies (seen
     * as pairs of vid_t types).
     *
     * Please notice that AdjacencyManager doesn't have any knowledge of high-level concepts such as adjacency weight,
     * or graph vertices. You can think of an AdjacencyManager simply as a ``container of pairs of vid_t types''.
     */
    class AdjacencyManager {

        //////////////////////////
        // Subtypes             //
        //////////////////////////

    public:
        /**
         * AdjSplayVertex (type)
         *
         * This type represents the Splay Tree vertex model. In order to support fast rank/select queries,
         * we augment the basic model with some information about the size of the subtree rooted in the current
         * splay vertex. This allows us to have logarithmic-time random access to the structure.
         */
        class AdjSplayVertex {
            friend class AdjacencyManager;

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
            /**
             * update (method)
             *
             * This method is called whenever the children of the current vertex change, for example after a splay
             * operation. It then updates the values of the augmented fields (such as subtree_size or left_subtree_size)
             * so that the tree remains in a consistent state.
             */
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

        /**
         * iterator (type)
         *
         * This defines an std::iterator compliant random access iterator over the splay tree structure. It internally
         * stores a pointer to the AdjSplayVertex storing the adjacency of interest.
         *
         * As any random access iterator, this supports operator++/--, as well as efficient operator+/operator-.
         *
         * Since we need to call AdjacencyManager::advance in order to efficiently advance the iterator (see, for
         * instance, operator+) we need to store a reference to the AdjacencyManager instance that forged the iterator.
         */
        class iterator : std::iterator<std::random_access_iterator_tag, adjacency_t> {
            friend class AdjacencyManager;

            // Members
        private:
            AdjacencyManager& am_instance;
            AdjSplayVertex* splay_vertex;

            iterator(AdjacencyManager& am_instance, AdjSplayVertex* splay_vertex)
                    : am_instance(am_instance), splay_vertex(splay_vertex) { }

            // Methods
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
                    return splay_vertex->adjacency == other.splay_vertex->adjacency;
            }

            bool operator!=(const iterator& other) const noexcept {
                if (splay_vertex == NULL || other.splay_vertex == NULL)
                    return splay_vertex != other.splay_vertex;
                else
                    return splay_vertex->adjacency != other.splay_vertex->adjacency;
            }

            iterator operator+(const size_t increment) const noexcept {
                iterator copy(*this);
                copy += increment;
                return copy;
            }

            iterator& operator+=(const size_t increment) noexcept {
                splay_vertex = am_instance.advance(splay_vertex, increment);
                return *this;
            }

            iterator operator-(const size_t decrement) const noexcept {
                iterator copy(*this);
                copy -= decrement;
                return copy;
            }

            iterator& operator-=(const size_t decrement) noexcept {
                splay_vertex = am_instance.advance(splay_vertex, -decrement);
                return *this;
            }
        };


        //////////////////////////
        // Members              //
        //////////////////////////

    private:
        AdjSplayVertex* tree_root = NULL;
        size_t tree_size = 0;



        //////////////////////////
        // Methods              //
        //////////////////////////

    private:
        /**
         * make_iterator (method)
         *
         * This creates an iterator, given a pointer to an AdjSplayVertex.
         */
        iterator make_iterator(AdjSplayVertex* const ptr) const noexcept {
            auto self = const_cast<AdjacencyManager*>(this);
            return iterator(*self, ptr);
        }

        /**
         * is_root (method)
         *
         * This checks whether `vertex` is the current root of the splay tree.
         *
         * @pre `vertex` in not NULL
         */
        bool is_root(AdjSplayVertex* const vertex) const noexcept {
            assert(vertex);
            return !vertex->parent;
        }

        /**
         * root (method)
         *
         * This returns the root of the tree.
         */
        AdjSplayVertex* root() const noexcept {
#ifdef KONIG_DEBUG
            assert(is_root(tree_root));
#endif
            return tree_root;
        }

        /**
         * is_left_child (method)
         *
         * This checks whether `vertex` is a left child.
         *
         * @pre `vertex` in *not* NULL
         */
        bool is_left_child(AdjSplayVertex* const vertex) const noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            if (is_root(vertex))
                return false;
            return vertex->parent->left_child == vertex;
        }

        /**
         * is_right_child (method)
         *
         * This hecks whether `vertex` is a right child.
         *
         * @pre `vertex` in *not* NULL
         */
        bool is_right_child(AdjSplayVertex* const vertex) const noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            if (is_root(vertex))
                return false;
            return vertex->parent->right_child == vertex;
        }

        /**
         * subtree_minimum (method)
         *
         * This returns a pointer to the minimum-key vertex in the subtree rooted in `vertex`.
         *
         * @pre `vertex` in *not* NULL
         */
        AdjSplayVertex* subtree_minimum(AdjSplayVertex* const vertex) const noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            auto ans = vertex;
            while (ans->left_child)
                ans = ans->left_child;
            return ans;
        }

        /**
         * subtree_maximum (method)
         *
         * This returns a pointer to the maximum-key vertex in the subtree rooted in `vertex`.
         *
         * @pre `vertex` in *not* NULL
         */
        AdjSplayVertex* subtree_maximum(AdjSplayVertex* const vertex) const noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            auto ans = vertex;
            while (ans->right_child)
                ans = ans->right_child;
            return ans;
        }

        /**
         * rotate_right (method)
         *
         * This performs a right (clockwise) tree rotation around `vertex`.

         * @pre `vertex` in *not* NULL
         */
        void rotate_right(AdjSplayVertex* const vertex) noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
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
         * rotate_left (method)
         *
         * This performs a left (counterclockwise) tree rotation around `vertex`.
         *
         * @pre `vertex` in *not* NULL
         */
        void rotate_left(AdjSplayVertex* const vertex) noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
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
         * splay (method)
         *
         * This reroots the splay tree in `vertex`.
         *
         * @pre `vertex` in *not* NULL
         */
        void splay(AdjSplayVertex* const vertex) noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
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
         * join (method)
         *
         * This joins two disjoint trees, given their roots. Keys belonging to the tree containing `v` *must* be greater
         *  than or equal to all the keys belonging to the tree containing `u`.
         *
         *  @pre: `u` and `v` must be roots.
         */
        void join(AdjSplayVertex* const u, AdjSplayVertex* const v) noexcept {
#ifdef KONIG_DEBUG
            assert(is_root(u) && is_root(v));
#endif
            auto max_u = subtree_maximum(u);

            splay(max_u);
            max_u->right_child = v;
            v->parent = max_u;
            max_u->update();

            tree_root = max_u;
        }

        /**
         * split (method)
         *
         * This splits the tree at `vertex`, and creates two trees, one containing all vertices up to `vertex`
         * (inclusive), and the other containing vertices from `successor(vertex)` onwards.
         */
        void split(AdjSplayVertex* const vertex) noexcept {
            splay(vertex);
            if (vertex->right_child)
                vertex->right_child->parent = NULL;
            vertex->right_child = NULL;
            vertex->update();
        }

        /**
         * tree_minimum (method)
         *
         * This returns a pointer to the minimum (leftmost) adjacency stored in the splay tree.
         */
        AdjSplayVertex* tree_minimum() const noexcept {
            if (root())
                return subtree_minimum(root());
            else
                return NULL;
        }

        /**
         * tree_maximum (method)
         *
         * This returns a pointer to the maximum (rightmost) adjacency stored in the splay tree.
         */
        AdjSplayVertex* tree_maximum() const noexcept {
            if (root())
                return subtree_maximum(root());
            else
                return NULL;
        }

        /**
         * _rank (method)
         *
         * This returns the rank of `vertex`.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         *
         * @pre `vertex` in *not* NULL
         */
        size_t _rank(AdjSplayVertex* vertex) noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            splay(vertex);
            return 1 + vertex->left_subtree_size;
        }

        /**
         * select (method)
         *
         * This returns the vertex whose rank is equal to `rank`.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        AdjSplayVertex* _select(size_t rank) noexcept {
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
         * advance (method)
         *
         * This returns the pointer to the vertex whose rank is `rank(vertex) + delta`.
         *
         * @pre `vertex` in *not* NULL
         */
        AdjSplayVertex* advance(AdjSplayVertex* vertex, std::ptrdiff_t delta) noexcept {
#ifdef KONIG_DEBUG
            assert(vertex);
#endif
            return _select(_rank(vertex) + delta);
        }

        /**
         * _lower_bound (method)
         *
         * This returns the leftmost node that evaluates as >= adjacency.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        AdjSplayVertex* _lower_bound(adjacency_t adjacency) noexcept {
            AdjSplayVertex* vertex = root();
            AdjSplayVertex* cut_point = NULL;

            while (vertex) {
                if (vertex->adjacency >= adjacency) {
                    cut_point = vertex;
                    vertex = vertex->left_child;

                    if (vertex->adjacency == adjacency) // There are no duplicates in this structure
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
         * _upper_bound (method)
         *
         * This returns the leftmost node that evaluates as > adjacency.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        AdjSplayVertex* _upper_bound(adjacency_t adjacency) noexcept {
            AdjSplayVertex* vertex = root();
            AdjSplayVertex* cut_point = NULL;

            while (vertex) {
                if (vertex->adjacency > adjacency) {
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
         * _find (method)
         *
         * This is the same as _lower_bound.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        AdjSplayVertex* _find(adjacency_t adjacency) noexcept {
            return _lower_bound(adjacency);
        }

        /**
         * _insert (method)
         *
         * This creates a new vertex corresponding to the given adjacency, and inserts it in the tree. It then returns
         * the pointer to the newly created vertex.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        AdjSplayVertex* _insert(adjacency_t adjacency) noexcept {
            AdjSplayVertex* new_vertex = new AdjSplayVertex(adjacency);

            if (root()) {
                auto cut_point = _lower_bound(adjacency);

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
         * _erase (method)
         *
         * It deletes `vertex` from the tree.
         *
         * As it is an internal function, working with raw pointers instead of iterators, it begins with an underscore.
         */
        void _erase(AdjSplayVertex* vertex) noexcept {
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

    public:

        /**
         * begin (method)
         *
         * This returns an iterator to the first adjacency stored in the structure.
         */
        iterator begin() {
            return make_iterator(tree_minimum());
        }

        /**
         * end (method)
         *
         * This returns an iterator corresponding to the past-the-end element of the structure.
         */
        iterator end() {
            return make_iterator(NULL);
        }

        /**
         * lower_bound (method)
         *
         * This returns an iterator to the first (leftmost) adjacency that evaluates as >= `adjacency`.
         *
         * @note: it is not required that `adjacency` belongs to the structure.
         */
        iterator lower_bound(const adjacency_t adjacency) noexcept {
            return make_iterator(_lower_bound(adjacency));
        }

        /**
         * upper_bound (method)
         *
         * This returns an iterator to the first (leftmost) adjacency that evaluates as > `adjacency`.
         *
         * @note: it is not required that `adjacency` belongs to the structure.
         */
        iterator upper_bound(const adjacency_t adjacency) noexcept {
            return make_iterator(_upper_bound(adjacency));
        }

        /**
         * insert (method)
         *
         * This inserts the given adjacency in the tree and returns an iterator to the inserted adjacency.
         */
        iterator insert(const adjacency_t adjacency) noexcept {
            return make_iterator(_insert(adjacency));
        }

        /**
         * erase (method)
         */
        void erase(const iterator it) noexcept {
            return _erase(it.splay_vertex);
        }

        /**
         * rank (method)
         *
         * This returns the rank of the adjacency represented by the supplied iterator.
         *
         * @pre: the iterator *must* be valid, i.e. point to a valid instance of AdjSplayVertex.
         */
        size_t rank(const iterator it) noexcept {
            return _rank(it.splay_vertex);
        }

        /**
         * select (method)
         *
         * This returns the adjacency given its rank.
         *
         */
        iterator select(const size_t rank) noexcept {
            return make_iterator(_select(rank));
        }


        /**
         * has (method)
         *
         * This checks whether the tree contains a vertex corresponding to `adjacency`.
         */
        bool has(adjacency_t adjacency) noexcept {
            auto key_lower_bound = _lower_bound(adjacency);

            return (key_lower_bound && key_lower_bound->adjacency == adjacency);
        }

    };

}

#endif //KONIG_ADJACENCYMANAGER_HPP