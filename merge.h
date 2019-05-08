#pragma once

#include "graph.h"

// specifically tailored for the degree 2 rule,
// won't work on other types of vertices

class MergedVertex : public Vertex {
public:
	Vertex *u, *v, *w;

	size_t u_deg;
	size_t w_deg;

	list< pair<Vertex *, Edge *> > u_edges;
	list< pair<Vertex *, Edge *> > w_edges;

	MergedVertex(Graph &G, size_t id, Vertex *a, Vertex *b, Vertex *c);
	~MergedVertex();
	
private:
	Graph &G;
};


class GM_Vertex_Merge : public GraphModification {
public:
	virtual void undo(Graph &G);
	virtual void translate_vc(Graph &G, list<Vertex *> &sol);
	GM_Vertex_Merge(MergedVertex *v) : m(v) {};

	MergedVertex *m;
};
