CXX = clang++
CXXFLAGS = -Wall -g -std=c++11
TARGET = main.cc
TESTTARGET = utils_test.cc
IDIR = include/
LDIR = lib/
INCLUDE = -I $(IDIR)
LIB = -I $(LDIR)
SOURCE = ./src/

main: main.o
	$(CXX) $(CXXFLAGS) -o $@ main.o

test: utils_test.o
	$(CXX) $(CXXFLAGS) -o $@ utils_test.o

main.o: $(SOURCE)$(TARGET)
	$(CXX) $(CXXFLAGS) -c $(SOURCE)$(TARGET) $(INCLUDE) $(LIB)

utils_test.o: $(IDIR)$(TESTTARGET)
	$(CXX) $(CXXFLAGS) -c $(IDIR)$(TESTTARGET) $(INCLUDE) $(LIB)

clean:
	rm -f *.o test main
