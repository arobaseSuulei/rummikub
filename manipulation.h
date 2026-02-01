#ifndef MANIPULATION_H
#define MANIPULATION_H

#include "struct.h"

void isoler_tuile(int tuile_id);
void ajouter_tuile_combinaison(int tuile_id, int combinaison_index);
bool valider_table_virtuelle(void);
void appliquer_modifications(void);
void annuler_manipulation(void);
bool valider_tour(Joueur* j);  // Ajouter Joueur* en paramètre
void isoler_tuile_chevalet(int tuile_id);
#endif