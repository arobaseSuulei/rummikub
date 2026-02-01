#ifndef TABLE_H
#define TABLE_H

#include "struct.h"

// Fonctions de base
void creer_table(void);
void afficher_combinaisons_table(void);





// Utilitaires
int compter_combinaisons_table(void);
bool est_combinaison_valide(int comb_index);
bool est_combinaison_brelan(int comb_index);
bool est_combinaison_suite(int comb_index);
int compter_tuiles_combinaison(int comb_index);
void obtenir_tuile_table(int comb_index, int tuile_index, Tuile* resultat);

// Validation globale
bool validation_table_complete(void);

#endif