#include <cassert>
#include <unordered_set>

#include "graph.h"
#include "mirror.h"
#include "snapshot.h"

bool mirror_induces_clique(Vertex *v) {
	size_t size = 0;
	
	for (auto edge: v->edges) {
		Vertex *u = edge.first;
		if (u->marked)
			size++;
	}

	for (auto edge: v->edges) {
		Vertex *u = edge.first;
		if (!u->marked)
			continue;

		size_t count = 0;
		for (auto edge2: u->edges) {
			Vertex *a = edge2.first;
			if (a->marked)
				count++;
		}

		if (count != size)
			return false;
	}
	
	return true;
}

void add_mirrors_to_vc(Graph &G, Vertex *v) {

	// set of vertices with distance 2 to v
	unordered_set<Vertex *> D2;

	assert(!v->marked);
	v->marked = true;
	for (auto edge: v->edges) {
		Vertex *u = edge.first;
		assert(!u->marked);
		u->marked = true;
	}

	for (auto edge: v->edges) {
		Vertex *u = edge.first;

		for (auto edge2: u->edges) {
			Vertex *a = edge2.first;
			if (a->marked)
				continue;
			D2.insert(a);
		}
	}

	vector<Vertex *> M;
	// check if m is a mirror
	for (Vertex *m: D2) {
		for (auto edge: m->edges) {
			Vertex *a = edge.first;
			a->marked = false;
		}

		if (mirror_induces_clique(v))
			M.push_back(m);

		
		for (auto edge: v->edges) {
			Vertex *u = edge.first;
			u->marked = true;
		}
	}

	for (auto edge: v->edges) {
		Vertex *u = edge.first;
		assert(u->marked);
		u->marked = false;
	}
	v->marked = false;
	
	for (Vertex *m: M)
		add_to_vc(G, m);
}
