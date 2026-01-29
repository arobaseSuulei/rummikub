#ifndef TABLE_H
#define TABLE_H

#include "struct.h"

// Fonctions de base
void creer_table(void);
void afficher_combinaisons_table(void);

// Fonctions de manipulation
bool peut_recuperer_joker(Tuile* tuile_joueur, int* comb_index, int* tuile_index);
bool recuperer_joker(Tuile* tuile_joueur, int comb_index, int tuile_index, Tuile* joker_recupere);

bool peut_etendre_suite(Tuile* tuile_joueur, int comb_index, bool gauche, Tuile* tuile_a_recuperer);
bool etendre_suite(Tuile* tuile_joueur, int comb_index, bool gauche, Tuile* tuile_recuperee);

bool peut_remplacer_tuile(Tuile* tuile_joueur, int comb_index, int tuile_index);
bool remplacer_tuile(Tuile* tuile_joueur, int comb_index, int tuile_index, Tuile* ancienne_tuile);

bool peut_diviser_suite(int comb_index, int position);
bool diviser_suite(int comb_index, int position);

bool peut_retirer_tuile(int comb_index, int tuile_index);
bool retirer_tuile(int comb_index, int tuile_index, Tuile* tuile_retiree);

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