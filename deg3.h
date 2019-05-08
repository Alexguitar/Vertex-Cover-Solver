#pragma once

#include "graph.h"
#include "snapshot.h"

class GM_deg3 : public GraphModification {
public:
	virtual void undo(Graph &G);
	virtual void translate_vc(Graph &G, list<Vertex *> &sol);

	GM_deg3(Vertex *V, Vertex *A, Vertex *B, Vertex *C) : v(V), a(A), b(B), c(C) {};

	Vertex *v;
	Vertex *a;
	Vertex *b;
	Vertex *c;
	vector<Edge *> new_edges;
};

bool vc_deg3_rule(Graph &G, long long &k);


/* configurables */
extern size_t DEG3_CUTOFF1;
extern size_t DEG3_CUTOFF2;
