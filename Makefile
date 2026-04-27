CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

SRC     = src/tuile.c src/plateau.c src/joueur.c src/gamestate.c

test_tuile: $(SRC) tests/test_tuile.c
	$(CC) $(CFLAGS) src/tuile.c tests/test_tuile.c -o tests/test_tuile
	./tests/test_tuile

test_plateau: $(SRC) tests/test_plateau.c
	$(CC) $(CFLAGS) src/plateau.c tests/test_plateau.c -o tests/test_plateau
	./tests/test_plateau

test_joueur: src/joueur.c tests/test_joueur.c
	$(CC) $(CFLAGS) src/joueur.c tests/test_joueur.c -o tests/test_joueur
	./tests/test_joueur

test_gamestate: $(SRC) tests/test_gamestate.c
	$(CC) $(CFLAGS) src/tuile.c src/joueur.c src/plateau.c src/gamestate.c \
	      tests/test_gamestate.c -o tests/test_gamestate
	./tests/test_gamestate


tests: test_tuile test_plateau test_joueur test_gamestate

valgrind_gamestate:
	valgrind --leak-check=full ./tests/test_gamestate
valgrind_tuile: 
	valgrind --leak-check=full ./tests/test_tuile

valgrind_plateau:
	valgrind --leak-check=full ./tests/test_plateau

valgrind_joueur:
	valgrind --leak-check=full ./tests/test_joueur