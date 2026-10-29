CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -Iinclude

.PHONY: all test clean

all: bin/clasica

bin/clasica: src/main.cpp src/aritmetica_decimal.cpp include/aritmetica_decimal.hpp
	@if not exist bin mkdir bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp src/aritmetica_decimal.cpp -o $@

bin/tests: tests/test_aritmetica_decimal.cpp src/aritmetica_decimal.cpp include/aritmetica_decimal.hpp
	@if not exist bin mkdir bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/test_aritmetica_decimal.cpp src/aritmetica_decimal.cpp -o $@

test: bin/tests
	./bin/tests

clean:
	@if exist bin rmdir /s /q bin
