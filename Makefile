CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

SRC     = src/tuile.c src/plateau.c

test_tuile: $(SRC) tests/test_tuile.c
	$(CC) $(CFLAGS) src/tuile.c tests/test_tuile.c -o tests/test_tuile
	./tests/test_tuile

test_plateau: $(SRC) tests/test_plateau.c
	$(CC) $(CFLAGS) src/plateau.c tests/test_plateau.c -o tests/test_plateau
	./tests/test_plateau

tests: test_tuile test_plateau

valgrind_tuile: 
	valgrind --leak-check=full ./tests/test_tuile

valgrind_plateau:
	valgrind --leak-check=full ./tests/test_plateau