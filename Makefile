.PHONY: all

# Project Name (executable)
PROJECT = dispatch_test
# Compiler
#CC = ~/clang-33/bin/clang++
CC = g++-4.8 -O4 -ggdb
#CC = ~/polly/llvm_build/bin/clang++ -Xclang -load -Xclang ~/polly/llvm_build/lib/LLVMPolly.so -O3 -mllvm -polly -mllvm -polly-vectorizer=polly
#CC = ~/polly/llvm_build/bin/clang++ -O3 -g3
BUILD_FLAGS = -I thirdparty/catch/single_include -I .
COMPILE_OPTIONS = -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-padded -march=native $(BUILD_FLAGS) #-D USE_PAPI_TRACE

HEADERS =
LIBS = -lpapi -L/usr/local/lib

# Subdirs to search for additional source files
SOURCE_FILES := $(shell find . -name "*.cpp" -and -not -path "*/thirdparty/*")
HEADERS := Makefile $(shell find . \( -name "*.h" -or -name "*.hpp" \) -and -not -path "*/thirdparty/*")
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
