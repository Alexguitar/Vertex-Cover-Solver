#include <cassert>
#include <limits>

#include "graph.h"
#include "merge.h"
#include "bipart.h"
#include "read_vc.h"
#include "snapshot.h"
#include "constraints.h"


size_t pos_placeholder =  (numeric_limits<size_t>::max() / 1000) * 1000;
list<Edge *> empty_list2;
auto empty_iterator2 = empty_list2.begin();

// redirect an edge to the merged vertex
void steal_edge(Graph &G, Edge *e, Vertex *from, Vertex *to, Vertex *neigh) {

	if (e->end[0] != from)
		e->flip();

	assert(e->end[0] == from);
	assert(e->end[1] == neigh);
	assert(neigh->edges[e->pos[1]].first == from);

	// redirect edge
	e->end[0] = to;
	e->pos[0] = to->edges.size();
	to->edges.emplace_back(make_pair(neigh, e));

	// update neighbour
	neigh->edges[e->pos[1]].first = to;
}


// delete an edge, but only from the neighbour b
// a will keep its outgoing edge, because it's getting merged and we'll have to add the edge back eventually
void delete_edge(Graph &G, Edge *e, Vertex *a, Vertex *to, Vertex *b) {
	if (e->end[1] != b)
		e->flip();
	size_t a_pos = e->pos[0];
	size_t b_pos = e->pos[1];

	assert(e->end[0] == a);
	assert(e->end[1] == b);
	assert(b->edges[b_pos].first == a);
	assert(e->iterE != empty_iterator2);
	
	// erase a from b's neighbours
	b->edges[b_pos] = b->edges.back();
	int i = b->edges[b_pos].second->end[0] == b ? 0 : 1;
	b->edges[b_pos].second->pos[i] = b_pos;
	b->edges.pop_back();

	e->pos[1] = pos_placeholder;
	G.E.erase(e->iterE);
	e->iterE = empty_iterator2;

	vert_change_deg(G, b, b->deg - 1);
}

MergedVertex::MergedVertex(Graph &G, size_t id, Vertex *u, Vertex *v, Vertex *w)
: Vertex(id, u->name + "+" + v->name + "+" + w->name), G(G) {
	assert(v->deg == 2);
	assert(u->merge == nullptr && v->merge == nullptr && w->merge == nullptr);
	v->merge = this;
	u->merge = this;
	w->merge = this;
	this->downcast = this;
	this->deg = 0;

	this->u = u;
	this->v = v;
	this->w = w;

	u_deg = u->deg;
	w_deg = w->deg;

	// add edges from u
	for (size_t i = 0; i < u->edges.size(); i++) {
		Vertex *a = u->edges[i].first;
		Edge *e   = u->edges[i].second;

		assert(a != w);
		assert(!a->marked);
		if (a == v) {
			G.E.erase(e->iterE);
			e->iterE = empty_iterator2;
			continue;
		}
		a->marked = true;

		steal_edge(G, e, u, this, a);
	}

	// add edges from w
	for (size_t i = 0; i < w->edges.size(); i++) {
		Vertex *a = w->edges[i].first;
		Edge *e   = w->edges[i].second;

		if (a == v) {
			G.E.erase(e->iterE);
			e->iterE = empty_iterator2;
			continue;
		}

		if (a->marked) {
			delete_edge(G, e, w, this, a);
		}
		else {
			steal_edge(G, e, w, this, a);
		}
	}

	// unmark vertices
	for (size_t i = 0; i < this->edges.size(); i++) {
		Vertex *a = this->edges[i].first;
		a->marked = false;
	}

	// delete u, v, w from graph
	// they may still have outgoing edges but no incoming ones
	// this is why we can treat them like degree 0 vertices

	vert_change_deg(G, u, 0);
	vert_change_deg(G, v, 0);
	vert_change_deg(G, w, 0);
	vert_change_deg(G, this, this->edges.size());

	bp_vertex_create_callback(G, this);
	bp_vertex_delete_callback(G, u);
	bp_vertex_delete_callback(G, v);
	bp_vertex_delete_callback(G, w);

	// disable constraint checking for these vertices
	vert_mark_uncertain(G, u);
	vert_mark_uncertain(G, v);
	vert_mark_uncertain(G, w);

	// register this graph transformation
	GM_Vertex_Merge *op = new GM_Vertex_Merge(this);
	G.changes.push_back(op);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


void readd_edge(Graph &G, Edge *e) {
	assert(e->iterE == empty_iterator2);

	G.E.push_back(e);
	auto it = G.E.end();
	it--;
	e->iterE = it;
}

// change the edges of the merged vertex back into the edges they were before
// if the edge was covered before, make sure it is in the list of covered edges
void restore_edges(Graph &G, Vertex *from, Vertex *to, Vertex *v, size_t expected_edges) {
	for (size_t i = 0; i < to->edges.size(); i++) {
		Vertex *b = to->edges[i].first;
		Edge   *e = to->edges[i].second;

		assert(b->merge == nullptr);

		if (b == v) {
			readd_edge(G, e);
			continue;
		}

		// make sure the neighbour is the second end of the edge
		if (e->end[0] == b)
			e->flip();
		assert(e->end[1] == b);

		if (e->end[0] == to) {
			assert(e->pos[1] == pos_placeholder);
			// restore deleted edge
			// since we're rolling back and before merging this vertex
			// the edge was not covered, after restoring it will also
			// not be covered, this means we don't have to check if the
			// neighbour b is in VC
			// since the edge was "deleted" there are no other pointers
			// to it, simple edge insertion is therefore ok
			e->pos[1] = b->edges.size();
			b->edges.emplace_back(make_pair(to, e));

			readd_edge(G, e);
			vert_change_deg(G, b, b->deg + 1);
		}
		else {
			// restore redirected edge
			assert(e->end[0] == from);

			e->end[0] = to;
			e->pos[0] = i;
			b->edges[e->pos[1]].first = to;
		}

		assert(e->end[0]->merge == nullptr);
		assert(e->end[1]->merge == nullptr);
		
	}

	assert(to->edges.size() == expected_edges);
}

MergedVertex::~MergedVertex() {
	// delete us from all relevant lists
	vert_change_deg(this->G, this, 0);
	bp_vertex_delete_callback(G, this);

	assert(u->deg == 0);
	assert(v->deg == 0);
	assert(w->deg == 0);

	v->merge = nullptr;
	u->merge = nullptr;
	w->merge = nullptr;
	restore_edges(this->G, this, u, v, this->u_deg);
	restore_edges(this->G, this, w, v, this->w_deg);

	assert(v->edges.size() == 2);

	vert_change_deg(this->G, u, u->edges.size());
	vert_change_deg(this->G, w, w->edges.size());
	vert_change_deg(this->G, v, 2);

	this->G.n--;
	assert(this->G.n == id);
}

void GM_Vertex_Merge::undo(Graph &G) {
	delete this->m;
}

void GM_Vertex_Merge::translate_vc(Graph &G, list<Vertex *> &sol) {
	MergedVertex *m = this->m;
	assert(!m->v->in_vc && !m->u->in_vc && !m->w->in_vc);

	if (m->in_vc) {
		rvc_add_vertex(m->u, sol);
		rvc_add_vertex(m->w, sol);
		rvc_remove_vertex(m, sol);
	}
	else {
		rvc_add_vertex(m->v, sol);
	}
}
