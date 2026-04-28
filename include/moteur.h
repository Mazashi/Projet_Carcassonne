#ifndef MOTEUR_H
#define MOTEUR_H

#include "types.h"
#include "gamestate.h"

/* Résultat d'une tentative d'action */
typedef enum
{
    MOTEUR_OK = 0,
    MOTEUR_ERR_POSE = -1,  /* Position invalide pour la tuile  */
    MOTEUR_ERR_PION = -2,  /* Partisan non plaçable ici        */
    MOTEUR_ERR_STOCK = -3, /* Plus de partisans disponibles    */
    MOTEUR_ERR_NULL = -4   /* Pointeur NULL reçu               */
} MoteurCode;

/* Liste de positions valides (tableau dynamique) */
typedef struct
{
    int *xs;   /* Tableau des coordonnées x */
    int *ys;   /* Tableau des coordonnées y */
    int count; /* Nombre de positions       */
} ListePositions;

/* ─── Cycle de vie ───────────────────────────────────── */
MoteurCode moteur_init(GameState *gs);
void moteur_jouer(GameState *gs);

/* ─── Actions d'un tour ──────────────────────────────── */
MoteurCode moteur_placer_tuile(GameState *gs, Tuile *t, int x, int y);
MoteurCode moteur_placer_partisan(GameState *gs, int x, int y, Position empl);

/* ─── Aide à la décision ─────────────────────────────── */
ListePositions moteur_positions_valides(GameState *gs, Tuile *t);
void moteur_positions_liberer(ListePositions *lp);
int moteur_a_position_valide(GameState *gs, Tuile *t);

/* ─── Tour complet ───────────────────────────────────── */
void moteur_tour(GameState *gs);

#endif