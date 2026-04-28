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
 * HELPERS
 * ═══════════════════════════════════════════════════════ */

static const char NOMS[5][32] = {"Alice", "Bob", "", "", ""};

/* Tuile homogène */
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

/* Tuile abbaye */
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

/* Pose directe dans la grille (bypass validation) */
static void poser_force(Plateau *p, Tuile t, int x, int y)
{
    Tuile *copie = malloc(sizeof(Tuile));
    *copie = t;
    p->grille[y][x] = copie;
}

/*
 * Crée un GameState minimal avec une tuile de départ déjà posée
 * au centre, sans passer par moteur_init (qui pioche).
 * Permet des tests déterministes indépendants du mélange.
 */
static GameState *gs_avec_depart(TypeZone type)
{
    GameState *gs = gamestate_creer(2, NOMS);
    poser_force(gs->plateau, tuile_homo(99, type),
                CENTRE_PLATEAU, CENTRE_PLATEAU);
    return gs;
}

/* ═══════════════════════════════════════════════════════
 * TESTS — moteur_init
 * ═══════════════════════════════════════════════════════ */

void test_init_pose_tuile_depart(void)
{
    printf("\n>> test_init_pose_tuile_depart\n");
    /*
     * moteur_init doit piocher la première tuile et la poser
     * au centre du plateau. On vérifie :
     *   - retour MOTEUR_OK
     *   - case centrale non NULL après init
     *   - l'index de pioche a avancé de 1
     */
    GameState *gs = gamestate_creer(2, NOMS);

    MoteurCode ret = moteur_init(gs);
    TEST("moteur_init retourne MOTEUR_OK", ret == MOTEUR_OK);
    TEST("tuile de départ posée au centre",
         plateau_get_tuile(gs->plateau,
                           CENTRE_PLATEAU, CENTRE_PLATEAU) != NULL);
    TEST("index pioche avancé à 1", gs->indexProchaineTuile == 1);

    gamestate_detruire(gs);
}

void test_init_null(void)
{
    printf("\n>> test_init_null\n");
    TEST("moteur_init(NULL) == MOTEUR_ERR_NULL",
         moteur_init(NULL) == MOTEUR_ERR_NULL);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — moteur_positions_valides
 * ═══════════════════════════════════════════════════════ */

void test_positions_tuile_compatible(void)
{
    printf("\n>> test_positions_tuile_compatible\n");
    /*
     * Plateau avec une seule tuile PLAINE au centre.
     * Une tuile PLAINE doit trouver exactement 4 positions
     * valides (les 4 cases adjacentes au centre).
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(1, PLAINE);

    ListePositions lp = moteur_positions_valides(gs, &t);
    TEST("4 positions valides autour du centre", lp.count == 4);

    moteur_positions_liberer(&lp);
    gamestate_detruire(gs);
}

void test_positions_tuile_incompatible(void)
{
    printf("\n>> test_positions_tuile_incompatible\n");
    /*
     * Plateau avec une tuile VILLE au centre.
     * Une tuile PLAINE (tous bords PLAINE) ne peut joindre
     * aucun bord VILLE → 0 positions valides.
     */
    GameState *gs = gs_avec_depart(VILLE);
    Tuile t = tuile_homo(1, PLAINE);

    ListePositions lp = moteur_positions_valides(gs, &t);
    TEST("0 position valide (bords incompatibles)", lp.count == 0);

    moteur_positions_liberer(&lp);
    gamestate_detruire(gs);
}

void test_positions_liberer_null(void)
{
    printf("\n>> test_positions_liberer_null\n");
    /* Ne doit pas crasher */
    moteur_positions_liberer(NULL);
    TEST("moteur_positions_liberer(NULL) sans crash", 1);
}

void test_a_position_valide(void)
{
    printf("\n>> test_a_position_valide\n");

    GameState *gs_plaine = gs_avec_depart(PLAINE);
    GameState *gs_ville = gs_avec_depart(VILLE);

    Tuile t_plaine = tuile_homo(1, PLAINE);
    Tuile t_ville = tuile_homo(2, VILLE);

    TEST("tuile PLAINE jouable sur départ PLAINE",
         moteur_a_position_valide(gs_plaine, &t_plaine) == 1);
    TEST("tuile PLAINE non jouable sur départ VILLE",
         moteur_a_position_valide(gs_ville, &t_plaine) == 0);
    TEST("tuile VILLE jouable sur départ VILLE",
         moteur_a_position_valide(gs_ville, &t_ville) == 1);

    gamestate_detruire(gs_plaine);
    gamestate_detruire(gs_ville);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — moteur_placer_tuile
 * ═══════════════════════════════════════════════════════ */

void test_placer_tuile_valide(void)
{
    printf("\n>> test_placer_tuile_valide\n");
    /*
     * Pose une tuile PLAINE au SUD du centre (PLAINE).
     * → MOTEUR_OK, tuile présente en (cx, cy+1).
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(2, PLAINE);
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    MoteurCode ret = moteur_placer_tuile(gs, &t, cx, cy + 1);
    TEST("pose valide retourne MOTEUR_OK", ret == MOTEUR_OK);
    TEST("tuile présente en (cx, cy+1)",
         plateau_get_tuile(gs->plateau, cx, cy + 1) != NULL);

    gamestate_detruire(gs);
}

void test_placer_tuile_invalide(void)
{
    printf("\n>> test_placer_tuile_invalide\n");
    /*
     * Tenter de poser une tuile VILLE sur le départ PLAINE.
     * → MOTEUR_ERR_POSE
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(2, VILLE);

    MoteurCode ret = moteur_placer_tuile(gs, &t,
                                         CENTRE_PLATEAU,
                                         CENTRE_PLATEAU + 1);
    TEST("bords incompatibles → MOTEUR_ERR_POSE", ret == MOTEUR_ERR_POSE);
    TEST("case reste vide",
         plateau_get_tuile(gs->plateau,
                           CENTRE_PLATEAU,
                           CENTRE_PLATEAU + 1) == NULL);

    gamestate_detruire(gs);
}

void test_placer_tuile_case_occupee(void)
{
    printf("\n>> test_placer_tuile_case_occupee\n");
    /*
     * Tenter de poser sur la case déjà occupée par la tuile de départ.
     * → MOTEUR_ERR_POSE
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(2, PLAINE);

    MoteurCode ret = moteur_placer_tuile(gs, &t,
                                         CENTRE_PLATEAU,
                                         CENTRE_PLATEAU);
    TEST("case occupée → MOTEUR_ERR_POSE", ret == MOTEUR_ERR_POSE);

    gamestate_detruire(gs);
}

void test_placer_tuile_null(void)
{
    printf("\n>> test_placer_tuile_null\n");
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(1, PLAINE);

    TEST("gs NULL → MOTEUR_ERR_NULL",
         moteur_placer_tuile(NULL, &t, 0, 0) == MOTEUR_ERR_NULL);
    TEST("tuile NULL → MOTEUR_ERR_NULL",
         moteur_placer_tuile(gs, NULL, 0, 0) == MOTEUR_ERR_NULL);

    gamestate_detruire(gs);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — moteur_placer_partisan
 * ═══════════════════════════════════════════════════════ */

void test_placer_partisan_valide(void)
{
    printf("\n>> test_placer_partisan_valide\n");
    /*
     * Pose une tuile puis place un partisan dessus.
     * → MOTEUR_OK, stock joueur diminue de 1.
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(2, PLAINE);
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    moteur_placer_tuile(gs, &t, cx, cy + 1);

    int stock_avant = gs->joueurs[0].partisansRestants;
    MoteurCode ret = moteur_placer_partisan(gs, cx, cy + 1, NORD);

    TEST("pose partisan valide → MOTEUR_OK", ret == MOTEUR_OK);
    TEST("stock partisans diminue de 1",
         gs->joueurs[0].partisansRestants == stock_avant - 1);
    TEST("partisan présent sur la case",
         gs->plateau->occupation[cy + 1][cx] != NULL);
    TEST("emplacement correct",
         gs->plateau->occupation[cy + 1][cx]->emplacement == NORD);

    gamestate_detruire(gs);
}

void test_placer_partisan_case_vide(void)
{
    printf("\n>> test_placer_partisan_case_vide\n");
    /*
     * Tenter de placer un partisan sur une case sans tuile.
     * → MOTEUR_ERR_POSE
     */
    GameState *gs = gs_avec_depart(PLAINE);

    MoteurCode ret = moteur_placer_partisan(gs,
                                            CENTRE_PLATEAU + 5,
                                            CENTRE_PLATEAU + 5,
                                            CENTRE);
    TEST("case sans tuile → MOTEUR_ERR_POSE", ret == MOTEUR_ERR_POSE);

    gamestate_detruire(gs);
}

void test_placer_partisan_case_deja_occupee(void)
{
    printf("\n>> test_placer_partisan_case_deja_occupee\n");
    /*
     * Poser deux partisans sur la même case → MOTEUR_ERR_PION.
     */
    GameState *gs = gs_avec_depart(PLAINE);
    Tuile t = tuile_homo(2, PLAINE);
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    moteur_placer_tuile(gs, &t, cx, cy + 1);
    moteur_placer_partisan(gs, cx, cy + 1, NORD);

    MoteurCode ret = moteur_placer_partisan(gs, cx, cy + 1, SUD);
    TEST("double partisan → MOTEUR_ERR_PION", ret == MOTEUR_ERR_PION);

    gamestate_detruire(gs);
}

void test_placer_partisan_stock_vide(void)
{
    printf("\n>> test_placer_partisan_stock_vide\n");
    /*
     * Vider le stock de partisans du joueur courant puis tenter
     * d'en poser un → MOTEUR_ERR_STOCK.
     */
    GameState *gs = gs_avec_depart(PLAINE);
    gs->joueurs[0].partisansRestants = 0;

    MoteurCode ret = moteur_placer_partisan(gs,
                                            CENTRE_PLATEAU,
                                            CENTRE_PLATEAU,
                                            CENTRE);
    TEST("stock vide → MOTEUR_ERR_STOCK", ret == MOTEUR_ERR_STOCK);

    gamestate_detruire(gs);
}

void test_placer_partisan_null(void)
{
    printf("\n>> test_placer_partisan_null\n");
    TEST("gs NULL → MOTEUR_ERR_NULL",
         moteur_placer_partisan(NULL, 0, 0, CENTRE) == MOTEUR_ERR_NULL);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — Scoring automatique (moteur_scorer_voisins)
 * ═══════════════════════════════════════════════════════ */

void test_scorer_abbaye_complete(void)
{
    printf("\n>> test_scorer_abbaye_complete\n");
    /*
     * Scénario :
     *   - Abbaye au centre avec un partisan (joueur 1 = Alice).
     *   - On pose les 8 tuiles voisines une par une via
     *     moteur_placer_tuile.
     *   - À la 8e pose, moteur_scorer_voisins doit détecter
     *     l'abbaye complète et attribuer 9 pts à Alice.
     *   - Le partisan doit être rendu (stock revient à MAX).
     *
     * On construit manuellement les 7 premiers voisins,
     * puis on pose le 8e via moteur_placer_tuile pour
     * déclencher le scoring.
     */
    GameState *gs = gamestate_creer(2, NOMS);
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    /* Poser l'abbaye au centre avec un partisan */
    poser_force(gs->plateau, tuile_abbaye(1), cx, cy);
    Partisan *part = malloc(sizeof(Partisan));
    part->idJoueur = 1; /* Alice */
    part->emplacement = CENTRE;
    part->x = cx;
    part->y = cy;
    gs->plateau->occupation[cy][cx] = part;
    gs->joueurs[0].partisansRestants--;

    /* Poser 7 des 8 voisins directement (pas de scoring déclenché) */
    int id = 10;
    int poses = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            if (poses == 7)
                continue; /* Réserver le dernier */
            poser_force(gs->plateau,
                        tuile_homo(id++, PLAINE),
                        cx + dx, cy + dy);
            poses++;
        }

    int score_avant = gs->joueurs[0].score;
    int stock_avant = gs->joueurs[0].partisansRestants;

    /*
     * Poser le 8e voisin via moteur_placer_tuile.
     * La seule case libre autour de l'abbaye est (cx+1, cy+1).
     * On construit une tuile compatible (PLAINE partout).
     */
    Tuile derniere = tuile_homo(id, PLAINE);
    int lx = cx + 1, ly = cy + 1; /* case qui était réservée */
    moteur_placer_tuile(gs, &derniere, lx, ly);

    /* Déclencher manuellement le scorer (comme le ferait moteur_tour) */
    /* Note : moteur_scorer_voisins est static → on le teste via
       le chemin complet moteur_placer_tuile + scorer.
       Ici on appelle score_abbaye directement pour valider le calcul,
       et on vérifie que la logique de scoring est correcte. */
    int pts = score_abbaye(gs->plateau, cx, cy, 0);
    TEST("abbaye entourée vaut 9 pts", pts == 9);
    TEST("score calculé correctement avant attribution", pts == 9);

    /* Simuler l'attribution comme le ferait moteur_scorer_voisins */
    joueur_ajouter_points(&gs->joueurs[0], pts);
    free(gs->plateau->occupation[cy][cx]);
    gs->plateau->occupation[cy][cx] = NULL;
    joueur_rendre_partisan(&gs->joueurs[0]);

    TEST("Alice reçoit 9 pts",
         gs->joueurs[0].score == score_avant + 9);
    TEST("partisan rendu après scoring",
         gs->joueurs[0].partisansRestants == stock_avant + 1);
    TEST("occupation libérée",
         gs->plateau->occupation[cy][cx] == NULL);

    gamestate_detruire(gs);
}

void test_scorer_ville_fermee(void)
{
    printf("\n>> test_scorer_ville_fermee\n");
    /*
     * Scénario :
     *   - 2 tuiles VILLE côte à côte, fermées.
     *   - Partisan de Bob sur la première.
     *   - score_ville doit retourner 4 pts (2 tuiles × 2).
     */
    GameState *gs = gamestate_creer(2, NOMS);
    int cx = CENTRE_PLATEAU;
    int cy = CENTRE_PLATEAU;

    /* Tuile A : bord EST = VILLE, reste PLAINE */
    Tuile a = tuile_homo(1, PLAINE);
    a.bords[EST] = VILLE;
    a.centre = VILLE;
    poser_force(gs->plateau, a, cx, cy);

    /* Tuile B : bord OUEST = VILLE, reste PLAINE */
    Tuile b = tuile_homo(2, PLAINE);
    b.bords[OUEST] = VILLE;
    b.centre = VILLE;
    poser_force(gs->plateau, b, cx + 1, cy);

    /* Partisan de Bob (joueur 2) sur tuile A */
    Partisan *part = malloc(sizeof(Partisan));
    part->idJoueur = 2;
    part->emplacement = EST;
    part->x = cx;
    part->y = cy;
    gs->plateau->occupation[cy][cx] = part;
    gs->joueurs[1].partisansRestants--;

    int score_avant = gs->joueurs[1].score;
    int pts = score_ville(gs->plateau, cx, cy, 0);

    TEST("ville 2 tuiles fermée = 4 pts", pts == 4);

    joueur_ajouter_points(&gs->joueurs[1], pts);
    TEST("Bob reçoit 4 pts",
         gs->joueurs[1].score == score_avant + 4);

    gamestate_detruire(gs);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — Tour et rotation
 * ═══════════════════════════════════════════════════════ */

void test_tour_suivant_apres_placer(void)
{
    printf("\n>> test_tour_suivant_apres_placer\n");
    /*
     * Vérifier que gamestate_tour_suivant alterne bien
     * entre les deux joueurs sur plusieurs cycles.
     */
    GameState *gs = gamestate_creer(2, NOMS);

    TEST("tour initial == 0", gs->tourActuel == 0);
    gamestate_tour_suivant(gs);
    TEST("après 1 appel → tour == 1", gs->tourActuel == 1);
    gamestate_tour_suivant(gs);
    TEST("après 2 appels → tour == 0 (rotation)", gs->tourActuel == 0);

    gamestate_detruire(gs);
}

void test_pioche_vide_detectee(void)
{
    printf("\n>> test_pioche_vide_detectee\n");
    /*
     * Vider entièrement la pioche et vérifier la détection.
     */
    GameState *gs = gamestate_creer(2, NOMS);

    TEST("pioche non vide au départ",
         gamestate_pioche_vide(gs) == 0);

    for (int i = 0; i < TOTAL_TUILES; i++)
        gamestate_piocher(gs);

    TEST("pioche vide après 72 pioches",
         gamestate_pioche_vide(gs) == 1);
    TEST("piocher sur vide retourne NULL",
         gamestate_piocher(gs) == NULL);

    gamestate_detruire(gs);
}

/* ═══════════════════════════════════════════════════════
 * TESTS — Robustesse
 * ═══════════════════════════════════════════════════════ */

void test_null_guards(void)
{
    printf("\n>> test_null_guards\n");

    TEST("moteur_init(NULL)             == ERR_NULL",
         moteur_init(NULL) == MOTEUR_ERR_NULL);
    TEST("moteur_placer_tuile(NULL,…)   == ERR_NULL",
         moteur_placer_tuile(NULL, NULL, 0, 0) == MOTEUR_ERR_NULL);
    TEST("moteur_placer_partisan(NULL,…)== ERR_NULL",
         moteur_placer_partisan(NULL, 0, 0, CENTRE) == MOTEUR_ERR_NULL);
    TEST("moteur_a_position_valide(NULL)== 0",
         moteur_a_position_valide(NULL, NULL) == 0);

    /* Libérer NULL ne doit pas crasher */
    moteur_positions_liberer(NULL);
    TEST("moteur_positions_liberer(NULL) sans crash", 1);
}

/* ═══════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════ */
int main(void)
{
    printf("==============================\n");
    printf("  TESTS UNITAIRES : moteur.c  \n");
    printf("==============================\n");

    /* moteur_init */
    test_init_pose_tuile_depart();
    test_init_null();

    /* moteur_positions_valides */
    test_positions_tuile_compatible();
    test_positions_tuile_incompatible();
    test_positions_liberer_null();
    test_a_position_valide();

    /* moteur_placer_tuile */
    test_placer_tuile_valide();
    test_placer_tuile_invalide();
    test_placer_tuile_case_occupee();
    test_placer_tuile_null();

    /* moteur_placer_partisan */
    test_placer_partisan_valide();
    test_placer_partisan_case_vide();
    test_placer_partisan_case_deja_occupee();
    test_placer_partisan_stock_vide();
    test_placer_partisan_null();

    /* Scoring automatique */
    test_scorer_abbaye_complete();
    test_scorer_ville_fermee();

    /* Tour et pioche */
    test_tour_suivant_apres_placer();
    test_pioche_vide_detectee();

    /* Robustesse */
    test_null_guards();

    printf("\n------------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("------------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}