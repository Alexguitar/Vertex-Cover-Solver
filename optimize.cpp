#include <cassert>
#include <algorithm>

#include "graph.h"
#include "merge.h"
#include "deg3.h"
#include "bipart.h"
#include "optimize.h"
#include "snapshot.h"
#include "clique_neigh.h"
#include "undeg3.h"


optimization_rules enabled_rules[NUM_RULES] = {
	OPT_DEG_12,
	OPT_UNCONF_COMBO,
	OPT_CN,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE,
	OPT_NONE
};


size_t UNCONF_CUTOFF  = 50000;
size_t UNCONF_MAX_DEG = 5000;

/* if an edge has a vertex with degree 1 select its neighbour */
bool vc_deg1_rule(Graph &G) {
	bool rerun;
	bool ran = false;

	do {
		rerun = false;

		/* find deg 1 vertices */
		while (G.deg1s.size() != 0) {
			Vertex *v = *G.deg1s.begin();
			
			assert(v->edges.size() == 1);
			Vertex *b = v->edges.begin()->first;

			add_to_vc(G, b);
			rerun = true;
			ran   = true;
		}
	} while (rerun);

	return ran;
}

// apply deg 2 rule to v
bool vc_deg2_rule_single(Graph &G, long long &k, Vertex *v) {
	assert(v->deg == 2);
	auto it = v->edges.begin();

	Vertex *u = it->first;
	it++;
	Vertex *w = it->first;

	bool uw_edge = false;

	// check if there's an edge between u and w
	Vertex *x = u->edges.size() < w->edges.size() ? u : w;
	Vertex *y = u->edges.size() < w->edges.size() ? w : u;
	
	for (it = x->edges.begin(); it != x->edges.end(); it++) {
		if (it->first == y) {
			uw_edge = true;
			break;
		}
	}

	if (uw_edge) {
		add_to_vc(G, u);
		add_to_vc(G, w);
	}
	else {
		// the constructor takes care of registering the transformation
		MergedVertex *m = new MergedVertex(G, G.n, u, v, w);
		G.n++;
		k--;
	}

	return true;
}

// apply degree 2 rule
bool vc_deg2_rule(Graph &G, long long &k) {
	bool rerun;
	bool ran = false;

	do {
        rerun = false;

		/* find deg 2 vertices */
		while (G.deg2s.size() != 0) {
			Vertex *v = *G.deg2s.begin();
			assert(v->edges.size() == 2);
			vc_deg2_rule_single(G, k, v);

			rerun = true;
			ran   = true;

		}
	} while (rerun);

	return ran;
}

bool vc_domination_single(Graph &G, Vertex *v) {
	bool success = false;

	// mark all neighbours
	v->marked = true;
	for (size_t i = 0; i < v->edges.size(); i++) {
		Vertex *u = v->edges[i].first;
		assert(!u->marked);
		u->marked = true;
	}



	for (size_t i = 0; i < v->edges.size(); ) {
		auto edge = v->edges[i];
		Vertex* u = edge.first;
		// check if u dominates v

		// vertex u needs to have this many marked neighbours (including v)
		size_t needed = v->edges.size();
		size_t count  = 0;

		if (needed > u->edges.size()) {
			i++;
			continue;
		}

		// count how many marked vertices u has
		for (size_t j = 0; j < u->edges.size(); j++) {
			Vertex *w = u->edges[j].first;
			size_t remaining = u->edges.size() - j - 1;

			if (w->marked)
				count++;
			if (remaining + count < needed)
				break;
		}


		// if the neighbourhoods are the same, add u to the vertex cover and run the algo again
		if (count == needed) {
			u->marked = false;
			add_to_vc(G, u);
			success = true;
		}
		else {
			i++;
		}
	}

	v->marked = false;
	for (size_t i = 0; i < v->edges.size(); i++) {
		Vertex *u = v->edges[i].first;
		assert(u->marked);
		u->marked = false;
	}

	if (success)
		vc_deg1_rule(G);
	return success;
}

bool vc_domination_rule(Graph &G) {
	bool rerun;
	bool ran = false;
	do {
		rerun = false;
		// iterate over all uncovered nodes

		list<Vertex *> list_V(G.V);
		for (Vertex* v: list_V) {
			if (vc_domination_single(G, v)) {
				rerun = true;
				ran   = true;
			}
		}
	}
	while (rerun);

    return ran;
}

bool vertex_unconfined(Graph &G, Vertex *v) {
	if (v->deg > UNCONF_MAX_DEG)
		return false;

	unconfined_data D;

	D.add(v);
	size_t n_iter = 1;

	while (true) {
		Vertex *u = nullptr;
		size_t count = 2;
		Vertex *z = nullptr; // w in the slides

		// find u
		for (Vertex *w: D.NS) {
			size_t S_count  = 0; // | N(w) intersect S |
			size_t NS_count = 0; // | N(w) \ N[S] |
			Vertex *x = nullptr;

			for (auto edge: w->edges) {
				Vertex *y = edge.first;
				if (y->S_marked) {
					S_count++;
					if (S_count > 1)
						break;
				}

				if (!y->S_marked && !y->NS_marked) {
					NS_count++;
					x = y;
					if (NS_count > 1)
						break;
				}
			}

			if (S_count != 1)
				continue;
			if (NS_count < count) {
				u = w;
				z = x;
				count = NS_count;
				//cout << "# unconfined with n_iter = " << n_iter << " |S| = " << D.S.size() << " and |N(S)| = " << D.NS.size() << endl;
				if (count == 0)
					return true;				
			}
		}

		if (u == nullptr)
			return false;

		// count == 1
		assert(count == 1);
		D.add(z); 

		// small feasability check
		if (D.NS.size() > UNCONF_CUTOFF)
			return false;

		// try again
		n_iter++;
	}
}

bool vc_unconfined_rule(Graph &G) {
	bool rerun;
	bool ran = false;

	do {
		rerun = false;
		// iterate over all uncovered nodes

		list<Vertex *> list_V(G.V);
		for (Vertex* v: list_V) {
			if (vertex_unconfined(G, v)) {
				add_to_vc(G, v);
				ran = true;
				rerun = true;

				vc_deg1_rule(G);
			}
		}
	}
	while (rerun);
	return ran;
}

bool vc_unconfined_rule_combo(Graph &G, long long &k) {
	bool rerun;
	bool ran = false;

	do {
		rerun = false;
		// iterate over all uncovered nodes

		list<Vertex *> list_V(G.V);
		for (Vertex* v: list_V) {
			if (v->merge != nullptr)
				continue;

			if (vertex_unconfined(G, v)) {
				add_to_vc(G, v);
				ran = true;
				rerun = true;

				vc_deg1_rule(G);
				vc_deg2_rule(G, k);
			}
		}
	}
	while (rerun);
	return ran;
}

bool vc_clique_neigh_rule(Graph &G, long long &k) {
	bool rerun;
	bool ran = false;

	do {
		rerun = false;
		// iterate over all uncovered nodes
		for (auto it = G.V.begin(); it != G.V.end(); ) {
			Vertex *v = *it;
			auto next = it;
			next++;
			it++;
			Vertex *tmp = nullptr;
			// keep track of the position in the list, because
			// the list can get mangled in all sorts of ways.
			// in the case of vc_clique_neigh_rule it is safe to modify G.V
			it = G.V.insert(it, tmp);

			if (vc_clique_neigh_single(G, v, k)) {
				//rerun = true;
				ran   = true;
			}

			it = G.V.erase(it);
		}
	}
	while (rerun);
	return ran;
}

void vc_execute_rrule(Graph &G, long long &k, optimization_rules rule) {
	bool rerun = false;

	switch (rule) {
	case OPT_NONE:
		break;
	case OPT_DEG_1:
		vc_deg1_rule(G);
		break;
	case OPT_DEG_2:
		vc_deg2_rule(G, k);
		break;
	case OPT_DEG_3:
		vc_deg3_rule(G, k);
		break;
	case OPT_DOM:
		vc_domination_rule(G);
		break;
	case OPT_UNCONF:
		vc_unconfined_rule(G);
		break;
	case OPT_CN:
		vc_clique_neigh_rule(G, k);
		break;
	case OPT_LP:
		lp_bound(G);
		break;
	case OPT_DEG_12:
		do {
			rerun = vc_deg1_rule(G);
			rerun = vc_deg2_rule(G, k);
		
		} while(rerun);
		break;
	case OPT_UNCONF_COMBO:
		vc_unconfined_rule_combo(G, k);
		break;
	case OPT_UNDEG_3:
		vc_undeg3_rule(G, k);
		break;
	}
}

/* optimize Graph */
void vc_optimize(Graph &G, long long &k) {
	for (int i = 0; i < NUM_RULES; i++) {
		vc_execute_rrule(G, k, enabled_rules[i]);
	}
}

void vc_preoptimize(Graph &G, long long &k) {
	for (int i = 0; i < 50; i++) {
		vc_execute_rrule(G, k, OPT_DEG_12);
		vc_execute_rrule(G, k, OPT_UNCONF_COMBO);
		vc_execute_rrule(G, k, OPT_LP);
		vc_execute_rrule(G, k, OPT_CN);
		vc_execute_rrule(G, k, OPT_LP);
		vc_execute_rrule(G, k, OPT_DEG_3);

		vc_execute_rrule(G, k, OPT_DEG_12);
		vc_execute_rrule(G, k, OPT_UNCONF_COMBO);
		vc_execute_rrule(G, k, OPT_LP);
		vc_execute_rrule(G, k, OPT_CN);
		vc_execute_rrule(G, k, OPT_LP);
		vc_execute_rrule(G, k, OPT_UNDEG_3);
	}

}
