#pragma once
#include <string>

#include "graph.h"


void vertex_cover_v2(Graph &G, string td, size_t n);


/* configurables */
extern bool CONFIG_MIRROR;
extern bool CONFIG_COMPONENTS;
extern bool CONFIG_BRANCHING_V2;
extern bool CONFIG_LP_BOUND;
extern bool CONFIG_CLIQUE_BOUND;
