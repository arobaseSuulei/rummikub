#ifndef JOUEUR_H
#define JOUEUR_H

#include "_struct.h"

/* Fonctions pour gérer les joueurs */
Joueur* ordre_joueur(Joueur* players, int nb_joueurs);
Joueur* creer_joueur(int nb_joueurs, char pseudos[][23]);
void charger_joueur(Joueur* j);
bool est_premier_tour(Joueur j);
void passer_tour(Joueur* j, Joueur* next);
bool a_fini(Joueur j);

/* Fonctions pour le chevalet */
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles);
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide);

/* Fonction de jeu */
bool jouer_combinaison(Joueur* j, int ids[], int nb_ids);
void ajouter_a_table(Tuile* comb, int n);

/* Scores */
void sauvegarder_scores(Joueur* players, int nb_joueurs, int index_gagnant);

#endif