#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/gamestate.h"
#include "../include/moteur.h"

#define NB_JOUEURS_MIN 2
#define NB_JOUEURS_MAX 5

/* ═══════════════════════════════════════════════════════
 * saisir_nb_joueurs
 * Demande et valide le nombre de joueurs (2 à 5).
 * ═══════════════════════════════════════════════════════ */
static int saisir_nb_joueurs(void)
{
    int n = 0;
    do
    {
        printf("Nombre de joueurs (%d-%d) : ",
               NB_JOUEURS_MIN, NB_JOUEURS_MAX);
        if (scanf("%d", &n) != 1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            n = 0;
        }
    } while (n < NB_JOUEURS_MIN || n > NB_JOUEURS_MAX);
    return n;
}

/* ═══════════════════════════════════════════════════════
 * saisir_noms
 * Demande le nom de chaque joueur.
 * ═══════════════════════════════════════════════════════ */
static void saisir_noms(int nb, char noms[MAX_JOUEURS][32])
{
    /* Vider le buffer laissé par scanf("%d") */
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    for (int i = 0; i < nb; i++)
    {
        printf("Nom du joueur %d : ", i + 1);
        if (fgets(noms[i], 32, stdin) == NULL)
            snprintf(noms[i], 32, "Joueur%d", i + 1);

        /* Supprimer le '\n' laissé par fgets */
        size_t len = strlen(noms[i]);
        if (len > 0 && noms[i][len - 1] == '\n')
            noms[i][len - 1] = '\0';

        /* Nom vide → nom par défaut */
        if (noms[i][0] == '\0')
            snprintf(noms[i], 32, "Joueur%d", i + 1);
    }
}

/* ═══════════════════════════════════════════════════════
 * main
 * ═══════════════════════════════════════════════════════ */
int main(void)
{
    printf("╔══════════════════════════════════╗\n");
    printf("║   CARCASSONNE — Projet S4 L2     ║\n");
    printf("╚══════════════════════════════════╝\n\n");

    /* 1. Configuration de la partie */
    int nb = saisir_nb_joueurs();

    char noms[MAX_JOUEURS][32];
    memset(noms, 0, sizeof(noms));
    saisir_noms(nb, noms);

    /* 2. Création du GameState */
    GameState *gs = gamestate_creer(nb, (const char (*)[32])noms);
    if (gs == NULL)
    {
        fprintf(stderr, "[Erreur] Impossible de créer la partie.\n");
        return EXIT_FAILURE;
    }

    /* 3. Lancement */
    moteur_jouer(gs);

    /* 4. Nettoyage */
    gamestate_detruire(gs);

    return EXIT_SUCCESS;
}