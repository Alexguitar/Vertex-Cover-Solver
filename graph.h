#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <string>

using namespace std;

class Edge;
class Vertex;
class Graph;
class MergedVertex;

#include "heap.h"
#include "snapshot.h"
#include "constraints.h"


class Vertex {
public:
	size_t id;
	string name;

	// here an edge is a pair of the other vertex and the edge object
	// edges is the list of not covered edges
	// covered edges are stored in the covered list instead
	vector< pair<Vertex *, Edge *> > edges;
	vector< pair<Vertex *, Edge *> > covered;

	// list for temporarily holding edges
	// make sure you clear this if you use it
	vector< pair<Vertex *, Edge *> > tmp_edges;

	// here the degree is the number of uncovered edges that are adjacent
	// meaning deg == edges.size()
	size_t deg;

	// calculated in select.cpp
	float score = 0.0; // we'll branch on vertices with the highest score
	bool dont_branch = false;	// if this vertex will be removed by a rule, never branch on it

	// if it's a degree 1 vertex it will be in a list of degree 1 vertices. This is the iterator to it
	list<Vertex *>::iterator iter1;
	// if it's a degree 2 vertex it will be in a list of degree 2 vertices. This is the iterator to it
	list<Vertex *>::iterator iter2;
	// if it's a degree 3 vertex it will be in a list of degree 3 vertices. This is the iterator to it
	list<Vertex *>::iterator iter3;

	// if its's a degree > 0 vertex it will be in the list V. This is the iterator to it
	list<Vertex *>::iterator iterV;

	// temporary bool, make sure you reset this back to false if you use it
	bool marked = false;

	// used for checking constrains
	bool cstr_in_vc = false;
	bool cstr_uncertain = false;

	// used in read_vc and local_search
	bool in_vc = false;			// is this vertex in the vertex cover?
	list<Vertex *>::iterator iterVC;	// this is the iterator to it
	bool ls_visited;

	// the MergedVertex object this vertex has been merged into, or nullptr
	MergedVertex *merge = nullptr;
	// if this is also a merged vertex this pointer is not null and points to the merged vertex object
	// C++ won't let me use dynamic casts
	MergedVertex *downcast = nullptr;

#if 1
	// stuff for deg_3 independent set
	vector< pair<Vertex *, Edge *> > d3_edges;
#endif

#if 1
	// stuff for clique neighbourhood rule
	bool in_c1 = false;
	bool in_c2 = false;
	size_t old_deg = 0;
#endif

#if 1
	/* stuff for Hopcroft-Karp */
	Vertex *hk_pair[2];
	size_t hk_dist;      // left vertex
	bool hk_alternating; // right vertex
	bool bp_vc[2];

	bool lp_visited[2];
	bool lp_marked[2];
	pair<bool, Vertex *> lp_root[2];
	list< pair<bool, Vertex *> > scc[2];
#endif

#if 1
	// stuff for clique
	Vertex *next_member;
	size_t clique_size; // only relevant for the leader vertex
#endif
	
	bool S_marked = false;
	bool NS_marked = false;

	size_t component;


	virtual ~Vertex() {};

	Vertex(size_t id, string name)
	: id(id), name(name), deg(0) {};

	Vertex(size_t id, string name, size_t degree)
	: id(id), name(name), deg(degree) {};

};

class Edge {
public:
	size_t id;
	// two endpoint vertices
	Vertex *end[2];
	bool covered;

#if 1
	// stuff for cycle bound
	bool mst;
#endif

	// iterator storing the position of this edge in G.E
	list<Edge *>::iterator iterE;
	// the position of this edge in the edgelists of the two endpoint vertices	
	size_t pos[2];

	Edge(size_t id, Vertex *a, Vertex *b)
	: id(id), covered(false) {
		end[0] = a;
		end[1] = b;

		pos[0] = a->edges.size();
		pos[1] = b->edges.size();
		a->edges.emplace_back( make_pair(b, this) );
		b->edges.emplace_back( make_pair(a, this) );
		
		a->deg++;
		b->deg++;
	}


	// swap the position of the two end verticies
	// just a convenience function as edges are undirected
	void flip() {
		swap(end[0], end[1]);
		swap(pos[0], pos[1]);
	}
};

class VertexCover {
public:
	vector<Vertex *> V;

	// after initial graph optimization the VC will be moved to V_backup
	vector<Vertex *> V_backup;
	/* the edges covered by V */
	vector<Edge *> E;
};


// a functor that compares the degrees of two vertices
struct vertex_cmp {
	bool operator() (const Vertex *lhs, const Vertex *rhs) const {
		if (lhs->deg == rhs->deg)
			return lhs->id < rhs->id;
		return lhs->deg < rhs->deg;
	}
};


class Graph {
public:
	// V does not contain degree 0 vertices
	list<Vertex *> V;
	list<Edge *> E;

#if 1
	// stuff for cycle bound
	size_t cnt;
#endif

	// list of degree 1 vertices
	list<Vertex *> deg1s;
	// list of degree 2 vertices
	list<Vertex *> deg2s;
	// list of degree 3 vertices
	list<Vertex *> deg3s;

	map<string, Vertex *> name_map;

	size_t recursive_steps = 0;
	size_t n  = 0;
	size_t m  = 0;

	VertexCover VC;
	vector<GraphModification *> changes;

	// stuff for constraints
	vector<Constraint *> constraints;
	vector<Vertex *> marked_uncertain;

#if 1
	/* stuff for Hopcroft-Karp */
	size_t matching;
#endif

	void print() {
		cout << "# Printing Graph" << endl;
		for (Vertex *v: this->V) {
			for (auto it = v->edges.begin(); it != v->edges.end(); it++) {
				Vertex *b = it->first;
				if (v->id > b->id)
					continue;

				cout << "# " << v->name << " " << b->name << endl;
			}
		}
		cout << "# Printing done" << endl;
	}
};

void vertex_cover(Graph &G, string td, size_t n);
pair<bool, long long> vc_branch(Graph &G, long long k);

/* global variables */
// a fake vertex that's not in the graph, has id 0 and degree 1
extern Vertex deg1_vertex;
