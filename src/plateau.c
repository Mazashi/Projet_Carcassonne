#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/plateau.h"

/**
 * Alloue et initialise un plateau vide.
 * Toutes les cases sont mises à NULL (aucune tuile posée).
 */
Plateau *plateau_creer(void)
{
    Plateau *p = malloc(sizeof(Plateau));
    if (p == NULL)
    {
        perror("Erreur malloc Plateau");
        return NULL;
    }
    // Initialise toutes les cases à NULL et partisans à NULL
    memset(p, 0, sizeof(Plateau));
    return p;
}

/**
 * Libère toute la mémoire du plateau.
 */
void plateau_detruire(Plateau *p)
{
    if (p == NULL)
        return;

    for (int i = 0; i < PLATEAU_TAILLE; i++)
    {
        for (int j = 0; j < PLATEAU_TAILLE; j++)
        {
            if (p->grille[i][j] != NULL)
            {
                free(p->grille[i][j]);
                p->grille[i][j] = NULL;
            }
            if (p->occupation[i][j] != NULL)
            {
                free(p->occupation[i][j]);
                p->occupation[i][j] = NULL;
            }
        }
    }
    free(p);
}

/**
 * Vérifie si une case est libre et dans les bornes.
 * @return 1 si libre, 0 sinon.
 */
int plateau_est_libre(Plateau *p, int x, int y)
{
    if (p == NULL)
        return 0;
    if (x < 0 || x >= PLATEAU_TAILLE || y < 0 || y >= PLATEAU_TAILLE)
        return 0;
    return p->grille[y][x] == NULL;
}

/**
 * Vérifie si une tuile peut être posée en (x, y).
 * Règle : chaque bord doit correspondre au bord opposé du voisin.
 * @return 1 si valide, 0 sinon.
 */
int plateau_est_valide(Plateau *p, Tuile *t, int x, int y)
{
    if (!plateau_est_libre(p, x, y))
        return 0;

    // Décalages [NORD, EST, SUD, OUEST]
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    // Bord opposé : NORD<->SUD, EST<->OUEST
    int oppose[] = {SUD, OUEST, NORD, EST};

    int a_voisin = 0;

    for (int dir = 0; dir < 4; dir++)
    {
        int nx = x + dx[dir];
        int ny = y + dy[dir];

        if (nx < 0 || nx >= PLATEAU_TAILLE || ny < 0 || ny >= PLATEAU_TAILLE)
            continue;

        Tuile *voisin = p->grille[ny][nx];
        if (voisin == NULL)
            continue;

        a_voisin = 1;
        // Le bord de t dans la direction dir doit correspondre
        // au bord opposé du voisin
        if (t->bords[dir] != voisin->bords[oppose[dir]])
            return 0;
    }

    return a_voisin; // Doit avoir au moins un voisin
}

/**
 * Pose une tuile sur le plateau en (x, y).
 * @return 0 succès, -1 position invalide.
 */
int plateau_poser_tuile(Plateau *p, Tuile *t, int x, int y)
{
    if (p == NULL || t == NULL)
        return -1;

    if (!plateau_est_valide(p, t, x, y))
    {
        fprintf(stderr, "Position (%d,%d) invalide pour cette tuile.\n", x, y);
        return -1;
    }

    Tuile *copie = malloc(sizeof(Tuile));
    if (copie == NULL)
    {
        perror("Erreur malloc tuile");
        return -1;
    }
    *copie = *t; // Copie par valeur
    p->grille[y][x] = copie;
    return 0;
}

/**
 * Retourne un pointeur sur la tuile en (x, y), ou NULL si vide.
 */
Tuile *plateau_get_tuile(Plateau *p, int x, int y)
{
    if (p == NULL)
        return NULL;
    if (x < 0 || x >= PLATEAU_TAILLE || y < 0 || y >= PLATEAU_TAILLE)
        return NULL;
    return p->grille[y][x];
}