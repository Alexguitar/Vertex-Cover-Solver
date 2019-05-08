sources := main.cpp graph.cpp snapshot.cpp optimize.cpp merge.cpp bipart.cpp clique.cpp heap.cpp stats.cpp time.cpp read_vc.cpp deg3.cpp clique_neigh.cpp mirror.cpp branch.cpp config.cpp undeg3.cpp constraints.cpp score.cpp
headers := graph.h util.h snapshot.h optimize.h merge.h bipart.h clique.h heap.h stats.h time.h read_vc.h deg3.h clique_neigh.h mirror.h branch.h config.h undeg3.h constraints.h score.h
DEFINES := -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM
DEFINES += -DCONFIG_LP
#DEFINES :=
CFLAGS := -std=c++11 -O2
#CFLAGS := -std=c++11 -g -Wall -Wextra
#CFLAGS := -std=c++11 -g -Wall -Wextra -pg -no-pie
CANDIDATE_0 :=
CANDIDATE_1 := -DOPTIM_1
CANDIDATE_2 := -DOPTIM_1 -DOPTIM_2
CANDIDATE_3 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM
CANDIDATE_4 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF
CANDIDATE_5 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_LP
CANDIDATE_6 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM
CANDIDATE_7 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM -DCONFIG_LP
CANDIDATE_8 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM -DCONFIG_LP -DTIMES_2
CANDIDATE_9 := -DOPTIM_1 -DOPTIM_2 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM -DCONFIG_LP -DTIMES_8

CANDIDATE_X := #-DOPTIM_1 -DOPTIM_2 -DOPTIM_3 -DOPTIM_DOM -DOPTIM_UNCONF -DCONFIG_CLIQUE -DCONFIG_CLIQUE_RANDOM -DCONFIG_MIRROR -DOPTIM_CN -DCONFIG_BRANCHING_V2 -DCONFIG_CYCLE_BOUND

main: $(sources) $(headers) Makefile
	g++ $(CFLAGS) $(CANDIDATE_X) -o $@ $(sources)
