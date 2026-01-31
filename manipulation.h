#ifndef MANIPULATION_H
#define MANIPULATION_H

#include "struct.h"

// Fonctions principales (appelées depuis le menu)
bool traiter_joker(Joueur* j);
bool traiter_extension_suite(Joueur* j);
bool traiter_division(Joueur* j);
bool ajouter_tuile_combinaison_existante(Joueur* j);

#endif