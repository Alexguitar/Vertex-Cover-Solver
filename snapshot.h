#pragma once

struct graph_snapshot;

#include "graph.h"

struct graph_snapshot {

	struct vc_snapshot {
		size_t num_vertices;
		size_t num_edges;
	} vc;

	size_t num_changes;
	size_t num_constraints;
	size_t num_marked_uncertain;
};


class GraphModification {
public:
	virtual void undo(Graph &G) = 0;	
	virtual void translate_vc(Graph &G, list<Vertex *> &sol) = 0;

	virtual ~GraphModification() {};
};

class GM_Edge_Deletion : public GraphModification {
public:
	vector<Edge *> edges;
	virtual void undo(Graph &G);
	virtual void translate_vc(Graph &G, list<Vertex *> &sol);
};

void restore_snapshot(Graph &G, struct graph_snapshot &s);
struct graph_snapshot create_snapshot(Graph &G);

void add_to_vc(Graph &G, Vertex *a);
void delete_vertex(Graph &G, Vertex *a);
void vert_change_deg(Graph &G, Vertex *v, size_t new_deg);

void move_edge_dirty(size_t pos, int i, Vertex *v, vector< pair<Vertex *, Edge *> > &from, vector< pair<Vertex *, Edge *> > &to);
