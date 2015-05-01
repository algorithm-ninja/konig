#include "Python.h"
#include "../include/konig.hpp"

#define CATCH(ret) \
    catch(PythonException& e) { \
        return ret; \
    } \
    catch(std::exception& e) { \
        PyErr_SetString(PyExc_ValueError, e.what()); \
        return ret; \
    }

#define METHOD_VOIDVOID(obj, name) \
    static PyObject* obj ## _ ## name(obj ## Obj* self) { \
        try { \
            self->g->name(); \
        } CATCH(NULL) \
        Py_RETURN_NONE; \
    }

#define METHOD_VOIDINT(obj, name) \
    static PyObject* obj ## _ ## name( \
        obj ## Obj* self, \
        PyObject *args, \
        PyObject *kwds \
    ) { \
        int param; \
        if (!PyArg_ParseTuple(args, "i", &param)) \
            return NULL; \
        try { \
            self->g->name(param); \
        } CATCH(NULL) \
        Py_RETURN_NONE; \
    }

#define ADD_OBJECT(module, obj) \
        if (PyType_Ready(&obj ## Type) < 0) return; \
        Py_INCREF(&obj ## Type); \
        PyModule_AddObject(module, #obj, (PyObject *)&obj ## Type);

#define DEF_NOARGS(obj, name, descr) \
    {#name, (PyCFunction)obj ## _ ## name, METH_NOARGS, descr}

#define DEF_ARGS(obj, name, descr) \
    {#name, (PyCFunction)obj ## _ ## name, METH_VARARGS, descr}

#define NEW_TYPE(obj, doc) \
    static PyTypeObject obj ## Type = { \
        PyObject_HEAD_INIT(NULL) \
        0,                                  /* ob_size */ \
        "graphgen." #obj,                   /* tp_name */ \
        sizeof(obj ## Obj),                 /* tp_basicsize */ \
        0,                                  /* tp_itemsize */ \
        (destructor) obj ## _dealloc,       /* tp_dealloc */ \
        0,                                  /* tp_print */ \
        0,                                  /* tp_getattr */ \
        0,                                  /* tp_setattr */ \
        0,                                  /* tp_compare */ \
        0,                                  /* tp_repr */ \
        0,                                  /* tp_as_number */ \
        0,                                  /* tp_as_sequence */ \
        0,                                  /* tp_as_mapping */ \
        0,                                  /* tp_hash */ \
        0,                                  /* tp_call */ \
        obj ## _str,                        /* tp_str */ \
        0,                                  /* tp_getattro */ \
        0,                                  /* tp_setattro */ \
        0,                                  /* tp_as_buffer */ \
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */ \
        doc,                                /* tp_doc */ \
        0,                                  /* tp_traverse */ \
        0,                                  /* tp_clear */ \
        0,                                  /* tp_richcompare */ \
        0,                                  /* tp_weaklistoffset */ \
        (getiterfunc) obj ## _iter,         /* tp_iter */ \
        (iternextfunc) obj ## _iternext,    /* tp_iternext */ \
        obj ## _methods,                    /* tp_methods */ \
        0,                                  /* tp_members */ \
        0,                                  /* tp_getset */ \
        0,                                  /* tp_base */ \
        0,                                  /* tp_dict */ \
        0,                                  /* tp_descr_get */ \
        0,                                  /* tp_descr_set */ \
        0,                                  /* tp_dictoffset */ \
        (initproc) obj ## _init,            /* tp_init */ \
        PyType_GenericAlloc,                /* tp_alloc */ \
        PyType_GenericNew                   /* tp_new */ \
    };


class PythonException: public std::exception {
    virtual const char* what() const noexcept {
        return "A Python exception happened!";
    }
};

class WTFException: public std::exception {
    virtual const char* what() const noexcept {
        return "WTF just happened?!?";
    }
};

struct pyObject {
    enum type_t { VAL_VOID, VAL_INT, VAL_DOUBLE, VAL_PYOBJECT };
    union {
        int _int;
        double _double;
        PyObject* _PyObject;
    } stored_val;
    type_t type;

    pyObject(): type(VAL_VOID) { }

    pyObject(int v): type(VAL_INT) {
        stored_val._int = v;
    }

    pyObject(double v): type(VAL_DOUBLE) {
        stored_val._double = v;
    }

    pyObject(PyObject* v): type(VAL_PYOBJECT) {
        Py_INCREF(v);
        stored_val._PyObject = v;
    }

    ~pyObject() {
        if (type == VAL_PYOBJECT) {
            Py_DECREF(stored_val._PyObject);
        }
    }
};

std::ostream& operator<<(std::ostream& os, const pyObject& obj) {
    PyObject* strrepr;
    char* str;
    switch (obj.type) {
        case pyObject::VAL_VOID: break;
        case pyObject::VAL_INT:
            os << obj.stored_val._int;
            break;
        case pyObject::VAL_DOUBLE:
            os << obj.stored_val._double;
            break;
        case pyObject::VAL_PYOBJECT:
            strrepr = PyObject_Str(obj.stored_val._PyObject);
            if (!strrepr) throw PythonException();
            str = PyString_AsString(strrepr);
            if (!str) throw PythonException();
            os << str;
            Py_DECREF(strrepr);
            break;
        default:
            throw WTFException();
    }
    return os;
}

template<typename T>
class pyLabelerWrapper: public Labeler<pyObject> {
private:
    Labeler<T>* l;

public:
    pyLabelerWrapper(Labeler<T>* l): l(l) {}
    ~pyLabelerWrapper() {
        delete l;
    }

    pyObject operator()(vertex_t v) override {
        T val = (*l)(v);
        return val;
    }
};

template<typename T>
class pyWeighterWrapper: public Weighter<pyObject> {
private:
    Weighter<T>* w;

public:
    pyWeighterWrapper(Weighter<T>& w): w(&w) {}
    ~pyWeighterWrapper() {
        delete w;
    }

    pyObject operator()(const edge_t& e) override {
        return (*w)(e);
    }
};

// I hate void

template<>
class pyWeighterWrapper<void>: public Weighter<pyObject> {
private:
    Weighter<void>* w;

public:
    pyWeighterWrapper(Weighter<void>* w): w(w) {}
    ~pyWeighterWrapper() {
        delete w;
    }

    pyObject operator()(const edge_t& e) override {
        return pyObject();
    }
};

// Both pyLabeler and pyWeighter will fail horribly if the object passed
// to the constructor are not callable

class pyLabeler: public Labeler<pyObject> {
private:
    PyObject* lbl;

public:
    pyLabeler(PyObject* l): lbl(l) {
        Py_INCREF(lbl);
    }

    ~pyLabeler() {
        Py_DECREF(lbl);
    }

    pyObject operator()(const vertex_t& v) {
        PyObject* res = PyObject_CallFunction(
            lbl,
            const_cast<char*>("n"),
            v
        );
        if (!res) throw PythonException();
        pyObject&& obj = pyObject(res);
        Py_DECREF(res);
        return obj;
    }
};

class pyWeighter: public Weighter<pyObject> {
private:
    PyObject* w;

public:
    pyWeighter(PyObject* w): w(w) {
        Py_INCREF(w);
    }

    ~pyWeighter() {
        Py_DECREF(w);
    }

    pyObject operator()(const edge_t& e) {
        PyObject* res = PyObject_CallFunction(
            w,
            const_cast<char*>("nn"),
            e.tail,
            e.head
        );
        if (!res) throw PythonException();
        pyObject&& obj = pyObject(res);
        Py_DECREF(res);
        return obj;
    }
};

extern "C" {

    // Module methods

    static PyObject * GG_srand(PyObject *self, PyObject *args) {
        int s;
        if (!PyArg_ParseTuple(args, "i", &s))
            return NULL;
        Random::srand(s);
        Py_RETURN_NONE;
    }

    static PyMethodDef graphgen_methods[] = {
        DEF_ARGS(GG, srand, "Call srand()."),
        {NULL}
    };

    // RangeSamplerIterator

    typedef struct {
        PyObject_HEAD
        std::vector<int64_t>::iterator it;
        std::vector<int64_t>::iterator end;
    } RangeSamplerIteratorObj;

    static initproc RangeSamplerIterator_init = 0;
    static reprfunc RangeSamplerIterator_str = 0;
    static PyMethodDef* RangeSamplerIterator_methods = 0;
    static getiterfunc RangeSamplerIterator_iter = PyObject_SelfIter;

    static void RangeSamplerIterator_dealloc(PyObject* self) {
        self->ob_type->tp_free(self);
    }

    static PyObject* RangeSamplerIterator_iternext(
        RangeSamplerIteratorObj* RangeSamplerIterator
    ) {
        if (RangeSamplerIterator->it == RangeSamplerIterator->end)
            return NULL;
        PyObject* res = PyInt_FromLong((long)*RangeSamplerIterator->it);
        RangeSamplerIterator->it++;
        return res;
    }

    NEW_TYPE(RangeSamplerIterator, "Iterator over a RangeSampler")

    // RangeSampler

    typedef struct {
        PyObject_HEAD
        RangeSampler* rs;
    } RangeSamplerObj;

    static reprfunc RangeSampler_str = 0;
    static PyMethodDef* RangeSampler_methods = 0;
    static iternextfunc RangeSampler_iternext = 0;

    static void RangeSampler_dealloc(RangeSamplerObj* self) {
        if (self->rs)
            delete self->rs;
        self->ob_type->tp_free((PyObject*)self);
    }

    static PyObject* RangeSampler_iter(RangeSamplerObj* self) {
        auto res = (RangeSamplerIteratorObj*) PyType_GenericAlloc(&RangeSamplerIteratorType, 0);
        res->it = self->rs->begin();
        res->end = self->rs->end();
        return (PyObject*) res;
    }

    static int RangeSampler_init(
        RangeSamplerObj* self,
        PyObject* args,
        PyObject* kwds
    ) {
        size_t num;
        long long min, max;

        if (!PyArg_ParseTuple(args, "nLL", &num, &min, &max))
            return -1;
        if (self->rs) {
            // Someone who feels playful could call __init__() twice
            delete self->rs;
            self->rs = NULL;
        }
        try {
            self->rs = new RangeSampler(num, min, max);
        } CATCH(-1)
        return 0;
    }

    NEW_TYPE(RangeSampler, "RangeSampler")

    // DisjointSet

    typedef struct {
        PyObject_HEAD
        DisjointSet* disjoint_set;
    } DisjointSetObj;

    static reprfunc DisjointSet_str = 0;
    static getiterfunc DisjointSet_iter = 0;
    static iternextfunc DisjointSet_iternext = 0;

    static void DisjointSet_dealloc(DisjointSetObj* self) {
        if (self->disjoint_set)
            delete self->disjoint_set;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int DisjointSet_init (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t sz;
        if (!PyArg_ParseTuple(args, "n", &sz))
            return -1;
        if (self->disjoint_set) {
            // Someone who feels playful could call __init__() twice
            delete self->disjoint_set;
            self->disjoint_set = NULL;
        }
        try {
            self->disjoint_set = new DisjointSet(sz);
        } CATCH(-1)
        return 0;
    }

    static PyObject* DisjointSet_find (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t a;
        if (!PyArg_ParseTuple(args, "n", &a))
            return NULL;
        if (a<0 || a>=self->disjoint_set->size()) {
            PyErr_SetString(PyExc_ValueError, "Value out of range!");
            return NULL;
        }
        return PyInt_FromLong(self->disjoint_set->find(a));
    }

    static PyObject* DisjointSet_merge (
        DisjointSetObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t a, b;
        if (!PyArg_ParseTuple(args, "nn", &a, &b))
            return NULL;
        if (a>=self->disjoint_set->size() || b>=self->disjoint_set->size()) {
            PyErr_SetString(PyExc_ValueError, "Values out of range!");
            return NULL;
        }
        if (self->disjoint_set->merge(a, b)) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    static PyMethodDef DisjointSet_methods[] = {
        DEF_ARGS(DisjointSet, find, "Find the representative of an element."),
        DEF_ARGS(DisjointSet, merge, "Merge two sets together."),
        {NULL}
    };

    NEW_TYPE(DisjointSet, "Disjoint Set data structure.")

    // UndirectedGraph

    typedef struct {
        PyObject_HEAD
        UndirectedGraph<pyObject, pyObject>* g;
        Labeler<pyObject>* labeler;
        Weighter<pyObject>* weighter;
    } UndirectedGraphObj;

    static getiterfunc UndirectedGraph_iter = 0;
    static iternextfunc UndirectedGraph_iternext = 0;

    static void UndirectedGraph_dealloc(UndirectedGraphObj* self) {
        if (self->g) delete self->g;
        if (self->labeler) delete self->labeler;
        if (self->weighter) delete self->weighter;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int UndirectedGraph_init (
        UndirectedGraphObj *self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t sz;
        if (!PyArg_ParseTuple(args, "i", &sz))
            return -1;
        if (self->g) {
            // Someone who feels playful could call __init__() twice
            delete self->g;
            delete self->labeler;
            delete self->weighter;
            self->g = NULL;
        }
        try {
            self->labeler = new pyLabelerWrapper<int>(new IotaLabeler());
            self->weighter = new pyWeighterWrapper<void>(new NoWeighter());
            self->g = new UndirectedGraph<pyObject, pyObject>(
                sz,
                *(self->labeler),
                *(self->weighter)
            );
        } CATCH(-1)
        return 0;
    }

    static PyObject* UndirectedGraph_str(PyObject* self) {
        try {
            const std::string& repr = ((UndirectedGraphObj*)self)->g->to_string();
            return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
        } CATCH(NULL)
    }

    static PyObject* UndirectedGraph_add_edge (
        UndirectedGraphObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int a, b;
        if (!PyArg_ParseTuple(args, "ii", &a, &b))
            return NULL;
        try {
            self->g->add_edge(a, b);
        } CATCH(NULL)
        Py_RETURN_NONE;
    }

    METHOD_VOIDINT(UndirectedGraph, add_edges)
    METHOD_VOIDINT(UndirectedGraph, build_forest)
    METHOD_VOIDVOID(UndirectedGraph, connect)
    METHOD_VOIDVOID(UndirectedGraph, build_path)
    METHOD_VOIDVOID(UndirectedGraph, build_cycle)
    METHOD_VOIDVOID(UndirectedGraph, build_tree)
    METHOD_VOIDVOID(UndirectedGraph, build_star)
    METHOD_VOIDVOID(UndirectedGraph, build_wheel)
    METHOD_VOIDVOID(UndirectedGraph, build_clique)

    static PyMethodDef UndirectedGraph_methods[] = {
        DEF_ARGS(UndirectedGraph, add_edge, "Add an edge to the graph."),
        DEF_ARGS(UndirectedGraph, add_edges, "Add some new edges to the graph."),
        DEF_NOARGS(UndirectedGraph, connect, "Make the graph connected."),
        DEF_ARGS(UndirectedGraph, build_forest, "Creates a forest with M edges."),
        DEF_NOARGS(UndirectedGraph, build_path, "Creates a path."),
        DEF_NOARGS(UndirectedGraph, build_cycle, "Creates a cycle."),
        DEF_NOARGS(UndirectedGraph, build_tree, "Creates a tree."),
        DEF_NOARGS(UndirectedGraph, build_star, "Creates a star."),
        DEF_NOARGS(UndirectedGraph, build_wheel, "Creates a wheel."),
        DEF_NOARGS(UndirectedGraph, build_clique, "Creates a clique."),
        {NULL}
    };

    NEW_TYPE(UndirectedGraph, "Undirected graph")

    // Directed graph

    typedef struct {
        PyObject_HEAD
        DirectedGraph<pyObject, pyObject>* g;

        Labeler<pyObject>* labeler;
        Weighter<pyObject>* weighter;
    } DirectedGraphObj;

    static getiterfunc DirectedGraph_iter = 0;
    static iternextfunc DirectedGraph_iternext = 0;

    static void DirectedGraph_dealloc(DirectedGraphObj* self) {
        if (self->g) delete self->g;
        if (self->labeler) delete self->labeler;
        if (self->weighter) delete self->weighter;
        self->ob_type->tp_free((PyObject*)self);
    }

    static int DirectedGraph_init (
        DirectedGraphObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        size_t sz;
        if (!PyArg_ParseTuple(args, "i", &sz))
            return -1;
        if (self->g) {
            // Someone who feels playful could call __init__() twice
            delete self->g;
            delete self->labeler;
            delete self->weighter;
            self->g = NULL;
        }
        try {
            self->labeler = new pyLabelerWrapper<int>(new IotaLabeler());
            self->weighter = new pyWeighterWrapper<void>(new NoWeighter());
            self->g = new DirectedGraph<pyObject, pyObject>(
                sz,
                *(self->labeler),
                *(self->weighter)
            );
        } CATCH(-1)
        return 0;
    }

    static PyObject* DirectedGraph_str(PyObject* self) {
        try {
            const std::string& repr = ((DirectedGraphObj*)self)->g->to_string();
            return PyString_FromStringAndSize(repr.c_str(), repr.size()-1);
        } CATCH(NULL)
    }

    static PyObject* DirectedGraph_add_edge (
        DirectedGraphObj* self,
        PyObject *args,
        PyObject *kwds
    ) {
        int a, b;
        if (!PyArg_ParseTuple(args, "ii", &a, &b))
            return NULL;
        try {
            self->g->add_edge(a, b);
        } CATCH(NULL)
        Py_RETURN_NONE;
    }


    METHOD_VOIDINT(DirectedGraph, add_edges)
    METHOD_VOIDINT(DirectedGraph, build_forest)
    METHOD_VOIDINT(DirectedGraph, build_dag)
    METHOD_VOIDVOID(DirectedGraph, connect)
    METHOD_VOIDVOID(DirectedGraph, build_path)
    METHOD_VOIDVOID(DirectedGraph, build_cycle)
    METHOD_VOIDVOID(DirectedGraph, build_tree)
    METHOD_VOIDVOID(DirectedGraph, build_star)
    METHOD_VOIDVOID(DirectedGraph, build_wheel)
    METHOD_VOIDVOID(DirectedGraph, build_clique)

    static PyMethodDef DirectedGraph_methods[] = {
        DEF_ARGS(DirectedGraph, add_edge, "Add an edge to the graph."),
        DEF_ARGS(DirectedGraph, add_edges, "Add some new edges to the graph."),
        DEF_NOARGS(DirectedGraph, connect, "Make the graph connected."),
        DEF_ARGS(DirectedGraph, build_forest, "Creates a forest with M edges."),
        DEF_ARGS(DirectedGraph, build_dag, "Creates a dag with M edges."),
        DEF_NOARGS(DirectedGraph, build_path, "Creates a path."),
        DEF_NOARGS(DirectedGraph, build_cycle, "Creates a cycle."),
        DEF_NOARGS(DirectedGraph, build_tree, "Creates a tree."),
        DEF_NOARGS(DirectedGraph, build_star, "Creates a star."),
        DEF_NOARGS(DirectedGraph, build_wheel, "Creates a wheel."),
        DEF_NOARGS(DirectedGraph, build_clique, "Creates a clique."),
        {NULL}
    };

    NEW_TYPE(DirectedGraph, "Directed graph")

    // Module initialization function

    PyMODINIT_FUNC initgraphgen(void) {
        PyObject* m;
        m = Py_InitModule3(
            "graphgen",
            graphgen_methods,
            "Module to generate graphs"
        );
        ADD_OBJECT(m, RangeSampler)
        ADD_OBJECT(m, RangeSamplerIterator)
        ADD_OBJECT(m, DisjointSet)
        ADD_OBJECT(m, UndirectedGraph)
        ADD_OBJECT(m, DirectedGraph)
    }
}
