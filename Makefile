.PHONY: all

# Project Name (executable)
PROJECT = dispatch
# Compiler
CC = clang++
COMPILE_OPTIONS = -std=c++11 -Wall -Wextra -Weffc++

HEADERS =
LIBS =

# Subdirs to search for additional source files
SOURCE_FILES := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp) $(wildcard *.h)
# Create an object file of every cpp file
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCE_FILES))


# Make $(PROJECT) the default target
all: $(DEPENDENCIES) $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS)

%.o: %.cpp $(HEADERS)
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $<

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
