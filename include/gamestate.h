#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "types.h"

/* Cycle de vie */
GameState *gamestate_creer(int nbJoueurs, const char noms[][32]);
void gamestate_detruire(GameState *gs);

/* Pioche */
void gamestate_melanger_pioche(GameState *gs);
Tuile *gamestate_piocher(GameState *gs);
int gamestate_pioche_vide(const GameState *gs);

/* Tour */
Joueur *gamestate_joueur_courant(GameState *gs);
void gamestate_tour_suivant(GameState *gs);

/* Affichage debug */
void gamestate_afficher_etat(const GameState *gs);

#endif