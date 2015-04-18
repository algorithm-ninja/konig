#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>
#include <functional>
#include <sstream>
#include <numeric>
#include <cmath>
#include "cpp-btree/btree_set.h"

typedef size_t vertex_t;
typedef struct{vertex_t tail, head;} edge_t;

template<typename T>
class Weighter;

bool operator<(const edge_t& a, const edge_t& b) {
    return a.tail < b.tail || (a.tail == b.tail && a.head < b.head);
}

class TooManyEdgesException: public std::exception {
    virtual const char* what() const noexcept {
        return "You specified too many edges!";
    }
};

class TooFewEdgesException: public std::exception {
    virtual const char* what() const noexcept {
        return "You specified too few edges!";
    }
};

class TooFewNodesException: public std::exception {
    virtual const char* what() const noexcept{
        return "You specified too few nodes!";
    }
};

class TooManySamplesException: public std::exception {
    virtual const char* what() const noexcept {
        return "You specified too many values to sample from the given range!";
    }
};

class NotImplementedException: public std::exception {
    virtual const char* what() const noexcept {
        return "This function is not implemented yet!";
    }
};

namespace Random {
    uint64_t rand_max = std::numeric_limits<uint64_t>::max();
    uint64_t x = 8867512362436069LL;
    uint64_t w;

    /**
     *  Simple 64-bit variant of the XorShift random number algorithm.
     */
    uint64_t xor128() {
        uint64_t t;
        t = x ^ (x << 11);
        x = w;
        return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
    }

    void srand(int S) {
        w = S;
        ::srand(S);
    }

    template<typename T1, typename T2>
    auto randrange(T1 bottom, T2 top)
    -> typename std::enable_if<!std::is_integral<decltype(bottom+top) >::value,
                               decltype(bottom+top)>::type {
        return double(xor128())/rand_max * (top - bottom) + bottom;
    }

    template<typename T1, typename T2>
    auto randrange(T1 bottom, T2 top)
    -> typename std::enable_if<std::is_integral< decltype(bottom+top) >::value,
                               decltype(bottom+top)>::type {
        return xor128() % (top - bottom) + bottom;
    }
}

namespace utils {
    template<typename T>
    void write_weight(
        Weighter<T>& weighter,
        const edge_t& edge,
        std::ostream& os
    ) {
        os << " " << weighter(edge);
    }

    template<>
    void write_weight(Weighter<void>&, const edge_t&, std::ostream&) {}
}

/**
 *  Labeler is the abstract class that defines the interface for a
 *  graph labeler functor, i.e. a callable object that assigns labels to vertices.
 */
template<typename T>
class Labeler {
public:
    typedef T label_t;

    virtual ~Labeler() {}

    /**
     *  label takes as argument the index of the node, and
     *  returns a label of type T for it. It must be a deterministic
     *  injective function.
     */
    virtual T operator()(const vertex_t i) = 0;
};

/**
 *  IotaLabeler is the simplest labeler. The label of the i-th vetex
 *  is simply the integer (i+start).
 */
class IotaLabeler: public virtual Labeler<int> {
private:
    int start;

public:
    IotaLabeler(int start = 0): start(start) {}
    ~IotaLabeler() {}

    int operator()(const vertex_t i) override {
        return start + i;
    }
};

/**
 *  RandIntLabeler assigns random labels from a given range
 */
class RandIntLabeler: public virtual Labeler<int> {
private:
    std::vector<int> labels;

public:
    /**
     *  Define the sample range [start, end)
     */
    RandIntLabeler(int start, int end) {
        labels.resize(end - start);
        std::iota(labels.begin(), labels.end(), start);
        std::random_shuffle(labels.begin(), labels.end());
    }
    ~RandIntLabeler() {}

    int operator()(const vertex_t i) {
        return labels.at(i);
    }
};

/**
 *  StaticLabeler assigns labels from a given vector
 */
template<typename T>
class StaticLabeler: public Labeler<T> {
private:
    std::vector<T>& labels;

public:
    StaticLabeler(const std::vector<T>& labels): labels(labels) {}
    ~StaticLabeler() {}

    T operator()(const vertex_t i) {
        return labels.at(i);
    }
};

/**
 *  Weighter is the abstract class that defines the interface for a
 *  graph weighter functor, i.e. a callable object that assigns weights to edges.
 */
template<typename T>
class Weighter {
public:
    typedef T weight_t;

    virtual ~Weighter() {}

    /**
     *  takes as arguments two vertex_t, corresponding to the tail and
     *  the head vertices of the edge of interest, and returns a weight
     *  of type T for the edge. It must be a deterministic function.
     */
    virtual T operator()(const edge_t& edge) = 0;
};

// TODO (?) Euclidean weights generator

/**
 *  RandomWeighter is the simplest weighter. It returns random weights taken
 *  from a given range of values
 */
template<typename T>
class RandomWeighter: public Weighter<T> {
private:
    T min, max; // Define the range

public:
    RandomWeighter(T min, T max): min(min), max(max) {};
    ~RandomWeighter() {};

    T operator()(const edge_t&) {
        return randrange(min, max);
    }
};

/**
 *  NoWeighter is a dummy weighter. It throws if called
 */
class NoWeighter: public Weighter<void> {
public:
    ~NoWeighter() {};

    void operator()(const edge_t&) {
        // TODO: Define a proper exception
        throw NotImplementedException();
    }
};

/**
 *  RangeSampler provides iterators for ranging over sampled integers
 *  in a given range.
 */
class RangeSampler {
private:
    std::vector<int64_t> samples;

public:
    /**
     *  The constructor generates the samples from the range [min, max].
     *
     *  @param sample_size the number of samples
     *  @param min the min of the range
     *  @param max the max of the range
     *  @param excl an optional vector of undesired values
     */
    RangeSampler(
        const size_t sample_size,
        const int64_t min,
        const int64_t max,
        std::vector<int64_t> excl = std::vector<int64_t>()
    ) {
        if (!std::is_sorted(excl.begin(), excl.end()))
            std::sort(excl.begin(), excl.end());

        // If the user requests too many samples, throw
        if (max - min < int64_t(sample_size + excl.size()))
            throw TooManySamplesException();

        auto top = max - sample_size - excl.size() + 1;
        samples.resize(sample_size);
        for (size_t i = 0; i < sample_size; i++)
            samples[i] = Random::randrange(min, top);

        // TODO: Is counting sort better than std::sort here?
        std::sort(samples.begin(), samples.end());
        size_t excl_idx = 0;
        for (size_t i = 0; i < sample_size; i++) {
            while (excl_idx < excl.size() &&
                   excl[excl_idx] <= samples[i] + int64_t(i + excl_idx))
                excl_idx++;
            samples[i] += i + excl_idx;
        }
    }

    std::vector<int64_t>::iterator begin() {
        return samples.begin();
    }

    std::vector<int64_t>::iterator end() {
        return samples.end();
    }
};

/**
 *  Graph is an abstract class
 */
template<typename label_t, typename weight_t = void>
class Graph {
protected:
    size_t vertices_no;
    Labeler<label_t>& labeler;
    Weighter<weight_t>& weighter;

    btree::btree_set<edge_t> adj_list;

    /**
     *  Creates random edges
     *
     *  @param edges_no      number of edges to be created
     *
     *  @param max_edges     the maximum number of edges in the graph
     *
     *  @param is_valid      this lambda specifies if the edges passed as
     *                       an argument can be passed to edge_to_rank
     *  @param edge_to_rank  this lambda takes an edge and returns its rank
     *                       among all the edges of the graph
     *  @param rank_to_edge  this lambda takes a number in the range
     *                       [0, max_edges) and returns an edge that has that
     *                       number as rank
     */
    void add_random_edges(
        const size_t edges_no,
        const size_t max_edges,
        const std::function<bool(const edge_t)> is_valid,
        const std::function<uint64_t(const edge_t)> edge_to_rank,
        const std::function<edge_t(const uint64_t)> rank_to_edge
    ) {
        // We remove the existing edges from the range of edges that
        // RangeSampler will choose from.
        std::vector<int64_t> excluded_ranks;
        for (edge_t e: adj_list)
            if (is_valid(e))
                excluded_ranks.push_back(edge_to_rank(e));

        // We now call RangeSampler and scan the sorted samples, adding edges
        // as we go
        for (auto r: RangeSampler(edges_no, 0, max_edges, excluded_ranks)) {
            add_edge(rank_to_edge(r));
        }
    }

    std::string _to_string(
        const std::function<bool(const edge_t)> is_valid
    ) const {
        std::ostringstream oss;
        std::vector<edge_t> valid_edges;
        std::copy_if(
            adj_list.begin(),
            adj_list.end(),
            std::back_inserter(valid_edges),
            is_valid
        );
        std::random_shuffle(valid_edges.begin(), valid_edges.end());
        oss << vertices_no << " " << valid_edges.size() << "\n";
        for (edge_t e: valid_edges) {
            oss << labeler(e.tail) << " " << labeler(e.head);
            utils::write_weight(weighter, e, oss);
            oss << "\n";
        }
        return oss.str();
    }

public:
    /**
     *  Initialize the graph
     *
     *  @param vertices_no number of vertices of the graph
     */
    Graph(
        const size_t vertices_no,
        Labeler<label_t>& labeler,
        Weighter<weight_t>& weighter
    ): vertices_no(vertices_no), labeler(labeler), weighter(weighter) { }

    virtual ~Graph() {};

    // Interface methods
    virtual void add_edge(const vertex_t a, const vertex_t b) = 0;
    virtual std::string to_string() const = 0;
    virtual void connect() = 0;
    virtual void add_edges(const size_t edges_t) = 0;

    void add_edge(const edge_t& v) {
        add_edge(v.tail, v.head);
    }

    void build_forest(size_t edges_no) {
        if (edges_no > vertices_no - 1)
            throw TooManyEdgesException();
        for(vertex_t v: RangeSampler(edges_no, 0, vertices_no-1))
            add_edge(Random::randrange(0, v+1), v+1);
    }

    void build_path() {
        for(vertex_t i = 0; i < vertices_no - 1; i++)
            add_edge(i, i+1);
    }

    void build_cycle() {
        for(vertex_t i = 0; i < vertices_no - 1; i++)
            add_edge(i, i+1);
        add_edge(vertices_no - 1, 0);
    }

    void build_tree() {
        build_forest(vertices_no - 1);
    }

    void build_star() {
        for(vertex_t i=1; i<vertices_no; i++)
            add_edge(0, i);
    }

    void build_wheel() {
        for(vertex_t i=1; i<vertices_no; i++) {
            add_edge(i-1, i);
            add_edge(0, i);
        }
        add_edge(vertices_no, 0);
    }

    void build_clique() {
        for(vertex_t i=0; i<vertices_no; i++)
            for(vertex_t j=i+1; j<vertices_no; j++)
                add_edge(i, j);
    }
 
    friend std::ostream& operator<<(
        std::ostream& os,
        const Graph<label_t, weight_t>& g
    ) {
        return os << g.to_string();
    }
};

/**
 *  Disjoint set data structure
 */
class DisjointSet {
private:
    size_t* parent;
    size_t* rank;
    size_t N;

public:
    DisjointSet(const size_t N): N(N) {
        parent = new size_t[N];
        rank = new size_t[N]();
        for (size_t i=0; i<N; i++)
            parent[i] = i;
    }

    ~DisjointSet() {
        delete[] parent;
        delete[] rank;
    }

    size_t size() const {
        return N;
    }

    size_t find(const size_t a) {
        if (parent[a] == a) return a;
        return parent[a] = find(parent[a]);
    }

    bool merge(const size_t a, const size_t b) {
        int va = find(a);
        int vb = find(b);
        if (va == vb) return false;
        if (rank[va] > rank[vb]) {
            parent[vb] = va;
        } else {
            parent[va] = vb;
            rank[vb] += (rank[va] == rank[vb]);
        }
        return true;
    }
};


template<typename label_t, typename weight_t = void>
class UndirectedGraph: public Graph<label_t, weight_t> {
private:
    using Graph<label_t, weight_t>::adj_list;
    using Graph<label_t, weight_t>::labeler;
    using Graph<label_t, weight_t>::weighter;
    using Graph<label_t, weight_t>::add_random_edges;
    using Graph<label_t, weight_t>::vertices_no;
    using Graph<label_t, weight_t>::_to_string;

public:
    using Graph<label_t, weight_t>::Graph;

    ~UndirectedGraph() {};

    void add_edge(const vertex_t tail, const vertex_t head) override {
        adj_list.insert({tail, head});
        adj_list.insert({head, tail});
    }

    std::string to_string() const override {
        auto is_valid = [](const edge_t e) -> bool {
            return e.tail > e.head;
        };

        return _to_string(is_valid);
    }

    void connect() override {
        DisjointSet connected_components(vertices_no);
        for (edge_t e: adj_list)
            connected_components.merge(e.tail, e.head);

        // We are going to scan through the vertices in random order
        std::vector<size_t> vertices(vertices_no);
        std::iota(vertices.begin(), vertices.end(), 0);
        std::random_shuffle(vertices.begin(), vertices.end());

        // repr contains K representative vertices, with K the
        // number of connected components in the graph. A representative is a
        // randomly chosen vertex among the vertices forming its connected
        // component
        std::vector<vertex_t> repr = { vertices[0] };
        for(size_t i = 1; i < vertices_no; i++) {
            if (connected_components.merge(vertices[0], vertices[i])) {
                repr.push_back(vertices[i]);
            }
        }

        // Build a random tree spanning the representative vertices
        for (size_t i = 1; i < repr.size(); i++)
            add_edge(repr[Random::randrange(0, i)], repr[i]);
    }

    void add_edges(const size_t edges_no) {
        auto is_valid = [](const edge_t e) -> bool {
            return e.tail > e.head;
        };

        auto edge_to_rank = [](edge_t e) -> uint64_t {
            return (uint64_t)e.tail*(e.tail+1)/2 + e.head;
        };

        auto rank_to_edge = [](uint64_t rank) -> edge_t {
            edge_t e;
            e.tail = round(sqrt(2*(rank+1)));
            e.head = rank - e.tail*(e.tail-1)/2;
            return e;
        };
        add_random_edges(
            edges_no,
            vertices_no*(vertices_no-1)/2,
            is_valid,
            edge_to_rank,
            rank_to_edge
        );
    }
};

template<typename label_t, typename weight_t = void>
class DirectedGraph: public Graph<label_t, weight_t> {
private:
    using Graph<label_t, weight_t>::adj_list;
    using Graph<label_t, weight_t>::labeler;
    using Graph<label_t, weight_t>::weighter;
    using Graph<label_t, weight_t>::add_random_edges;
    using Graph<label_t, weight_t>::vertices_no;
    using Graph<label_t, weight_t>::_to_string;

public:
    using Graph<label_t, weight_t>::Graph;

    ~DirectedGraph() {};

    void add_edge(const vertex_t tail, const vertex_t head) override {
        adj_list.insert({tail, head});
    }

    std::string to_string() const override {
        auto is_valid = [](const edge_t e) -> bool {
            return e.tail != e.head;
        };

        return _to_string(is_valid);
    }

    void add_edges(const size_t edges_no) {
        auto is_valid = [](const edge_t e) -> bool {
            return e.tail != e.head;
        };

        auto edge_to_rank = [&](edge_t e) -> uint64_t {
            return (uint64_t)e.tail*(vertices_no-1) + e.head - (e.head>e.tail);
        };

        auto rank_to_edge = [&](uint64_t rank) -> edge_t {
            edge_t e;
            e.tail = rank / (vertices_no-1);
            e.head = rank - e.tail*(vertices_no-1);
            if (e.head >= e.tail) e.head++;
            return e;
        };
        add_random_edges(
            edges_no,
            vertices_no*(vertices_no-1),
            is_valid,
            edge_to_rank,
            rank_to_edge
        );
    }

    void build_dag(const size_t edges_no) {
        auto is_valid = [](const edge_t e) -> bool {
            return e.tail > e.head;
        };

        auto edge_to_rank = [](edge_t e) -> uint64_t {
            return (uint64_t)e.tail*(e.tail+1)/2 + e.head;
        };

        auto rank_to_edge = [](uint64_t rank) -> edge_t {
            edge_t e;
            e.tail = round(sqrt(2*(rank+1)));
            e.head = rank - e.tail*(e.tail-1)/2;
            return e;
        };
        add_random_edges(
            edges_no,
            vertices_no*(vertices_no-1)/2,
            is_valid,
            edge_to_rank,
            rank_to_edge
        );
    }

    /**
     *  Add the minimum number of edges so that the resulting digraph is
     *  STRONGLY connected
     */
    virtual void connect() {
        // TODO: implement this. (tarjan?)
        throw NotImplementedException();
    }
};
