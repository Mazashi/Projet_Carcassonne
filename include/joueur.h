#ifndef JOUEUR_H
#define JOUEUR_H

#include "types.h"

/* Initialisation */
void joueur_init(Joueur *j, int id, const char *nom);

/* Partisans */
int joueur_a_partisan(const Joueur *j);
int joueur_retirer_partisan(Joueur *j);
void joueur_rendre_partisan(Joueur *j);

/* Score */
void joueur_ajouter_points(Joueur *j, int points);

/* Affichage */
void joueur_afficher(const Joueur *j);

#endif