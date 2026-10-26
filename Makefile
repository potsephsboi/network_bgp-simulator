CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra -Wpedantic

.PHONY: all run clean

all: setup router

setup: setup.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

router: router.cpp communication.cpp router.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) router.cpp communication.cpp $(LDLIBS) -o $@

run: all
	./setup

clean:
	$(RM) setup router
