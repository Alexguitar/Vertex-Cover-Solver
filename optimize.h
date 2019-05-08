#pragma once

#include <cassert>

#include "graph.h"

bool vc_deg1_rule(Graph &G);
bool vc_deg2_rule(Graph &G, long long &k);
bool vc_domination_rule(Graph &G);
bool vc_unconfined_rule(Graph &G);

bool vc_deg2_rule_single(Graph &G, long long &k, Vertex *v);

void vc_optimize(Graph &G, long long &k);
void vc_preoptimize(Graph &G, long long &k);


#define NUM_RULES 16

enum optimization_rules {
	OPT_NONE,
	OPT_DEG_1,
	OPT_DEG_2,
	OPT_DEG_3,
	OPT_DOM,
	OPT_UNCONF,
	OPT_CN,
	OPT_LP,
	OPT_DEG_12,
	OPT_UNCONF_COMBO,
	OPT_UNDEG_3
};

class unconfined_data {
public:
	vector<Vertex *> S;
	vector<Vertex *> NS;	// N(S)

	void add(Vertex *v) {
		assert(!v->S_marked);
		v->S_marked = true;
		S.push_back(v);

		v->NS_marked = false;

		// remove v from NS if it's there
		for (size_t i = 0; i < NS.size(); i++) {
			if (NS[i] == v) {
				NS[i] = NS.back();
				NS.pop_back();
				break;
			}
		}

		for (auto edge: v->edges) {
			Vertex *u = edge.first;

			if (!u->NS_marked && !u->S_marked) {
				u->NS_marked = true;
				NS.push_back(u);
			}
		}
	}

	~unconfined_data() {
		for (Vertex *v: S) {
			v->S_marked = false;
		}
		for (Vertex *v: NS) {
			v->NS_marked = false;
		}
	}
};

/* configurables */

extern size_t UNCONF_CUTOFF;
extern size_t UNCONF_MAX_DEG;
extern optimization_rules enabled_rules[NUM_RULES];
