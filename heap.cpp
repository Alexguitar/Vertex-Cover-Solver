#include <cassert>
#include "heap.h"

#define left(i)		(2 * (i) + 1)
#define right(i)	(2 * (i) + 2)
#define parent(i)	(((i) - 1) / 2)

#include <iostream>

#if 0

bool vert_cmp(Vertex *a, Vertex *b) {
	if (a->deg == b->deg)
		return a->id > b->id;
	return a->deg > b->deg;
}

// under the assumption that the left and right for subheaps, correct their parent
// i.e. let the parent "sink"
void VertexMaxHeap::sift_down(size_t pos) {
	size_t l, r, largest;

	l = left(pos);
	r = right(pos);

	largest = pos;

	if (l < this->array.size() && vert_cmp(this->array[l], this->array[pos]))
		largest = l;

	if (r < this->array.size() && vert_cmp(this->array[r], this->array[largest]))
		largest = r;

	if (largest != pos) {
		Vertex *tmp = this->array[pos];
		this->array[pos] = this->array[largest];
		this->array[largest] = tmp;

		this->array[pos]->heap_pos = pos;
		this->array[largest]->heap_pos = largest;

		sift_down(largest);
	}
}

void VertexMaxHeap::sift_up(size_t pos) {
	Vertex *v = this->array[pos];

	while (pos != 0 && vert_cmp(v, this->array[parent(pos)])) {
		this->array[pos] = this->array[parent(pos)];
		this->array[pos]->heap_pos = pos;
		pos = parent(pos);
	}
	this->array[pos] = v;
	v->heap_pos = pos;
}

void VertexMaxHeap::update(Vertex *v) {
	size_t pos = v->heap_pos;
	assert(this->array[pos] == v);

	size_t l = left(pos);
	size_t r = right(pos);

	size_t largest = pos;

	if (l < this->array.size() && vert_cmp(this->array[l], this->array[pos]))
		largest = l;

	if (r < this->array.size() && vert_cmp(this->array[r], this->array[largest]))
		largest = r;

	if (largest != pos) {
		// let it sink
		sift_down(pos);
	} 
	else {
		// let it rise
		sift_up(pos);
	}
}

void VertexMaxHeap::insert(Vertex *v) {
	size_t pos;

	pos = this->array.size();
	this->array.push_back(v);

	sift_up(pos);
}

void VertexMaxHeap::erase(Vertex *v) {
	assert(this->array.size() > 0);
	size_t pos = v->heap_pos;
	assert(this->array[pos] == v);

	// overwrite
	this->array[pos] = this->array[this->array.size() - 1];
	this->array[pos]->heap_pos = pos;

	this->array.resize(this->array.size() - 1);

	update(this->array[pos]);
}

void VertexMaxHeap::insert_lazy(Vertex *v) {
	size_t pos;

	pos = this->array.size();
	this->array.push_back(v);
	v->heap_pos = pos;
}

void VertexMaxHeap::erase_lazy(Vertex *v) {
	assert(this->array.size() > 0);
	size_t pos = v->heap_pos;
	assert(this->array[pos] == v);

	// overwrite
	this->array[pos] = this->array[this->array.size() - 1];
	this->array[pos]->heap_pos = pos;

	this->array.resize(this->array.size() - 1);
}

void VertexMaxHeap::make_heap() {
	for (size_t i = this->array.size(); i-- > 0; ) {
		sift_down(i);
	}
}

Vertex *VertexMaxHeap::pop() {
	Vertex *v = this->array[0];
	erase(this->array[0]);
	return v;
}

Vertex *VertexMaxHeap::peak() {
	assert(this->array.size() > 0);
	return  this->array[0];
}

#endif
