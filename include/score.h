#ifndef SCORE_H
#define SCORE_H

#include "types.h"
#include "plateau.h"

/* ─── Scoring en cours de partie ─────────────────────── */
int score_ville(Plateau *p, int x, int y, int fin_partie);
int score_route(Plateau *p, int x, int y, int fin_partie);
int score_abbaye(Plateau *p, int x, int y, int fin_partie);

/* ─── Scoring global fin de partie ───────────────────── */
void score_fin_partie(GameState *gs);

#endif