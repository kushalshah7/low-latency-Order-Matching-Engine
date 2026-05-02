# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

# Source files
SOURCES = OrderBook.cpp MatchingEngine.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Executables
DEMO = demo
TEST = test_engine

# Default target
all: $(DEMO) $(TEST)

# Build demo
$(DEMO): main.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build test
$(TEST): test_matching_engine.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) main.o test_matching_engine.o $(DEMO) $(TEST)

# Run demo
run-demo: $(DEMO)
	./$(DEMO)

# Run tests
run-tests: $(TEST)
	./$(TEST)

.PHONY: all clean run-demo run-tests
