// les guards --> permettent la définition qu'une seule fois


#ifndef TUILE_H
#define TUILE_H

#include "struct.h"

void initialiser_tuile(Tuile t[], int *nb_tuiles);
void stocker_tuile(Tuile t[], int nb_tuiles, const char *filename);
void melanger_tuiles(Tuile t[], int nb_tuiles);
void creer_pioche();

#endif


