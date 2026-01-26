#include <stdio.h>
#include <stdbool.h>

#ifndef JOUEUR_H
#define JOUEUR_H
#include "cJSON.h" // bibliotheque externne pour manipuler le JSON


#include "struct.h"

Joueur* creer_joueur(int* nb_joueurs);
Joueur* ordre_joueur(Joueur* players, int nb_joueurs);
int nbr_joueur();
void distribuer_tuile();
void piocher_tuile(Joueur j);
void afficher_chevalet(Joueur j);
int nbr_tuiles(const char* fichier_json); // on a besoin du nbre de tuiles restant de chaque joueur



#endif