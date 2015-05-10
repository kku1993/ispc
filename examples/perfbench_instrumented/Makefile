
CXX=clang++ -m64
CXXFLAGS=-Iobjs/ -g3 -Wall
ISPC=ispc
ISPCFLAGS=-O2 --instrument --arch=x86-64 --target=sse2

default: perfbench

.PHONY: dirs clean

dirs:
	/bin/mkdir -p objs/

clean:
	/bin/rm -rf objs *~ perfbench

perfbench: objs/perfbench.o objs/instrument.o objs/perfbench_instrumented_ispc.o objs/perfbench_serial.o ../tasksys.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm -lpthread

objs/%.o: %.cpp dirs
	$(CXX) $< $(CXXFLAGS) -c -o $@

objs/perfbench.o: objs/perfbench_instrumented_ispc.h

objs/%_ispc.h objs/%_ispc.o: %.ispc dirs
	$(ISPC) $(ISPCFLAGS) $< -o objs/$*_ispc.o -h objs/$*_ispc.h
