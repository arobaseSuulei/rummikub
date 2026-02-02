#ifndef TUILE_H
#define TUILE_H

#include "struct.h"
#include "joueur.h"  

/* Fonctions pour la pioche */
Tuile* initialiser_tuile(void);
Tuile* melanger_tuiles(void);
void creer_pioche(void);
void piocher_tuile(Joueur* j);
void distribuer_tuile(void);

/* Fonctions de validation et affichage */
bool combinaison_valide(Tuile* tuiles, int nb);
void afficher_tuiles(Tuile tuiles[], int nb_tuiles);

#endif