#include <cassert>

#include "graph.h"
#include "util.h"
#include "undeg3.h"
#include "bipart.h"
#include "read_vc.h"
#include "snapshot.h"
#include "optimize.h"

bool vc_undeg3_single(Graph &G, Vertex *b);
size_t vc_undeg3_count(Graph &G, Vertex *a, Vertex *b, Vertex *c);
void vc_undeg3_apply(Graph &G, Vertex *a, Vertex *b, Vertex *c);


bool vc_undeg3_rule(Graph &G, long long &k) {
	bool success = false;
	for (auto it = G.V.begin(); it != G.V.end(); it++) {
		Vertex *v = *it;
		if (vc_undeg3_single(G, v))
			success = true;
	}

	return success;
}

bool vc_undeg3_single(Graph &G, Vertex *b) {
	pair<Vertex *, Vertex *> p_best(nullptr, nullptr);
	size_t best = 0;

	// find neighbours a and c of b such that there's no edge between a and c

	for (auto it = b->edges.begin(); it != b->edges.end(); ) {
		Vertex *a = it->first;
		it++;

		for (auto it2 = it; it2 != b->edges.end(); it2++) {
			Vertex *c = it->first;

			bool ac_edge = false;
			for (auto edge: a->edges) {
				Vertex *v = edge.first;
				if (v == c) {
					ac_edge = true;
					break;
				}
			}

			if (ac_edge)
				continue;

			// see if the undeg3 rule can be applied
			size_t count = vc_undeg3_count(G, a, b, c);

			if (count > best) {
				best = count;
				p_best.first  = a;
				p_best.second = c;
			}
			
		}
	}

	if (best >= 2) {
		//cout << "# best = " << best << endl;
		vc_undeg3_apply(G, p_best.first, b, p_best.second);
		return true;
	}

	return false;
}

size_t vc_undeg3_count(Graph &G, Vertex *a, Vertex *b, Vertex *c) {
	Vertex *S[] = {a, b, c};

	size_t twos   = 0;
	size_t threes = 0;

	for (int i = 0; i < 3; i++) {
		Vertex *v = S[i];

		for (auto edge: v->edges) {
			Vertex *u = edge.first;
			if (u == S[0] || u == S[1] || u == S[2])
				continue;

			int num = 1;

			for (int j = 0; j < 3; j++) {
				if (i == j)
					continue;

				for (auto edge2: S[j]->edges) {
					if (edge2.first == u) {
						num++;
						break;
					}
				}
			}

			if (num == 1)
				goto fail;
			if (num == 2)
				twos++;
			if (num == 3)
				threes++;
		}
	}


	assert(twos % 2 == 0);
	assert(threes % 3 == 0);

	return twos / 2 + threes / 3;
fail:
	return 0;
}



void undeg3_delete_edge(Graph &G, GM_undeg3 *op, Edge *e) {
	op->deleted_edges.push_back(e);
}

void vc_undeg3_apply(Graph &G, Vertex *a, Vertex *b, Vertex *c) {
	Vertex *v = new Vertex(G.n, "undeg3_vertex");
	G.n++;
	bp_vertex_create_callback(G, v);

	GM_undeg3 *op = new GM_undeg3(v, a, b, c);

	Vertex *S[] = {a, b, c};
	vector<Vertex *> marked;

	for (int i = 0; i < 3; i++) {
		Vertex *w = S[i];

		for (auto edge: w->edges) {
			Vertex *u = edge.first;
			if (u == S[0] || u == S[1] || u == S[2] || u->marked)
				continue;

			int num = 1;
			bool found[3] = {false, false, false};
			found[i] = true;

			for (int j = 0; j < 3; j++) {
				if (i == j)
					continue;

				for (auto edge2: S[j]->edges) {
					if (edge2.first == u) {
						num++;
						found[j] = true;
						break;
					}
				}
			}
			assert(num >= 2);

			Vertex *x = nullptr;
			if (found[0] && found[1] && found[2]) {
				x = b;
			}
			else {
				if (!found[0])
					x = b;
				if (!found[1])
					x = c;
				if (!found[2])
					x = a;
			}	

			// remove x's edge to u
			assert(x != nullptr);
			for (auto edge2: x->edges) {
				if (edge2.first == u) {
					undeg3_delete_edge(G, op, edge2.second);
					break;
				}
			}
			u->marked = true;
			marked.push_back(u);
		}
	}

	// delete {a, b} and {b, c}
	for (auto edge: b->edges) {
		if (edge.first == a || edge.first == c)
			undeg3_delete_edge(G, op, edge.second);
	}


	// create edges from v to a, b and c
	for (int i = 0; i < 3; i++) {
		Vertex *x = S[i];

		Edge *e = new Edge(G.m, v, x);
		G.m++;
		G.E.push_back(e);
		auto it = G.E.end();
		it--;
		e->iterE = it;

		// the Edge constructor increases deg but we don't want that,
		// rather vert_change_deg should handle it
		v->deg--;
		x->deg--;
		vert_change_deg(G, v, v->edges.size());
		vert_change_deg(G, x, x->edges.size());

		op->new_edges.push_back(e);
	}

	for (Vertex *x: marked)
		x->marked = false;

	for (Edge *e: op->deleted_edges) {
		Vertex *u = e->end[0];
		Vertex *v = e->end[1];


		// move edge to the vertex's covered edges
		move_edge_dirty(e->pos[0], 0, u, u->edges, u->covered);
		move_edge_dirty(e->pos[1], 1, v, v->edges, v->covered);

		bp_edge_delete_callback(G, e);
		vert_change_deg(G, u, u->edges.size());
		vert_change_deg(G, v, v->edges.size());

		G.E.erase(e->iterE);
	}

	// register graph transformation
	G.changes.push_back(op);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

void GM_undeg3::undo(Graph &G) {
	// readd deleted edges
	for (Edge *e: this->deleted_edges) {
		Vertex *u = e->end[0];
		Vertex *v = e->end[1];
		move_edge_dirty(e->pos[0], 0, u, u->covered, u->edges);
		move_edge_dirty(e->pos[1], 1, v, v->covered, v->edges);

		vert_change_deg(G, u, u->edges.size());
		vert_change_deg(G, v, v->edges.size());

		G.E.push_back(e);
		auto it = G.E.end();
		it--;
		e->iterE = it;
	}

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

	delete this->v;
	G.n--;
}

/* translate the VC for the transformed graph into a VC for the original graph */
void GM_undeg3::translate_vc(Graph &G, list<Vertex *> &sol) {
	Vertex *v = this->v;
	Vertex *a = this->a;
	Vertex *b = this->b;
	Vertex *c = this->c;

	int num = 0;
	num += a->in_vc ? 1 : 0;
	num += b->in_vc ? 1 : 0;
	num += c->in_vc ? 1 : 0;
	num += v->in_vc ? 1 : 0;

	assert(num != 0 && num != 4);

	if (num == 3) {
		if (v->in_vc) {
			rvc_remove_vertex(v, sol);
			if (!a->in_vc)
				rvc_add_vertex(a, sol);
			else if (!b->in_vc)
				rvc_add_vertex(b, sol);
			else if (!c->in_vc)
				rvc_add_vertex(c, sol);
		}
	}
	if (num == 2) {
		assert(v->in_vc);
		rvc_remove_vertex(v, sol);

		if (a->in_vc)
			rvc_add_vertex(c, sol);
		else if (b->in_vc)
			rvc_add_vertex(a, sol);
		else if (c->in_vc)
			rvc_add_vertex(b, sol);
	}
	if (num == 1) {
		assert(v->in_vc);
		rvc_remove_vertex(v, sol);
		rvc_add_vertex(b, sol);
	}
}
