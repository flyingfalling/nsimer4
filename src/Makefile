CC = g++
INCLDIR=./include
IMPLDIR=./impl
CPPDIR=./cpp
CPPFLAG=-std=c++11 -O3 #-g -Wall
INCLFLAG=-I. -I$(INCLDIR) -I$(IMPLDIR)
LIBFLAG=-L.
LIBS= #-lboost_mpi -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system


default: testfparser

testfparser:
	$(CC) $(CPPFLAG) $(INCLFLAG) $(LIBFLAG) unit_tests/testfparser.cpp -o testfparser.exe $(LIBS)

clean:
	rm *.exe *.o
