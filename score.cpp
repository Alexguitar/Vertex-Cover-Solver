#include <limits>
#include <algorithm>

#include "graph.h"
#include "optimize.h"
#include "score.h"

float inf = numeric_limits<float>::infinity();

void vc_deg2_single_score(Vertex *v) {
	if (v->deg == 1 || v->deg > 4)
		return;

	// never branch on this, can be simply optimized away
	if (v->deg == 2) {
		v->dont_branch = true;
		return;
	}
	

	float score = 1.0 / (v->deg - 2); //+ v->score;

	for (auto edge: v->edges) {
		Vertex *b = edge.first;

		b->score += score;
	}
}

class custom_queue {
public:
	vector< pair<Vertex *, size_t> > array;

	size_t num = 20;

	static bool tmp_cmp(const pair<Vertex *, size_t> & a, const pair<Vertex *, size_t> & b) {
		return a.second < b.second;
	}

	void add(Vertex *v, size_t NS_count) {
		array.push_back(make_pair(v, NS_count));
		sort(array.begin(), array.end(), tmp_cmp);

		if (array.size() > num)
			array.pop_back();
	}

};

void vc_unconf_single_score(Vertex *v) {
	unconfined_data D;

	D.add(v);

	while (true) {
		Vertex *u = nullptr;
		size_t count = 2;
		custom_queue u_queue;
		Vertex *z = nullptr; // w in the slides

		// find u
		for (Vertex *w: D.NS) {
			size_t S_count  = 0; // | N(w) intersect S |
			size_t NS_count = 0; // | N(w) \ N[S] |
			Vertex *x = nullptr;

			for (auto edge: w->edges) {
				Vertex *y = edge.first;
				if (y->S_marked) {
					S_count++;
					if (S_count > 1)
						break;
				}

				if (!y->S_marked && !y->NS_marked) {
					NS_count++;
					x = y;
				}
			}

			if (S_count != 1)
				continue;

			if (NS_count < count) {
				u = w;
				z = x;
				count = NS_count;
				if (count == 0) {
					// never branch on this vertex
					v->dont_branch = true;
					return;
				}
			}

			// relax the condition somewhat
			if (NS_count < 8) {
				u_queue.add(w, NS_count);
			}
		}

		// we can almost apply the rule
		for (auto pair: u_queue.array) {
			size_t NS_count = pair.second;
			Vertex *a = pair.first;

			float score = (1.0) / (float) (NS_count);

			for (auto edge: a->edges) {
				Vertex *y = edge.first;

				if (!y->S_marked && !y->NS_marked) {
					y->score += score;
				}
			}
		}

		if (u == nullptr)
			break;

		assert(count == 1);
		D.add(z); 
		// try again
	}
}

void graph_assign_scores(Graph &G) {
	for (Vertex *v: G.V) {
		v->score = 0.0;
		v->dont_branch = false;
	}

	for (Vertex *v: G.V) {
		vc_deg2_single_score(v);
		vc_unconf_single_score(v);
	}
	
	for (Vertex *v: G.V) {
		v->score += (float) v->deg;
	}
}
