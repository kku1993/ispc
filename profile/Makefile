CXX=g++ -m64
CXXFLAGS=-Iobjs/ -g3 -O3 -Wall -Werror -Wextra

APP_NAME=profile
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

OBJS=
PCMOBJS=$(PCMDIR)/cpucounters.o $(PCMDIR)/client_bw.o $(PCMDIR)/pci.o $(PCMDIR)/msr.o

$(LIB_NAME).o: $(APP_NAME).o $(OBJS) $(PCMOBJS)
		ld -r $(OBJDIR)/$(APP_NAME).o $(PCMOBJS) -o $@

$(APP_NAME).o: $(APP_NAME).cpp dirs $(OBJS) 
		$(CXX) $(CXXFLAGS) $(APP_NAME).cpp -c -o $(OBJDIR)/$@ $(OBJS)

$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(PCMDIR)/%.o: $(PCMDIR)/%.cpp
		cd $(PCMDIR); make
