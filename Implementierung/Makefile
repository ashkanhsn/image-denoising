.PHONY: all

all: release

SOURCE = src/main.c src/convolution.c src/combine.c src/grayscale.c src/image.c tests/functional_tests.c src/denoise.c tests/performance_tests.c
PROGRAM_NAME = denoise

ifeq ($(origin CC),default)
CC = gcc
endif

# Warning flagsr
# -Wall:		        Print warnings
# -Wextra:		        Enable additional warnings not covered by "-Wall"
# -Wpedantic:	        Reject everything that is not ISO C
# -g                    Generates debug information to be used by GDB debugger
WFLAGS = -Wall -Wextra -Wpedantic -g
CFLAGS = -lm -std=c17 -msse4.1

# Compile without warnings and with O2 optimisation, for release
release: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O2 $(CFLAGS)

# Compile for debugging
debug: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O0 $(WFLAGS) $(CFLAGS)

# Compile without sanitizers and disable optimisation
asan: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O0 $(WFLAGS) $(CFLAGS) -Werror -fsanitize=address

# Compile with undefined behavior sanitizer enabled
ubsan: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O0 $(WFLAGS) $(CFLAGS) -Werror -fsanitize=undefined

# Compile with leak sanitizer enabled
lsan: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O0 $(WFLAGS) $(CFLAGS) -Werror -fsanitize=leak

# Compile with GCC static analysis enabled
staticAnalysis: 
	$(CC) $(SOURCE) -o $(PROGRAM_NAME) -O0 $(WFLAGS) $(CFLAGS) -fanalyzer

clean: 
	rm $(PROGRAM_NAME)

run-tests: 
	./$(PROGRAM_NAME) -t
