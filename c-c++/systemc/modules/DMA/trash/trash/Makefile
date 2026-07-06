# Makefile for DMA Testbench
# SystemC configuration
SYSTEMC_HOME ?= /usr/local/systemc-2.3.3
SYSTEMC_INCLUDE = $(SYSTEMC_HOME)/include
SYSTEMC_LIB = $(SYSTEMC_HOME)/lib-linux64

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I$(SYSTEMC_INCLUDE) -I. -I./include
LDFLAGS = -L$(SYSTEMC_LIB) -lsystemc -lm

# Target executable
TARGET = dma_testbench

# Source files
SOURCES = DMA_testbench.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Compile the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run the testbench
run: $(TARGET)
	./$(TARGET)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.csv *.vcd

# Clean everything including generated files
distclean: clean
	rm -f source_memory.csv dest_memory.csv
	rm -f source_dump.csv dest_dump.csv

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the DMA testbench"
	@echo "  run       - Build and run the testbench"
	@echo "  clean     - Remove object files and executable"
	@echo "  distclean - Remove all generated files"
	@echo "  help      - Show this help message"

.PHONY: all run clean distclean help 