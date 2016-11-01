
#OOQP=/home-1/dhollan9@jhu.edu/work/DavidCode/net_all_code/OOQP-0.99.26
##Set OOQP in your ~/.bashrc and your ~/.bash_profile file using: export OOQP="/Users/my_name/OOQP-0.99.22" where you list the full path to its directory in "".

#OOQPINCLUDEDIR=$(OOQP)/include
#OOQPLIBDIR=$(OOQP)/lib


CFLAGS = $(shell gsl-config --cflags)
CFLAGS += -O3 -I include/
#CFLAGS += -I$(OOQPINCLUDEDIR)
#LDFLAGS  =-L$(OOQPLIBDIR)
LDFLAGS = 

##MA27LIB  = -L$(OOQP)/ma27-1.0.0/src
##BLAS	= /usr/lib64/atlas/libf77blas.so.3
##GSL      = /usr/lib64/libgsl.so /usr/lib64/libgslcblas.so /usr/lib64/libgsl.so.0
##FLIBSLIN    =  -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2 -L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../../../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.1.1 -lgfortran -lblas -lm -lg2c -lstdc++
FLIBSLIN    =  -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2 -L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/ -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../../../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.1.1 -L/usr/lib -L/usr/local/lib -lgsl -lgfortran -lm -lg2c -lstdc++
FLIBSMAC    =  -L/usr/local/gfortran/lib/gcc/x86_64-apple-darwin12/4.8.2 -L/usr/local/gfortran/lib/gcc/x86_64-apple-darwin12/4.8.2/../../.. -L/usr/local/lib -L/usr/lib -lgsl -lgfortran -lquadmath -lm -lstdc++ -framework accelerate

LIBS   = $(shell gsl-config --libs)
LIBS  += $(MA27LIB) $(BLAS)

CC     = g++
BDIR   = bin
ODIR   = obj
SDIR2  = shared_subs/
CDIR   = #../code_assignabund_ppi/subroutines
CDIRO  = #../code_assignabund_ppi/obj
QDIR   = #../shared_ooqp
SDIR   = subroutines
EDIR   = EXES
OS    := $(shell uname)

ifeq ($(OS),Linux)
	LIBS  +=$(FLIBSLIN)
	_OBJS = $(shell ls $(SDIR)/*.cpp | xargs -n 1 basename | sed -r 's/(\.cc|.cpp)/.o/')
	_OBJS2 = $(shell ls $(SDIR2)/*.cpp | xargs -n 1 basename | sed -r 's/(\.cc|.cpp)/.o/')
	_OBJS3 = $(shell ls $(CDIR)/*.cpp | xargs -n 1 basename | sed -r 's/(\.cc|.cpp)/.o/')
else
	LIBS  +=$(FLIBSMAC)
	_OBJS = $(shell ls $(SDIR)/*.cpp | xargs -n 1 basename | sed -E 's/(\.cc|.cpp)/.o/')
	_OBJS2 = $(shell ls $(SDIR2)/*.cpp | xargs -n 1 basename | sed -E 's/(\.cc|.cpp)/.o/')
	#_OBJS3 = $(shell ls $(CDIR)/*.cpp | xargs -n 1 basename | sed -E 's/(\.cc|.cpp)/.o/')
endif

#LIBS2  = -looqpgensparse -looqpsparse  -looqpgondzio -looqpbase -lma27 $(LIBS) 
LIBS2 = $(LIBS)

_EXECUTABLES = mc_iin_opt.exe \


##look to find all subroutines
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

OBJS2 = $(patsubst %,$(SDIR2)/%,$(_OBJS2))

#OBJS3 = $(patsubst %,$(CDIRO)/%,$(_OBJS3))

##specify subroutines
QOBJS  = #$(QDIR)/qp_solvers.o



EXECUTABLES = $(patsubst %,$(BDIR)/%,$(_EXECUTABLES))



_SOURCES = ${_EXECUTABLES:=.cpp}
SOURCES = $(patsubst %,$(EDIR)/%,$(_SOURCES))

all: dirs $(EXECUTABLES)

$(ODIR)/%.o: $(SDIR)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@  
	@echo "------------"

$(SDIR2)/%.o: $(SDIR2)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@  
	@echo "------------"

$(CDIRO)/%.o: $(CDIR)/%.cpp
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(CFLAGS2) -c $< -o $@  
	@echo "------------"

#$(QDIR)/qp_solvers.o: $(QDIR)/qp_solvers.cpp 
#		$(CC) $(CFLAGS) $(LDFLAGS) -c $(QDIR)/qp_solvers.cpp -o $(QDIR)/qp_solvers.o 


$(EXECUTABLES): $(OBJS) $(OBJS2) $(QOBJS) #$(OBJS3)
	@echo "Compiling $(EDIR)/$(@F:.exe=.cpp)"
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(EDIR)/$(@F:.exe=.cpp) $(OBJS) $(OBJS2) $(OBJS3) $(QOBJS) $(LIBS2)
	@echo "------------"

dirs: 
	mkdir -p bin
	mkdir -p obj
	#mkdir -p $(CDIRO)

clean:
	rm -f $(ODIR)/*.o $(ODIR2)/*.o $(EXECUTABLES)


