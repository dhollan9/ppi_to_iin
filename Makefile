


CFLAGS = $(shell gsl-config --cflags)
CFLAGS += -O3 -I include/
LDFLAGS = 


FLIBSLIN    =  -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2 -L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../../../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.1.1 -L/usr/lib -L/usr/local/lib -lgsl -lgfortran -lm -lblas -lstdc++
FLIBSMAC    =  -L/usr/local/gfortran/lib/gcc/x86_64-apple-darwin12/4.8.2 -L/usr/local/gfortran/lib/gcc/x86_64-apple-darwin12/4.8.2/../../.. -L/usr/local/lib -L/usr/lib -lgsl -lgfortran -lquadmath -lm -lstdc++ -framework accelerate

LIBS   = $(shell gsl-config --libs)

CC     = g++
BDIR   = bin
ODIR   = obj
SDIR   = subroutines
EDIR   = EXES
OS    := $(shell uname)

ifeq ($(OS),Linux)
	LIBS  +=$(FLIBSLIN)
	_OBJS = $(shell ls $(SDIR)/*.cpp | xargs -n 1 basename | sed -r 's/(\.cc|.cpp)/.o/')
else
	LIBS  +=$(FLIBSMAC)
	_OBJS = $(shell ls $(SDIR)/*.cpp | xargs -n 1 basename | sed -E 's/(\.cc|.cpp)/.o/')
endif

LIBS2 = $(LIBS)

_EXECUTABLES = mc_iin_opt.exe \


##look to find all subroutines
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))


EXECUTABLES = $(patsubst %,$(BDIR)/%,$(_EXECUTABLES))



_SOURCES = ${_EXECUTABLES:=.cpp}
SOURCES = $(patsubst %,$(EDIR)/%,$(_SOURCES))

all: dirs $(EXECUTABLES)

$(ODIR)/%.o: $(SDIR)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@  
	@echo "------------"

$(CDIRO)/%.o: $(CDIR)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@  
	@echo "------------"


$(EXECUTABLES): $(OBJS) 
	@echo "Compiling $(EDIR)/$(@F:.exe=.cpp)"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(EDIR)/$(@F:.exe=.cpp) $(OBJS) $(LIBS2)
	@echo "------------"

dirs: 
	mkdir -p bin
	mkdir -p obj

clean:
	rm -f $(ODIR)/*.o $(ODIR2)/*.o $(EXECUTABLES)


