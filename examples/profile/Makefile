CXX=g++ -m64
CXXFLAGS=-Iobjs/ -g3 -O3 -Wall -Werror -Wextra 
ISPC=/home/kku/ispc/ispc
# note: change target to avx-x2 for AVX capable machines
ISPCFLAGS=-O2 --profile --target=sse4-x2 --arch=x86-64 

APP_NAME=test
OBJDIR=objs

#TASKSYS_OBJ=$(OBJDIR)/tasksys.o
#TASKSYS_LIB=-lpthread

default: $(APP_NAME)

.PHONY: dirs clean src

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *~ $(APP_NAME) profile_*

OBJS=$(OBJDIR)/test.o $(OBJDIR)/foo_ispc.o $(OBJDIR)/instrument.o #$(TASKSYS_OBJ)

$(APP_NAME): dirs $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm -lpthread

$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/$(APP_NAME).o: $(OBJDIR)/foo_ispc.h 

$(OBJDIR)/%_ispc.h $(OBJDIR)/%_ispc.o: %.ispc
		$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h
