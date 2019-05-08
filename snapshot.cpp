#include <cassert>

#include "graph.h"
#include "merge.h"
#include "deg3.h"
#include "bipart.h"
#include "snapshot.h"

/* change the degree of a vertex
 * takes care of placing v in the correct lists
 */
void vert_change_deg(Graph &G, Vertex *v, size_t new_deg) {
	if (v->deg == new_deg)
		return;

	if (v->deg == 0) {
		v->deg = new_deg;

		G.V.push_back(v);
		auto it = G.V.end();
		it--;
		v->iterV = it;

		if (v->deg == 1) {
			G.deg1s.push_back(v);
			it = G.deg1s.end();
			it--;
			v->iter1 = it;
		}
		else if (v->deg == 2) {
			G.deg2s.push_back(v);
			it = G.deg2s.end();
			it--;
			v->iter2 = it;
		}
		else if (v->deg == 3) {
			G.deg3s.push_back(v);
			it = G.deg3s.end();
			it--;
			v->iter3 = it;
		}
	}
	else if (new_deg == 0) {
		if (v->deg == 1)
			G.deg1s.erase(v->iter1);
		else if (v->deg == 2)
			G.deg2s.erase(v->iter2);
		else if (v->deg == 3)
			G.deg3s.erase(v->iter3);
		G.V.erase(v->iterV);

		v->deg = 0;
	}
	else {
		if (v->deg == 1)
			G.deg1s.erase(v->iter1);
		else if (v->deg == 2)
			G.deg2s.erase(v->iter2);
		else if (v->deg == 3)
			G.deg3s.erase(v->iter3);

		if (new_deg == 1) {
			G.deg1s.push_back(v);
			auto it = G.deg1s.end();
			it--;
			v->iter1 = it;
		}
		else if (new_deg == 2) {
			G.deg2s.push_back(v);
			auto it = G.deg2s.end();
			it--;
			v->iter2 = it;
		}
		else if (new_deg == 3) {
			G.deg3s.push_back(v);
			auto it = G.deg3s.end();
			it--;
			v->iter3 = it;
		}

		v->deg = new_deg;
	}
}

void register_edge_deletion(Graph &G, Edge *e) {
	GM_Edge_Deletion *last = nullptr;
	bool merged = false;

	if (G.changes.size() != 0) {
		// if the last transformation was an edge deletion merge them
		last = dynamic_cast<GM_Edge_Deletion *> (G.changes.back());
		merged = last != nullptr;
	}

	if (last == nullptr) {
		last = new GM_Edge_Deletion();
	}

	last->edges.push_back(e);
	if (!merged) {
		G.changes.push_back(last);
	}
}

void move_edge_dirty(size_t pos, int i, Vertex *v, vector< pair<Vertex *, Edge *> > &from, vector< pair<Vertex *, Edge *> > &to) {
	// the edge will be moved to the new list
	// its spot in the old list will be taken by the last element
	int j = from.back().second->end[0] == v ? 0 : 1;

	// copy the edge into the new list
	to.emplace_back(from[pos]);

	// move the last element into the same spot
	// Note: this can be the same edge! Gotta be careful
	from.back().second->pos[j] = pos;
	from[pos] = from.back();
	from.pop_back();

	// set the position in the new list
	to.back().second->pos[i] = to.size() - 1;
}

/* deletes a vertex from the graph
 * deleted edges will be treated like covered edges
 */
void delete_vertex(Graph &G, Vertex *v) {
	assert(v->merge == nullptr);

	size_t pos = v->covered.size();

	// cover the edges
	while (v->edges.size() > 0) {
		Vertex *u = v->edges[0].first;
		Edge *e = v->edges[0].second;

		if (e->end[0] != v)
			e->flip();
		assert(e->end[0] == v);
		assert(e->pos[0] == 0);
		assert(u->edges[e->pos[1]].first == v);
		assert(v->edges[e->pos[0]].first == u);

		// move edge to the vertex's covered edges
		move_edge_dirty(e->pos[0], 0, v, v->edges, v->covered);
		move_edge_dirty(e->pos[1], 1, u, u->edges, u->covered);

		assert(u->covered[e->pos[1]].first == v);
		assert(v->covered[e->pos[0]].first == u);

		// add the edge to the covered edges
		register_edge_deletion(G, e);
		G.VC.E.push_back(e);
		G.E.erase(e->iterE);

		e->covered = true;
	}

	// update degrees in the graph
	bp_vertex_delete_callback(G, v);
	vert_change_deg(G, v, 0);
	for (size_t i = pos; i < v->covered.size(); i++) {
		Vertex *u = v->covered[i].first;
		assert(u->deg == u->edges.size() + 1);

		// decrease the degree of the vertex
		vert_change_deg(G, u, u->edges.size());
		
	}

}

/* add a vertex to the vertex cover
 * 
 * this function also does all the housekeeping for making sure operations can be reversed
 */
void add_to_vc(Graph &G, Vertex *v) {
	delete_vertex(G, v);
	G.VC.V.push_back(v);
	assert(!v->cstr_in_vc);
	v->cstr_in_vc = true;
}

struct graph_snapshot create_snapshot(Graph &G) {
	struct graph_snapshot snapshot;

	snapshot.vc.num_vertices = G.VC.V.size();
	snapshot.vc.num_edges    = G.VC.E.size();
	snapshot.num_changes     = G.changes.size();

	snapshot.num_constraints = G.constraints.size();
	snapshot.num_marked_uncertain = G.marked_uncertain.size();

	// create first graph modification entry
	G.changes.push_back(new GM_Edge_Deletion());

	return snapshot;
}




/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/




/* mark an edge as uncovered - move the edge to the appropriate lists */
void uncover_edge(Graph &G, Edge *e) {
	assert(e->covered == true);
	e->covered = false;

	// add the edge back to the vertices
	for (int i = 0; i < 2; i++) {
		Vertex *v = e->end[i];
		move_edge_dirty(e->pos[i], i, v, v->covered, v->edges);
	}
	Vertex *v = e->end[0];
	Vertex *u = e->end[1];

	assert(v->edges[e->pos[0]].first == u);
	assert(u->edges[e->pos[1]].first == v);

	assert(v->deg + 1 == v->edges.size());
	assert(u->deg + 1 == u->edges.size());

	vert_change_deg(G, v, v->deg + 1);
	vert_change_deg(G, u, u->deg + 1);

	G.E.push_back(e);
	auto it = G.E.end();
	it--;
	e->iterE = it;
}

void GM_Edge_Deletion::undo(Graph &G) {
	for (Edge *e: this->edges) {
		uncover_edge(G, e);
	}
}

void GM_Edge_Deletion::translate_vc(Graph &G, list<Vertex *> &sol) {
	// nothing to do
	return;
}

void restore_snapshot(Graph &G, struct graph_snapshot &s)
{
	// unmark picked verticies
	for (size_t i = s.vc.num_vertices; i < G.VC.V.size(); i++) {
		assert(G.VC.V[i]->cstr_in_vc);
		G.VC.V[i]->cstr_in_vc = false;
	}
	G.VC.V.erase(G.VC.V.begin() + s.vc.num_vertices, G.VC.V.end());

	// unmark uncertain vertices
	for (size_t i = s.num_marked_uncertain; i < G.marked_uncertain.size(); i++) {
		assert(G.marked_uncertain[i]->cstr_uncertain);
		G.marked_uncertain[i]->cstr_uncertain = false;
	}
	G.marked_uncertain.erase(G.marked_uncertain.begin() + s.num_marked_uncertain, G.marked_uncertain.end());

	// undo graph transformations
	for (size_t i = 0; i < G.changes.size() - s.num_changes; i++) {
		size_t j = G.changes.size() - i - 1;
		GraphModification *op = G.changes[j];
		op->undo(G);
		delete op;
	}
	G.changes.erase(G.changes.begin() + s.num_changes, G.changes.end());

	// erase the edges from the VC
	G.VC.E.erase(G.VC.E.begin() + s.vc.num_edges, G.VC.E.end());

	// delete constraints
	for (size_t i = s.num_constraints; i < G.constraints.size(); i++) {
		delete G.constraints[i];
	}
	G.constraints.erase(G.constraints.begin() + s.num_constraints, G.constraints.end());
}

