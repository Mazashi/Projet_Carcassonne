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
        return -1;

    char line[100];
    int count = 0;
    while (fgets(line, sizeof(line), file) && count < 72)
    {
        if (line[0] == '#' || line[0] == '\n')
            continue; // Sauter commentaires et vides

        sscanf(line, "%d|%d|%d|%d|%d|%d|%d",
               &pioche[count].id,
               (int *)&pioche[count].bords[0],
               (int *)&pioche[count].bords[1],
               (int *)&pioche[count].bords[2],
               (int *)&pioche[count].bords[3],
               (int *)&pioche[count].centre,
               &pioche[count].nbBlasons);

        pioche[count].rotation = 0; // Initialisation par défaut
        count++;
    }

    fclose(file);
    return 0;
}