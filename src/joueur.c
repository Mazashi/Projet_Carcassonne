#include <stdio.h>
#include <string.h>
#include "../include/types.h"
#include "../include/joueur.h"

/**
 * Initialise un joueur avec son ID, son nom, score à 0
 * et NB_PARTISANS_MAX partisans disponibles.
 */
void joueur_init(Joueur *j, int id, const char *nom)
{
    if (j == NULL || nom == NULL)
        return;

    j->id = id;
    j->score = 0;
    j->partisansRestants = NB_PARTISANS_MAX;

    /* strncpy + forçage du '\0' final : protection buffer overflow */
    strncpy(j->nom, nom, sizeof(j->nom) - 1);
    j->nom[sizeof(j->nom) - 1] = '\0';
}

/**
 * Retourne 1 si le joueur possède au moins un partisan disponible.
 */
int joueur_a_partisan(const Joueur *j)
{
    if (j == NULL)
        return 0;
    return j->partisansRestants > 0;
}

/**
 * Retire un partisan du stock du joueur (pose sur le plateau).
 * @return 0 succès, -1 si stock vide ou pointeur NULL.
 */
int joueur_retirer_partisan(Joueur *j)
{
    if (j == NULL || j->partisansRestants <= 0)
        return -1;

    j->partisansRestants--;
    return 0;
}

/**
 * Rend un partisan au joueur (récupération en fin de score).
 * Plafond à NB_PARTISANS_MAX pour éviter la corruption d'état.
 */
void joueur_rendre_partisan(Joueur *j)
{
    if (j == NULL)
        return;

    if (j->partisansRestants < NB_PARTISANS_MAX)
        j->partisansRestants++;
}

/**
 * Ajoute des points au score. Accepte les valeurs négatives
 * (pénalités potentielles) mais bloque à 0 par sécurité.
 */
void joueur_ajouter_points(Joueur *j, int points)
{
    if (j == NULL)
        return;

    j->score += points;
    if (j->score < 0)
        j->score = 0;
}

/**
 * Affiche l'état courant d'un joueur (debug / interface texte).
 */
void joueur_afficher(const Joueur *j)
{
    if (j == NULL)
    {
        printf("[Joueur NULL]\n");
        return;
    }
    printf("Joueur %d | %-20s | Score : %3d | Partisans : %d/%d\n",
           j->id, j->nom, j->score,
           j->partisansRestants, NB_PARTISANS_MAX);
}