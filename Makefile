.PHONY: all

# Project Name (executable)
PROJECT = dispatch
# Compiler
#CC = ~/clang-33/bin/clang++
CC = g++-4.8
BUILD_FLAGS = -O4 -DNDEBUG 
COMPILE_OPTIONS = -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-padded -march=native $(BUILD_FLAGS) -fomit-frame-pointer

HEADERS =
LIBS = -lpapi -L/usr/local/lib

# Subdirs to search for additional source files
SOURCE_FILES := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp) $(wildcard *.h) Makefile
# Create an object file of every cpp file
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCE_FILES))

# Make $(PROJECT) the default target
all: $(DEPENDENCIES) $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(BUILD_FLAGS) $(LIBS) -flto

dispatch_test.o: dispatch_test.cpp $(HEADERS)
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $<

%.o: %.cpp $(HEADERS)
	$(CC) -c $(COMPILE_OPTIONS) -flto -o $@ $<

# Build & Run Project
run: $(PROJECT)
	./$(PROJECT) $(COMMANDLINE_OPTIONS)

# Clean & Debug
.PHONY: makefile-debug
makefile-debug:

.PHONY: clean
clean:
	rm -f $(PROJECT) $(OBJECTS)

clean-all: clean depclean
