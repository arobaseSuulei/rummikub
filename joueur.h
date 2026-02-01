#ifndef JOUEUR_H
#define JOUEUR_H

#include "struct.h"

Joueur* creer_joueur(int* nb_joueurs);
void charger_joueur(Joueur* j);
void jouer_combinaison(Joueur* j);
bool est_premier_tour(Joueur j);


#endif