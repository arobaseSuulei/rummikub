#ifndef TUILE_H
#define TUILE_H

#include "_struct.h"
#include "_joueur.h"  

/* Fonctions pour la pioche */
Tuile* initialiser_tuile(void);
Tuile* melanger_tuiles(void);
void creer_pioche(void);
void piocher_tuile(Joueur* j);
void distribuer_tuile(int nb_joueurs, char pseudos[][23]);

/* Fonctions de validation */
bool combinaison_valide(Tuile* tuiles, int nb);

#endif