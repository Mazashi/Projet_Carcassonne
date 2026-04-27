#ifndef PLATEAU_H
#define PLATEAU_H

#include "types.h"

Plateau *plateau_creer(void);
void plateau_detruire(Plateau *p);
int plateau_est_libre(Plateau *p, int x, int y);
int plateau_est_valide(Plateau *p, Tuile *t, int x, int y);
int plateau_poser_tuile(Plateau *p, Tuile *t, int x, int y);
Tuile *plateau_get_tuile(Plateau *p, int x, int y);

#endif