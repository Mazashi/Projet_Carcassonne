#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/plateau.h"
#include "../include/joueur.h"
#include "../include/gamestate.h"
#include "../include/score.h"

/* ═══════════════════════════════════════════════════════
 * CONSTANTES INTERNES
 * ═══════════════════════════════════════════════════════ */

static const int DX[4] = {0, 1, 0, -1}; /* NORD EST SUD OUEST */
static const int DY[4] = {-1, 0, 1, 0};
static const int OPPOSE[4] = {SUD, OUEST, NORD, EST};

/* ═══════════════════════════════════════════════════════
 * FLOOD-FILL VILLE
 * Parcourt récursivement toutes les tuiles connectées
 * par un bord VILLE à partir de (x, y).
 * Accumule : nb_tuiles, nb_blasons.
 * Détecte  : si un bord VILLE donne sur une case vide → ouverte.
 * ═══════════════════════════════════════════════════════ */
static void flood_ville(Plateau *p,
                        int x, int y,
                        int visite[PLATEAU_TAILLE][PLATEAU_TAILLE],
                        int *nb_tuiles, int *nb_blasons, int *fermee)
{
    if (x < 0 || x >= PLATEAU_TAILLE || y < 0 || y >= PLATEAU_TAILLE)
        return;
    if (visite[y][x])
        return;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL)
        return;

    visite[y][x] = 1;
    (*nb_tuiles)++;
    (*nb_blasons) += t->nbBlasons;

    for (int dir = 0; dir < 4; dir++)
    {
        if (t->bords[dir] != VILLE)
            continue;

        int nx = x + DX[dir];
        int ny = y + DY[dir];

        Tuile *voisin = plateau_get_tuile(p, nx, ny);

        if (voisin == NULL)
        {
            *fermee = 0;
            continue;
        }
        if (voisin->bords[OPPOSE[dir]] != VILLE)
        {
            /* Voisin présent mais son bord ne prolonge pas la ville
               → la ville est fermée de ce côté */
            continue;
        }
        if (!visite[ny][nx])
            flood_ville(p, nx, ny, visite, nb_tuiles, nb_blasons, fermee);
    }
}

/* ═══════════════════════════════════════════════════════
 * FLOOD-FILL ROUTE
 * Parcourt récursivement toutes les tuiles connectées
 * par un bord ROUTE à partir de (x, y).
 * Accumule : nb_tuiles.
 * Détecte  : si un bord ROUTE donne sur une case vide → ouverte.
 * ═══════════════════════════════════════════════════════ */
static void flood_route(Plateau *p,
                        int x, int y,
                        int visite[PLATEAU_TAILLE][PLATEAU_TAILLE],
                        int *nb_tuiles, int *terminee)
{
    if (x < 0 || x >= PLATEAU_TAILLE || y < 0 || y >= PLATEAU_TAILLE)
        return;
    if (visite[y][x])
        return;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL)
        return;

    visite[y][x] = 1;
    (*nb_tuiles)++;

    for (int dir = 0; dir < 4; dir++)
    {
        if (t->bords[dir] != ROUTE)
            continue;

        int nx = x + DX[dir];
        int ny = y + DY[dir];

        Tuile *voisin = plateau_get_tuile(p, nx, ny);

        if (voisin == NULL)
        {
            *terminee = 0; /* bord ROUTE sur le vide → route ouverte */
            continue;
        }
        if (voisin->bords[OPPOSE[dir]] != ROUTE)
        {
            /* Voisin présent mais bord opposé non-ROUTE
               → la route s'arrête ici proprement (terminée) */
            continue;
        }
        if (!visite[ny][nx])
            flood_route(p, nx, ny, visite, nb_tuiles, terminee);
    }
}

/* ═══════════════════════════════════════════════════════
 * FLOOD MARQUER
 * Après avoir scoré une région, marque toutes ses tuiles
 * dans la grille "traite" pour éviter de la scorer deux fois
 * (cas de co-propriété ou de deux partisans dans la même ville).
 * ═══════════════════════════════════════════════════════ */
static void flood_marquer(Plateau *p,
                          int x, int y,
                          TypeZone type,
                          int traite[PLATEAU_TAILLE][PLATEAU_TAILLE])
{
    if (x < 0 || x >= PLATEAU_TAILLE || y < 0 || y >= PLATEAU_TAILLE)
        return;
    if (traite[y][x])
        return;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL)
        return;

    traite[y][x] = 1;

    for (int dir = 0; dir < 4; dir++)
    {
        if (t->bords[dir] != type)
            continue;

        int nx = x + DX[dir];
        int ny = y + DY[dir];

        if (!traite[ny][nx])
            flood_marquer(p, nx, ny, type, traite);
    }
}

/* ═══════════════════════════════════════════════════════
 * score_ville
 * ═══════════════════════════════════════════════════════ */
int score_ville(Plateau *p, int x, int y, int fin_partie)
{
    if (p == NULL)
        return 0;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL)
        return 0;

    /* Vérifier que la tuile de départ fait partie d'une ville */
    int est_ville = (t->centre == VILLE);
    for (int d = 0; d < 4 && !est_ville; d++)
        est_ville = (t->bords[d] == VILLE);
    if (!est_ville)
        return 0;

    /* Tableau de visite alloué localement → zéro fuite */
    int visite[PLATEAU_TAILLE][PLATEAU_TAILLE];
    memset(visite, 0, sizeof(visite));

    int nb_tuiles = 0;
    int nb_blasons = 0;
    int fermee = 1; /* Hypothèse optimiste, invalidée par flood si besoin */

    flood_ville(p, x, y, visite, &nb_tuiles, &nb_blasons, &fermee);

    /*
     * Règle officielle :
     *   Ville FERMÉE              → 2 pts/tuile + 2 pts/blason
     *   Ville NON FERMÉE (fin)    → 1 pt/tuile  + 1 pt/blason
     *   Ville NON FERMÉE (en jeu) → 0 (pas encore scorée)
     */
    if (fermee)
        return (nb_tuiles + nb_blasons) * 2;

    if (fin_partie)
        return nb_tuiles + nb_blasons;

    return 0;
}

/* ═══════════════════════════════════════════════════════
 * score_route
 * ═══════════════════════════════════════════════════════ */
int score_route(Plateau *p, int x, int y, int fin_partie)
{
    if (p == NULL)
        return 0;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL)
        return 0;

    int est_route = 0;
    for (int d = 0; d < 4 && !est_route; d++)
        est_route = (t->bords[d] == ROUTE);
    if (!est_route)
        return 0;

    int visite[PLATEAU_TAILLE][PLATEAU_TAILLE];
    memset(visite, 0, sizeof(visite));

    int nb_tuiles = 0;
    int terminee = 1;

    flood_route(p, x, y, visite, &nb_tuiles, &terminee);

    /*
     * Règle officielle :
     *   Route TERMINÉE            → 1 pt/tuile
     *   Route NON TERMINÉE (fin)  → 1 pt/tuile (même règle)
     *   Route NON TERMINÉE (jeu)  → 0
     */
    if (terminee || fin_partie)
        return nb_tuiles;

    return 0;
}

/* ═══════════════════════════════════════════════════════
 * score_abbaye
 * ═══════════════════════════════════════════════════════ */
int score_abbaye(Plateau *p, int x, int y, int fin_partie)
{
    (void)fin_partie; /* Même calcul en cours de partie et en fin */

    if (p == NULL)
        return 0;

    Tuile *t = plateau_get_tuile(p, x, y);
    if (t == NULL || t->centre != ABBAYE)
        return 0;

    int score = 1; /* La tuile abbaye elle-même vaut 1 pt */

    /* Parcours des 8 cases voisines (cardinaux + diagonales) */
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            if (plateau_get_tuile(p, x + dx, y + dy) != NULL)
                score++;
        }

    return score; /* Maximum 9 si entourée de 8 tuiles */
}

/* ═══════════════════════════════════════════════════════
 * score_fin_partie
 * Orchestre le scoring final :
 *   - Parcourt toutes les cases du plateau
 *   - Pour chaque tuile avec un partisan, calcule les points
 *   - Marque la région comme traitée (anti-double-comptage)
 *   - Distribue les points au joueur propriétaire
 * ═══════════════════════════════════════════════════════ */
void score_fin_partie(GameState *gs)
{
    if (gs == NULL)
        return;

    /*
     * traite[y][x] = 1 si la région de cette tuile a déjà été scorée.
     * Cela évite de compter deux fois une ville avec deux partisans
     * (règle de co-propriété : les deux joueurs auraient les mêmes points,
     * mais la région n'est scorée qu'une seule fois).
     * NOTE : la gestion complète de la co-propriété (ex-aequo → deux gagnants)
     * est laissée comme évolution future du module score.
     */
    int traite[PLATEAU_TAILLE][PLATEAU_TAILLE];
    memset(traite, 0, sizeof(traite));

    for (int y = 0; y < PLATEAU_TAILLE; y++)
    {
        for (int x = 0; x < PLATEAU_TAILLE; x++)
        {
            Tuile *t = plateau_get_tuile(gs->plateau, x, y);
            if (t == NULL || traite[y][x])
                continue;

            Partisan *part = gs->plateau->occupation[y][x];
            if (part == NULL || part->idJoueur < 0)
                continue;

            /* Retrouver le joueur propriétaire par son ID */
            Joueur *j = NULL;
            for (int k = 0; k < gs->nbJoueurs; k++)
            {
                if (gs->joueurs[k].id == part->idJoueur)
                {
                    j = &gs->joueurs[k];
                    break;
                }
            }
            if (j == NULL)
                continue;

            /* Calculer les points selon le type de zone */
            int pts = 0;

            if (t->centre == ABBAYE)
            {
                pts = score_abbaye(gs->plateau, x, y, 1);
                traite[y][x] = 1; /* Abbaye = tuile unique, pas de région */
            }
            else if (part->emplacement == CENTRE && t->centre == VILLE)
            {
                pts = score_ville(gs->plateau, x, y, 1);
                flood_marquer(gs->plateau, x, y, VILLE, traite);
            }
            else
            {
                /* Déduire le type depuis les bords selon l'emplacement */
                TypeZone type = t->bords[part->emplacement];

                if (type == VILLE)
                {
                    pts = score_ville(gs->plateau, x, y, 1);
                    flood_marquer(gs->plateau, x, y, VILLE, traite);
                }
                else if (type == ROUTE)
                {
                    pts = score_route(gs->plateau, x, y, 1);
                    flood_marquer(gs->plateau, x, y, ROUTE, traite);
                }
            }

            joueur_ajouter_points(j, pts);
        }
    }
}