#pragma once

#include <vector>
using namespace std;

class Vertex;

// homebaked heap because we need more flexibility than STL provides
class VertexMaxHeap {
public:
	vector<Vertex *> array;


	void insert(Vertex *v);
	void erase(Vertex *v);
	void update(Vertex *v);

	void insert_lazy(Vertex *v);
	void erase_lazy(Vertex *v);
	void make_heap();

	Vertex *pop();
	Vertex *peak();

private:
	void sift_down(size_t pos);
	void sift_up(size_t pos);
};

#include "graph.h"
