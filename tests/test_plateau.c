#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/types.h"
#include "../include/plateau.h"

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

/* ─── Helpers ────────────────────────────────────────── */

// Crée une tuile simple avec tous les bords identiques
static Tuile tuile_simple(int id, TypeZone type)
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

/* ─── Tests ──────────────────────────────────────────── */

void test_plateau_creer(void)
{
    printf("\n>> test_plateau_creer\n");
    Plateau *p = plateau_creer();

    TEST("pointeur non NULL", p != NULL);

    // Vérifier que quelques cases sont bien NULL
    TEST("case (0,0) vide", p->grille[0][0] == NULL);
    TEST("case centre vide", p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] == NULL);
    TEST("occupation nulle", p->occupation[0][0] == NULL);

    plateau_detruire(p);
}

void test_plateau_est_libre(void)
{
    printf("\n>> test_plateau_est_libre\n");
    Plateau *p = plateau_creer();

    TEST("case vide est libre", plateau_est_libre(p, CENTRE_PLATEAU, CENTRE_PLATEAU) == 1);
    TEST("hors bornes x négatif", plateau_est_libre(p, -1, 0) == 0);
    TEST("hors bornes y négatif", plateau_est_libre(p, 0, -1) == 0);
    TEST("hors bornes x trop grand", plateau_est_libre(p, PLATEAU_TAILLE, 0) == 0);
    TEST("NULL plateau", plateau_est_libre(NULL, 0, 0) == 0);

    plateau_detruire(p);
}

void test_poser_tuile_depart(void)
{
    printf("\n>> test_poser_tuile_depart\n");
    Plateau *p = plateau_creer();

    // La première tuile est une exception : elle n'a pas de voisin
    // On force la pose directe via grille pour simuler la tuile de départ
    Tuile *depart = malloc(sizeof(Tuile));
    *depart = tuile_simple(1, VILLE);
    p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = depart;

    TEST("tuile posée manuellement",
         p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] != NULL);
    TEST("id correct",
         p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU]->id == 1);

    plateau_detruire(p);
}

void test_poser_tuile_voisin_valide(void)
{
    printf("\n>> test_poser_tuile_voisin_valide\n");
    Plateau *p = plateau_creer();

    // Pose manuelle de la tuile de départ (VILLE partout)
    Tuile *depart = malloc(sizeof(Tuile));
    *depart = tuile_simple(1, VILLE);
    p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = depart;

    // Tuile compatible au SUD (VILLE partout)
    Tuile t2 = tuile_simple(2, VILLE);
    int ret = plateau_poser_tuile(p, &t2, CENTRE_PLATEAU, CENTRE_PLATEAU + 1);

    TEST("pose valide retourne 0", ret == 0);
    TEST("tuile bien présente",
         p->grille[CENTRE_PLATEAU + 1][CENTRE_PLATEAU] != NULL);

    plateau_detruire(p);
}

void test_poser_tuile_bords_incompatibles(void)
{
    printf("\n>> test_poser_tuile_bords_incompatibles\n");
    Plateau *p = plateau_creer();

    // Tuile de départ VILLE
    Tuile *depart = malloc(sizeof(Tuile));
    *depart = tuile_simple(1, VILLE);
    p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = depart;

    // Tuile ROUTE — incompatible avec VILLE
    Tuile t_route = tuile_simple(99, ROUTE);
    int ret = plateau_poser_tuile(p, &t_route, CENTRE_PLATEAU, CENTRE_PLATEAU + 1);

    TEST("pose invalide retourne -1", ret == -1);
    TEST("case reste vide",
         p->grille[CENTRE_PLATEAU + 1][CENTRE_PLATEAU] == NULL);

    plateau_detruire(p);
}

void test_poser_tuile_case_occupee(void)
{
    printf("\n>> test_poser_tuile_case_occupee\n");
    Plateau *p = plateau_creer();

    Tuile *depart = malloc(sizeof(Tuile));
    *depart = tuile_simple(1, VILLE);
    p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = depart;

    // Tenter de poser sur la même case
    Tuile t2 = tuile_simple(2, VILLE);
    int ret = plateau_poser_tuile(p, &t2, CENTRE_PLATEAU, CENTRE_PLATEAU);

    TEST("pose sur case occupée retourne -1", ret == -1);

    plateau_detruire(p);
}

void test_plateau_get_tuile(void)
{
    printf("\n>> test_plateau_get_tuile\n");
    Plateau *p = plateau_creer();

    TEST("case vide retourne NULL",
         plateau_get_tuile(p, CENTRE_PLATEAU, CENTRE_PLATEAU) == NULL);

    Tuile *depart = malloc(sizeof(Tuile));
    *depart = tuile_simple(42, ROUTE);
    p->grille[CENTRE_PLATEAU][CENTRE_PLATEAU] = depart;

    Tuile *lu = plateau_get_tuile(p, CENTRE_PLATEAU, CENTRE_PLATEAU);
    TEST("tuile récupérée non NULL", lu != NULL);
    TEST("id correct", lu->id == 42);
    TEST("hors bornes retourne NULL",
         plateau_get_tuile(p, -1, 0) == NULL);

    plateau_detruire(p);
}

/* ─── Main ───────────────────────────────────────────── */
int main(void)
{
    printf("==============================\n");
    printf("  TESTS UNITAIRES : plateau.c \n");
    printf("==============================\n");

    test_plateau_creer();
    test_plateau_est_libre();
    test_poser_tuile_depart();
    test_poser_tuile_voisin_valide();
    test_poser_tuile_bords_incompatibles();
    test_poser_tuile_case_occupee();
    test_plateau_get_tuile();

    printf("\n------------------------------\n");
    printf("  Résultat : %d OK / %d KO\n", tests_ok, tests_ko);
    printf("------------------------------\n");
    return (tests_ko == 0) ? 0 : 1;
}