// les guards --> permettent la définition qu'une seule fois
#ifndef TUILE_H 
#define TUILE_H

#include "struct.h"
#include "Tuile.c"




void initialiser_tuile(Tuile t[],int *nb_tuiles);
void afficher_tuiles(Tuile t[],int nb_tuiles);

#endif