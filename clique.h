#pragma once

#include "graph.h"

long long clique_bound(Graph &G);

/* configurables */
extern int    CLIQUE_BOUND_ITER;
extern bool   CLIQUE_BOUND_ASCEND;
extern bool   CLIQUE_BOUND_MIXED;
extern size_t CLIQUE_BOUND_SHUFFLE_PCT;
extern float  CLIQUE_BOUND_SHUFFLE_DIST;
