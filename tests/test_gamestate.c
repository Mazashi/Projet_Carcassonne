#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/plateau.h"
#include "../include/joueur.h"
#include "../include/gamestate.h"

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

/* ─── Helpers ────────────────────────────────────────── */

static const char NOMS[5][32] = {
    "Alice", "Bob", "Carol", "Dan", "Eve"};

/* ─── Tests ──────────────────────────────────────────── */

void test_creer_valide(void)
{
    printf("\n>> test_creer_valide\n");
    GameState *gs = gamestate_creer(2, NOMS);

    TEST("gs non NULL", gs != NULL);
    TEST("plateau non NULL", gs->plateau != NULL);
    TEST("nbJoueurs == 2", gs->nbJoueurs == 2);
    TEST("tourActuel == 0", gs->tourActuel == 0);
    TEST("index pioche == 0", gs->indexProchaineTuile == 0);
    TEST("joueur 0 nom correct", strcmp(gs->joueurs[0].nom, "Alice") == 0);
    TEST("joueur 1 nom correct", strcmp(gs->joueurs[1].nom, "Bob") == 0);
    TEST("partisans initialisés", gs->joueurs[0].partisansRestants == NB_PARTISANS_MAX);

    gamestate_detruire(gs);
}

void test_creer_invalide(void)
{
    printf("\n>> test_creer_invalide (robustesse)\n");
    TEST("1 joueur retourne NULL", gamestate_creer(1, NOMS) == NULL);
    TEST("6 joueurs retourne NULL", gamestate_creer(6, NOMS) == NULL);
    TEST("noms NULL retourne NULL", gamestate_creer(2, NULL) == NULL);
}

void test_pioche_complete(void)
{
    printf("\n>> test_pioche_complete\n");
    GameState *gs = gamestate_creer(2, NOMS);

    /* Toutes les tuiles doivent avoir un ID dans [1..72] */
    int tous_valides = 1;
    for (int i = 0; i < TOTAL_TUILES; i++)
        if (gs->pioche[i].id < 1 || gs->pioche[i].id > 72)
            tous_valides = 0;
    TEST("72 tuiles valides dans la pioche", tous_valides);

    /* IDs uniques : somme de Gauss = 72*73/2 = 2628 */
    int somme = 0;
    for (int i = 0; i < TOTAL_TUILES; i++)
        somme += gs->pioche[i].id;
    TEST("somme des IDs == 2628 (unicité)", somme == 2628);

    gamestate_detruire(gs);
}

void test_melange(void)
{
    printf("\n>> test_melange (Fisher-Yates)\n");
    GameState *gs = gamestate_creer(2, NOMS);

    /* Capture l'ordre initial */
    int ordre_initial[TOTAL_TUILES];
    for (int i = 0; i < TOTAL_TUILES; i++)
        ordre_initial[i] = gs->pioche[i].id;

    gamestate_melanger_pioche(gs);

    int identique = 1;
    for (int i = 0; i < TOTAL_TUILES; i++)
        if (gs->pioche[i].id != ordre_initial[i])
        {
            identique = 0;
            break;
        }

    /* Probabilité d'ordre identique après mélange : 1/72! ≈ 0 */
    TEST("ordre modifié après mélange", identique == 0);

    /* L'intégrité des données doit être conservée */
    int somme = 0;
    for (int i = 0; i < TOTAL_TUILES; i++)
        somme += gs->pioche[i].id;
    TEST("somme conservée après mélange", somme == 2628);

    gamestate_detruire(gs);
}

void test_piocher(void)
{
    printf("\n>> test_piocher\n");
    GameState *gs = gamestate_creer(2, NOMS);

    Tuile *t = gamestate_piocher(gs);
    TEST("première tuile non NULL", t != NULL);
    TEST("index avancé à 1", gs->indexProchaineTuile == 1);

    /* Vider toute la pioche */
    for (int i = 1; i < TOTAL_TUILES; i++)
        gamestate_piocher(gs);

    TEST("pioche vide détectée", gamestate_pioche_vide(gs) == 1);
    TEST("piocher sur vide retourne NULL", gamestate_piocher(gs) == NULL);

    gamestate_detruire(gs);
}

void test_tours(void)
{
    printf("\n>> test_tours\n");
    GameState *gs = gamestate_creer(3, NOMS);

    Joueur *j = gamestate_joueur_courant(gs);
    TEST("joueur courant initial == joueur 0", j == &gs->joueurs[0]);

    gamestate_tour_suivant(gs);
    TEST("tour 1 → joueur 1", gs->tourActuel == 1);

    gamestate_tour_suivant(gs);
    TEST("tour 2 → joueur 2", gs->tourActuel == 2);

    gamestate_tour_suivant(gs);
    TEST("tour 3 → retour joueur 0 (rotation)", gs->tourActuel == 0);

    gamestate_detruire(gs);
}

void test_null_guard(void)
{
    printf("\n>> test_null_guard\n");
    TEST("piocher(NULL) == NULL", gamestate_piocher(NULL) == NULL);
    TEST("pioche_vide(NULL) == 1", gamestate_pioche_vide(NULL) == 1);
    TEST("joueur_courant(NULL) == NULL", gamestate_joueur_courant(NULL) == NULL);
    gamestate_tour_suivant(NULL);  /* ne doit pas crasher */
    gamestate_detruire(NULL);      /* ne doit pas crasher */
    gamestate_afficher_etat(NULL); /* ne doit pas crasher */
    TEST("fonctions NULL sans crash", 1);
}

/* ─── Main ───────────────────────────────────────────── */
int main(void)
{
    printf("================================\n");
    printf("  TESTS UNITAIRES : gamestate.c \n");
    printf("================================\n");

    test_creer_valide();
    test_creer_invalide();
    test_pioche_complete();
    test_melange();
    test_piocher();
    test_tours();
    test_null_guard();

    printf("\n--------------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("--------------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}