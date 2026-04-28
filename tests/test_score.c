#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/plateau.h"
#include "../include/joueur.h"
#include "../include/gamestate.h"
#include "../include/score.h"

/* ═══════════════════════════════════════════════════════
 * UTILITAIRE
 * ═══════════════════════════════════════════════════════ */
static int tests_ok = 0;
static int tests_ko = 0;

#define TEST(nom, condition)            \
    do                                  \
    {                                   \
        if (condition)                  \
        {                               \
            printf("  [OK] %s\n", nom); \
            tests_ok++;                 \
        }                               \
        else                            \
        {                               \
            printf("  [KO] %s\n", nom); \
            tests_ko++;                 \
        }                               \
    } while (0)

/* ═══════════════════════════════════════════════════════
 * HELPERS — Constructeurs de tuiles de test
 * ═══════════════════════════════════════════════════════ */

/* Tuile homogène : tous les bords et le centre du même type */
static Tuile tuile_homo(int id, TypeZone type)
{
    Tuile t;
    t.id = id;
    t.rotation = 0;
    t.nbBlasons = 0;
    t.centre = type;
    for (int i = 0; i < 4; i++)
        t.bords[i] = type;
    return t;
}

/* Tuile abbaye : centre ABBAYE, bords PLAINE */
static Tuile tuile_abbaye(int id)
{
    Tuile t;
    t.id = id;
    t.rotation = 0;
    t.nbBlasons = 0;
    t.centre = ABBAYE;
    for (int i = 0; i < 4; i++)
        t.bords[i] = PLAINE;
    return t;
}

/* Tuile VILLE avec un blason */
static Tuile tuile_ville_blason(int id)
{
    Tuile t = tuile_homo(id, VILLE);
    t.nbBlasons = 1;
    return t;
}

/*
 * Pose une tuile directement dans la grille (bypass validation)
 * Utilisé pour construire des configurations de test précises.
 */
static void poser_force(Plateau *p, Tuile t, int x, int y)
{
    Tuile *copie = malloc(sizeof(Tuile));
    *copie = t;
    p->grille[y][x] = copie;
}

/*
 * Pose un partisan sur une case du plateau.
 */
static void poser_partisan(Plateau *p, int x, int y,
                           int idJoueur, Position empl)
{
    Partisan *part = malloc(sizeof(Partisan));
    part->idJoueur = idJoueur;
    part->emplacement = empl;
    part->x = x;
    part->y = y;
    p->occupation[y][x] = part;
}

/* ═══════════════════════════════════════════════════════
 * TESTS — score_ville
 * ═══════════════════════════════════════════════════════ */

void test_ville_une_tuile_fermee(void)
{
    printf("\n>> test_ville_une_tuile_fermee\n");
    /*
     * Cas : ville d'une seule tuile entourée de PLAINE sur 3 côtés
     * et d'une autre tuile PLAINE qui referme le 4e bord.
     *
     * Pour être "fermée", il ne faut aucun bord VILLE donnant sur le vide.
     * On pose une tuile VILLE entourée de 4 voisins PLAINE.
     *
     *   [ P ]
     * [P][V][P]
     *   [ P ]
     *
     * Mais la tuile VILLE a tous ses bords VILLE → ses 4 voisins doivent
     * avoir un bord VILLE côté V. On construit donc des voisins mixtes.
     */

    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    /* Tuile centrale : VILLE partout, 1 blason */
    Tuile ville = tuile_ville_blason(1);
    poser_force(p, ville, cx, cy);

    /* 4 voisins avec le bord qui touche la ville == VILLE,
       et les 3 autres bords == PLAINE → ville "fermée" côté voisin */
    Tuile nord_t;
    nord_t = tuile_homo(2, PLAINE);
    nord_t.bords[SUD] = VILLE; /* côté qui touche la tuile centrale */
    poser_force(p, nord_t, cx, cy - 1);

    Tuile sud_t = tuile_homo(3, PLAINE);
    sud_t.bords[NORD] = VILLE;
    poser_force(p, sud_t, cx, cy + 1);

    Tuile est_t = tuile_homo(4, PLAINE);
    est_t.bords[OUEST] = VILLE;
    poser_force(p, est_t, cx + 1, cy);

    Tuile ouest_t = tuile_homo(5, PLAINE);
    ouest_t.bords[EST] = VILLE;
    poser_force(p, ouest_t, cx - 1, cy);

    /*
     * Score attendu : ville fermée
     * nb_tuiles = 5 (la centrale + 4 voisins qui ont un bord VILLE)
     * nb_blasons = 1 (uniquement sur la tuile centrale)
     * → (5 + 1) × 2 = 12 pts
     */
    int pts = score_ville(p, cx, cy, 0);
    TEST("ville fermée 1 blason = 12 pts", pts == 12);

    plateau_detruire(p);
}

void test_ville_ouverte_en_jeu(void)
{
    printf("\n>> test_ville_ouverte_en_jeu\n");
    /*
     * Cas : ville d'une seule tuile VILLE isolée.
     * Tous ses bords VILLE donnent sur le vide → ville ouverte.
     * En cours de partie → 0 pts.
     */
    Plateau *p = plateau_creer();
    Tuile v = tuile_homo(1, VILLE);
    poser_force(p, v, CENTRE_PLATEAU, CENTRE_PLATEAU);

    int pts = score_ville(p, CENTRE_PLATEAU, CENTRE_PLATEAU, 0);
    TEST("ville ouverte en jeu = 0 pts", pts == 0);

    plateau_detruire(p);
}

void test_ville_ouverte_fin_partie(void)
{
    printf("\n>> test_ville_ouverte_fin_partie\n");
    /*
     * Cas : ville d'une seule tuile VILLE, 1 blason, non fermée.
     * En fin de partie → 1 pt/tuile + 1 pt/blason = 2 pts.
     */
    Plateau *p = plateau_creer();
    Tuile v = tuile_ville_blason(1);
    poser_force(p, v, CENTRE_PLATEAU, CENTRE_PLATEAU);

    int pts = score_ville(p, CENTRE_PLATEAU, CENTRE_PLATEAU, 1);
    TEST("ville ouverte fin partie 1 blason = 2 pts", pts == 2);

    plateau_detruire(p);
}

void test_ville_deux_tuiles_fermee(void)
{
    printf("\n>> test_ville_deux_tuiles_fermee\n");
    /*
     * Cas : 2 tuiles VILLE côte à côte, fermées de chaque côté.
     *
     * [PLAINE|VILLE][VILLE|PLAINE]
     *     tuileA         tuileB
     *
     * Chaque tuile n'a que le bord interne en VILLE, les 3 autres en PLAINE.
     * → Les bords externes donnent sur le vide mais ils sont PLAINE → OK.
     * → Seul le bord interne (EST de A / OUEST de B) est VILLE, et il est couvert.
     * → Ville fermée : 2 tuiles, 0 blasons → 2 × 2 = 4 pts.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    Tuile a = tuile_homo(1, PLAINE);
    a.bords[EST] = VILLE;
    a.centre = VILLE;

    Tuile b = tuile_homo(2, PLAINE);
    b.bords[OUEST] = VILLE;
    b.centre = VILLE;

    poser_force(p, a, cx, cy);
    poser_force(p, b, cx + 1, cy);

    int pts = score_ville(p, cx, cy, 0);
    TEST("ville 2 tuiles fermée = 4 pts", pts == 4);

    plateau_detruire(p);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — score_route
 * ═══════════════════════════════════════════════════════ */

void test_route_une_tuile_terminee(void)
{
    printf("\n>> test_route_une_tuile_terminee\n");
    /*
     * Cas : tuile avec ROUTE au NORD et au SUD, bouchée des deux côtés
     * par des voisins dont les bords NORD/SUD ne sont pas ROUTE
     * (ce qui ferme la route).
     *
     * Ici on triche : on pose des voisins avec PLAINE côté route.
     * → La route ne peut pas se propager → 1 tuile → 1 pt.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    Tuile r = tuile_homo(1, PLAINE);
    r.bords[NORD] = ROUTE;
    r.bords[SUD] = ROUTE;
    r.centre = ROUTE;
    poser_force(p, r, cx, cy);

    /* Voisin NORD : son bord SUD est PLAINE (pas ROUTE) → bloque */
    Tuile vn = tuile_homo(2, PLAINE);
    poser_force(p, vn, cx, cy - 1);

    /* Voisin SUD : son bord NORD est PLAINE (pas ROUTE) → bloque */
    Tuile vs = tuile_homo(3, PLAINE);
    poser_force(p, vs, cx, cy + 1);

    int pts = score_route(p, cx, cy, 0);
    TEST("route 1 tuile terminée = 1 pt", pts == 1);

    plateau_detruire(p);
}

void test_route_ouverte_en_jeu(void)
{
    printf("\n>> test_route_ouverte_en_jeu\n");
    /*
     * Cas : tuile avec ROUTE au NORD, voisin NORD absent.
     * → Route ouverte, en cours de partie → 0 pts.
     */
    Plateau *p = plateau_creer();
    Tuile r = tuile_homo(1, PLAINE);
    r.bords[NORD] = ROUTE;
    r.centre = ROUTE;
    poser_force(p, r, CENTRE_PLATEAU, CENTRE_PLATEAU);

    int pts = score_route(p, CENTRE_PLATEAU, CENTRE_PLATEAU, 0);
    TEST("route ouverte en jeu = 0 pts", pts == 0);

    plateau_detruire(p);
}

void test_route_ouverte_fin_partie(void)
{
    printf("\n>> test_route_ouverte_fin_partie\n");
    /*
     * Cas : 3 tuiles ROUTE en ligne, extrémité EST ouverte (vide).
     * En fin de partie → 3 pts.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    for (int i = 0; i < 3; i++)
    {
        Tuile r = tuile_homo(i + 1, PLAINE);
        r.bords[EST] = ROUTE;
        r.bords[OUEST] = ROUTE;
        r.centre = ROUTE;
        poser_force(p, r, cx + i, cy);
    }

    int pts = score_route(p, cx, cy, 1);
    TEST("route 3 tuiles ouverte fin partie = 3 pts", pts == 3);

    plateau_detruire(p);
}

void test_route_trois_tuiles_terminee(void)
{
    printf("\n>> test_route_trois_tuiles_terminee\n");
    /*
     * Cas : 3 tuiles en ligne EST-OUEST terminées des deux côtés
     * par des voisins PLAINE (non ROUTE).
     * → route terminée, 3 tuiles → 3 pts.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    for (int i = 0; i < 3; i++)
    {
        Tuile r = tuile_homo(i + 1, PLAINE);
        r.bords[EST] = ROUTE;
        r.bords[OUEST] = ROUTE;
        r.centre = ROUTE;
        poser_force(p, r, cx + i, cy);
    }

    /* Boucher les extrémités */
    Tuile bout_ouest = tuile_homo(10, PLAINE); /* bords PLAINE → bloque */
    poser_force(p, bout_ouest, cx - 1, cy);

    Tuile bout_est = tuile_homo(11, PLAINE);
    poser_force(p, bout_est, cx + 3, cy);

    int pts = score_route(p, cx, cy, 0);
    TEST("route 3 tuiles terminée = 3 pts", pts == 3);

    plateau_detruire(p);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — score_abbaye
 * ═══════════════════════════════════════════════════════ */

void test_abbaye_isolee(void)
{
    printf("\n>> test_abbaye_isolee\n");
    /*
     * Cas : abbaye seule, aucun voisin.
     * → 1 pt (la tuile elle-même).
     */
    Plateau *p = plateau_creer();
    Tuile a = tuile_abbaye(1);
    poser_force(p, a, CENTRE_PLATEAU, CENTRE_PLATEAU);

    int pts = score_abbaye(p, CENTRE_PLATEAU, CENTRE_PLATEAU, 0);
    TEST("abbaye isolée = 1 pt", pts == 1);

    plateau_detruire(p);
}

void test_abbaye_entouree(void)
{
    printf("\n>> test_abbaye_entouree\n");
    /*
     * Cas : abbaye entourée de ses 8 voisins → 9 pts.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    poser_force(p, tuile_abbaye(1), cx, cy);

    int id = 2;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            poser_force(p, tuile_homo(id++, PLAINE), cx + dx, cy + dy);
        }

    int pts = score_abbaye(p, cx, cy, 0);
    TEST("abbaye entourée = 9 pts", pts == 9);

    plateau_detruire(p);
}

void test_abbaye_4_voisins(void)
{
    printf("\n>> test_abbaye_4_voisins\n");
    /*
     * Cas : abbaye avec seulement les 4 voisins cardinaux
     * → 1 + 4 = 5 pts.
     */
    Plateau *p = plateau_creer();
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    poser_force(p, tuile_abbaye(1), cx, cy);
    poser_force(p, tuile_homo(2, PLAINE), cx, cy - 1); /* NORD */
    poser_force(p, tuile_homo(3, PLAINE), cx + 1, cy); /* EST  */
    poser_force(p, tuile_homo(4, PLAINE), cx, cy + 1); /* SUD  */
    poser_force(p, tuile_homo(5, PLAINE), cx - 1, cy); /* OUEST*/

    int pts = score_abbaye(p, cx, cy, 0);
    TEST("abbaye 4 voisins cardinaux = 5 pts", pts == 5);

    plateau_detruire(p);
}

void test_abbaye_pas_abbaye(void)
{
    printf("\n>> test_abbaye_pas_abbaye\n");
    /*
     * Cas : appel sur une tuile VILLE → doit retourner 0.
     */
    Plateau *p = plateau_creer();
    poser_force(p, tuile_homo(1, VILLE), CENTRE_PLATEAU, CENTRE_PLATEAU);

    int pts = score_abbaye(p, CENTRE_PLATEAU, CENTRE_PLATEAU, 0);
    TEST("score_abbaye sur tuile VILLE = 0", pts == 0);

    plateau_detruire(p);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — score_fin_partie
 * ═══════════════════════════════════════════════════════ */

void test_fin_partie_abbaye(void)
{
    printf("\n>> test_fin_partie_abbaye (via GameState)\n");
    /*
     * Scénario :
     *   - 2 joueurs, joueur 1 (Alice) a un partisan sur une abbaye isolée.
     *   - Aucun voisin → 1 pt attribué à Alice en fin de partie.
     */
    const char noms[5][32] = {"Alice", "Bob", "", "", ""};
    GameState *gs = gamestate_creer(2, noms);

    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    /* Effacer la pioche et poser manuellement */
    poser_force(gs->plateau, tuile_abbaye(1), cx, cy);
    poser_partisan(gs->plateau, cx, cy, 1, CENTRE); /* joueur ID=1 = Alice */

    int score_avant = gs->joueurs[0].score;
    score_fin_partie(gs);
    int score_apres = gs->joueurs[0].score;

    TEST("Alice gagne 1 pt (abbaye isolée)", score_apres == score_avant + 1);

    gamestate_detruire(gs);
}

void test_fin_partie_route_ouverte(void)
{
    printf("\n>> test_fin_partie_route_ouverte\n");
    /*
     * Scénario :
     *   - Bob a un partisan sur une route de 3 tuiles non terminée.
     *   - En fin de partie → 3 pts pour Bob.
     */
    const char noms[5][32] = {"Alice", "Bob", "", "", ""};
    GameState *gs = gamestate_creer(2, noms);

    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    for (int i = 0; i < 3; i++)
    {
        Tuile r = tuile_homo(i + 10, PLAINE);
        r.bords[EST] = ROUTE;
        r.bords[OUEST] = ROUTE;
        r.centre = ROUTE;
        poser_force(gs->plateau, r, cx + i, cy);
    }
    /* Partisan de Bob (ID=2) sur la première tuile de la route */
    poser_partisan(gs->plateau, cx, cy, 2, OUEST);

    int score_avant = gs->joueurs[1].score;
    score_fin_partie(gs);
    int score_apres = gs->joueurs[1].score;

    TEST("Bob gagne 3 pts (route 3 tuiles)", score_apres == score_avant + 3);

    gamestate_detruire(gs);
}

void test_fin_partie_null(void)
{
    printf("\n>> test_fin_partie_null (robustesse)\n");
    /* Ne doit pas crasher */
    score_fin_partie(NULL);
    TEST("score_fin_partie(NULL) sans crash", 1);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — Robustesse générale
 * ═══════════════════════════════════════════════════════ */

void test_null_guards(void)
{
    printf("\n>> test_null_guards\n");
    TEST("score_ville(NULL)   == 0", score_ville(NULL, 0, 0, 0) == 0);
    TEST("score_route(NULL)   == 0", score_route(NULL, 0, 0, 0) == 0);
    TEST("score_abbaye(NULL)  == 0", score_abbaye(NULL, 0, 0, 0) == 0);
}

void test_hors_bornes(void)
{
    printf("\n>> test_hors_bornes\n");
    Plateau *p = plateau_creer();

    TEST("score_ville  hors bornes == 0",
         score_ville(p, -1, 0, 0) == 0);
    TEST("score_route  hors bornes == 0",
         score_route(p, 0, PLATEAU_TAILLE, 0) == 0);
    TEST("score_abbaye hors bornes == 0",
         score_abbaye(p, PLATEAU_TAILLE, 0, 0) == 0);

    plateau_detruire(p);
}

/* ═══════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════ */
int main(void)
{
    printf("=============================\n");
    printf("  TESTS UNITAIRES : score.c  \n");
    printf("=============================\n");

    /* score_ville */
    test_ville_une_tuile_fermee();
    test_ville_ouverte_en_jeu();
    test_ville_ouverte_fin_partie();
    test_ville_deux_tuiles_fermee();

    /* score_route */
    test_route_une_tuile_terminee();
    test_route_ouverte_en_jeu();
    test_route_ouverte_fin_partie();
    test_route_trois_tuiles_terminee();

    /* score_abbaye */
    test_abbaye_isolee();
    test_abbaye_entouree();
    test_abbaye_4_voisins();
    test_abbaye_pas_abbaye();

    /* score_fin_partie */
    test_fin_partie_abbaye();
    test_fin_partie_route_ouverte();
    test_fin_partie_null();

    /* Robustesse */
    test_null_guards();
    test_hors_bornes();

    printf("\n-----------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("-----------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}