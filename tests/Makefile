MKDEP_FLAGS=-I.. -I -DDEBUG_QOS_OPT
CXXFLAGS=$(MKDEP_FLAGS) -Wall -Wformat -std=gnu9x -O3

LDFLAGS=-lglpk

PROGS=test-qos-opt-glpk test-perf-opt test-perc

all: $(PROGS)

test-qos-opt-glpk: test-qos-opt-glpk.o qos_opt.o
	g++ $(LDFLAGS) -o $@ $^

test-perf-opt: test-perf-opt.o qos_opt.o
	g++ $(LDFLAGS) -o $@ $^

test-modules: test-modules.cpp
	g++ -o $@ $^ -I../ -L../Debug/ -larsim-modules -Xlinker -rpath -Xlinker ../Debug

test-perc: test-perc.cpp
	g++ -o $@ $^ -I../ -L../Debug/ -larsim-modules -Xlinker -rpath -Xlinker ../Debug

qos_opt.o: ../qos_opt.c
	g++ -c $(CXXFLAGS) -o $@ $<

clean:
	rm -f *~ *.bak *.o $(PROGS)

dep:
	makedepend $(MKDEP_FLAGS) -- -Y -- *.cpp

# DO NOT DELETE

test-gc.o: ../GlobalOptimizer.hpp ../Component.hpp ../ResourceManager.hpp
test-gc.o: ../TaskScheduler.hpp ../Task.hpp ../LinearModel.hpp ../util.hpp
test-gc.o: ../TimeStat.hpp ../StatBase.hpp ../Events.hpp ../Controller.hpp
test-gc.o: ../Stat.hpp ../Interval.hpp ../TaskPredictor.hpp
test-gc.o: ../ValuePredictor.hpp ../RangePredictor.hpp ../RPStatBased.hpp
test-gc.o: ../Supervisor.hpp ../qos_opt.h
test-perf-opt.o: ../qos_opt.h
test-qos-opt-glpk.o: ../qos_opt.h
