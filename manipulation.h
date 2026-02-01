#ifndef MANIPULATION_H
#define MANIPULATION_H

#include "struct.h"
#include "joueur.h"



/* Fonctions de manipulation */
void isoler_tuile(int tuile_id);
void isoler_tuile_chevalet(int tuile_id);
void ajouter_tuile_combinaison(int tuile_id, int combinaison_index);

/* Validation et finalisation */
bool valider_tour(Joueur* j);

#endif