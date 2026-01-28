#ifndef JOUEUR_H
#define JOUEUR_H

#include <stdio.h>
#include <stdbool.h>
#include "struct.h"

// Gestion joueurs
Joueur* creer_joueur(int* nb_joueurs);
Joueur* ordre_joueur(Joueur* players, int nb_joueurs);
int nbr_joueur(void);

// Gestion tuiles
void distribuer_tuile(void);
void piocher_tuile(Joueur* j);

// Combinaisons
bool combinaison_valide(Tuile* comb, int n);
bool combinaison_valide_30(Tuile* comb, int n);
void jouer_combinaison(Joueur* j);

// Premier tour
bool est_premier_tour(Joueur j);

// Utilitaires affichage / fichier
void afficher_tuiles(Tuile tuiles[], int nb_tuiles);
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles);
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide);
void ajouter_a_table(Tuile* comb, int n);

// **NOUVEAU**
void charger_joueur(Joueur* j);  // charge pseudo, tour, premier_tour depuis le fichier JSON

#endif
