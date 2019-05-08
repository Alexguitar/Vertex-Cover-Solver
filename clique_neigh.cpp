#include <cassert>
#include <algorithm>
#include <cmath>

#include "graph.h"
#include "util.h"
#include "bipart.h"
#include "read_vc.h"
#include "clique_neigh.h"
#include "constraints.h"


/* a whole bunch of feasability tests */

bool   CN_CHECK1_ENABLED = true;
size_t CN_CHECK1_MIN_DEG = 1;
size_t CN_CHECK1_MAX_DEG = 20;

bool cn_check_feasablility1(Vertex *v) {
	if (!CN_CHECK1_ENABLED)
		return true;

	if (v->deg < CN_CHECK1_MIN_DEG || v->deg > CN_CHECK1_MAX_DEG)
		return false;

	// find lower bound for C1

	size_t m = 0;
	for (auto edge: v->edges) {
		Vertex *a = edge.first;

		// don't count v
		m += a->edges.size() - 1;
	}

	size_t d = v->deg;
	// round up
	size_t half = (d + 1) / 2;
	size_t min_c1 = (half * (half - 1)) / 2;	// smallest C1 possible
	// we counted a bunch of edges twice, if a C1 exists
	// substract the number of edges in the smallest C1
	if (m < 2 * min_c1)	// m must be at least twice the number of edges in C1
		return false;

	//return true;

	size_t m_upper = m - min_c1;
	size_t m_lower = 0;
	

	// use the formula for the size of C1 from cn_find_partition
	size_t c1_lower = 0;
	if ( (d * (d-1)) / 2 >= m_upper) {
		c1_lower = (d * (d-1)) / 2 - m_upper;
	}
	size_t c1_upper = d - 1;
	if ( (d * (d-1)) / 2 >= m_lower) {
		c1_upper = (d * (d-1)) / 2 - m_lower;
	}

	if (c1_lower >= d || 2 * c1_upper < d) {
		// partitioning impossible or C1 would be smaller than C2
		return false;
	}

	return true;
}

bool   CN_CHECK2_ENABLED = true;
float  CN_CHECK2_CUTOFF  = 1000.0;
size_t CN_CHECK2_RELAX_N = 12;
size_t CN_CHECK2_LARGE_N = 20;
size_t CN_CHECK2_LARGE_K = 3;
#define E_CONST 2.71828182845904523536

bool cn_check_feasablility2(vector< pair<Vertex *, Vertex *> > &C1_candidates, size_t c1_n) {
	if (!CN_CHECK2_ENABLED)
		return true;

	// There may be binom(C1_candidates.size(), c1_n) different possibilities
	// to select C1. Try to avoid it if it's too costly

	size_t n = C1_candidates.size();
	size_t k = c1_n;

	if (n <= CN_CHECK2_RELAX_N)
		return true;

	k = k > n/2 ? n - k : k;

	if (k == 0)
		return true;

	if (n >= CN_CHECK2_LARGE_N && k >= CN_CHECK2_LARGE_K)
		return false;

	// approximate binom
	float x = (E_CONST * (float) n) / ((float) k);

	while (k > 0) {
		if (x > CN_CHECK2_CUTOFF) {
			return false;
		}

		x = x * x;
		k--;
	}

	return true;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

bool cn_dfs(vector< pair<Vertex *, Vertex *> > &C1_candidates, size_t pos, size_t size, vector< pair<Vertex *, Vertex *> > &C1) {
	if (C1.size() == size)
		return true;

	size_t remaining = size - C1.size();

	// try adding a vertex from C1_candidates to C1
	for (size_t i = pos; i < C1_candidates.size(); i++) {
		if (C1_candidates.size() - i < remaining)
			return false;

		auto pair  = C1_candidates[i];
		Vertex *a  = pair.first;
		Vertex *nn = pair.second;

		// the non-neighbour can't be in C1
		if (nn->in_c1)
			continue;

		C1.push_back(pair);
		a->in_c1 = true;

		bool found = cn_dfs(C1_candidates, i + 1, size, C1);
		if (found)
			return true;

		C1.pop_back();
		a->in_c1 = false;
	}

	return false;
}

void cn_find_candidates(Graph &G, Vertex *v, vector< pair<Vertex *, Vertex *> > &C1_candidates) {
	for (auto edge: v->edges) {
		Vertex *a  = edge.first;
		Vertex *nn = nullptr;

		a->marked = true;
		for (auto edge2: a->edges) {
			Vertex *b = edge2.first;
			b->marked = true;
		}

		for (auto edge2: v->edges) {
			Vertex *b = edge2.first;

			if (!b->marked) {
				if (nn != nullptr) {
					nn = nullptr;
					break;
				}

				nn = b;
			}

		}

		for (auto edge2: a->edges) {
			Vertex *b = edge2.first;
			b->marked = false;
		}
		a->marked = false;

		if (nn != nullptr) {
			C1_candidates.emplace_back(make_pair(a, nn));
		}
	}
}

bool cn_find_partition(Graph &G, Vertex *v, vector< pair<Vertex *, Vertex *> > &C1, vector<Vertex *> &C2) {
	for (auto edge: v->edges) {
		Vertex *a = edge.first;
		assert(!a->marked);
		assert(!a->in_c1);
		assert(!a->in_c2);
		a->marked = true;
	}

	// count edges in G[N(v)]
	size_t m = 0;
	for (auto edge: v->edges) {
		Vertex *a = edge.first;

		for (auto edge2: a->edges) {
			Vertex *b = edge2.first;

			if (b->marked) {
				m++;
			}

		}
	}

	for (auto edge: v->edges) {
		Vertex *a = edge.first;
		a->marked = false;
	}

	assert(m % 2 == 0);
	m = m / 2;

	// using the edge count compute the sizes of the partitions
	size_t d = v->deg;
	size_t c1_n = (d * (d - 1)) / 2 - m;

	if (c1_n >= d || 2 * c1_n < d) {
		// partitioning impossible or C1 would be smaller than C2
		return false;
	}
	size_t c2_n = d - c1_n;

	vector< pair<Vertex *, Vertex *> > C1_candidates;
	cn_find_candidates(G, v, C1_candidates);
	if (C1_candidates.size() < c1_n) {
		return false;
	}

	// finding C1 may still be too difficult
	if (!cn_check_feasablility2(C1_candidates, c1_n)) {
		return false;
	}

	// use DFS to find C1
	if (!cn_dfs(C1_candidates, 0, c1_n, C1))
		return false;
	assert(C1.size() == c1_n);

	// C2 = N(v) - C1
	for (auto edge: v->edges) {
		Vertex *a = edge.first;

		if(!a->in_c1) {
			assert(!a->in_c2);
			a->in_c2 = true;
			C2.push_back(a);
		}
	}
	assert(C2.size() == c2_n);
	
	return true;
}

void cn_create_edge(Graph &G, GM_clique_neigh *op, Vertex *a, Vertex *b) {
	Edge *e = new Edge(G.m, a, b);
	G.m++;
	
	G.E.push_back(e);
	auto it = G.E.end();
	it--;
	e->iterE = it;

	// the Edge constructor increases deg but we don't want that,
	// rather vert_change_deg should handle it
	a->deg--;
	b->deg--;

	vert_change_deg(G, a, a->deg + 1);
	vert_change_deg(G, b, b->deg + 1);

	op->new_edges.push_back(e);
}

GM_clique_neigh *cn_setup(Graph &G, Vertex *v, vector< pair<Vertex *, Vertex *> > &C1, vector<Vertex *> &C2) {
	GM_clique_neigh *op = new GM_clique_neigh();
	op->v = v;

	// move C1 and C2 into op without copying anything
	op->C1 = move(C1);	
	op->C2 = move(C2);	

	// add new edges
	for (auto pair: op->C1) {
		Vertex *a  = pair.first;
		Vertex *nn = pair.second;

		for (auto edge: a->edges) {
			Vertex *c = edge.first;
			assert(!c->marked);
			c->marked = true;
		}

		// add edges from a to N(nn)
		for (auto edge: nn->edges) {
			Vertex *c = edge.first;
			// don't add already existing edges
			if (!c->marked) {
				assert(!c->in_c1 && !c->in_c2);
				cn_create_edge(G, op, a, c);
			}
		}

		for (auto edge: a->edges) {
			Vertex *c = edge.first;
			c->marked = false;
		}
	}

	return op;
}


bool vc_clique_neigh_single(Graph &G, Vertex *v, long long &k) {
	bool feasable = cn_check_feasablility1(v);
	if (!feasable)
		return false;
	
	vector< pair<Vertex *, Vertex *> > C1;
	vector<Vertex *> C2;

	// partition N(v) into C1 and C2
	bool success = cn_find_partition(G, v, C1, C2);
	GM_clique_neigh *op = nullptr;

	// create the graph transformation by adding edges
	if (success)
		op = cn_setup(G, v, C1, C2);
	
	// cn_setup may decide not do the transformation
	success = op != nullptr;

	// apply rule - remove v and C2
	if (success) {
		k -= op->C2.size();
		delete_vertex(G, v);
		for (Vertex *b: op->C2)
			delete_vertex(G, b);

		G.changes.push_back(op);

		// cleanup
		for (auto pair: op->C1)
			pair.first->in_c1 = false;
		for (auto a: op->C2)
			a->in_c2 = false;
	}

	return success;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void GM_clique_neigh::undo(Graph &G) {
	// adding v and C2 back will be undone next as it's the previous registered
	// transformation 

	// delete newly created edges
	for (auto it = this->new_edges.rbegin(); it != this->new_edges.rend(); it++) {
		Edge *e = *it;
		G.m--;
		assert(e->id == G.m); // not so important, remove it if it breaks
		G.E.erase(e->iterE);

		Vertex *a = e->end[0];
		Vertex *b = e->end[1];
		
		assert(a->edges.size() == a->deg);
		assert(b->edges.size() == b->deg);

		remove_edge_dirty(a, e->pos[0], a->edges);
		remove_edge_dirty(b, e->pos[1], b->edges);

		vert_change_deg(G, a, a->deg - 1);
		vert_change_deg(G, b, b->deg - 1);
		bp_edge_delete_callback(G, e);
		delete e;
	}
}

void GM_clique_neigh::translate_vc(Graph &G, list<Vertex *> &sol) {
	size_t count = 0;

	// count = |C1 itersect sol|
	for (auto pair: this->C1) {
		Vertex *a = pair.first;
		if (a->in_vc)
			count++;
	}

	if (count == this->C1.size()) {
		// add C2 to the vertex cover
		for (Vertex *a: this->C2) {
			rvc_add_vertex(a, sol);
		}
	}
	else if (count == this->C1.size() - 1) {
		Vertex *a  = nullptr;
		Vertex *nn = nullptr;

		// find a vertex in C1 that's not in the VC and its non-neighbour
		for (auto pair: this->C1) {
			Vertex *b = pair.first;
			if (!b->in_vc) {
				a  = b;
				nn = pair.second; 
				break;
			}
		}
		assert(a  != nullptr);
		assert(nn != nullptr);

	
		// add v and C2 \ nn to the vertex cover
		rvc_add_vertex(this->v, sol);
		for (Vertex *b: this->C2) {
			rvc_add_vertex(b, sol);
		}
		rvc_remove_vertex(nn, sol);
	}
	else	assert(false);
}
