CXX = g++
CXXFLAGS = -Wall -g -std=c++11
TARGET = main.cc
TESTTARGET = utils_test.cc
IDIR = include/
LDIR = lib/
INCLUDE = -I $(IDIR)
LIB = -I $(LDIR)
SOURCE = ./src/

main:
	$(CXX) $(CXXFLAGS) -o main $(SOURCE)$(TARGET) $(INCLUDE) $(LIB)

test: 
	$(CXX) $(CXXFLAGS) -o test $(IDIR)$(TESTTARGET) $(INCLUDE) $(LIB)

clean:
	rm -f *.o test main
