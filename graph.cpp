#include <utility>
#include <cassert>
#include <algorithm>

#include "graph.h"
#include "branch.h"
#include "read_vc.h"
#include "merge.h"
#include "mirror.h"
#include "optimize.h"
#include "bipart.h"
#include "clique.h"
#include "stats.h"

// a fake vertex used as an object for comparison
Vertex deg1_vertex(0, "fake_deg1", 1);

// returns true if a VC has been found, if not false and a lower bound is returned
pair<bool, long long> vc_branch(Graph &G, long long k) {
	if (G.E.size() == 0) {
		if (k < 0) {
			return make_pair(false, G.VC.V.size());
		}
		assert(k == 0);		// we must never overshoot k
		return make_pair(true, 0ll);
	}

	G.recursive_steps++;

	/* information for rollback */
	// lp_bound can modify the graph, which we need to undo afterwards
	struct graph_snapshot pre_snapshot;
	pre_snapshot = create_snapshot(G);

	/* compute lower bound */
	long long lower_bound = 1;	// since we have at least one edge
	size_t tmp_vc = G.VC.V.size();

	if (CONFIG_LP_BOUND)
		lower_bound = max(lower_bound, lp_bound(G)); // modifies the graph

	k = k - (G.VC.V.size() - tmp_vc);
	if (G.E.size() == 0) {
		if (k < 0) {
			return make_pair(false, G.VC.V.size());
		}
		assert(k == 0);		// we must never overshoot k
		return make_pair(true, 0ll);
	}

	if (CONFIG_CLIQUE_BOUND)
		lower_bound = max(lower_bound, clique_bound(G));

	Vertex *v = nullptr;
	
	long long bounds[2] = {0, 0};
	long long old_k = k;

	// use lower_bound to give vertex_cover a better prediction for the next k
	if (k < lower_bound) {
		lower_bound = G.VC.V.size() + lower_bound;
		goto fail;
	}

	/* select the highest degree vertex */
	v = G.V.front();
	for (Vertex *a: G.V) {
		if (v->deg < a->deg)
			v = a;
	}

	/* simple check if k is too small */
	if (v->deg <= k) {
		bool ran = false;

		while (G.V.size() > k*k + k || G.E.size() > k*k) {
			ran = true;
			k++;
		}
		if (ran) {
			lower_bound = G.VC.V.size() + k;
			goto fail;
		}
	}

	lower_bound += G.VC.V.size();
	for (int i = 0; i < 2; i++) {
		/* information for rollback */
		struct graph_snapshot snapshot;
		snapshot = create_snapshot(G);
		size_t old_vc_size = G.VC.V.size();
		size_t old_vce_size = G.VC.E.size();
		size_t old_e_size = G.E.size();

		/* add v or N(v) to the vertex cover */
		if (i == 0) {
			if (CONFIG_MIRROR) {
				add_mirrors_to_vc(G, v);
			}
			add_to_vc(G, v);
		} else {
			// iterate over the neighbours
			while (v->edges.size() > 0) {
				Vertex *u = v->edges[0].first;

				// add the neighbour u
				add_to_vc(G, u);
			}
		}

		/* optimize */
		vc_optimize(G, k);	// can decrement k

		// some rules don't add vertices to the VC, but instead reserve
		// a space (by decrementing k). GraphModification.translate()
		// finds which vertex needs to be added
		bounds[i] = old_k - k;

		/* branch */
		size_t vc_diff = G.VC.V.size() - old_vc_size;
		auto found = vc_branch(G, k - vc_diff);
		if (found.first == true)
			return found;

		bounds[i] += found.second;

		/* rollback */
		k = old_k;
		restore_snapshot(G, snapshot);
		assert(G.VC.V.size() == old_vc_size);
		assert(G.VC.E.size() == old_vce_size);
		assert(G.E.size()    == old_e_size);
	}	

	// use the result of the branching to improve the lower bound
	assert(lower_bound <= min(bounds[0], bounds[1]));
	lower_bound = min(bounds[0], bounds[1]);

fail:
	/* undo changes by lp_bound */
	restore_snapshot(G, pre_snapshot);

	return make_pair(false, lower_bound);
}

void vertex_cover(Graph &G, string td, size_t n) {
	long long k = 0;
	list<Vertex *> solution;

	/* init */
	bp_matching_init(G);
	/* optimize graph */
	vc_optimize(G, k);

	if (CONFIG_LP_BOUND)
		k = lp_bound(G);

	print_graph_optimization(G);

	// save the current VC into a different vector
	// this has to be done because vc_branch assumes that when it is first
	// called G.VC.V is empty
	swap(G.VC.V, G.VC.V_backup);

	k = max(k, 0ll);
	if (CONFIG_CLIQUE_BOUND)
		k = max(k, clique_bound(G));

	cout << "c initial lower bound k = " << k << endl;

	while (true) {
		assert(G.VC.V.size() == 0);
		auto found = vc_branch(G, k);

		if (found.first == true)
			break;
		assert(found.second >= k+1);
		k = max(k+1, found.second);
	//	cout << "# k = " << k << endl;
	}

	// add the old VC back to the current one
	G.VC.V_backup.insert(G.VC.V_backup.end(), G.VC.V.begin(), G.VC.V.end());
	swap(G.VC.V, G.VC.V_backup);

	// read off the vertex cover for the original graph from the transformed graph
	read_vc(G, solution);

	cout << "c VC size = " << solution.size() << endl;
	cout << "c recursive steps: " << G.recursive_steps << endl;
	cout << "s vc " << n << " " << solution.size() << endl;

	for (Vertex *v: solution) {
		assert(v->downcast == nullptr);
		cout << v->name << endl;
	}
}
