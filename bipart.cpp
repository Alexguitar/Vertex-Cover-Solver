#include <queue>
#include <limits>
#include <cassert>
#include <algorithm>
#include <cmath>

#include "graph.h"

size_t LP_BOUND_CUTOFF = 10000000.0;
size_t INFTY = numeric_limits<size_t>::max();

bool lp_bound_check_feasability(Graph &G) {
	float val = sqrt(G.V.size());
	val *= G.E.size();

	if (val > LP_BOUND_CUTOFF)
		return false;

	return true;
}

Vertex NIL(0, "NIL");

Vertex *&Pair_U(Vertex *u) {
	return u->hk_pair[0];
}

Vertex *&Pair_V(Vertex *v) {
	return v->hk_pair[1];
}

size_t &Dist(Vertex *a) {
	return a->hk_dist;
}

/* whenever we remove a vertex from the graph call this function */
void bp_vertex_delete_callback(Graph &G, Vertex *v) {
	Dist(v) = INFTY;

	Vertex *p = Pair_U(v);
	if (p != &NIL) {
		Pair_V(p) = &NIL;
		Pair_U(v) = &NIL;
		G.matching--;
	}

	p = Pair_V(v);
	if (p != &NIL) {
		Pair_V(v) = &NIL;
		Pair_U(p) = &NIL;
		G.matching--;
	}
}

/* whenever we remove an edge from the graph call this function */
void bp_edge_delete_callback(Graph &G, Edge *e) {
	Vertex *a = e->end[0];
	Vertex *b = e->end[1];

	Vertex *p = Pair_U(a);
	if (p == b) {
		Pair_V(p) = &NIL;
		Pair_U(a) = &NIL;
		G.matching--;
	}

	p = Pair_V(a);
	if (p == b) {
		Pair_V(a) = &NIL;
		Pair_U(p) = &NIL;
		G.matching--;
	}
}

/* whenever we add a newly CREATED vertex call this function */
void bp_vertex_create_callback(Graph &G, Vertex *v) {
	Pair_U(v) = &NIL;
	Pair_V(v) = &NIL;
	v->bp_vc[0] = false;
	v->bp_vc[1] = false;
}

bool BFS(Graph &G) {
	static queue<Vertex *> Q;
	for (auto it = G.V.begin(); it != G.V.end(); it++) {
		Vertex *u = *it;
		// initially there is no alternating path that ends in the right variant of v
		u->hk_alternating = false;

		// reset the bipartite VC
		u->bp_vc[0] = false;
		u->bp_vc[1] = false;

		if (Pair_U(u) == &NIL) {
			Dist(u) = 0;
			Q.push(u);
		} else {
			Dist(u) = INFTY;
		}
	}
	Dist(&NIL) = INFTY;
	
	while (!Q.empty()) {
		Vertex *u = Q.front();
		Q.pop();

		if (Dist(u) < Dist(&NIL)) {
			for (auto it = u->edges.begin(); it != u->edges.end(); it++) {
				Vertex *v = it->first;
				if (v != Pair_U(u))
					v->hk_alternating = true;

				if (Dist(Pair_V(v)) == INFTY) {
					Dist(Pair_V(v)) = Dist(u) + 1;
					Q.push(Pair_V(v));
				}
			}
		}
	}
	return Dist(&NIL) != INFTY;
}

bool DFS(Graph &G, Vertex *u) {
	if (u != &NIL) {
		for (auto it = u->edges.begin(); it != u->edges.end(); it++) {
			Vertex *v = it->first;

			if (Dist(Pair_V(v)) == Dist(u) + 1) {
				if (DFS(G, Pair_V(v)) == true) {
					Pair_V(v) = u;
					Pair_U(u) = v;
					return true;
				}
			}
		}
		Dist(u) = INFTY;

		return false;	
	}
	return true;
}

void bp_matching_init(Graph &G) {
	for (auto it = G.V.begin(); it != G.V.end(); it++) {
		Vertex *a = *it;
		bp_vertex_create_callback(G, a);
	}

	G.matching = 0;
}

// hopcroft-karp algorithm
void bp_matching(Graph &G) {
	for (Vertex *v: G.V) {
		if (Pair_U(v) != &NIL) {
			assert(Pair_V(Pair_U(v)) == v);
		}

		if (Pair_V(v) != &NIL) {
			assert(Pair_U(Pair_V(v)) == v);
		}
	}

	while (BFS(G)) {
		for (auto it = G.V.begin(); it != G.V.end(); it++) {
			Vertex *u = *it;

			if (Pair_U(u) == &NIL) {
				if (DFS(G, u) == true)
					G.matching++;
			}
		}
	}

	for (Vertex *v: G.V) {
		if (Pair_U(v) != &NIL) {
			assert(Pair_V(Pair_U(v)) == v);
		}

		if (Pair_V(v) != &NIL) {
			assert(Pair_U(Pair_V(v)) == v);
		}
	}
}

// according to Diestel - Graph Theory(2000) page 39
void bp_vertex_cover(Graph &G) {
	bp_matching(G);

	// check that it's at least a maximal matching
	for (Vertex *v: G.V) {
		if (Pair_U(v) == &NIL) {
			for (auto pair: v->edges) {
				Vertex *u = pair.first;
				assert(Pair_V(u) != &NIL);
			}
		
		}

		if (Pair_V(v) == &NIL) {
			for (auto pair: v->edges) {
				Vertex *u = pair.first;
				assert(Pair_U(u) != &NIL);
			}
		}
	}


	for (auto it = G.V.begin(); it != G.V.end(); it++) {
		Vertex *v = *it;
		if (v->hk_alternating) {
			v->bp_vc[1] = true;
		}
		else {
			// add the other end of the matched pair
			// might end up being NIL, but that's ok
			Pair_V(v)->bp_vc[0] = true;
		}
	}


	// check that we have a bipartite vertex cover
	for (Vertex *v: G.V) {
		if (!v->bp_vc[0]) {
			for (auto pair: v->edges) {
				Vertex *u = pair.first;
				assert(u->bp_vc[1]);
			}
		
		}

		if (!v->bp_vc[1]) {
			for (auto pair: v->edges) {
				Vertex *u = pair.first;
				assert(u->bp_vc[0]);
			}
		
		}
	}
}

Vertex s(0, "lp-S-vertex");
Vertex t(0, "lp-T-vertex");

void visit(Graph &G, Vertex *v, bool left, list< pair<bool, Vertex *> > &L) {
	int i = left ? 0 : 1;
	bool next = left ? false : true;

	if (v->lp_visited[i] == true)
		return;
	v->lp_visited[i] = true;

	/* left vertices can take any outgoing edge */
	if (left) {
		/* iterate over right neighbours */
		for (auto edge: v->edges) {
			Vertex *u = edge.first;
			visit(G, u, false, L);
		}

		L.push_front(make_pair(true, v));
	}

	/* right vertices can only take matched edges */
	else {
		if (Pair_V(v) != &NIL) {
			visit(G, Pair_V(v), true, L);
		}

		L.push_front(make_pair(false, v));
	}
}

// u ist left when left == true
void assign(Graph &G, Vertex *v, Vertex *root, bool left, bool root_left) {
	int i = left ? 0 : 1;
	int j = root_left ? 0 : 1;
	bool next = left ? false : true;

	if (v->lp_root[i].second != nullptr)
		return;

	v->lp_root[i].second = root;
	v->lp_root[i].first = root_left;
	root->scc[j].push_back(make_pair(left, v));

	if (left) {
		if (Pair_U(v) != &NIL) {
			assign(G, Pair_U(v), root, false, root_left);
		}
	}
	else {
		/* iterate over left neighbours */
		for (auto edge: v->edges) {
			Vertex *u = edge.first;
			assign(G, u, root, true, root_left);
		}
	}
}

bool check_scc(Graph &G, Vertex *root, bool left, long long &lower_bound) {
	int i = left ? 0 : 1;
	auto &SCC = root->scc[i];
	bool useable = true;

	assert(SCC.size() > 0);
	if (SCC.size() == 1)
		return false;

	for (auto pair: SCC) {
		Vertex *v = pair.second;
		bool left2 = pair.first;

		if (v == &s || v == &t)
			continue;
		if (v->deg == 0) {
			/* v already is in the vertex cover */
			useable = false;
			break;
		}

		if (v->lp_root[0].second == v->lp_root[1].second && v->lp_root[0].first == v->lp_root[1].first) {
			useable = false;
			break;
		}

		// go over all outgoing edges
		if (left2) {
			for (auto edge: v->edges) {
				Vertex *u = edge.first;
				// check if u is in this SCC
				if (u->lp_root[1].second != root || u->lp_root[1].first != left) {
					useable = false;
					break;
				}
			}
		}
		else {
			if (Pair_V(v) != &NIL) {
				Vertex *p = Pair_V(v);
				// check if p is in this SCC
				if (p->lp_root[0].second != root || p->lp_root[0].first != left) {
					useable = false;
					break;
				}

			}
		}

		if (!useable)
			break;
	}


	if (!useable) {
		return false;
	}


	// apply optimization
	for (auto pair: SCC) {
		Vertex *v = pair.second;
		bool left2 = pair.first;

		if (v == &s || v == &t)
			continue;

		if (left2) {
			delete_vertex(G, v);
		}
		else {
			add_to_vc(G, v);
			lower_bound--;
		}
	}


	return true;
}

/* kosaraju's algorithm - wikipedia */
void lp_flow_optimize(Graph &G, long long &lower_bound) {
	// is left vertex? + Vertex *
	list< pair<bool, Vertex *> > L;

	size_t n_iter = 3;
start:
	/* step 1 */
	for (Vertex *v: G.V) {
		v->lp_visited[0] = false;
		v->lp_visited[1] = false;
		v->lp_root[0].second = nullptr;
		v->lp_root[1].second = nullptr;
		v->lp_root[0].first = false;
		v->lp_root[1].first = false;
		v->lp_marked[0] = false;
		v->lp_marked[1] = false;
		v->scc[0].clear();
		v->scc[1].clear();
		v->marked = false;
	}

	/* step 2 */
	for (Vertex *v: G.V) {
		visit(G, v, false, L);
		visit(G, v, true, L);
	}

	/* step 3 */
	for (auto pair: L) {
		bool left = pair.first;
		Vertex *v = pair.second;

		assign(G, v, v, left, left);
	}

	/* step 4 - VL */

	bool ran = false;

	list<Vertex *> listV(G.V);
	for (Vertex *v: listV) {
		if (v->lp_root[0].second == v && v->lp_root[0].first == true) {
			ran = ran || check_scc(G, v, true, lower_bound);
		}
	//	if (ran)
	//		break;
		if (v->lp_root[1].second == v && v->lp_root[1].first == false) {
			ran = ran || check_scc(G, v, false, lower_bound);
		}
	//	if (ran)
	//		break;
	}
	if (ran) {
		L.clear();
		goto start;
	}
}


long long lp_bound(Graph &G) {
	long long ones = 0;
	long long halves = 0;

	if (!lp_bound_check_feasability(G))
		return 0ll;

	bp_vertex_cover(G);

	list<Vertex *> listV(G.V);
#if 1
	for (Vertex *v: listV) {

		if (!v->bp_vc[0] && !v->bp_vc[1]) {
			delete_vertex(G, v);
			; //nothing
		}
		else if (v->bp_vc[0] && v->bp_vc[1]) {
			add_to_vc(G, v);
			//ones++;
		}
		else	halves++;
	}
#endif

	// round up;
	long long lower_bound = ones + (halves + 1) / 2;

	lp_flow_optimize(G, lower_bound);

	return lower_bound;
}
