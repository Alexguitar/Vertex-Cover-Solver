#include <cassert>
#include <algorithm>
#include <cstdlib>

#include "clique.h"


int    CLIQUE_BOUND_ITER         = 1;
bool   CLIQUE_BOUND_ASCEND       = false;
bool   CLIQUE_BOUND_MIXED        = false;
size_t CLIQUE_BOUND_SHUFFLE_PCT  = 50;
float  CLIQUE_BOUND_SHUFFLE_DIST = 0.3;

Vertex *find_leader_marked(Vertex *v) {
	if (!v->marked)
		return nullptr;

	if (v->next_member == nullptr)
		return v;
	return find_leader_marked(v->next_member);
	
}

void join_largest_clique(Vertex *a) {
	// mark all neighbours
	for (auto it = a->edges.begin(); it != a->edges.end(); it++) {
		Vertex *b = it->first;
		assert(!b->marked);
		b->marked = true;
	}

	pair<size_t, Vertex *> largest(0, nullptr);

	// find largest clique we can join
	for (auto it = a->edges.begin(); it != a->edges.end(); it++) {
		Vertex *b = it->first;
		if (b->clique_size != 1)
			continue;

		Vertex *leader = find_leader_marked(b);
		if (leader == nullptr)
			continue;

		assert(leader->clique_size <= a->edges.size());

		if (leader->clique_size > largest.first) {
			largest.first  = leader->clique_size;
			largest.second = b;
		}
	
	}

	// join largest clique
	for (Vertex *b = largest.second; b != nullptr; b = b->next_member) {
		b->clique_size++;
	}

	a->next_member = largest.second;

	// unmark neighbours
	for (auto it = a->edges.begin(); it != a->edges.end(); it++) {
		Vertex *b = it->first;
		assert(b->marked);
		b->marked = false;
	}
}


void clique_bound_sort(Graph &G, vector<Vertex *> &array) {
	static vector<Vertex *> bucket[30];
	static vector<Vertex *> high_deg;
	static struct vertex_cmp cmp;

	for (Vertex *v: G.V) {
		size_t deg = v->deg;

		if (deg < 30)
			bucket[deg].push_back(v);
		else
			high_deg.push_back(v);
	}

	sort(high_deg.begin(), high_deg.end(), cmp);

	for (int i = 0; i < 30; i++) {
		array.insert(array.end(), bucket[i].begin(), bucket[i].end());
		bucket[i].clear();
	}
	array.insert(array.end(), high_deg.begin(), high_deg.end());
	high_deg.clear();
}

void clique_bound_shuffle(vector<Vertex *> &array) {

	for (long long i = 0; i < array.size(); i++) {
		if (rand() % 100 >= CLIQUE_BOUND_SHUFFLE_PCT)
			continue;

		long long dist = (long long) ((rand() % array.size()) * CLIQUE_BOUND_SHUFFLE_DIST);
		if (rand() % 2 == 0)
			dist = -dist;

		if (0 <= i + dist && i + dist < array.size())
			swap(array[i], array[i + dist]);
	}
}

long long clique_bound(Graph &G) {	

	static vector<Vertex *> array;
	array.reserve(G.V.size());

	clique_bound_sort(G, array);
	
	long long best = 0;

	for (int iter = 0; iter < CLIQUE_BOUND_ITER; iter++) {
		if (CLIQUE_BOUND_MIXED && iter >= 2 && iter % 2 == 0)
			clique_bound_shuffle(array);
		if (!CLIQUE_BOUND_MIXED && iter >= 1 )
			clique_bound_shuffle(array);

		bool ascending = (CLIQUE_BOUND_ASCEND && !CLIQUE_BOUND_MIXED) || (iter % 2 == 1 && CLIQUE_BOUND_MIXED);
		long long bound = 0;
		
		for (size_t i = 0; i < array.size(); i++) {
			Vertex *v = array[i];
			v->next_member = nullptr;
			v->clique_size = 1;
		}

		for (size_t i = 0; i < array.size(); i++) {
			size_t j;
			if (ascending)
				j = i;
			else
				j = array.size() - 1 - i;

			Vertex *v = array[j];

			if (v->clique_size > 1)
				continue;

			join_largest_clique(v);
		}

		for (size_t i = 0; i < array.size(); i++) {
			Vertex *v = array[i];
			if (v->next_member == nullptr)
				bound += v->clique_size - 1;
		}

		best = max(best, bound);
	}

	array.clear();
	return best;
}
