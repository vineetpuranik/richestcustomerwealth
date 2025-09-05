CXX = g++
CXXFLAGS = -O2 -std=c++17

all: single multi openmp

single: richestcustomerwealth_single.cpp
	$(CXX) $(CXXFLAGS) richestcustomerwealth_single.cpp -o single

multi: richestcustomerwealth_multi.cpp
	$(CXX) $(CXXFLAGS) richestcustomerwealth_multi.cpp -o multi -pthread

openmp: richestcustomerwealth_openmp.cpp
	$(CXX) $(CXXFLAGS) richestcustomerwealth_openmp.cpp -o openmp -fopenmp

clean:
	rm -f single multi openmp