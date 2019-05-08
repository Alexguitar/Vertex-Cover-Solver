#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#include "graph.h"
#include "branch.h"
#include "optimize.h"
#include "clique_neigh.h"
#include "clique.h"
#include "deg3.h"
#include "bipart.h"

static vector<string> split(const string& str, const string& delim)
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


void read_config_from_file(char *file) {
	ifstream fin;
	fin.open(file);

	
	string line;
	while (getline(fin, line)) {
		if (line[0] == '#' || line.size() == 0)
			continue;

		// for some reason "\r\n" is not handled correctly
		if (!line.empty() && line[line.size() - 1] == '\r')
		    line.erase(line.size() - 1);

		vector<string> config = split(line, " ");

		if (false) {
			cout << "Hi" << endl;
		}

		// BRANCHING
		else if (config[0] == "CONFIG_BRANCHING_V2") {
			CONFIG_BRANCHING_V2 = config[1] == "true";
		}
		else if (config[0] == "CONFIG_COMPONENTS") {
			CONFIG_COMPONENTS = config[1] == "true";
		}
		else if (config[0] == "CONFIG_MIRROR") {
			CONFIG_MIRROR = config[1] == "true";
		}

		// BOUNDS
		else if (config[0] == "CONFIG_LP_BOUND") {
			CONFIG_LP_BOUND = config[1] == "true";
		}
		else if (config[0] == "CONFIG_CLIQUE_BOUND") {
			CONFIG_CLIQUE_BOUND = config[1] == "true";
		}


		// RULES
		else if (config[0] == "CONFIG_RULE") {
			assert(config.size() == 3);
			int i = stoi(config[1]);
			assert( i >= 1 && i <= NUM_RULES);
			i--;

			if (false)
				;
			else if (config[2] == "OPT_NONE") {
				enabled_rules[i] = OPT_NONE;
			}
			else if (config[2] == "OPT_DEG_1") {
				enabled_rules[i] = OPT_DEG_1;
			}
			else if (config[2] == "OPT_DEG_2") {
				enabled_rules[i] = OPT_DEG_2;
			}
			else if (config[2] == "OPT_DEG_3") {
				enabled_rules[i] = OPT_DEG_3;
			}
			else if (config[2] == "OPT_DOM") {
				enabled_rules[i] = OPT_DOM;
			}
			else if (config[2] == "OPT_UNCONF") {
				enabled_rules[i] = OPT_UNCONF;
			}
			else if (config[2] == "OPT_CN") {
				enabled_rules[i] = OPT_CN;
			}
			else if (config[2] == "OPT_LP") {
				enabled_rules[i] = OPT_LP;
			}
			else if (config[2] == "OPT_DEG_12") {
				enabled_rules[i] = OPT_DEG_12;
			}
			else if (config[2] == "OPT_UNCONF_COMBO") {
				enabled_rules[i] = OPT_UNCONF_COMBO;
			}
			else if (config[2] == "OPT_UNDEG_3") {
				enabled_rules[i] = OPT_UNDEG_3;
			}
			else {
				cout << "Unknown rule " << config[2] << endl;
				exit(1);
			}

			
		}

		// CLIQUE NEIGHBOURHOOD
		else if (config[0] == "CN_CHECK1_ENABLED") {
			CN_CHECK1_ENABLED = config[1] == "true";
		}
		else if (config[0] == "CN_CHECK1_MIN_DEG") {
			CN_CHECK1_MIN_DEG = stoull(config[1]);
		}
		else if (config[0] == "CN_CHECK1_MAX_DEG") {
			CN_CHECK1_MAX_DEG = stoull(config[1]);
		}

		else if (config[0] == "CN_CHECK2_ENABLED") {
			CN_CHECK2_ENABLED = config[1] == "true";
		}
		else if (config[0] == "CN_CHECK2_CUTOFF") {
			CN_CHECK2_CUTOFF = stof(config[1]);
		}
		else if (config[0] == "CN_CHECK2_RELAX_N") {
			CN_CHECK2_RELAX_N = stoull(config[1]);
		}
		else if (config[0] == "CN_CHECK2_LARGE_N") {
			CN_CHECK2_LARGE_N = stoull(config[1]);
		}
		else if (config[0] == "CN_CHECK2_LARGE_K") {
			CN_CHECK2_LARGE_K = stoull(config[1]);
		}


		// DEG 3
		else if (config[0] == "DEG3_CUTOFF1") {
			DEG3_CUTOFF1 = stoull(config[1]);
		}
		else if (config[0] == "DEG3_CUTOFF2") {
			DEG3_CUTOFF2 = stoull(config[1]);
		}

		
		// LP BOUND
		else if (config[0] == "LP_BOUND_CUTOFF") {
			LP_BOUND_CUTOFF = stof(config[1]);
		}


		// CLIQUE BOUND
		else if (config[0] == "CLIQUE_BOUND_ITER") {
			CLIQUE_BOUND_ITER = stoull(config[1]);
		}
		else if (config[0] == "CLIQUE_BOUND_ASCEND") {
			CLIQUE_BOUND_ASCEND = config[1] == "true";
		}
		else if (config[0] == "CLIQUE_BOUND_MIXED") {
			CLIQUE_BOUND_MIXED = config[1] == "true";
		}
		else if (config[0] == "CLIQUE_BOUND_SHUFFLE_DIST") {
			CLIQUE_BOUND_SHUFFLE_DIST = stof(config[1]);
		}
		else if (config[0] == "CLIQUE_BOUND_SHUFFLE_PCT") {
			CLIQUE_BOUND_SHUFFLE_PCT = stoull(config[1]);
		}


		// UNFONFINED RULE
		else if (config[0] == "UNCONF_CUTOFF") {
			UNCONF_CUTOFF = stoull(config[1]);
		}
		else if (config[0] == "UNCONF_MAX_DEG") {
			UNCONF_MAX_DEG = stoull(config[1]);
		}


		else {
			cout << "Unknown option " << config[0] << endl;
			exit(1);
		}
	}
}
