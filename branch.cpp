#include <utility>
#include <cassert>
#include <algorithm>
#include <string>

#include "graph.h"
#include "branch.h"
#include "read_vc.h"
#include "merge.h"
#include "mirror.h"
#include "optimize.h"
#include "bipart.h"
#include "clique.h"
#include "stats.h"
#include "constraints.h"
#include "score.h"


bool CONFIG_MIRROR           = true;
bool CONFIG_COMPONENTS       = true;
bool CONFIG_BRANCHING_V2     = true;
bool CONFIG_LP_BOUND         = true;
bool CONFIG_CLIQUE_BOUND     = true;

void vc_branch_v2(Graph &G, list<Vertex *> &best, long long size, long long &u);

void write_best_sol(Graph &G, list<Vertex *> &best, long long size, long long &u) {
	if (size < u) {
		u = size;
		best.clear();
		read_vc(G, best);

		for (Vertex *a: best)
			a->in_vc = false;
		assert(best.size() == u);
	}
}

void component_dfs(Vertex* v, size_t counter) {
	v->component = counter;
	for (auto edge : v->edges) {
		Vertex* neighbour = edge.first;
		if(neighbour->component == 0) {
			component_dfs(neighbour, counter);
		}
	}
}

void vc_branch_split(Graph &G, size_t n_comp, list<Vertex *> &best, long long size, long long &u) {
	// every node is part of a component
	for(auto node : G.V) {
		assert(node->component != 0);
	}

	size_t old_v_size = G.V.size();


	list<Vertex *> old_V;;
	list<Vertex *> old_deg1s;
	list<Vertex *> old_deg2s;
	list<Vertex *> old_deg3s;
	vector<GraphModification *> old_changes;
	vector<Vertex *> old_VC;

	old_V.splice(old_V.end(), G.V);
	old_deg1s.splice(old_deg1s.end(), G.deg1s);
	old_deg2s.splice(old_deg2s.end(), G.deg2s);
	old_deg3s.splice(old_deg3s.end(), G.deg3s);

	old_changes.swap(G.changes);
	old_VC.swap(G.VC.V);


	list<Vertex *> all_comp_sol;
	bool found = true;


	list<Vertex *> components[n_comp+1];
	for (auto it = old_V.begin(); it != old_V.end(); ) {
		Vertex *node = *it;
		auto next = it;
		next++;

		assert(node->component >= 1 && node->component <= n_comp);
		assert(!node->in_vc);

		size_t i = node->component;
		components[i].splice(components[i].end(), old_V, it);
		it = next;
	}



	for (size_t i = 1; i <= n_comp; i++) {
		G.V.splice(G.V.end(), components[i]);

		for (Vertex *node: G.V) {
			if (node->deg == 1) {
				G.deg1s.splice(G.deg1s.end(), old_deg1s, node->iter1);
			}
			if (node->deg == 2) {
				G.deg2s.splice(G.deg2s.end(), old_deg2s, node->iter2);
			}
			if (node->deg == 3) {
				G.deg3s.splice(G.deg3s.end(), old_deg3s, node->iter3);
			}
		}

		assert(G.V.size() != 0);
		assert(G.VC.V.size() == 0);
		assert(G.changes.size() == 0);


		list<Vertex*> comp_sol;
		long long u_comp = u - size;

		vc_branch_v2(G, comp_sol, 0, u_comp);

		if (comp_sol.size() == 0) {
			// could not find better solution
			found = false;
			goto fail;
		}

		size += comp_sol.size();
		all_comp_sol.splice(all_comp_sol.end(), comp_sol);

	fail:

		components[i].splice(components[i].end(), G.V);
		old_deg1s.splice(old_deg1s.end(), G.deg1s);
		old_deg2s.splice(old_deg2s.end(), G.deg2s);
		old_deg3s.splice(old_deg3s.end(), G.deg3s);

		if (!found)
			break;
	}



	assert(G.V.size() == 0);
	assert(G.VC.V.size() == 0);
	assert(G.changes.size() == 0);


	// cleanup

	for (size_t i = 1; i <= n_comp; i++) {
		G.V.splice(G.V.end(), components[i]);
	}
	G.deg1s.splice(G.deg1s.end(), old_deg1s);
	G.deg2s.splice(G.deg2s.end(), old_deg2s);
	G.deg3s.splice(G.deg3s.end(), old_deg3s);
	G.changes.swap(old_changes);
	G.VC.V.swap(old_VC);

	if (found) {
		assert(size < u);
		u = size;
		best.clear();
		best.splice(best.end(), all_comp_sol);
		for (auto it = best.begin(); it != best.end(); it++) {
			Vertex *a = *it;
			a->in_vc  = true;
			a->iterVC = it;
		}

		read_vc(G, best);
		for (Vertex *a: best)
			a->in_vc = false;

		assert(u == best.size());
	}


	return;

}

void vc_branch_v2(Graph &G, list<Vertex *> &best, long long size, long long &u) {
	if (G.V.empty()) {
		write_best_sol(G, best, size, u);
		return;
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

	size += G.VC.V.size() - tmp_vc;
	Vertex *v = nullptr;
	Vertex *max_deg = nullptr;
	size_t counter = 0;
	long long fake_k = 0;
	size_t m = 0;

	if (G.V.empty()) {
		write_best_sol(G, best, size, u);
		goto end;
	}

	if (CONFIG_CLIQUE_BOUND)
		lower_bound = max(lower_bound, clique_bound(G));

	// stop branching if we can't hope to improve the solution
	if (size + lower_bound >= u) {
		goto fail;
	}

	// check constraints
//	for (size_t i = 0; i < G.constraints.size(); i++) {
//		if (!G.constraints[i]->check(G))
//			goto fail;
//	}


	if (CONFIG_COMPONENTS) {
		// compute connected components
		for(auto node : G.V) {
			node->component = 0;
		}
		for(auto node : G.V) {
			if(node->component == 0) {
				counter++;
				component_dfs(node, counter);
			}
		}
		

		// make sure that there is atleast one component
		assert(counter >= 1);

		if(counter >= 2) {
			vc_branch_split(G, counter, best, size, u);
			goto end;
		}
	}

	/* select vertex with the highest degree */
	//graph_assign_scores(G);

	v = G.V.front();
	
	for (Vertex *a: G.V) {
		m += a->deg;

		if (a->deg > v->deg)
			v = a;
	}

	m /= 2;

	// apply k^2 bound
	fake_k = u - size;
	if (v->deg <= fake_k) {
		if (G.V.size() > fake_k * fake_k + fake_k || m > fake_k * fake_k) {
			goto fail;
		}
	}

	for (int i = 0; i < 2; i++) {
		/* information for rollback */
		struct graph_snapshot snapshot;
		snapshot = create_snapshot(G);
		size_t old_vc_size = G.VC.V.size();
		size_t old_vce_size = G.VC.E.size();
		size_t old_e_size = G.E.size();

		long long k = 0;

		/* add v or N(v) to the vertex cover */
		if (i == 0) {
			//G.constraints.push_back(new NotAllNeighboursConstraint(v));

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

		/* branch */
		size_t vc_diff = G.VC.V.size() - old_vc_size - k; // k <= 0
		vc_branch_v2(G, best, size + vc_diff, u);

		/* rollback */
		restore_snapshot(G, snapshot);
		assert(G.VC.V.size() == old_vc_size);
		assert(G.VC.E.size() == old_vce_size);
		assert(G.E.size()    == old_e_size);
	}	

end:
fail:
	/* undo changes by lp_bound */
	restore_snapshot(G, pre_snapshot);

	return;
}

void vertex_cover_v2(Graph &G, string td, size_t n) {
	long long k = 0;
	list<Vertex *> sol;
	long long u = G.n;


	/* init */
	bp_matching_init(G);

	/* optimize graph */
	vc_preoptimize(G, k);
	long long reserved = -k;

	print_graph_optimization(G);

	size_t size = G.VC.V.size() + reserved;

	vc_branch_v2(G, sol, size, u);

	cout << "c VC size = " << sol.size() << endl;
	cout << "c recursive steps: " << G.recursive_steps << endl;
	cout << "s vc " << n << " " << sol.size() << endl;

	for (Vertex *v: sol) {
		assert(v->downcast == nullptr);
		cout << v->name << endl;
	}
}
