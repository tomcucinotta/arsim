# Type 'make show-xxx' to invoke show-xxx.gplot with white background
#   xxx = eu for a e(k) vs. c(k)u(k)-T plot
#   xxx = ek for a k vs e(k) plot

#GPROF_FLAGS=-pg
GPROF_FLAGS=

CXX_MODS = $(wildcard *.cpp)
C_MODS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(C_MODS)) $(patsubst %.cpp, %.o, $(CXX_MODS))
LIBS = -lm $(GPROF_FLAGS) -lz

PROG = arsim
PROG_DBG = arsim-dbg

MODULES_LIB = libarsim-modules.so
MODULES_LIB_DBG = libarsim-modules-dbg.so

MODULES_SRCS := \
	$(wildcard *Predictor*.cpp) $(wildcard TP*.cpp) $(wildcard RP*.cpp) \
	$(wildcard *Controller*.cpp) \
	BaseStat.cpp Stat.cpp TimeStat.cpp LinearModel.cpp Task.cpp \
	util.cpp FileUtil.cpp

MODULES_SRCS := $(filter-out DoubleInvariantController.cpp, $(MODULES_SRCS))
MODULES_SRCS := $(filter-out LimitedControllerK.cpp, $(MODULES_SRCS))

include variables.mk

PACKAGE=arsim
VERSION:=$(shell cat VERSION)
CURR_DIR=$(shell pwd)

mkoctfile=$(octave_path)/bin/mkoctfile
OCT_INCL := $(shell $(mkoctfile) -p INCFLAGS)
OCT_LIBS := $(shell $(mkoctfile) -p LFLAGS)	\
	$(shell $(mkoctfile) -p LIBS)		\
	$(shell $(mkoctfile) -p OCTAVE_LIBS)	\
	$(shell $(mkoctfile) -p BLAS_LIBS)	\
	$(shell $(mkoctfile) -p FFTW_LIBS)	\
	$(shell $(mkoctfile) -p FLIBS)		\
	$(shell $(mkoctfile) -p RLD_FLAG)

GLPK_INCL := -I$(glpk_path)/include
GLPK_LIBS := -L$(glpk_path)/lib -lglpk
# $(glpk_path)/lib/libglpk.so

LIB_CXXFLAGS_DEBUG   = -Wall -Wno-long-long -g $(OCT_INCL) $(GLPK_INCL) -DDEBUG_LOGGER -DDEBUG_QOS_OPT
LIB_CXXFLAGS_RELEASE = -Wall -Wno-long-long -O3 $(OCT_INCL) $(GLPK_INCL)

CXXFLAGS_DEBUG   = $(LIB_CXXFLAGS_DEBUG) -DWITH_DOUBLE_LIMITED
CXXFLAGS_RELEASE = $(LIB_CXXFLAGS_RELEASE) -DWITH_DOUBLE_LIMITED

#CXXFLAGS = -Wall -pedantic -Wno-long-long -O3 $(OCT_INCL)
#CXXFLAGS = -Wall -pedantic -Wno-long-long -O3 $(GPROF_FLAGS) $(OCT_INCL)
#CXXFLAGS = -Wall -pedantic -Wno-long-long -O3 -fprofile-arcs $(OCT_INCL)
#CXXFLAGS = -Wall -pedantic -Wno-long-long -O3 -fbranch-probabilities $(OCT_INCL)

CFLAGS_DEBUG     = $(CXXFLAGS_DEBUG) -std=gnu9x
CFLAGS_RELEASE   = $(CXXFLAGS_RELEASE) -std=gnu9x

LIBS += $(OCT_LIBS) $(GLPK_LIBS) -lz

all: all-debug all-release

include Release/Makefile.deps
include Debug/Makefile.deps

install-mkdir:
	mkdir -p $(bindir)
	mkdir -p $(libdir)
	mkdir -p $(includedir)/arsim

install-includes: install-mkdir
	cp -a $(wildcard *.hpp) $(includedir)/arsim

install-release: install-mkdir install-includes Release/$(PROG)
	cp Release/$(PROG) $(bindir)
	cp Release/$(MODULES_LIB) $(libdir)

install-debug: install-mkdir install-includes Debug/$(PROG)
	cp Debug/$(PROG) $(bindir)/$(PROG_DBG)
	cp Debug/$(MODULES_LIB) $(libdir)/$(MODULES_LIB_DBG)

install: install-release install-debug

ALL_PROGS = $(PROG) # test-gc test-template-base
ALL_LIBS = $(MODULES_LIB)

.PHONY: all-debug
all-debug: $(ALL_PROGS:%=Debug/%) $(ALL_LIBS:%=Debug/%)

.PHONY: all-release
all-release: $(ALL_PROGS:%=Release/%) $(ALL_LIBS:%=Release/%)

Debug/$(PROG): $(patsubst %,Debug/%,$(OBJS))
	$(CXX) $(CXXFLAGS_DEBUG) -o $@ $^ $(LIBS)

Release/$(PROG): $(patsubst %,Release/%,$(OBJS))
	$(CXX) $(CXXFLAGS_RELEASE) -o $@ $^ $(LIBS)

Debug/$(MODULES_LIB): $(MODULES_SRCS)
	$(CXX) $(LIB_CXXFLAGS_DEBUG) -shared -fpic -fPIC -o $@ $^ $(LIBS)

Release/$(MODULES_LIB): $(MODULES_SRCS)
	$(CXX) $(LIB_CXXFLAGS_RELEASE) -shared -fpic -fPIC -o $@ $^ $(LIBS)

Debug/%.o : %.cpp Debug/Makefile.deps
	mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS_DEBUG) -I`pwd` -c $< -o $@

Release/%.o : %.cpp Release/Makefile.deps
	mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS_RELEASE) -I`pwd` -c $< -o $@

Debug/%.o : %.c Debug/Makefile.deps
	$(CC) $(CFLAGS_DEBUG) -I`pwd` -c $< -o $@

Release/%.o : %.c Release/Makefile.deps
	$(CC) $(CFLAGS_RELEASE) -I`pwd` -c $< -o $@

run: Release/$(PROG)
	./Release/$(PROG) > output.dat

%.fig : %.dat
	./show_plot

show:
	@echo "Show targets are: $(patsubst %.gplot,%, $(wildcard show-*.gplot))"

stat_eps.eps: stat_eps.dat draw-stat.gplot
	gnuplot draw-stat.gplot

stat-sdb-%.eps : stat-sdb-%.dat
	cp $< stat_eps.dat
	gnuplot draw-stat.gplot
	mv stat_eps.eps $@

stat-sdb-%.dat: $(PROG) run-trace-sdb.sh Makefile
	./run-trace-sdb.sh -tp mumm -mmm-sp $(@:stat-sdb-%.dat=%) -mm-ss 8
	mv stat.dat $@

cmp-stats.eps: draw-cmp-stats.gplot stat-sdb-01.dat stat-sdb-03.dat stat-sdb-12.dat
	gnuplot draw-cmp-stats.gplot

stat_eps_sdb.dat: $(PROG) Makefile
	./run-trace-sdb.sh -tf ../traces/flod4 -N 164000 -u 1.0 -tp mumm
	mv stat_eps.dat stat_eps_sdb.dat

stat_eps_sdb_dyn.dat: $(PROG) Makefile
	./run-trace-sdb.sh -tf ../traces/flod4 -N 164000 -u 1.0 -tp mumm -dyn-bw
	mv stat_eps.dat stat_eps_sdb_dyn.dat

cmp_stat_eps_dyn.eps: stat_eps_sdb.dat stat_eps_sdb_dyn.dat draw-cmp-stats-dyn.gplot
	./draw-cmp-stats-dyn.gplot
	gv -scale 3 $@

show-%: show-%.gplot
	@/usr/bin/gnuplot -background white $@.gplot

draw-%: draw-%.gplot
	@/usr/bin/gnuplot -background white $@.gplot
	gv -scale 3 output.eps

doc: doc/html/index.html

doc/html/index.html: $(wildcard *.[ch]pp) Doxyfile
	doxygen

clean:
	rm -f *~ *.o a.out *.eps output.dat *.bak $(PROG) $(MODULES_LIB) test/*~ test/*.o .#*
	cd Release && rm -rf *
	cd Debug && rm -rf *
	rm -rf doc/html

distclean: clean clean-dat
	rm -rf .settings .project .cproject
	rm -rf autom4te.cache config.log config.status config.h variables.mk
	rm -rf `find . -name CVS -o -name .cvsignore`

clean-dat:
	rm -f *.dat info.txt
	rm -rf data-*

dist:
	cd .. && rm -f "/tmp/$(PACKAGE)-$(VERSION).tar.gz" && rm -rf "/tmp/$(PACKAGE)-$(VERSION)" && cp -aR "$(CURR_DIR)" "/tmp/$(PACKAGE)-$(VERSION)" && cd "/tmp/$(PACKAGE)-$(VERSION)" && make distclean && cd .. && tar -czf "$(PACKAGE)-$(VERSION).tar.gz" "$(PACKAGE)-$(VERSION)" && rm -rf "$(PACKAGE)-$(VERSION)" && echo "" && echo "Distribution package is /tmp/$(PACKAGE)-$(VERSION).tar.gz" && echo ""

dep:
	rm -f  Release/Makefile.deps Debug/Makefile.deps
	make Release/Makefile.deps Debug/Makefile.deps

Release/Makefile.deps:
	mkdir -p $(shell dirname $@)
	touch $@ && makedepend -p Release/ -f$@ -Y -- -- *.cpp tests/*.cpp

Debug/Makefile.deps:
	mkdir -p $(shell dirname $@)
	touch $@ && makedepend -p Debug/ -f$@ -Y -- -- *.cpp tests/*.cpp

Release/test-gc: Release/tests/test-gc.o Release/GlobalOptimizer.o Release/ResourceManager.o Release/qos_opt.o Release/util.o Release/Events.o
	mkdir -p $(shell dirname $@)
	$(CXX) -o $@ $^ $(LIBS)

Debug/test-gc: Debug/tests/test-gc.o Debug/GlobalOptimizer.o Debug/ResourceManager.o Debug/qos_opt.o Debug/util.o Debug/Events.o
	mkdir -p $(shell dirname $@)
	$(CXX) -o $@ $^ $(LIBS)

Release/test-%: Release/tests/test-%.o
	mkdir -p $(shell dirname $@)
	$(CXX) -o $@ $^ $(LIBS)

Debug/test-%: Debug/tests/test-%.o
	mkdir -p $(shell dirname $@)
	$(CXX) -o $@ $^ $(LIBS)
