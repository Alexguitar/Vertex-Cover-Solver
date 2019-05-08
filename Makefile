sources := main.cpp graph.cpp snapshot.cpp optimize.cpp merge.cpp bipart.cpp clique.cpp heap.cpp stats.cpp time.cpp read_vc.cpp deg3.cpp clique_neigh.cpp mirror.cpp branch.cpp config.cpp undeg3.cpp constraints.cpp score.cpp
headers := graph.h util.h snapshot.h optimize.h merge.h bipart.h clique.h heap.h stats.h time.h read_vc.h deg3.h clique_neigh.h mirror.h branch.h config.h undeg3.h constraints.h score.h
#DEFINES :=
CFLAGS := -std=c++11 -O2
#CFLAGS := -std=c++11 -g -Wall -Wextra
#CFLAGS := -std=c++11 -g -Wall -Wextra -pg -no-pie

main: $(sources) $(headers) Makefile
	g++ $(CFLAGS) -o $@ $(sources)
