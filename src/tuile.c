#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
/**
 * Charge les 72 tuiles depuis le fichier texte.
 * @param pioche : tableau de 72 tuiles à remplir.
 * @return : 0 en cas de succès, -1 en cas d'erreur d'ouverture.
 */
int charger_tuiles(Tuile pioche[72])
{
    FILE *file = fopen("data/tuiles.txt", "r");
    if (file == NULL)
    {
        printf("Erreur : Impossible d'ouvrir le fichier tuiles.txt\n");
        return -1;
    }

    char line[100];
    int count = 0;
    while (fgets(line, sizeof(line), file) && count < 72)
    {
        if (line[0] == '#' || line[0] == '\n')
            continue; // Sauter commentaires et vides

        int n, e, s, o, centre;
        sscanf(line, "%d|%d|%d|%d|%d|%d|%d", &pioche[count].id, &n, &e, &s, &o, &centre, &pioche[count].nbBlasons);
        if (n < 0 || n > 3 || e < 0 || e > 3 ||
            s < 0 || s > 3 || o < 0 || o > 3 || centre < 0 || centre > 3)
        {
            fprintf(stderr, "Valeur invalide ligne %d\n", count + 1);
            fclose(file);
            return -1;
        }
        pioche[count].bords[NORD] = (TypeZone)n;
        pioche[count].bords[EST] = (TypeZone)e;
        pioche[count].bords[SUD] = (TypeZone)s;
        pioche[count].bords[OUEST] = (TypeZone)o;
        pioche[count].centre = (TypeZone)centre;

        pioche[count].rotation = 0; // Initialisation par défaut
        count++;
    }

    fclose(file);
    return 0;
}