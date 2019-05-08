#pragma once

#include "graph.h"
#include "snapshot.h"

class GM_clique_neigh : public GraphModification {
public:
	virtual void undo(Graph &G);
	virtual void translate_vc(Graph &G, list<Vertex *> &sol);


	Vertex *v = nullptr;
	// pair of vertex C1 and its non neighbour in C2
	vector< pair<Vertex *, Vertex *> > C1;
	vector<Vertex *> C2;

	vector<Edge *> new_edges;
};

bool vc_clique_neigh_single(Graph &G, Vertex *v, long long &k);

/* configurables */

extern bool   CN_CHECK1_ENABLED;
extern size_t CN_CHECK1_MIN_DEG;
extern size_t CN_CHECK1_MAX_DEG;

extern bool   CN_CHECK2_ENABLED;
extern float  CN_CHECK2_CUTOFF;
extern size_t CN_CHECK2_RELAX_N;
extern size_t CN_CHECK2_LARGE_N;
extern size_t CN_CHECK2_LARGE_K;
