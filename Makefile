.PHONY: all

# Project Name (executable)
PROJECT = dispatch
# Compiler
CC = ~/clang-33/bin/clang++
#CC = clang++
BUILD_FLAGS = -O3
COMPILE_OPTIONS = -std=c++11 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-missing-prototypes -Wno-unused -D USE_PAPI_TRACE -march=native $(BUILD_FLAGS)

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
	$(CC) -o $(PROJECT) $(OBJECTS) $(BUILD_FLAGS) $(LIBS)

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
