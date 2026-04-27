#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/types.h"

// Prototype
int charger_tuiles(Tuile pioche[72]);

/* ─── Utilitaire ─────────────────────────────────────── */
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

void test_chargement_succes(void)
{
    printf("\n>> test_chargement_succes\n");
    Tuile pioche[72];
    int ret = charger_tuiles(pioche);
    TEST("retour == 0", ret == 0);
}

void test_nombre_tuiles(void)
{
    printf("\n>> test_nombre_tuiles\n");
    Tuile pioche[72];
    charger_tuiles(pioche);

    // On compte les IDs valides (1 à 72)
    int count = 0;
    for (int i = 0; i < 72; i++)
        if (pioche[i].id >= 1 && pioche[i].id <= 72)
            count++;

    TEST("72 tuiles chargées", count == 72);
}

void test_tuile_1(void)
{
    printf("\n>> test_tuile_1 (toute VILLE, 1 blason)\n");
    Tuile pioche[72];
    charger_tuiles(pioche);
    Tuile t = pioche[0]; // ID 1

    TEST("id == 1", t.id == 1);
    TEST("NORD == VILLE", t.bords[NORD] == VILLE);
    TEST("EST  == VILLE", t.bords[EST] == VILLE);
    TEST("SUD  == VILLE", t.bords[SUD] == VILLE);
    TEST("OUEST== VILLE", t.bords[OUEST] == VILLE);
    TEST("centre == VILLE", t.centre == VILLE);
    TEST("nbBlasons == 1", t.nbBlasons == 1);
    TEST("rotation == 0", t.rotation == 0);
}

void test_tuile_abbaye(void)
{
    printf("\n>> test_tuile_abbaye (ID 63)\n");
    Tuile pioche[72];
    charger_tuiles(pioche);
    Tuile t = pioche[62]; // ID 63

    TEST("id == 63", t.id == 63);
    TEST("centre == ABBAYE", t.centre == ABBAYE);
    TEST("NORD == PLAINE", t.bords[NORD] == PLAINE);
    TEST("EST  == PLAINE", t.bords[EST] == PLAINE);
    TEST("SUD  == PLAINE", t.bords[SUD] == PLAINE);
    TEST("OUEST== PLAINE", t.bords[OUEST] == PLAINE);
    TEST("nbBlasons == 0", t.nbBlasons == 0);
}

void test_tuile_abbaye_route(void)
{
    printf("\n>> test_tuile_abbaye_route (ID 67)\n");
    Tuile pioche[72];
    charger_tuiles(pioche);
    Tuile t = pioche[66]; // ID 67

    TEST("id == 67", t.id == 67);
    TEST("centre == ABBAYE", t.centre == ABBAYE);
    TEST("SUD == ROUTE", t.bords[SUD] == ROUTE);
    TEST("NORD == PLAINE", t.bords[NORD] == PLAINE);
}

void test_valeurs_bornes(void)
{
    printf("\n>> test_valeurs_bornes (toutes tuiles)\n");
    Tuile pioche[72];
    charger_tuiles(pioche);

    int tous_valides = 1;
    for (int i = 0; i < 72; i++)
    {
        for (int d = 0; d < 4; d++)
        {
            if (pioche[i].bords[d] < PLAINE || pioche[i].bords[d] > ABBAYE)
                tous_valides = 0;
        }
        if (pioche[i].centre < PLAINE || pioche[i].centre > ABBAYE)
            tous_valides = 0;
    }
    TEST("toutes les zones sont dans [PLAINE..ABBAYE]", tous_valides);
}

/* ─── Main ───────────────────────────────────────────── */
int main(void)
{
    printf("============================\n");
    printf("  TESTS UNITAIRES : tuile.c \n");
    printf("============================\n");

    test_chargement_succes();
    test_nombre_tuiles();
    test_tuile_1();
    test_tuile_abbaye();
    test_tuile_abbaye_route();
    test_valeurs_bornes();

    printf("\n----------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("----------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}