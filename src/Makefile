CC = g++
INCLDIR=./include
IMPLDIR=./impl
CPPDIR=./cpp
CPPFLAG=-std=c++11 -O3 #-g -Wall
INCLFLAG=-I. -I$(INCLDIR) -I$(IMPLDIR)
LIBFLAG=-L.
LIBS= #-lboost_regex #-lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system

headers = $(wildcard (INCLDIR)/*.h)
srcs = $(wildcard $(CPPDIR)/*.cpp)
objs = $(srcs:.cpp=.o)
deps = $(srcs:.cpp=.d)

default: testgen #testsymmodel #testfparser

testfparser:
	$(CC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/testfparser.cpp -o testfparser.exe $(LIBS)

testsymmodel:  $(objs) $(headers) Makefile
	$(CC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) unit_tests/testsymmodel.cpp -o testsymmodel.exe $(LIBS)

testgen:  $(objs) $(headers) Makefile
	$(CC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) $(objs) unit_tests/testgen.cpp -o testgen.exe $(LIBS)


%.o: %.cpp
	$(CC) -MMD -MP -c $(CPPFLAG) $(LIBFLAG) $(INCLFLAG) $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) $(objs) $(deps) *.exe

-include $(deps)
