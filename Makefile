.PHONY: clean
CC= g++
CFLAGS= -std=c++2a -pedantic -Wfatal-errors -Wconversion -Wredundant-decls -Wshadow -Wall -Wextra -O3

OBJECTS_FILES= heuristic.o bounds.o solver.o bins.o instance.o main.o

main: $(OBJECTS_FILES)
	$(CC) $(CFLAGS) $(OBJECTS_FILES) -lglpk -o main

%.o: %.cpp %.hpp
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	@rm -rf *.o main