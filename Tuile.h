#ifndef TUILE_H
#define TUILE_H

#include "struct.h"

void creer_pioche(void);
void piocher_tuile(Joueur* j);
bool combinaison_valide(Tuile* comb, int n);
void ajouter_a_table(Tuile* comb, int n);
void afficher_tuiles(Tuile tuiles[], int nb_tuiles);
void afficher_table(const char* fichier_table);
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles);
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide);

#endif