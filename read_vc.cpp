#include <cassert>

#include "graph.h"
#include "merge.h"
#include "deg3.h"
#include "read_vc.h"

void rvc_add_vertex(Vertex *v, list<Vertex *> &sol) {
		assert(!v->in_vc);
		v->in_vc = true;
		sol.push_back(v);
		auto it = sol.end();
		it--;
		v->iterVC = it;
}

void rvc_remove_vertex(Vertex *v, list<Vertex *> &sol) {
		assert(v->in_vc);
		v->in_vc = false;
		sol.erase(v->iterVC);
}

void check_vc(Graph &G) {
	for (Vertex *a: G.V) {
		for (auto edge: a->edges) {
			Vertex *b = edge.first;

			assert(a->in_vc || b->in_vc);
		}
	}
}

/* read the VC G.VC.V for the transformed graph and translate it into a VC for
 * the original graph */
void read_vc(Graph &G, list<Vertex *> &sol) {
	for (Vertex *v: G.VC.V) {
		rvc_add_vertex(v, sol);
	}
	
	//check_vc(G);

	for (size_t i = 0; i < G.changes.size(); i++) {
		size_t j = G.changes.size() - i - 1;
		GraphModification *op = G.changes[j];

		op->translate_vc(G, sol);
		//op->undo(G); // it should also work without this (maybe)
		//check_vc(G);
	}
}
