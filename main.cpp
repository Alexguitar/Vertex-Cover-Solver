#include <iostream>

#include <string>
#include <sstream>
#include <cassert>
#include <unordered_set>

#include "graph.h"
#include "branch.h"
#include "config.h"
#include "time.h"

using namespace std;



/*********************************************************************************************************************/

vector<string> split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos-prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}

template<typename T>
inline void hash_combine(std::size_t& seed, const T& val)
{
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

//  taken from https://stackoverflow.com/a/7222201/916549
//
template<typename S, typename T>
struct hash<std::pair<S, T>>
{
    inline size_t operator()(const std::pair<S, T>& val) const
    {
        size_t seed = 0;
        hash_combine(seed, val.first);
        hash_combine(seed, val.second);
        return seed;
    }
};

int main(int argc, char **argv) {
	// start measuring time
	//TIME_start = chrono::steady_clock::now(); 

	Graph G;

	string line;
	string td;
	size_t n = 0;


	// read configuration from file
	if (argc == 2) {
		read_config_from_file(argv[1]);
	}
	if (argc > 2) {
		cout << "Usage: " << argv[0] << " [config.conf]" << endl;
	}

	// little performance boost for iostream
	std::ios::sync_with_stdio(false);

	// add memory for seen edges and nodes
	std::unordered_set<std::pair<Vertex*, Vertex*>> edgeSet;

	// parse the input
	while (getline(cin, line)) {
		if (line[0] == 'c' || line.size() == 0)
			continue;

		// for some reason "\r\n" is not handled correctly
		if (!line.empty() && line[line.size() - 1] == '\r')
		    line.erase(line.size() - 1);

		vector<string> vertex = split(line, " ");

		if (vertex[0][0] == 'p') {
			td = vertex[1];
			n = stoul(vertex[2]);
			continue;
		}


		if (vertex.size() != 2) {
			cout << "Parsing error\n";
			return 1;
		}

		// the two verticies of the edge
		Vertex *a, *b;
		a = nullptr;
		b = nullptr;

		// find them
		for (int i = 0; i < 2; i++) {
			string name = vertex[i];

			// create the verticies if they don't already exist
			if (G.name_map.find(name) == G.name_map.end()) {
				Vertex *v = new Vertex(G.n, name);

				// map the name of the vertex to an id
				G.name_map[name] = v;

				G.V.push_back(v);

				if (i == 0)
					a = v;
				else	
					b = v;

				G.n++;
			}
			else {
				if (i == 0)
					a = G.name_map[name];
				else	
					b = G.name_map[name];
			}
		}

		if (a == b)
		{
			// create a new vertex with the same name
			Vertex *cov = new Vertex(G.n, a->name);
			G.V.push_back(cov);
			G.n++;

			Edge *e = new Edge(G.m, a, cov);
			G.E.push_back(e);
			auto it = G.E.end();
			it--;
			e->iterE = it;
			G.m++;
		} 
		else if (edgeSet.find(std::make_pair(a,b)) == edgeSet.end() && edgeSet.find(std::make_pair(b,a)) == edgeSet.end()) {
			// create the edge
			Edge *e = new Edge(G.m, a, b);
			
			G.E.push_back(e);
			auto it = G.E.end();
			it--;
			e->iterE = it;

			G.m++;

			edgeSet.emplace(a,b);
		}
	}

	/* init other data structures */

	// populate deg1s and set up iterators
	for (auto it = G.V.begin(); it != G.V.end(); it++) {
		Vertex *v = *it;
	
		v->iterV = it;

		if (v->deg == 1) {
			G.deg1s.push_back(v);
			auto it = G.deg1s.end();
			it--;
			v->iter1 = it;
		}
		else if (v->deg == 2) {
			G.deg2s.push_back(v);
			auto it = G.deg2s.end();
			it--;
			v->iter2 = it;
		}
		else if (v->deg == 3) {
			G.deg3s.push_back(v);
			auto it = G.deg3s.end();
			it--;
			v->iter3 = it;
		}
	}

	cout << "c nodes: " << G.V.size() << endl;
	cout << "c edges: " << G.E.size() << endl;

	srand(G.V.size());

	if (CONFIG_BRANCHING_V2) {
		vertex_cover_v2(G, td, n);
	}
	else {
		vertex_cover(G, td, n);
	}


	return 0;
}
