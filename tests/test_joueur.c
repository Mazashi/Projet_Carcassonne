#include <stdio.h>
#include <string.h>
#include "../include/types.h"
#include "../include/joueur.h"

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

/* ─── Tests ──────────────────────────────────────────── */

void test_init(void)
{
    printf("\n>> test_joueur_init\n");
    Joueur j;
    joueur_init(&j, 1, "Alice");

    TEST("id == 1", j.id == 1);
    TEST("score == 0", j.score == 0);
    TEST("partisans == MAX", j.partisansRestants == NB_PARTISANS_MAX);
    TEST("nom correct", strcmp(j.nom, "Alice") == 0);
}

void test_init_null(void)
{
    printf("\n>> test_init_null (robustesse)\n");
    /* Ne doit pas crasher */
    joueur_init(NULL, 1, "X");
    joueur_init(NULL, 1, NULL);
    TEST("init NULL sans crash", 1);
}

void test_nom_trop_long(void)
{
    printf("\n>> test_nom_trop_long\n");
    Joueur j;
    joueur_init(&j, 2, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"); /* > 32 */
    TEST("pas de débordement buffer", j.nom[31] == '\0');
}

void test_partisan(void)
{
    printf("\n>> test_partisan\n");
    Joueur j;
    joueur_init(&j, 1, "Bob");

    TEST("a partisan au départ", joueur_a_partisan(&j) == 1);

    int ret = joueur_retirer_partisan(&j);
    TEST("retrait retourne 0", ret == 0);
    TEST("stock diminue", j.partisansRestants == NB_PARTISANS_MAX - 1);

    joueur_rendre_partisan(&j);
    TEST("rendu restitue le stock", j.partisansRestants == NB_PARTISANS_MAX);
}

void test_partisan_vide(void)
{
    printf("\n>> test_partisan_vide\n");
    Joueur j;
    joueur_init(&j, 1, "Carol");

    /* Vider tous les partisans */
    for (int i = 0; i < NB_PARTISANS_MAX; i++)
        joueur_retirer_partisan(&j);

    TEST("stock à 0", j.partisansRestants == 0);
    TEST("plus de partisan", joueur_a_partisan(&j) == 0);
    TEST("retrait sur vide retourne -1", joueur_retirer_partisan(&j) == -1);
}

void test_rendre_plafond(void)
{
    printf("\n>> test_rendre_plafond\n");
    Joueur j;
    joueur_init(&j, 1, "Dan");

    /* Rendre sans jamais avoir retiré → ne doit pas dépasser MAX */
    joueur_rendre_partisan(&j);
    TEST("plafond respecté", j.partisansRestants == NB_PARTISANS_MAX);
}

void test_score(void)
{
    printf("\n>> test_score\n");
    Joueur j;
    joueur_init(&j, 1, "Eve");

    joueur_ajouter_points(&j, 10);
    TEST("score == 10", j.score == 10);

    joueur_ajouter_points(&j, 5);
    TEST("score == 15", j.score == 15);

    /* Score ne descend pas sous 0 */
    joueur_ajouter_points(&j, -100);
    TEST("score plancher à 0", j.score == 0);
}

void test_null_guard(void)
{
    printf("\n>> test_null_guard\n");
    TEST("a_partisan(NULL) == 0", joueur_a_partisan(NULL) == 0);
    TEST("retirer(NULL) == -1", joueur_retirer_partisan(NULL) == -1);
    /* Les fonctions void ne doivent pas crasher */
    joueur_rendre_partisan(NULL);
    joueur_ajouter_points(NULL, 5);
    joueur_afficher(NULL);
    TEST("fonctions void NULL sans crash", 1);
}

/* ─── Main ───────────────────────────────────────────── */
int main(void)
{
    printf("==============================\n");
    printf("  TESTS UNITAIRES : joueur.c  \n");
    printf("==============================\n");

    test_init();
    test_init_null();
    test_nom_trop_long();
    test_partisan();
    test_partisan_vide();
    test_rendre_plafond();
    test_score();
    test_null_guard();

    printf("\n------------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("------------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}