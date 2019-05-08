#pragma once

#include "graph.h"

inline void remove_edge_dirty(Vertex *v, size_t pos, vector< pair<Vertex *, Edge *> > &edges) {
	edges[pos] = edges.back();
	int j = edges[pos].second->end[0] == v ? 0 : 1;
	edges[pos].second->pos[j] = pos;
	edges.pop_back();
}
