CXX=g++ -m64
CXXFLAGS=-Iobjs/ -Irapidjson/include/ -g3 -O3 -Wall -Werror -Wextra -std=c++11

LIB_NAME=libprofile
OBJDIR=objs
PCMDIR=intel_pcm

default: $(LIB_NAME).o

.PHONY: dirs clean veryclean

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *~ $(LIB_NAME).o

veryclean:
		/bin/rm -rf $(OBJDIR) *~ $(LIB_NAME).o; cd $(PCMDIR); make clean

OBJS=$(OBJDIR)/profile_ctx.o $(OBJDIR)/profile.o
PCMOBJS=$(PCMDIR)/cpucounters.o $(PCMDIR)/client_bw.o $(PCMDIR)/pci.o $(PCMDIR)/msr.o

$(LIB_NAME).o: $(OBJS) $(PCMOBJS) dirs
		ld -r $(OBJS) $(PCMOBJS) -o $@

$(OBJDIR)/%.o: %.cpp dirs
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(PCMDIR)/%.o: $(PCMDIR)/%.cpp 
		cd $(PCMDIR); make
