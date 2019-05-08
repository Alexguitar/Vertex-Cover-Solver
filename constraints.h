#pragma once

#include <vector>
using namespace std;

class Graph;
class Vertex;

class Constraint {
public:
	virtual bool check(Graph &G) = 0;
	virtual ~Constraint() {};
};

class NotAllNeighboursConstraint : public Constraint {
public:
	Vertex *v;
	vector<Vertex *> neighbours;

	virtual bool check(Graph &G);
	NotAllNeighboursConstraint(Vertex *v);
};

void vert_mark_uncertain(Graph &G, Vertex *v);
