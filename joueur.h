#ifndef JOUEUR_H
#define JOUEUR_H

#include "struct.h"

/* Fonctions pour gérer les joueurs */
int nbr_joueur(void);
Joueur* ordre_joueur(Joueur* players, int nb_joueurs);
Joueur* creer_joueur(int* nb_joueurs);
void charger_joueur(Joueur* j);
bool est_premier_tour(Joueur j);
void passer_tour(Joueur* j, Joueur* next);

/* Fonctions pour le chevalet */
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles);
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide);

/* Fonction de jeu */
void jouer_combinaison(Joueur* j);
void ajouter_a_table(Tuile* comb, int n);

#endif