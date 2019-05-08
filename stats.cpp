#include "graph.h"


void print_graph_optimization(Graph &G) {
	size_t covered_edges = 0;

	for (auto it = G.E.begin(); it != G.E.end(); it++) {
		Edge *e = *it;
		if (e->covered == true)
			covered_edges++;
	}

	cout << "c graph optimized" << endl;
	cout << "c n: " << G.n << " -> " << G.V.size() << " (" << (float) 100 * ((float) G.V.size()) / G.n << " %)" << endl;
	cout << "c m: " << G.m << " -> " << G.E.size() << " (" << (float) 100 * ((float) G.E.size()) / G.m << " %)" << endl;
}
