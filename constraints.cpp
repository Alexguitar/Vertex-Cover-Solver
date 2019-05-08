#include <vector>

#include "constraints.h"
#include "graph.h"

bool NotAllNeighboursConstraint::check(Graph &G) {
	size_t count = 0;
	size_t uncertain = 0;

	for (Vertex *a: neighbours) {
		if (a->cstr_uncertain)
			uncertain++;
		if (a->cstr_in_vc && !a->cstr_uncertain) {
			count++;
		}
	}

	if (neighbours.size() - count <= 1)
		cout << "# diff = " << neighbours.size() - count << " ; uncertain " << uncertain << endl;

	return count != neighbours.size();
}

NotAllNeighboursConstraint::NotAllNeighboursConstraint(Vertex *V) : v(V) {
	for (auto edge: v->edges) {
		neighbours.push_back(edge.first);
	}
}

void vert_mark_uncertain(Graph &G, Vertex *v) {
	if (v->cstr_uncertain)
		return;

	v->cstr_uncertain = true;
	G.marked_uncertain.push_back(v);
}
