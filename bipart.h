#pragma once

#include "graph.h"

void bp_matching_init(Graph &G);
void bp_matching(Graph &G);
void bp_vertex_cover(Graph &G);

long long lp_bound(Graph &G);

void bp_vertex_delete_callback(Graph &G, Vertex *v);
void bp_vertex_create_callback(Graph &G, Vertex *v);
void bp_edge_delete_callback(Graph &G, Edge *e);

void tarjan(Vertex* v, list<Vertex *>* S, size_t maxdfs, bool left);
void strongly_connected_components(Graph &G);
void lp_flow(Graph &G);

/* configurables */
extern size_t LP_BOUND_CUTOFF;
