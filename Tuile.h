#ifndef TUILE_H
#define TUILE_H

#include "struct.h"
#include "joueur.h"  // Pour Joueur*

/* Fonctions pour la pioche */
Tuile* initialiser_tuile(void);
Tuile* melanger_tuiles(void);
void creer_pioche(void);
void piocher_tuile(Joueur* j);
void distribuer_tuile(void);

/* Fonctions de validation et affichage */
bool combinaison_valide(Tuile* tuiles, int nb);
void afficher_tuiles(Tuile tuiles[], int nb_tuiles);

/* Fonctions pour la table */
void ajouter_a_table(Tuile* comb, int n);

/* Fonctions de chargement/sauvegarde */
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles);
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide);

#endif