#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/types.h"
#include "../include/plateau.h"
#include "../include/joueur.h"
#include "../include/gamestate.h"

/* Prototype interne */
int charger_tuiles(Tuile pioche[72]);

/* ─────────────────────────────────────────────────────────
 * gamestate_creer
 * Alloue et initialise une partie complète :
 *   - Plateau vide
 *   - Joueurs initialisés
 *   - Pioche chargée depuis le fichier puis mélangée
 * ───────────────────────────────────────────────────────── */
GameState *gamestate_creer(int nbJoueurs, const char noms[][32])
{
    if (nbJoueurs < 2 || nbJoueurs > MAX_JOUEURS || noms == NULL)
    {
        fprintf(stderr, "gamestate_creer : paramètres invalides\n");
        return NULL;
    }

    GameState *gs = malloc(sizeof(GameState));
    if (gs == NULL)
    {
        perror("Erreur malloc GameState");
        return NULL;
    }

    /* 1. Plateau */
    gs->plateau = plateau_creer();
    if (gs->plateau == NULL)
    {
        free(gs);
        return NULL;
    }

    /* 2. Joueurs */
    gs->nbJoueurs = nbJoueurs;
    for (int i = 0; i < nbJoueurs; i++)
        joueur_init(&gs->joueurs[i], i + 1, noms[i]);

    /* 3. Pioche */
    if (charger_tuiles(gs->pioche) != 0)
    {
        fprintf(stderr, "gamestate_creer : échec chargement tuiles\n");
        plateau_detruire(gs->plateau);
        free(gs);
        return NULL;
    }

    gs->indexProchaineTuile = 0;
    gs->tourActuel = 0;

    /* 4. Mélange */
    srand((unsigned int)time(NULL));
    gamestate_melanger_pioche(gs);

    return gs;
}

/* ─────────────────────────────────────────────────────────
 * gamestate_detruire
 * ───────────────────────────────────────────────────────── */
void gamestate_detruire(GameState *gs)
{
    if (gs == NULL)
        return;

    plateau_detruire(gs->plateau);
    gs->plateau = NULL;
    free(gs);
}

/* ─────────────────────────────────────────────────────────
 * gamestate_melanger_pioche  — Algorithme Fisher-Yates
 * Chaque tuile a une probabilité égale d'atterrir
 * en n'importe quelle position.
 * ───────────────────────────────────────────────────────── */
void gamestate_melanger_pioche(GameState *gs)
{
    if (gs == NULL)
        return;

    for (int i = TOTAL_TUILES - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Tuile tmp = gs->pioche[i];
        gs->pioche[i] = gs->pioche[j];
        gs->pioche[j] = tmp;
    }
}

/* ─────────────────────────────────────────────────────────
 * gamestate_pioche_vide
 * ───────────────────────────────────────────────────────── */
int gamestate_pioche_vide(const GameState *gs)
{
    if (gs == NULL)
        return 1;
    return gs->indexProchaineTuile >= TOTAL_TUILES;
}

/* ─────────────────────────────────────────────────────────
 * gamestate_piocher
 * Retourne un pointeur sur la prochaine tuile (dans le
 * tableau, pas une copie) ou NULL si pioche vide.
 * ───────────────────────────────────────────────────────── */
Tuile *gamestate_piocher(GameState *gs)
{
    if (gs == NULL || gamestate_pioche_vide(gs))
        return NULL;

    return &gs->pioche[gs->indexProchaineTuile++];
}

/* ─────────────────────────────────────────────────────────
 * gamestate_joueur_courant
 * ───────────────────────────────────────────────────────── */
Joueur *gamestate_joueur_courant(GameState *gs)
{
    if (gs == NULL)
        return NULL;
    return &gs->joueurs[gs->tourActuel];
}

/* ─────────────────────────────────────────────────────────
 * gamestate_tour_suivant
 * Passe au joueur suivant (rotation circulaire).
 * ───────────────────────────────────────────────────────── */
void gamestate_tour_suivant(GameState *gs)
{
    if (gs == NULL)
        return;
    gs->tourActuel = (gs->tourActuel + 1) % gs->nbJoueurs;
}

/* ─────────────────────────────────────────────────────────
 * gamestate_afficher_etat  — Debug / interface texte
 * ───────────────────────────────────────────────────────── */
void gamestate_afficher_etat(const GameState *gs)
{
    if (gs == NULL)
    {
        printf("[GameState NULL]\n");
        return;
    }
    printf("=== État de la partie ===\n");
    printf("Tour : joueur %d/%d | Tuiles restantes : %d\n",
           gs->tourActuel + 1,
           gs->nbJoueurs,
           TOTAL_TUILES - gs->indexProchaineTuile);
    for (int i = 0; i < gs->nbJoueurs; i++)
        joueur_afficher(&gs->joueurs[i]);
    printf("=========================\n");
}