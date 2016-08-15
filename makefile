
CXX_FLAGS = -std=c++11 -I.

BENCH_SRC = bench.cpp
BENCH_HEADERS = connectivitymatrix.h symmetricmatchmatrix.h


release/bench : $(BENCH_SRC) $(BENCH_HEADERS)
	mkdir -p $(dir $@)
	g++ $(CXX_FLAGS) -O3 $(BENCH_SRC) -o $@

debug/bench : $(BENCH_SRC) $(BENCH_HEADERS)
	mkdir -p $(dir $@)
	g++ $(CXX_FLAGS) -g $(BENCH_SRC) -o $@

profile/bench: $(BENCH_SRC) $(BENCH_HEADERS)
	mkdir -p $(dir $@)
	g++ $(CXX_FLAGS) -pg $(BENCH_SRC) -o $@

release : release/bench
debug : debug/bench
profile : profile/bench

all : release debug profile

clean :
	rm -rf release debug bench

.PHONY: all release debug profile clean
