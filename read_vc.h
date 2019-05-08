#pragma once

#include "graph.h"

void read_vc(Graph &G, list<Vertex *> &sol);
void rvc_add_vertex(Vertex *v, list<Vertex *> &sol);
void rvc_remove_vertex(Vertex *v, list<Vertex *> &sol);
