#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/plateau.h"
#include "../include/joueur.h"
#include "../include/gamestate.h"
#include "../include/score.h"
#include "../include/moteur.h"

/* ═══════════════════════════════════════════════════════
 * CONSTANTES INTERNES
 * ═══════════════════════════════════════════════════════ */

static const int DX[4] = {0, 1, 0, -1};
static const int DY[4] = {-1, 0, 1, 0};

static const char *typezone_nom(TypeZone zone)
{
    switch (zone)
    {
    case PLAINE:
        return "PLAINE";
    case ROUTE:
        return "ROUTE";
    case VILLE:
        return "VILLE";
    case ABBAYE:
        return "ABBAYE";
    case VIDE:
        return "VIDE";
    default:
        return "INCONNU";
    }
}

static const char *position_nom(Position pos)
{
    switch (pos)
    {
    case NORD:
        return "NORD";
    case EST:
        return "EST";
    case SUD:
        return "SUD";
    case OUEST:
        return "OUEST";
    case CENTRE:
        return "CENTRE";
    default:
        return "INCONNU";
    }
}

/* ═══════════════════════════════════════════════════════
 * moteur_init
 * Pose la tuile de départ (index 0 de la pioche, avant
 * mélange) au centre du plateau. Cette tuile est posée
 * directement sans voisin requis — c'est la seule exception
 * aux règles de validation du plateau.
 * ═══════════════════════════════════════════════════════ */
MoteurCode moteur_init(GameState *gs)
{
    if (gs == NULL)
        return MOTEUR_ERR_NULL;

    /*
     * On pioche la première tuile comme tuile de départ.
     * Elle est posée directement dans la grille sans passer
     * par plateau_est_valide (qui exige un voisin).
     */
    Tuile *depart = gamestate_piocher(gs);
    if (depart == NULL)
        return MOTEUR_ERR_NULL;

    Tuile *copie = malloc(sizeof(Tuile));
    if (copie == NULL)
        return MOTEUR_ERR_NULL;

    *copie = *depart;
    gs->plateau->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = copie;

    printf("[Moteur] Tuile de départ (ID %d) posée en (%d, %d).\n",
           copie->id, CENTRE_PLATEAU, CENTRE_PLATEAU);

    return MOTEUR_OK;
}

/* ═══════════════════════════════════════════════════════
 * moteur_positions_valides
 * Parcourt toutes les cases adjacentes aux tuiles posées
 * et retourne celles où la tuile t peut être placée.
 * Alloue dynamiquement le tableau — libérer avec
 * moteur_positions_liberer().
 * ═══════════════════════════════════════════════════════ */
ListePositions moteur_positions_valides(GameState *gs, Tuile *t)
{
    ListePositions lp = {NULL, NULL, 0};

    if (gs == NULL || t == NULL)
        return lp;

    /* Capacité initiale — redimensionnée si besoin */
    int capacite = 64;
    lp.xs = malloc(sizeof(int) * capacite);
    lp.ys = malloc(sizeof(int) * capacite);
    if (lp.xs == NULL || lp.ys == NULL)
    {
        free(lp.xs);
        free(lp.ys);
        lp.xs = NULL;
        lp.ys = NULL;
        return lp;
    }

    /*
     * Pour éviter de tester chaque case du plateau (145×145),
     * on ne teste que les cases adjacentes à une tuile existante.
     * visite[y][x] évite les doublons.
     */
    static int visite[PLATEAU_TAILLE][PLATEAU_TAILLE];
    memset(visite, 0, sizeof(visite));

    for (int y = 0; y < PLATEAU_TAILLE; y++)
    {
        for (int x = 0; x < PLATEAU_TAILLE; x++)
        {
            if (plateau_get_tuile(gs->plateau, x, y) == NULL)
                continue;

            /* Examiner les 4 cases voisines de cette tuile */
            for (int dir = 0; dir < 4; dir++)
            {
                int nx = x + DX[dir];
                int ny = y + DY[dir];

                if (nx < 0 || nx >= PLATEAU_TAILLE ||
                    ny < 0 || ny >= PLATEAU_TAILLE)
                    continue;
                if (visite[ny][nx])
                    continue;

                visite[ny][nx] = 1;

                if (!plateau_est_valide(gs->plateau, t, nx, ny))
                    continue;

                /* Case valide → ajouter à la liste */
                if (lp.count == capacite)
                {
                    capacite *= 2;
                    int *tmp_x = realloc(lp.xs, sizeof(int) * capacite);
                    int *tmp_y = realloc(lp.ys, sizeof(int) * capacite);
                    if (tmp_x == NULL || tmp_y == NULL)
                    {
                        free(lp.xs);
                        free(lp.ys);
                        lp.xs = NULL;
                        lp.ys = NULL;
                        lp.count = 0;
                        return lp;
                    }
                    lp.xs = tmp_x;
                    lp.ys = tmp_y;
                }

                lp.xs[lp.count] = nx;
                lp.ys[lp.count] = ny;
                lp.count++;
            }
        }
    }

    return lp;
}

/* ═══════════════════════════════════════════════════════
 * moteur_positions_liberer
 * ═══════════════════════════════════════════════════════ */
void moteur_positions_liberer(ListePositions *lp)
{
    if (lp == NULL)
        return;
    free(lp->xs);
    free(lp->ys);
    lp->xs = NULL;
    lp->ys = NULL;
    lp->count = 0;
}

/* ═══════════════════════════════════════════════════════
 * moteur_a_position_valide
 * Retourne 1 si au moins une position existe pour t,
 * 0 sinon (tuile non jouable → doit être défaussée).
 * ═══════════════════════════════════════════════════════ */
int moteur_a_position_valide(GameState *gs, Tuile *t)
{
    ListePositions lp = moteur_positions_valides(gs, t);
    int res = lp.count > 0;
    moteur_positions_liberer(&lp);
    return res;
}

/* ═══════════════════════════════════════════════════════
 * moteur_placer_tuile
 * Délègue la validation à plateau_est_valide puis pose.
 * ═══════════════════════════════════════════════════════ */
MoteurCode moteur_placer_tuile(GameState *gs, Tuile *t, int x, int y)
{
    if (gs == NULL || t == NULL)
        return MOTEUR_ERR_NULL;

    if (plateau_poser_tuile(gs->plateau, t, x, y) != 0)
        return MOTEUR_ERR_POSE;

    return MOTEUR_OK;
}

/* ═══════════════════════════════════════════════════════
 * moteur_placer_partisan
 * Vérifie que le joueur courant a un partisan disponible
 * puis le pose sur la tuile en (x, y) à l'emplacement empl.
 *
 * Note : la règle "un emplacement déjà revendiqué ne peut
 * pas accueillir un nouveau partisan" est marquée comme
 * évolution future — elle nécessite le module score complet
 * pour détecter la connexion de régions.
 * ═══════════════════════════════════════════════════════ */
MoteurCode moteur_placer_partisan(GameState *gs, int x, int y, Position empl)
{
    if (gs == NULL)
        return MOTEUR_ERR_NULL;

    Joueur *j = gamestate_joueur_courant(gs);
    if (j == NULL)
        return MOTEUR_ERR_NULL;

    if (!joueur_a_partisan(j))
        return MOTEUR_ERR_STOCK;

    Tuile *t = plateau_get_tuile(gs->plateau, x, y);
    if (t == NULL)
        return MOTEUR_ERR_POSE;

    /* Vérifier qu'aucun partisan n'occupe déjà cette case */
    if (gs->plateau->occupation[y][x] != NULL)
        return MOTEUR_ERR_PION;

    Partisan *part = malloc(sizeof(Partisan));
    if (part == NULL)
        return MOTEUR_ERR_NULL;

    part->idJoueur = j->id;
    part->emplacement = empl;
    part->x = x;
    part->y = y;

    gs->plateau->occupation[y][x] = part;
    joueur_retirer_partisan(j);

    return MOTEUR_OK;
}

/* ═══════════════════════════════════════════════════════
 * moteur_scorer_voisins
 * Après la pose d'une tuile, vérifie si des structures
 * adjacentes sont maintenant fermées et les score.
 * ═══════════════════════════════════════════════════════ */
static void moteur_scorer_voisins(GameState *gs, int x, int y)
{
    /* Vérifier la tuile posée et ses 4 voisins */
    int xs_check[5] = {x, x, x + 1, x, x - 1};
    int ys_check[5] = {y, y - 1, y, y + 1, y};

    for (int i = 0; i < 5; i++)
    {
        int cx = xs_check[i];
        int cy = ys_check[i];

        Tuile *t = plateau_get_tuile(gs->plateau, cx, cy);
        if (t == NULL)
            continue;

        Partisan *part = gs->plateau->occupation[cy][cx];
        if (part == NULL)
            continue;

        Joueur *j = NULL;
        for (int k = 0; k < gs->nbJoueurs; k++)
            if (gs->joueurs[k].id == part->idJoueur)
            {
                j = &gs->joueurs[k];
                break;
            }
        if (j == NULL)
            continue;

        int pts = 0;

        if (t->centre == ABBAYE)
        {
            /* Abbaye : scorer seulement si entourée (9 pts) */
            pts = score_abbaye(gs->plateau, cx, cy, 0);
            if (pts == 9)
            {
                joueur_ajouter_points(j, pts);
                free(gs->plateau->occupation[cy][cx]);
                gs->plateau->occupation[cy][cx] = NULL;
                joueur_rendre_partisan(j);
                printf("[Score] Abbaye complète ! +%d pts pour %s\n",
                       pts, j->nom);
            }
        }
        else
        {
            /* Ville ou route : scorer seulement si fermée */
            TypeZone type = (part->emplacement == CENTRE)
                                ? t->centre
                                : t->bords[part->emplacement];

            if (type == VILLE)
                pts = score_ville(gs->plateau, cx, cy, 0);
            else if (type == ROUTE)
                pts = score_route(gs->plateau, cx, cy, 0);

            if (pts > 0)
            {
                joueur_ajouter_points(j, pts);
                free(gs->plateau->occupation[cy][cx]);
                gs->plateau->occupation[cy][cx] = NULL;
                joueur_rendre_partisan(j);
                printf("[Score] Structure fermée ! +%d pts pour %s\n",
                       pts, j->nom);
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════
 * moteur_tour
 * Exécute un tour complet (mode console interactif) :
 *   1. Afficher l'état
 *   2. Piocher une tuile
 *   3. Afficher les positions valides
 *   4. Demander les coordonnées au joueur
 *   5. Demander si le joueur pose un partisan
 *   6. Scorer les structures fermées
 *   7. Passer au joueur suivant
 * ═══════════════════════════════════════════════════════ */
void moteur_tour(GameState *gs)
{
    if (gs == NULL)
        return;

    gamestate_afficher_etat(gs);

    Joueur *j = gamestate_joueur_courant(gs);
    printf("\n>>> Tour de %s <<<\n", j->nom);

    /* 1. Piocher */
    Tuile *t = gamestate_piocher(gs);
    if (t == NULL)
    {
        printf("[Moteur] Pioche vide.\n");
        return;
    }
    printf("[Pioche] Tuile ID %d — N:%s E:%s S:%s O:%s Centre:%s\n",
           t->id,
           typezone_nom(t->bords[NORD]), typezone_nom(t->bords[EST]),
           typezone_nom(t->bords[SUD]), typezone_nom(t->bords[OUEST]),
           typezone_nom(t->centre));

    /* 2. Trouver les positions valides */
    ListePositions lp = moteur_positions_valides(gs, t);
    if (lp.count == 0)
    {
        printf("[Moteur] Aucune position valide. Tuile défaussée.\n");
        moteur_positions_liberer(&lp);
        gamestate_tour_suivant(gs);
        return;
    }

    printf("[Positions] %d case(s) disponible(s) :\n", lp.count);
    for (int i = 0; i < lp.count && i < 10; i++)
        printf("  %d) (%d, %d)\n", i + 1, lp.xs[i], lp.ys[i]);
    if (lp.count > 10)
        printf("  ... et %d autres.\n", lp.count - 10);

    /* 3. Saisie des coordonnées */
    int x = -1, y = -1;
    int pose_ok = 0;

    while (!pose_ok)
    {
        printf("Entrez x y : ");
        if (scanf("%d %d", &x, &y) != 2)
        {
            /* Vider le buffer en cas de saisie invalide */
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
            continue;
        }

        if (moteur_placer_tuile(gs, t, x, y) == MOTEUR_OK)
            pose_ok = 1;
        else
            printf("[Erreur] Position (%d, %d) invalide. Réessayez.\n", x, y);
    }

    moteur_positions_liberer(&lp);
    printf("[Pose] Tuile ID %d placée en (%d, %d).\n", t->id, x, y);

    /* 4. Proposer la pose d'un partisan */
    if (joueur_a_partisan(j))
    {
        printf("Poser un partisan ? (1=oui / 0=non) : ");
        int rep = 0;
        scanf("%d", &rep);

        if (rep == 1)
        {
            printf("Emplacement (0=%s 1=%s 2=%s 3=%s 4=%s) : ",
                   position_nom(NORD), position_nom(EST),
                   position_nom(SUD), position_nom(OUEST),
                   position_nom(CENTRE));
            int empl = 0;
            scanf("%d", &empl);

            if (empl >= 0 && empl <= 4)
            {
                MoteurCode res = moteur_placer_partisan(gs, x, y,
                                                        (Position)empl);
                if (res == MOTEUR_OK)
                    printf("[Partisan] Posé sur emplacement %d.\n", empl);
                else
                    printf("[Erreur] Impossible de poser le partisan (%d).\n",
                           res);
            }
        }
    }
    else
        printf("[Info] %s n'a plus de partisans.\n", j->nom);

    /* 5. Scorer les structures fermées par cette pose */
    moteur_scorer_voisins(gs, x, y);

    /* 6. Tour suivant */
    gamestate_tour_suivant(gs);
}

/* ═══════════════════════════════════════════════════════
 * moteur_jouer
 * Boucle principale : tourne jusqu'à ce que la pioche
 * soit vide, puis déclenche le scoring final.
 * ═══════════════════════════════════════════════════════ */
void moteur_jouer(GameState *gs)
{
    if (gs == NULL)
        return;

    printf("\n╔══════════════════════════════╗\n");
    printf("║     CARCASSONNE — Début      ║\n");
    printf("╚══════════════════════════════╝\n\n");

    if (moteur_init(gs) != MOTEUR_OK)
    {
        fprintf(stderr, "[Moteur] Échec de l'initialisation.\n");
        return;
    }

    while (!gamestate_pioche_vide(gs))
        moteur_tour(gs);

    /* Scoring final */
    printf("\n╔══════════════════════════════╗\n");
    printf("║     FIN DE PARTIE            ║\n");
    printf("╚══════════════════════════════╝\n\n");

    score_fin_partie(gs);
    gamestate_afficher_etat(gs);

    /* Afficher le vainqueur */
    Joueur *vainqueur = &gs->joueurs[0];
    for (int i = 1; i < gs->nbJoueurs; i++)
        if (gs->joueurs[i].score > vainqueur->score)
            vainqueur = &gs->joueurs[i];

    printf("\n Vainqueur : %s avec %d points !\n",
           vainqueur->nom, vainqueur->score);
}