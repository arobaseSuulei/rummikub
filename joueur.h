#include <stdio.h>
#include <stdbool.h>

#ifndef JOUEUR_H
#define JOUEUR_H


#include "struct.h"

Joueur* creer_joueur(int* nb_joueurs);
Joueur* ordre_joueur(Joueur* players, int nb_joueurs);
int nbr_joueur();
void distribuer_tuile();
void piocher_tuile(Joueur j);




#endif