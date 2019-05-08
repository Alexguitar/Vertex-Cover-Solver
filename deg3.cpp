#include <cassert>

#include "graph.h"
#include "util.h"
#include "deg3.h"
#include "bipart.h"
#include "read_vc.h"
#include "snapshot.h"
#include "optimize.h"
#include "constraints.h"

size_t DEG3_CUTOFF1 = 30;
size_t DEG3_CUTOFF2 = 15;

bool deg3_is_check_feasability(Graph &G, Vertex *v, Vertex *a, Vertex *b, Vertex *c) {
	if (a->deg + b->deg + c->deg > DEG3_CUTOFF1 + 1)
		return false;

	Vertex *S[] = {a, b, c};


	size_t new_edges = 0;

	for (int i = 0; i < 3; i++) {
		Vertex *u = S[i];
		Vertex *w = S[(i+1) % 3];
		vector< pair<Vertex *, Edge *> > &u_edges = S[i]->edges;
		vector< pair<Vertex *, Edge *> > &w_edges = S[(i+1) % 3]->edges;
		// count new edges from u to N(w)


		// don't count existing edges
		for (auto edge: u_edges) {
			assert(!edge.first->marked);
			edge.first->marked = true;
		}

		for (auto edge: w_edges) {
			Vertex *x = edge.first;
			if (x->marked)
				continue;

			new_edges++;
		}

		for (auto edge: u_edges) {
			if (!edge.first->marked)
				break;
			edge.first->marked = false;
		}
	}

	if (new_edges > DEG3_CUTOFF2)
		return false;

	return true;
}

// return the number of edges in G[{a, b, c}]
int deg3_num_edges(Vertex *a, Vertex *b, Vertex *c) {
	Vertex *S[] = {a, b, c};
	int n = 0;

	for (int i = 0; i < 3; i++) {
		Vertex *v = S[i];

		for (auto edge: v->edges) {
			Vertex *u = edge.first;

			if (u == a || u == b || u == c) {
				// we don't actually make a new edge object
				// so this is only to make lookups faster
				v->d3_edges.push_back(edge);
				n++;
			}
		}
	}

	return n / 2;
}


void deg3_create_edge(Graph &G, GM_deg3 *op, Vertex *u, Vertex *w) {
	Edge *e = new Edge(G.m, u, w);
	
	G.E.push_back(e);
	auto it = G.E.end();
	it--;
	e->iterE = it;

	G.m++;
	// the Edge constructor increases deg but we don't want that
	u->deg--;
	w->deg--;
	vert_change_deg(G, u, u->deg + 1);
	vert_change_deg(G, w, w->deg + 1);

	op->new_edges.push_back(e);
}

bool deg3_independent_set(Graph &G, Vertex *v, Vertex *a, Vertex *b, Vertex *c) {
	if (!deg3_is_check_feasability(G, v, a, b, c))
		return false;

	GM_deg3 *op = new GM_deg3(v, a, b, c);

	delete_vertex(G, v);

	/* add the new edges F */
	Vertex *S[] = {a, b, c};
	vector< pair<Vertex *, Edge *> > tmp_edges[3];

	// temporarily move all edges to avoid conflicts
	for (int i = 0; i < 3; i++) {
		tmp_edges[i].swap(S[i]->edges);
	}

	for (int i = 0; i < 3; i++) {
		Vertex *u = S[i];
		Vertex *w = S[(i+1) % 3];
		vector< pair<Vertex *, Edge *> > &u_edges = tmp_edges[i];
		vector< pair<Vertex *, Edge *> > &w_edges = tmp_edges[(i+1) % 3];
		// add edges from u to N(w)


		// don't add existing edges
		for (auto edge: u_edges) {
			assert(!edge.first->marked);
			edge.first->marked = true;
		}

		for (auto edge: w_edges) {
			Vertex *x = edge.first;
			if (x->marked)
				continue;

			deg3_create_edge(G, op, u, x);
		}

		for (auto edge: u_edges) {
			if (!edge.first->marked)
				break;
			edge.first->marked = false;
		}
	}

	// add {a, b}, {b, c}
	deg3_create_edge(G, op, a, b);
	deg3_create_edge(G, op, b, c);

	// move the edges back
	for (int i = 0; i < 3; i++) {
		// old edges are back in the same order they were before
		tmp_edges[i].swap(S[i]->edges);

		// make sure to properly insert the new edges
		for (size_t j = 0; j < tmp_edges[i].size(); j++) {
			auto edge = tmp_edges[i][j];
			Edge *e   = edge.second;

			if (S[i] != e->end[0])
				e->flip();
			e->pos[0] = S[i]->edges.size();
			S[i]->edges.emplace_back(edge);
		}
	}

	// disable constrain checking for these vertices
	vert_mark_uncertain(G, a);
	vert_mark_uncertain(G, b);
	vert_mark_uncertain(G, c);

	// register graph transformation
	G.changes.push_back(op);

	return true;
}

void deg3_domination(Graph &G, Vertex *a, Vertex *b, Vertex *c) {
	Vertex *S[] = {a, b, c};
	Vertex *max = nullptr;

	// select dominating vertex with the highest degree
	for (int i = 0; i < 3; i++) {
		Vertex *u = S[i];

		if (u->d3_edges.size() >= 2) {
			if (max == nullptr || u->edges.size() > max->edges.size())
				max = u;
		}
	}
	assert(max != nullptr);
	assert(max->d3_edges.size() >= 2);

	add_to_vc(G, max);
}

// apply degree 3 rule
bool vc_deg3_rule(Graph &G, long long &k) {
	bool rerun;
	bool ran = false;

	do {
		rerun = false;

		/* find deg 3 vertices */
		for (auto it = G.deg3s.begin(); it != G.deg3s.end(); ) {
			Vertex *v = *it;
			auto next = it;
			next++;
			it++;
			Vertex *tmp = nullptr;
			// keep track of the position in the list, because
			// the list can get mangled in all sorts of ways
			it = G.deg3s.insert(it, tmp);

			
			assert(v->edges.size() == 3);
			auto it2 = v->edges.begin();


			Vertex *a = it2->first;
			it2++;
			Vertex *b = it2->first;
			it2++;
			Vertex *c = it2->first;

			int n_edges = deg3_num_edges(a, b, c);


			if (n_edges == 0) {
				if (deg3_independent_set(G, v, a, b, c)) {
					rerun = true;
					ran   = true;
				}
			}

			a->d3_edges.clear();
			b->d3_edges.clear();
			c->d3_edges.clear();

			it = G.deg3s.erase(it);
		}
	} while (rerun);

	return ran;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

// undo any changes we've made while applying the deg3 rule to v
void GM_deg3::undo(Graph &G) {
	// edges that have been removed will be taken care of by restore_edge
	// in restore_snapshot

	// remove inserted edges
	for (auto it = this->new_edges.rbegin(); it != this->new_edges.rend(); it++) {
		Edge *e = *it;
		G.m--;
		assert(e->id == G.m); // not so important, remove it if it breaks
		Vertex *x = e->end[0];
		Vertex *y = e->end[1];

		G.E.erase(e->iterE);
		assert(x->edges.size() == x->deg);
		assert(y->edges.size() == y->deg);

		remove_edge_dirty(x, e->pos[0], x->edges);
		remove_edge_dirty(y, e->pos[1], y->edges);

		vert_change_deg(G, x, x->deg - 1);
		vert_change_deg(G, y, y->deg - 1);
		bp_edge_delete_callback(G, e);
		delete e;
	}
}

/* translate the VC for the transformed graph into a VC for the original graph */
void GM_deg3::translate_vc(Graph &G, list<Vertex *> &sol) {
	Vertex *v = this->v;
	Vertex *a = this->a;
	Vertex *b = this->b;
	Vertex *c = this->c;

	int num = 0;
	num += a->in_vc ? 1 : 0;
	num += b->in_vc ? 1 : 0;
	num += c->in_vc ? 1 : 0;

	assert(num != 0);
	if (num == 3)
		return;
	if (num == 1) {
		assert(b->in_vc);

		rvc_remove_vertex(b, sol);
		rvc_add_vertex(v, sol);
		return;
	}

	assert(num == 2);
	if (!c->in_vc) {
		rvc_remove_vertex(a, sol);
	}
	else if (!b->in_vc) {
		rvc_remove_vertex(c, sol);
	}
	else {
		rvc_remove_vertex(b, sol);
	}

	rvc_add_vertex(v, sol);
}
