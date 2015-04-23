CXX=g++ -m64
CXXFLAGS=-Iobjs/ -g3 -O3 -Wall -Werror -Wextra

APP_NAME=profile
OBJDIR=objs

default: $(APP_NAME).o

.PHONY: dirs clean 

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *~ $(APP_NAME).o

OBJS=

$(APP_NAME).o: $(APP_NAME).cpp dirs $(OBJS)
		$(CXX) $(CXXFLAGS) $(APP_NAME).cpp -c -o $@ $(OBJS) 

$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@
