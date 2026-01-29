#ifndef MANIPULATION_H
#define MANIPULATION_H

#include "struct.h"

// Fonction principale
bool manipuler_table(Joueur* j);

// Options de manipulation
void afficher_options_manipulation(void);

// Manipulations spécifiques
bool traiter_joker(Joueur* j);
bool traiter_extension_suite(Joueur* j);
bool traiter_remplacement(Joueur* j);
bool traiter_division(Joueur* j);
bool traiter_retrait(Joueur* j);

// Utilitaires
int compter_combinaisons_table(void);

#endif