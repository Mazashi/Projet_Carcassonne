CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

SRC = src/tuile.c src/plateau.c src/joueur.c \
      src/gamestate.c src/score.c src/moteur.c

# ─── Cibles de test ───────────────────────────────────
test_tuile: src/tuile.c tests/test_tuile.c
	$(CC) $(CFLAGS) src/tuile.c tests/test_tuile.c \
	      -o tests/test_tuile && ./tests/test_tuile

test_plateau: src/plateau.c tests/test_plateau.c
	$(CC) $(CFLAGS) src/plateau.c tests/test_plateau.c \
	      -o tests/test_plateau && ./tests/test_plateau

test_joueur: src/joueur.c tests/test_joueur.c
	$(CC) $(CFLAGS) src/joueur.c tests/test_joueur.c \
	      -o tests/test_joueur && ./tests/test_joueur

test_gamestate: $(SRC) tests/test_gamestate.c
	$(CC) $(CFLAGS) $(SRC) tests/test_gamestate.c \
	      -o tests/test_gamestate && ./tests/test_gamestate

test_score: $(SRC) tests/test_score.c
	$(CC) $(CFLAGS) $(SRC) tests/test_score.c \
	      -o tests/test_score && ./tests/test_score

test_moteur: $(SRC) tests/test_moteur.c
	$(CC) $(CFLAGS) $(SRC) tests/test_moteur.c \
	      -o tests/test_moteur && ./tests/test_moteur

tests: test_tuile test_plateau test_joueur \
       test_gamestate test_score test_moteur

# ─── Cible de jeu ─────────────────────────────────────
play: $(SRC) src/main.c
	$(CC) $(CFLAGS) $(SRC) src/main.c -o carcassonne
	./carcassonne

# ─── Valgrind ─────────────────────────────────────────
valgrind_%:
	valgrind --leak-check=full --error-exitcode=1 \
	         ./tests/test_$*

# ─── Nettoyage ────────────────────────────────────────
clean:
	rm -f tests/test_tuile tests/test_plateau \
	      tests/test_joueur tests/test_gamestate \
	      tests/test_score tests/test_moteur \
	      carcassonne